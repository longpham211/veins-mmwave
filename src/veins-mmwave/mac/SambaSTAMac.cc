#include "veins-mmwave/mac/SambaSTAMac.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::SambaSTAMac);

veins::SambaSTAMac::~SambaSTAMac() {
    findHost()->unsubscribe(BaseMobility::mobilityStateChangedSignal, this);
}

void veins::SambaSTAMac::receiveSignal(cComponent* source, simsignal_t signalID,
        cObject* obj, cObject* details) {
    Enter_Method_Silent();

    if (signalID == BaseMobility::mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
}

void veins::SambaSTAMac::handlePositionUpdate(cObject* obj) {
    bestBeamTowardRSU = phy11ad->getBestBeamByTheOtherPosition(rsuPosition);
    phy11ad->setReceivingSectorID(bestBeamTowardRSU); // Always set the RX toward RSU
}

void veins::SambaSTAMac::initialize(int stage) {
    MmWaveMac::initialize(stage);

    if(stage == 0) {
        rsuPosition = Coord(par("rsuX").doubleValue(), par("rsuY").doubleValue(), par("rsuZ").doubleValue());
        findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
        bestBeamTowardRSU = OMNIDIRECTIONAL_ANTENNA;

        beginRSS = nullptr;

        if(evaluateAlgorithms) {
            addedConstructionSiteNearRSUUrban = par("addedConstructionSiteNearRSUUrban").boolValue();
            if(myMacAddr == 23 && addedConstructionSiteNearRSUUrban){
                mobility = TraCIMobilityAccess().get(getParentModule()->getParentModule());
                traciCommandInterface = mobility->getCommandInterface();
                std::list<Coord> points;

                points.push_back(Coord(1703, 1058));
                points.push_back(Coord(1703, 1078));
                points.push_back(Coord(1723, 1078));
                points.push_back(Coord(1723, 1058));
                std::string constructionSite = "constructionSite";
                traciCommandInterface->addPolygon(constructionSite, "building", TraCIColor::fromTkColor("SpringGreen"), true, -1.00, points);
                EV_TRACE << "Added polygon" << std::endl;


                std::string typeID  = traciCommandInterface->polygon(constructionSite).getTypeId();
                EV_TRACE <<"addedConstructionsite type from SUMO: " << typeID << std::endl;

                std::list<Coord> shape = traciCommandInterface->polygon(constructionSite).getShape();
                EV_TRACE << "first Coord: " << shape.begin()->x << std::endl;

                ObstacleControl* obstacles = ObstacleControlAccess().getIfExists();
                if (obstacles) {
                    std::string typeId = traciCommandInterface->polygon(constructionSite).getTypeId();
                    if (obstacles->isTypeSupported(typeId)) {
                    std::list<Coord> coords = traciCommandInterface->polygon(constructionSite).getShape();
                    std::vector<Coord> shape;
                    std::copy(coords.begin(), coords.end(), std::back_inserter(shape));
                    obstacles->addFromTypeAndShape(constructionSite, typeId, shape);
                    EV_TRACE <<"Added construcitonSite to obstacle successfully!" << std::endl;
                    }
                }
            }
        }


    }
}

bool veins::SambaSTAMac::handleUnicast(MacPkt* macPkt,
        DeciderResult80211* res) {

    if(! MmWaveMac::handleUnicast(macPkt, res)) {
        // If I receive a Poll Frame, then I need to send back a SPR frame
        if (macPkt->getKind() == packet_kind::POLL_FRAME) {
            EV_TRACE <<"Received a poll frame, schedule to send back a SPR frame!" <<std::endl;
            SPRFrame * sprFrame = new SPRFrame();
            sprFrame->setName("SPR Frame");
            sprFrame->setKind(packet_kind::SPR_FRAME);
            sprFrame->setDuration(0);
            DynamicAllocationInfo dynamicAllocationInfo;

            if(requestSP) {
                dynamicAllocationInfo.allocationType = static_cast<uint32_t>(AllocationType::SP);
                dynamicAllocationInfo.allocationDuration = spRequestedDuration;

            }
            else {
                dynamicAllocationInfo.allocationType = static_cast<uint32_t>(AllocationType::CBAP);
                dynamicAllocationInfo.allocationDuration = 0; // TODO 0 means this STA accepts whatever PCP/AP assigns

            }
            dynamicAllocationInfo.sourceAID = myMacAddr;
            dynamicAllocationInfo.destinationAID = macPkt->getSrcAddr(); // TODO check this, currently, the destination address is to the PCP_AP

            sprFrame->setDynamicAllocationInfo(dynamicAllocationInfo);

            sprFrame->setRecipientAddress(macPkt->getSrcAddr());
            sprFrame->setBitLength(SPR_FRAME_LENGTH);

            BestBeamInfo* info = &(beamStates[macPkt->getSrcAddr()]);
            info->finished = true;
            info->myBestAntennaID = 1;
            info->myBestSectorID = bestBeamTowardRSU;

            int num = myEDCA[MmWaveChannelType::control]->queuePacket(sprFrame);

            if (num == -1)
                statsDroppedPackets++;
            if (num == 1) {
                if(nextMacControlEvent->isScheduled())
                    cancelEvent(nextMacControlEvent);
                //9.33.3 ATI transmission rules: The transmission of the response frame shall commence
                //one SIFS period after the successful reception of the request frame
                scheduleAt(simTime() + SIFS_11AD, nextMacControlEvent);
            }

            delete res;
        }
        else if (macPkt->getKind() == packet_kind::ANNOUNCE_FRAME) {
            auto * announceFrame = check_and_cast<AnnounceFrame *> (macPkt->decapsulate());

            EV_TRACE << "Receive an announce frame with " << announceFrame->getAllocationFieldsArraySize() << " assigned allocations" << std::endl;

            for(uint32_t i = 0 ; i < announceFrame->getAllocationFieldsArraySize(); i++)
                assignedAllocations.push_back(announceFrame->getAllocationFields(i));

            scheduleAt(SimTime(assignedAllocations.front().allocationStart, SimTimeUnit::SIMTIME_US), beginOfAnAllocation);

            delete announceFrame;
            delete res;
        }

        else {
            //normal frame
            unique_ptr<BaseFrame1609_4> receivedFrame(check_and_cast<BaseFrame1609_4*>(macPkt->decapsulate()));
            receivedFrame->setControlInfo(new PhyToMacControlInfo(res));
            EV_TRACE << "Received a data packet addressed to me, sending ACK then sending up the frame to splitter!" << std::endl;
            if(useAcks){
                if(evaluateAlgorithms) {
                    EV_TRACE << "If we are evaluating, because of no beam forming, polling and SPR," <<
                            "\nthis vehicle hasn't updated the beamStates toward RSU yet." <<
                            "\nUpdate the beamStates" << std::endl;

                    BestBeamInfo* info = &(beamStates[macPkt->getSrcAddr()]);
                    info->finished = true;
                    info->myBestAntennaID = 1;
                    info->myBestSectorID = bestBeamTowardRSU;
                }
                sendAck(macPkt->getSrcAddr(), receivedFrame->getTreeId());
            }
            if(std::strcmp(receivedFrame->getName(), "Evaluate message") == 0) {
                ASSERT(evaluateAlgorithms);

                EV_TRACE << "Receive an evaluate frame! Add to statistic collection!" << std::endl;
                addAggregateDataToCollection(receivedFrame->getBitLength());
                receivedFrame.reset();
            }
            else
                sendUp(std::move(receivedFrame).release());
        }
    }
}

void veins::SambaSTAMac::finish() {
    if(recordInTimeSlot && aggregateDataPerTimeSlot > 0) {
        simtime_t recordTime = simTime();

        if((int)(ceil(simTime().dbl())) % recordTimeSlot == 0)
            recordTime = recordTime - SimTime(1, SimTimeUnit::SIMTIME_MS);

        aggregateDataVector.recordWithTimestamp(recordTime, aggregateDataPerTimeSlot);
    }
}

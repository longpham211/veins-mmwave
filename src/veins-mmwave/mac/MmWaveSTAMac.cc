
/*
 * MmWaveSTAMac.cc
 *
 *  Created on: Apr 7, 2020
 *      Author: longpham211
 */

#include "veins-mmwave/mac/MmWaveSTAMac.h"
#include "veins/modules/obstacle/ObstacleControl.h"
#include "veins/modules/obstacle/VehicleObstacleControl.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::MmWaveSTAMac);

void veins::MmWaveSTAMac::initialize(int stage) {
    MmWaveMac::initialize(stage);

    if(stage == 0) {
        requestSP = par("requestSP").boolValue();
        spRequestedDuration = par("spRequestedDuration").intValue();
        beginRSS = new cMessage("Responder starts RSS");

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

void veins::MmWaveSTAMac::handleSelfMsg(cMessage* msg) {
    MmWaveMac::handleSelfMsg(msg);

    if (msg == beginRSS) {
        if(currentAccessPeriod == AccessPeriod::A_BFT)
            performA_BFT(allowed_a_bft_from_pcp_ap, allowed_fss_from_pcp_ap);
        else {
            ASSERT(!assignedAllocations.empty() && assignedAllocations.front().allocationType == static_cast<int32_t>(AllocationType::SP));
            SSWFrameRSS* sswFrameRSS = generateSSWFrameRSS();

            //TODO how about fss in this case?
            performTXSS(sswFrameRSS, SimTime(), fss);
        }
    }
    else if (msg == beginOfA_BFT) {
        scheduleAt(simTime(), beginRSS); // schedule RSS now!
    }
    else if (msg == sendSSWFeedbackFrame) {
        ASSERT (currentAccessPeriod == AccessPeriod::SP);
        // 9.35.5.2 Operation during the A-BFT IEEE 802.11ad 2012
        // In an A-BFT, the responder shall not initiate SSW ACK (9.35.2.5) in response to the reception of a SSW-
        // Feedback frame from the initiator. The SSW ACK only occurs within the DTI of a beacon interval

        waitUntilAckRXorTimeout = true;

        BestBeamInfo * info = &(beamStates[communicatingSTA]);
        generateAndQueueSSWFeedbackFrame(info);
    }
}

veins::MmWaveSTAMac::~MmWaveSTAMac() {
    if(beginRSS) {
        if(beginRSS->isScheduled())
            cancelEvent(beginRSS);
        delete(beginRSS);
        beginRSS = nullptr;
    }
}

void veins::MmWaveSTAMac::handleAckTimeOut(AckTimeOutMessage* ackTimeOutMsg) {
    MmWaveMac::handleAckTimeOut(ackTimeOutMsg);

//    ASSERT (currentAccessPeriod == AccessPeriod::SP); // TODO do we need to add this to PCP/AP as well?
//    handleRetransmit();
}

void veins::MmWaveSTAMac::performA_BFT(uint32_t a_bft, uint32_t fss)
{
    uint32_t selectedSlot = intrand(a_bft);

    //TODO handle the case if this station doesn't receive SSW Feedback from PCP/AP because
    //either collision in selected slot with another station
    //or the SSW frames are received wrongly at the PCP/AP (maybe because of the received signal at the specific SectorID are too small)

    simtime_t aSSSlotTime = getSSSlotTime(fss);

    EV_TRACE<<"A-BFT: aSSSlotTime is: " << aSSSlotTime << ", selectedSlot: " << selectedSlot<<std::endl;

    SSWFrameRSS* sswFrameRSS = generateSSWFrameRSS();

    performTXSS(sswFrameRSS, selectedSlot * aSSSlotTime, allowed_fss_from_pcp_ap);

    simtime_t timeWhenA_BFTEnds = simTime() + a_bft * aSSSlotTime;

    //9.33.3 The ATI shall not start sooner than Max(guard time, MBIFS)
    simtime_t timeWhenATIBegins = timeWhenA_BFTEnds + std::max(GUARD_TIME_NONPSEUDO_STATIC, MBIFS_11AD);
    scheduleAt(timeWhenATIBegins, beginOfATI);
}

void veins::MmWaveSTAMac::handleLowerControl(cMessage* msg) {
    MmWaveMac::handleLowerControl(msg);

    if (msg->getKind() == MacToPhyInterface::PHY_RX_END_WITH_FAILURE) {
        // STA receives this lower control message only when retransmitting feedback frame.
        // then it will retransmit the feedback frame

        //TODO Check this if we want to deal with retransmit because this might interfere with ATI sending poll frames
        if(dynamic_cast<SSWFeedbackFrame*>(lastSentPacket))
            handleRetransmit();
    }
}

bool veins::MmWaveSTAMac::handleUnicast(MacPkt* macPkt,
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

        //Well, I think only STA can receives SSW_Frame_ISS.
        //If a station wants to communicate with a PCP/AP, it should have beam-forming information in BTI phase already.
        else if (macPkt->getKind() == packet_kind::SSW_FRAME_ISS) {
            EV_TRACE << "Receive a SSW Frame ISS from " << macPkt->getSrcAddr() << " to me!" << std::endl;

            // we will performTXSS after ISS
            SSWFrameISS * sswFrameISS = check_and_cast<SSWFrameISS*>(macPkt->decapsulate());

            if(!beginRSS->isScheduled()) {
                communicatingSTA = macPkt->getSrcAddr();

                //TODO what is fss in this case?

                //TODO how about if the requested SP is not large enough for the RSS?
                simtime_t timeWhenRSSShouldStart = simTime() + SimTime(((double) sswFrameISS->getDuration())/pow(10,6)); // MBIFS is already added in sswFrameISS.duration.

                EV_TRACE<< "long trace MAC: schedule timeWhenRSSShouldStart: "<< timeWhenRSSShouldStart<< std::endl;
                scheduleAt(timeWhenRSSShouldStart, beginRSS);
            }

            updateBeamformingDataFromSSWFrame(sswFrameISS, res);

            delete sswFrameISS;
            delete res;
        }

        else if (macPkt->getKind() == packet_kind::SSW_FEEDBACK) {
            BestBeamInfo * info = &(beamStates[communicatingSTA]);
            SSWFeedbackFrame * feedbackFrame = check_and_cast<SSWFeedbackFrame * >(macPkt->decapsulate());
            info->myBestAntennaID = feedbackFrame->getDmgAntennaSelect();
            info->myBestSectorID = feedbackFrame->getSectorSelect();
            info->myBestSNR = feedbackFrame->getSnrReport();


            //TODO here, implement the SSW ACK in SP allocation (!= A-BFT) and add finished flags to the beams in A-BFT as well
            if (currentAccessPeriod == AccessPeriod::A_BFT) {
                info->finished = true; // we don't need to send back SSW-ACK in A-BFT.
                EV_TRACE << "I am in A-BFT phase, don't need to do anything else!" << std::endl;

                if(evaluateAlgorithms)
                    phy11ad->setReceivingSectorID(info->myBestSectorID); // Point antenna towards RSU in evaluateAlgorithms.
            }
            else {
                ASSERT (currentAccessPeriod == AccessPeriod::SP) ;
                EV_TRACE << "I am in SP phase, will send SSW ACK for the initiator!" << std::endl;
                generateAndQueueSSWACK(info);
                info->finished = true;
            }

            delete feedbackFrame;
            delete res;
        }

        else if (macPkt->getKind() == packet_kind::SSW_ACK) {
            ASSERT2(rxStartIndication, "rxStartIndication should be true to wait for ACK");
            ASSERT(currentAccessPeriod == AccessPeriod::SP);
            phy11ad->notifyMacAboutRxStart(false);

            EV_TRACE <<"turn off notifyMacAboutRxStart because of receiving SSW ACK Frame!" << std::endl;
            rxStartIndication = false;

            //This is for the SSW ACK Frame
            BaseFrame1609_4* msg = myEDCA[MmWaveChannelType::control]->myQueue.queue.front();
            myEDCA[MmWaveChannelType::control]->myQueue.queue.pop();
            delete msg;

            myEDCA[MmWaveChannelType::control]->myQueue.rc = 0;
            myEDCA[MmWaveChannelType::control]->myQueue.waitForAck = false;
            myEDCA[MmWaveChannelType::control]->myQueue.waitOnUnicastID = -1;
            if(myEDCA[MmWaveChannelType::control]->myQueue.ackTimeOut->isScheduled())
                cancelEvent(myEDCA[MmWaveChannelType::control]->myQueue.ackTimeOut);

            waitUntilAckRXorTimeout = false;

            BestBeamInfo * info = &(beamStates[communicatingSTA]);
            SSWACK* sswACK = check_and_cast<SSWACK * >(macPkt->decapsulate());
            info->myBestAntennaID = sswACK->getDmgAntennaSelect();
            info->myBestSectorID = sswACK->getSectorSelect();
            info->myBestSNR = sswACK->getSnrReport();

            info->finished = true;

            delete sswACK;
            delete res;
        }

        else {
            //normal frame
            unique_ptr<BaseFrame1609_4> receivedFrame(check_and_cast<BaseFrame1609_4*>(macPkt->decapsulate()));
            receivedFrame->setControlInfo(new PhyToMacControlInfo(res));
            EV_TRACE << "Received a data packet addressed to me, sending ACK then sending up the frame to splitter!" << std::endl;
            if(useAcks)
                sendAck(macPkt->getSrcAddr(), receivedFrame->getTreeId());

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

SSWFrameRSS* veins::MmWaveSTAMac::generateSSWFrameRSS() {
    SSWFrameRSS* sswFrameRSS = new SSWFrameRSS();

    sswFrameRSS->setName("SSW Frame from Responder");
    sswFrameRSS->setKind(packet_kind::SSW_FRAME_RSS);

    sswFrameRSS->setDirection(FROM_RESPONDER);

    sswFrameRSS->setRecipientAddress(communicatingSTA);

    BestBeamInfo * info = &(beamStates[communicatingSTA]);
    sswFrameRSS->setSectorSelect(info->theirBestSectorID);
    sswFrameRSS->setDmgAntennaSelect(info->theirBestAntennaID);
    sswFrameRSS->setSnrReport(info->theirBestSNR);

    sswFrameRSS->setBitLength(SSW_FRAME_LENGTH_WITHOUT_SSW_FIELD);

    return sswFrameRSS;
}

void veins::MmWaveSTAMac::handleBroadcast(MacPkt* macPkt,
        DeciderResult80211* res) {
    MmWaveMac::handleBroadcast(macPkt, res);

    // If the message received is a DMG_Beacon, and I am not a PCP/AP, then I am a responder, and prepare to perform the A-BFT
    if (macPkt->getKind() == packet_kind::DMG_BEACON ) {
        phy11ad->setReceivingSectorID(OMNIDIRECTIONAL_ANTENNA);
        if(refreshBeamStateEveryBI)
            beamStates.clear();
        else
            ASSERT2(false, "Just support true, otherwise, we must refresh manually, like each 5sec?");

        DMGBeacon * dmgBeacon = check_and_cast<DMGBeacon*>(macPkt->decapsulate());
        if(!beginOfA_BFT->isScheduled()){
            currentAccessPeriod = AccessPeriod::BTI;
            communicatingSTA = macPkt->getSrcAddr();
            allowed_a_bft_from_pcp_ap = dmgBeacon->getA_bft_length();
            allowed_fss_from_pcp_ap = dmgBeacon->getFss();

            simtime_t timeWhenBTIEnds = simTime() + SimTime(((double) dmgBeacon->getDuration())/pow(10,6));
            simtime_t timeWhenA_BFTShouldStart = timeWhenBTIEnds + MBIFS_11AD; //MBIFS should be use between BTI and A-BFT
            EV_TRACE<< "Schedule A_BFT at: " << timeWhenA_BFTShouldStart<<std::endl;
            scheduleAt(timeWhenA_BFTShouldStart, beginOfA_BFT);
        }
        else
            ASSERT2(communicatingSTA == macPkt->getSrcAddr(), "Receives DMGBeacon from a PCP/AP while doing beamforming with another PCP/AP!");

        updateBeamformingDataFromSSWFrame(dmgBeacon, res);

        delete dmgBeacon;
        delete res;
    }
    else { // normal packets
        unique_ptr<BaseFrame1609_4> receivedFrame(check_and_cast<BaseFrame1609_4*>(macPkt->decapsulate()));
        receivedFrame->setControlInfo(new PhyToMacControlInfo(res));
        sendUp(receivedFrame.release());
    }
}

void veins::MmWaveSTAMac::generateAndQueueSSWACK(BestBeamInfo* info) {

    SSWACK* sswACK = new SSWACK();

    sswACK->setName("SSW ACK");
    sswACK->setKind(packet_kind::SSW_ACK);
    sswACK->setRecipientAddress(communicatingSTA);
    sswACK->setBitLength(SSW_ACK_FRAME_LENGTH);

    sswACK->setDmgAntennaSelect(info->theirBestAntennaID);
    sswACK->setSectorSelect(info->theirBestSectorID);
    sswACK->setSnrReport(info->theirBestSNR);

    int num = myEDCA[MmWaveChannelType::control]->queuePacket(sswACK);
    if (num == -1){
        statsDroppedPackets++;
    }

    if (num == 1) {
        if(nextMacControlEvent->isScheduled())
            cancelEvent(nextMacControlEvent);
        scheduleAt(simTime(), nextMacControlEvent); // now is at aSSFBDuraion + MBIFS before the end of the SSW slot
    }
}

void veins::MmWaveSTAMac::updateBeamformingDataFromSSWFrame(
        FramesContainSSWField* frame, DeciderResult80211* res) {
    BestBeamInfo * info = &(beamStates[communicatingSTA]);
    if (res->getSnr() > info->theirBestSNR) {
        info->theirBestSectorID = frame->getSectorID();
        info->theirBestAntennaID = frame->getAntennaID();
        info->theirBestSNR= res->getSnr();
    }
}

void veins::MmWaveSTAMac::finish() {
    if(recordInTimeSlot && aggregateDataPerTimeSlot > 0) {
        simtime_t recordTime = simTime();

        if((int)(ceil(simTime().dbl())) % recordTimeSlot == 0)
            recordTime = recordTime - SimTime(1, SimTimeUnit::SIMTIME_MS);

        aggregateDataVector.recordWithTimestamp(recordTime, aggregateDataPerTimeSlot);
    }
}

/*
 * FmlSTAMac.cc
 *
 *  Created on: Jul 2, 2020
 *      Author: longpham211
 */

#include "veins-mmwave/mac/FmlSTAMac.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::FmlSTAMac);

void veins::FmlSTAMac::initialize(int stage) {
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

veins::FmlSTAMac::~FmlSTAMac() {
    findHost()->unsubscribe(BaseMobility::mobilityStateChangedSignal, this);
}

void veins::FmlSTAMac::receiveSignal(cComponent* source, simsignal_t signalID,
        cObject* obj, cObject* details) {
    Enter_Method_Silent();

    if (signalID == BaseMobility::mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
}

void veins::FmlSTAMac::handlePositionUpdate(cObject* obj) {
    bestBeamTowardRSU = phy11ad->getBestBeamByTheOtherPosition(rsuPosition);
    phy11ad->setReceivingSectorID(bestBeamTowardRSU); // Always set the RX toward RSU
}

bool veins::FmlSTAMac::handleUnicast(MacPkt* macPkt, DeciderResult80211* res) {
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

void veins::FmlSTAMac::finish() {
    if(recordInTimeSlot && aggregateDataPerTimeSlot > 0) {
        simtime_t recordTime = simTime();

        if((int)(ceil(simTime().dbl())) % recordTimeSlot == 0)
            recordTime = recordTime - SimTime(1, SimTimeUnit::SIMTIME_MS);

        aggregateDataVector.recordWithTimestamp(recordTime, aggregateDataPerTimeSlot);
    }
}

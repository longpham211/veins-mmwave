#include "veins-mmwave/mac/SambaAPMac2.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::SambaAPMac2);

veins::SambaAPMac2::~SambaAPMac2() {
}

void veins::SambaAPMac2::initialize(int stage) {
    MmWaveAPMac::initialize(stage);
    if (stage == 0) {
        rsuCoverRange = par("rsuCoverRange").doubleValue();
    }
}

void veins::SambaAPMac2::performBTI() {
    communicatingSTA = LAddress::L2NULL();
    mcs = MmWaveMCS::cphy_mcs_0;
    availableVehicles.clear();
    setActiveChannel(MmWaveChannelType::control);

    updatePositionMap();
    scheduleAt(simTime(), beginOfATI);
}

void veins::SambaAPMac2::updatePositionMap() {
    beamStates.clear();

    for (auto it = vehiclesPosition.begin(); it != vehiclesPosition.end();) {
        predictPosition(it->first, it->second);

        //UPDATE: We just receive and update the vehicle position for the vehicles
        // which are inside the RSU coverage or go toward the RSU, then it's fine to delete
        if(it->second.distance > rsuCoverRange) {
            Coord positionAfter1s = it->second.predictedPosition +
                                    it->second.direction *
                                    it->second.speed *
                                    (SimTime(1, SimTimeUnit::SIMTIME_S)).dbl();
            double distanceAfter1s = positionAfter1s.distance(phy11ad->getCurrentPosition());

            //The standing till vehicles (out side of rsucoverage) might be teleported out of the map
            if(distanceAfter1s < it->second.distance) { // The vehicle, is even out of rsuCoverange, but is going toward rsu (not stand still). Leave it in the map
                ++it;
                continue;
            }
            else { // the vehicle is out of rsuCoverage and leaving (or standing still out of) the rsuCoverage, delete it from the map.
                EV_TRACE << "Delete "<< it->first << " from beamStates and vehiclesPosition because out of (and leaving) rsuCoverRange!"<< std::endl;
                beamStates.erase(it->first); //Delete in beamStates
                it = vehiclesPosition.erase(it);
            }
        }
        else { // vehicles is inside the rsuCoverange, add it to the beamState.
            std::map<std::string, cModule*> availableCars = traciManager->getManagedHosts();
            App11adToMac11adInterface* staMAC;
            //Mac11adToPhy11adInterface* staPhyMmWave;

            //If this vehicle is in the "beam coverage range", add to the map
            for(auto const& car : availableCars) {
                staMAC = FindModule<App11adToMac11adInterface*>::findSubModule(car.second);
                ASSERT(staMAC);

                uint32_t bestBeamTowardVehicle = phy11ad->getBestBeamByTheOtherPosition(it->second.predictedPosition);
                uint32_t rsuDirID = phy11ad->getTransmissionDirID(phy11ad->getCurrentPosition()
                                                                , phy11ad->getCurrentOrientation()
                                                                , bestBeamTowardVehicle, it->second.predictedPosition);
                double rxPower = phy11ad->getReceivePowerdBm(it->second.distance, rsuDirID, OMNIDIRECTIONAL_ANTENNA, bestBeamTowardVehicle, OMNIDIRECTIONAL_ANTENNA, false);

                if(rxPower >= -78) { // SC-PHY
                    BestBeamInfo* bestBeam = &(beamStates[it->first]);
                    bestBeam->finished = true;
                    bestBeam->myBestSectorID = bestBeamTowardVehicle;
                    bestBeam->myBestAntennaID = 1; // doesn't matter, we have 1 antenna only, better to call a dummy function in AntennaArray.cc

                    //If we are waiting for a frame (eg. SPR) from vehicle, if will beam toward this vehicle
                    //This will not likely to happen, because the updateInterval is 30ms (beginning of BI),
                    // While the waiting for SPRframe takes much less time than this, and happens a little afterward
                    if(rxStartIndication && lastSentPacket && lastSentPacket->getRecipientAddress() == it->first) {
                        phy11ad->setReceivingSectorID(bestBeamTowardVehicle);
                    }
                    EV_TRACE << "Add vehicle " << it->first << " to beamState" << std::endl;
                }

//                if(staMAC->getMACAddress() == it->first) {
//                    staPhyMmWave = FindModule<Mac11adToPhy11adInterface*>::findSubModule(car.second);
//                    ASSERT(staPhyMmWave);
//
//                    uint32_t bestBeamTowardVehicle = phy11ad->getBestBeamByTheOtherPosition(it->second.predictedPosition);
//                    uint32_t bestBeamTowardRSU = staPhyMmWave->getBestBeamByTheOtherPosition(phy11ad->getCurrentPosition());
//
//                    uint32_t rsuDirID = phy11ad->getTransmissionDirID(phy11ad->getCurrentPosition()
//                                                                    , phy11ad->getCurrentOrientation()
//                                                                    , bestBeamTowardVehicle, it->second.predictedPosition);
//
//                    uint32_t carDirID = staPhyMmWave->getTransmissionDirID(staPhyMmWave->getCurrentPosition()
//                                                                        , staPhyMmWave->getCurrentOrientation()
//                                                                        , bestBeamTowardRSU, phy11ad->getCurrentPosition());
//
//                    double rxPower = phy11ad->getReceivePowerdBm(it->second.distance, rsuDirID, carDirID, bestBeamTowardVehicle, bestBeamTowardRSU, false);
//                    EV_TRACE << "Long test: rxPower = " << rxPower << ", distance = " << it->second.distance << std::endl;
//
//                    if(rxPower >= -78) { // SC-PHY
//                        BestBeamInfo* bestBeam = &(beamStates[it->first]);
//                        bestBeam->finished = true;
//                        bestBeam->myBestSectorID = bestBeamTowardVehicle;
//                        bestBeam->myBestAntennaID = 1; // doesn't matter, we have 1 antenna only, better to call a dummy function in AntennaArray.cc
//
//                        //If we are waiting for a frame (eg. SPR) from vehicle, if will beam toward this vehicle
//                        //This will not likely to happen, because the updateInterval is 30ms (beginning of BI),
//                        // While the waiting for SPRframe takes much less time than this, and happens a little afterward
//                        if(rxStartIndication && lastSentPacket && lastSentPacket->getRecipientAddress() == it->first) {
//                            phy11ad->setReceivingSectorID(bestBeamTowardVehicle);
//                        }
//                        EV_TRACE << "Add vehicle " << it->first << " to beamState" << std::endl;
//                    }
//                }
            }
            ++it;
        }
    }

    EV_TRACE << "Beam state size " << beamStates.size() << std::endl;
}

void veins::SambaAPMac2::predictPosition(LAddress::L2Type address, PositionInfo &info) {
    simtime_t timeDelta = simTime() - info.at;
    info.predictedPosition = info.position +  info.direction * info.speed * timeDelta.dbl();
    info.distance = info.predictedPosition.distance(phy11ad->getCurrentPosition());

    EV_TRACE <<"predictPosition: Vehicle "<<address
            <<"\nat: " << info.at
            <<"\ncurrentPosition: " << info.position
            <<"\nspeed: "<< info.speed
            <<"\ndirection: " << info.direction
            << "\npredictedPosition: " << info.predictedPosition
            << "\ndistance: " << info.distance
            <<"\n inOnce: " << info.inOnce
            << std::endl;
}

void veins::SambaAPMac2::handleUpperMsg(cMessage* msg) {
    EV_TRACE << "Samba receives message from upper layer" << std::endl;

    if(PositionAidMessage* positionMsg = dynamic_cast<PositionAidMessage*> (msg)) {
        double currentDistance = positionMsg->getSenderPosition().distance(phy11ad->getCurrentPosition());
        Coord positionAfter1s = positionMsg->getSenderPosition() +
                                positionMsg->getSenderDirection() *
                                positionMsg->getSenderSpeed() *
                                (SimTime(1, SimTimeUnit::SIMTIME_S)).dbl();
        double distanceAfter1s = positionAfter1s.distance(phy11ad->getCurrentPosition());

//        if(positionMsg->getSender11adAddress() == 353)
//            EV_TRACE <<"Piece of shit vehicle making the hanging up"<<std::endl;

        //We just take the vehicles, inside the RSU coverage or go toward the RSU, into account
        //Even if the vehicle is standing still, we should accept this message by
        //adding the vehicle to vehiclesPosition map, then we will handle the speed = 0 later
        // if we ignore the message (by not updating the map), then the old speed of the vehicles might
        //be greater than 0, and after a long time waiting (traffic lights, jams, ...)
        // vehicles might teleport out of the simulation.. (sumo characteristics...)
        //https://sumo.dlr.de/docs/Simulation/Why_Vehicles_are_teleporting.html
        if(currentDistance < rsuCoverRange || distanceAfter1s <= currentDistance) {
            PositionInfo* info = &(vehiclesPosition[positionMsg->getSender11adAddress()]);
            info->position = info->predictedPosition = positionMsg->getSenderPosition();
            info->speed = positionMsg->getSenderSpeed();
            info->direction = positionMsg->getSenderDirection();
            info->orientation = positionMsg->getSenderOrientation();
            info->heading = positionMsg->getSenderHeading();
            info->at = positionMsg->getSentAt();
            info->inOnce = false;
        }
    }

    delete msg;
}

void veins::SambaAPMac2::handleLowerControl(cMessage* msg) {
    if (msg->getKind() == MacToPhyInterface::TX_OVER) {
        if (dynamic_cast<PollFrame * > (lastSentPacket)) {
            // No, because we call handleLowerControl  at the end (to avoid postTransmit), rxStartIndication is still false
            LAddress::L2Type receiverAddress = lastSentPacket->getRecipientAddress();

            PositionInfo* info = &(vehiclesPosition[receiverAddress]);
            uint32_t bestBeamTowardReceiver = phy11ad->getBestBeamByTheOtherPosition(info->predictedPosition);

            phy11ad->setReceivingSectorID(bestBeamTowardReceiver);
            EV_TRACE <<"Poll frame is sent, point beam " << bestBeamTowardReceiver
                    << " toward receiver " << receiverAddress << " to receive SPR frame "<< std::endl;
        }
    }

    MmWaveAPMac::handleLowerControl(msg);
}

void veins::SambaAPMac2::addAnnounceFramesToQueue(simtime_t delay) {
    EV_TRACE << "All poll frames are sent, set rxAntenna to omnidirectional!"<< std::endl;
    phy11ad->setReceivingSectorID(OMNIDIRECTIONAL_ANTENNA);

    MmWaveAPMac::addAnnounceFramesToQueue(delay);
}

int32_t veins::SambaAPMac2::getReceivingSectorIDForOracle(
        LAddress::L2Type vehicleMAC, Mac11adToPhy11adInterface* phyMmWave) {
    return phyMmWave->getBestBeamByTheOtherPosition(phy11ad->getCurrentPosition());
}

bool veins::SambaAPMac2::doesServingVehicleLeaveTheBeam(Mac11adToPhy11adInterface* staPhyMmWave, LAddress::L2Type vehicleMAC, double sensitivity) {
    auto it = vehiclesPosition.find(evaluationServingSTA);

    ASSERT(it != vehiclesPosition.end());
//
//    if(it->second.predictedPosition.distance(phy11ad->getCurrentPosition()) > rsuCoverRange)
//        return true;
//
//    return false;
//    Coord rsuPosition = phy11ad->getCurrentPosition();
//    Coord rsuOrientation = phy11ad->getCurrentOrientation();
    uint32_t bestBeamTowardVehicle = phy11ad->getBestBeamByTheOtherPosition(it->second.predictedPosition);
    //uint32_t bestBeamTowardRSU = staPhyMmWave->getBestBeamByTheOtherPosition(phy11ad->getCurrentPosition());

    uint32_t rsuDirID = phy11ad->getTransmissionDirID(phy11ad->getCurrentPosition()
                                                    , phy11ad->getCurrentOrientation()
                                                    , bestBeamTowardVehicle, it->second.predictedPosition);

//    uint32_t carDirID = staPhyMmWave->getTransmissionDirID(staPhyMmWave->getCurrentPosition()
//                                                        , staPhyMmWave->getCurrentOrientation()
//                                                        , bestBeamTowardRSU, phy11ad->getCurrentPosition());

    //double rxPower = phy11ad->getReceivePowerdBm(it->second.distance, rsuDirID, carDirID, bestBeamTowardVehicle, bestBeamTowardRSU, false);
    double rxPower = phy11ad->getReceivePowerdBm(it->second.distance, rsuDirID, OMNIDIRECTIONAL_ANTENNA, bestBeamTowardVehicle, OMNIDIRECTIONAL_ANTENNA, false);
    EV_TRACE << "Long test: rxPower = " << rxPower << ", distance = " << it->second.distance << std::endl;

    if(rxPower >= -78)
        return false;
    else
        return true;

}

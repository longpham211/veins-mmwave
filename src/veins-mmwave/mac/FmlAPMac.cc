#include "veins-mmwave/mac/FmlAPMac.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::FmlAPMac);

void veins::FmlAPMac::initialize(int stage) {
    MmWaveAPMac::initialize(stage);
    if(stage == 0) {
        estimateDuration = SimTime(par("estimateDuration").doubleValue(), SimTimeUnit::SIMTIME_MS); // to second
        selectedBeam = beamIsGoingToBeSelected = -1;
        isBeamSelected = false;

        dataSent = 0;

        rsuCoverRange = par("rsuCoverRange").doubleValue();

        considerObstaclesInVehiclesLeavingBeam = par("considerObstaclesInVehiclesLeavingBeam").boolValue();

        initializeFML();
        selectingANewBeam = new cMessage("Selecting a new beam, wait till the beginning of BTI!");

        estimateRX = new cMessage("serveTheNewVehicle is called, estimate RX of all vehicles");

        periodSignal = registerSignal("period");

        vehicleTriggeringBeamSelection = -1;
    }
}

veins::FmlAPMac::~FmlAPMac() {
    if(selectingANewBeam->isScheduled()) {
        cancelAndDelete(selectingANewBeam);
        selectingANewBeam = nullptr;
    }
}

void veins::FmlAPMac::handleUpperMsg(cMessage* msg) {
    ASSERT(dynamic_cast<cPacket*>(msg));

    //TODO Assert if this msg is mmWave msg.
    BaseFrame1609_4* thisMsg = check_and_cast<BaseFrame1609_4*>(msg);

    if(FmlServiceRequest* request = dynamic_cast<FmlServiceRequest*>(thisMsg)) {
        EV_TRACE << "Received a message from upper layer!" << std::endl;
        EV_TRACE << "It's Service Request message from "<<
                    request->getSenderDSRCAddress() << " DSRC address and " <<
                    request->getSender11adAddress() << " 11ad address!" << std::endl;

        EV_TRACE << "Add the requested vehicle to doAMap"<< std::endl;
        doAMap[request->getSender11adAddress()] = request->getNormalizedDoA();


        if(!isBeamSelected && !selectingANewBeam->isScheduled()) {
            EV_TRACE << "Processing FML algorithm now, by selecting a beam!" << std::endl;
            handleServiceRequest(request);
        }
        else
            EV_TRACE << "Beam is selected! or we are waiting to selecting a beam at the beginning of BTI, ignore this request!" << std::endl;

        delete request;
    }
    else if (FmlServiceFeedback* feedback = dynamic_cast<FmlServiceFeedback*>(thisMsg)) {
        EV_TRACE << "Receive leaving report from eNB, delete vehicle from doAMap!" << std::endl;
//        doAMap.erase(feedback->getSenderAddress());
//        if(nextMacEvent->isScheduled() && evaluateAlgorithms && evaluationServingSTA == feedback->getSenderAddress()){
//            MmWaveMac::whenServingCarLeaveTheBeam(false);
//            ASSERT(false);
//        }


        if(feedback->getSenderAddress() == vehicleTriggeringBeamSelection) {
            if (!updatedTheTriggeringVehicle) { // Probably because this vehicle is never in the selected beam, aggregateData is zero
                updateFMLStep(feedback);
            }
            isBeamSelected = false; // ready to receive new serving request, to choose a new beam
            vehicleTriggeringBeamSelection = -1;
        }

        delete feedback;
    }
    else {
        //Other types of message, let's see if parent classes handle it or not
        MmWaveAPMac::handleUpperMsg(msg);
    }
}

void veins::FmlAPMac::handleServiceRequest(FmlServiceRequest* request) {
    periodCount ++; // counting the period Interval as the orignial paper

    //Select a new beam;

    //run FML algrothim to get the beam
    beamIsGoingToBeSelected = getSelectedBeamByFMLAlgorithm(request->getNormalizedDoA());
    vehicleTriggeringBeamSelection = servingSTA11ad = request->getSender11adAddress();
    servingSTA11p = request->getSenderDSRCAddress();

    //Then send this beam to DSRC and then it will be sent by DSRC to the vehicle
    FmlRequestResponse* response = new FmlRequestResponse();

    response->setServingVehicleDSRCAddress(servingSTA11p);
    response->setServingVehicle11adAddress(servingSTA11ad);
    response->setSelectedBeam(beamIsGoingToBeSelected);
    response->setVehicleTriggeringBeamSelection(vehicleTriggeringBeamSelection);

    sendUp(response);

    updatedTheTriggeringVehicle = false;

    ASSERT(newBeaconInterval->isScheduled());
    //1ms to make sure that this selectingANewBeam arrives after newBeacon, then in the newBeacon, we will cancel it.
    scheduleAt(newBeaconInterval->getArrivalTime() + SimTime(1,SimTimeUnit::SIMTIME_MS), selectingANewBeam);




    std::map<std::string, cModule*> availableCars = traciManager->getManagedHosts();
    Mac11adToPhy11adInterface* staPhyMmWave;
    App11adToMac11adInterface* staMACFML;
    for(auto const& it : availableCars) {
        staPhyMmWave = FindModule<Mac11adToPhy11adInterface*>::findSubModule(it.second);
        ASSERT(staPhyMmWave);

        staMACFML = FindModule<App11adToMac11adInterface*>::findSubModule(it.second);
        ASSERT(staMACFML);

        if(staMACFML->getMACAddress() == vehicleTriggeringBeamSelection) {
            vehicleTriggeringBeamSelectionPhy80211ad = staPhyMmWave;
        }
    }
}

uint32_t veins::FmlAPMac::getSelectedBeamByFMLAlgorithm(uint32_t doAIndex) {
    std::vector<uint32_t> explorationGroup;
    uint32_t selected_beam;

    double exploreFactor = getFactorControlFunction() * getControlFunction(z, periodCount);

    std::vector<uint32_t> testCountAt4;
    for(uint32_t i = 0; i < countSelectionPerBeam.size(); i++) {
        testCountAt4.push_back(countSelectionPerBeam[i][doAIndex]);
    }

    for(uint32_t i = 0; i < countSelectionPerBeam.size(); i++)
        if(countSelectionPerBeam[i][doAIndex] <= exploreFactor)
            explorationGroup.push_back(i); // set of under-explored beams


    if(!explorationGroup.empty()) {
        //Exploration Step
        uint32_t u = explorationGroup.size();

        if(u == m)
            selected_beam = explorationGroup.front(); //select all under-explored actions
        else if (u > m)
            selected_beam = explorationGroup[intrand(u)]; // Randomly select 1 under-explored actions
        else {
            //Nothing. because m = 1, therefore if u < m, then u = 0, and it's exploitation step.
            ASSERT(false);
        }
    }
    else {
        //Exploitation Step

        double maxMeanReward = -1;
        uint32_t maxIndex = 0;
        for(uint32_t i = 0 ; i < meanRewardPerBeam.size() ; i ++)
            if(meanRewardPerBeam[i][doAIndex] > maxMeanReward) {
                maxMeanReward = meanRewardPerBeam[i][doAIndex];
                maxIndex = i;
            }
        selected_beam = maxIndex;
    }

    emit(periodSignal, periodCount);
//    emit(explorationGroupSizeSignal, (long) explorationGroup.size());
    return selected_beam; // dummy one
}

void veins::FmlAPMac::updateFMLStep(FmlServiceFeedback* feedback) {
    //if(feedback->getAggregateData() > 0) {
        //TODO just in case the feedbackValue is data. if it's received power signal (smaller than 0), need to change
        //TODO make sure msBS receives the feedback frame, using ACK?
        //sumRewardPerBeam[selectedBeam][]
        sumRewardPerBeam[feedback->getSelectedBeam()][feedback->getDoAIndex()] += feedback->getAggregateData(); // Add reward of selected action
        countSelectionPerBeam[feedback->getSelectedBeam()][feedback->getDoAIndex()] += 1; // Increase one for selected action per car
        meanRewardPerBeam[feedback->getSelectedBeam()][feedback->getDoAIndex()] =
                sumRewardPerBeam[feedback->getSelectedBeam()][feedback->getDoAIndex()]
              / countSelectionPerBeam[feedback->getSelectedBeam()][feedback->getDoAIndex()];
   // }
}

void veins::FmlAPMac::updateFMLStep() {
    auto it = doAMap.find(evaluationServingSTA);
    ASSERT(it != doAMap.end());

    sumRewardPerBeam[selectedBeam][it->second] += dataSent; // Add reward of selected action
    countSelectionPerBeam[selectedBeam][it->second] += 1; // Increase one for selected action per car
    meanRewardPerBeam[selectedBeam][it->second] =
            sumRewardPerBeam[selectedBeam][it->second]
          / countSelectionPerBeam[selectedBeam][it->second];
}

double veins::FmlAPMac::getFactorControlFunction() {
    return factorControlFunctionValue;
}

double veins::FmlAPMac::getControlFunction(uint32_t z, uint32_t t) {
    return pow(t, z) * log(t);
}

void veins::FmlAPMac::initializeFML() {
    // set system variables
    context_no = 1;
    m = 1;

    alpha = 1;
    z = (2*alpha)/(3*alpha + context_no);

    //initialization
    sumRewardPerBeam = countSelectionPerBeam = meanRewardPerBeam = {}; // zero all 3 array's elements

    periodCount = 0;

    factorControlFunction = par("factorControlFunction").intValue();
    totalNo = par("totalNo").intValue();

    factorControlFunctionValue = ((double) factorControlFunction) / (pow(totalNo, 0.5) * log(totalNo));
}

void veins::FmlAPMac::handleSelfMsg(cMessage* msg) {
    MmWaveAPMac::handleSelfMsg(msg);

    if(msg == selectingANewBeam) {
        if(useOracle) {
            isBeamSelected = true;
            selectedBeam = beamIsGoingToBeSelected;
        }

        //scheduleAt(simTime() + SimTime(2, SimTimeUnit::SIMTIME_S), findNewCarToServeAfterBeamSelection);
    }
    else if (msg == estimateRX) {
        //This if is to make sure estimateRX doesn do stupid thing when we are evaluating main station, set by performBTI
        if(evaluationServingSTA == vehicleTriggeringBeamSelection) {
            double rxPower = vehicleTriggeringBeamSelectionPhy80211ad->getRxPowerToRSUAt(simTime(), phy11ad->getCurrentPosition(), phy11ad->getCurrentOrientation(), selectedBeam, OMNIDIRECTIONAL_ANTENNA);

            if(considerObstaclesInVehiclesLeavingBeam) {
                double newReceivedPower = phy11ad->getReceivedPowerAfterApplyingAnalogueModels(rxPower,
                                                                    {phy11ad->getAntennaPosition(), phy11ad->getCurrentOrientation(), nullptr},
                                                                    {vehicleTriggeringBeamSelectionPhy80211ad->getAntennaPosition(), vehicleTriggeringBeamSelectionPhy80211ad->getCurrentOrientation(), nullptr});
                EV_TRACE << "FML-AP: rxPower of " << evaluationServingSTA <<" after applying analogue models: "<< newReceivedPower <<std::endl;
                rxPower = newReceivedPower;
            }

            //When the main station enters the beam, if BS is serving another station, stop it, and serve the new main one
            if(rxPower > enterBeamThreshold) {
                return;
            }
        }

        std::map<std::string, cModule*> availableCars = traciManager->getManagedHosts();
        if(availableCars.size() == 0) {
            EV_TRACE << "We have no car to serve at this moment!" << std::endl;
            scheduleAt(simTime() + estimateDuration, estimateRX);
            return;
        }

        //This case is likely to happen only at the very beginning of the simulation
        //when no car has sent the FMLService Request
        if(selectedBeam == -1) {
            EV_TRACE << "We haven't picked any beam yet!" << std::endl;
            scheduleAt(simTime() + estimateDuration, estimateRX);
            return;
        }

        Mac11adToPhy11adInterface* staPhyMmWave;
        App11adToMac11adInterface* staMACFML;
        serveTill = simTime();

        std::map<LAddress::L2Type, double> carsInTheBeam;

        for(auto const& it : availableCars) {
            staPhyMmWave = FindModule<Mac11adToPhy11adInterface*>::findSubModule(it.second);
            ASSERT(staPhyMmWave);

            staMACFML = FindModule<App11adToMac11adInterface*>::findSubModule(it.second);
            ASSERT(staMACFML);

            //TODO comment out just to debug
            if(staMACFML->getMACAddress() == evaluationServingSTA){
                if(availableCars.size() == 1) {
                    EV_TRACE <<"Only 1 just serve car in the beam, ignore it, schedule to estimate again!" << std::endl;
                    scheduleAt(simTime() + estimateDuration, estimateRX);
                    return;
                }
                else
                    continue; // We just ignore the serving car, don't consider it
                          // This case normally won't happen, but might be because of the random value in calculating pathloss
                          // the receive sensitivity might be higher than the threshold...

            }
            //Okay, we are going to serve the car which approaches the beam soonest.
            // If there are cars in the beam coverage already, we will pick the one which leaves last
            // This will increases the throughput of the serving car.
            Coord rsuPosition = phy11ad->getCurrentPosition();
            Coord rsuOrientation = phy11ad->getCurrentOrientation();
//            uint32_t bestBeamTowardRSU = staPhyMmWave->getBestBeamByTheOtherPositionAt(rsuPosition, simTime());
            double rxPower = staPhyMmWave->getRxPowerToRSUAt(simTime(), rsuPosition, rsuOrientation, selectedBeam, OMNIDIRECTIONAL_ANTENNA);
            EV_TRACE << "Long test rxPower to RSU at current time in measure Rx to RSU: " << rxPower << std::endl;

            if(considerObstaclesInVehiclesLeavingBeam) {
                double newReceivedPower = phy11ad->getReceivedPowerAfterApplyingAnalogueModels(rxPower,
                                                                    {phy11ad->getAntennaPosition(), phy11ad->getCurrentOrientation(), nullptr},
                                                                    {staPhyMmWave->getAntennaPosition(), staPhyMmWave->getCurrentOrientation(), nullptr});
                EV_TRACE << "FML-AP: rxPower of " << staMACFML->getMACAddress() <<" after applying analogue models: "<< newReceivedPower <<std::endl;
                rxPower = newReceivedPower;
            }

            if(rxPower >= enterBeamThreshold) {
                if(doAMap.find(staMACFML->getMACAddress()) != doAMap.end()) {
                    EV_TRACE << it.first << " is already in the coverage of selected beam, add it to the list of vehicles in the beam!" << std::endl;
                    carsInTheBeam.insert(std::pair<LAddress::L2Type, double>(staMACFML->getMACAddress(), rxPower));
                }
                else {
                    EV_TRACE<< "Wierd stuff happens, probably the distance is greater than dsrc coverage??" << std::endl;
                }
            }
        }


        if(carsInTheBeam.size() > 0) {
//            uint32_t vehicleIndex = RNGCONTEXT intrand(carsInTheBeam.size());
//            evaluationServingSTA = carsInTheBeam[vehicleIndex];

            auto pr = std::max_element
            (
                std::begin(carsInTheBeam), std::end(carsInTheBeam),
                [] (const std::pair<LAddress::L2Type, double> & p1, const std::pair<LAddress::L2Type, double> & p2) {
                    return p1.second < p2.second;
                }
            );
            evaluationServingSTA = pr->first;

            EV_TRACE <<"We found a candidate: " << evaluationServingSTA <<std::endl;

            dataSent = 0;

            beamStates.clear();

            auto* it = &(beamStates[evaluationServingSTA]);
            it->myBestSectorID = selectedBeam;
            it->myBestAntennaID = 0;
            it->finished = true;

            if(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() == 0){
                pumpEvaluatePacketToQueue();
                phy11ad->requestChannelStatusIfIdle();
            }
            else {
                std::stringstream ss;
                ss <<"evaluationServingSTA: " << evaluationServingSTA << ", queueSize: " << myEDCA[MmWaveChannelType::service]->myQueue.queue.size() << ", beamStates size: " << beamStates.size() << ", availableVehicles size: " << availableVehicles.size();

                ss<<", beamSates: ";
                for(auto beamstate: beamStates)
                    ss << beamstate.first << ",";
                ss<<"framesSent: ";
                std::queue<BaseFrame1609_4*> queue = myEDCA[MmWaveChannelType::service]->myQueue.queue;
                for(uint32_t i = 0; i < queue.size(); i++)
                    ss<< queue.front()->getRecipientAddress() << ", ";

                ASSERT2(myEDCA[MmWaveChannelType::service]->myQueue.queue.front()->getRecipientAddress() == evaluationServingSTA, ss.str().c_str());
            }
        }
        else {
            EV_TRACE <<"We found no candidates, schedule estimate again!" << std::endl;
            scheduleAt(simTime() + estimateDuration, estimateRX);
        }

    }
}

bool veins::FmlAPMac::serveTheNewVehicle() {
    //This should be comment because we need the previous evaluateServingSTA in order to avoid reserve it again (because of random in FSPL)
//    evaluationServingSTA = LAddress::L2NULL();

    if(!estimateRX->isScheduled())
        scheduleAt(simTime(), estimateRX);

    return false; // phy11ad->requestchannelStatus will be called in estimateRX, once we have found a candidate vehicle
}

int32_t veins::FmlAPMac::getReceivingSectorIDForOracle(
        LAddress::L2Type vehicleMAC, Mac11adToPhy11adInterface* phyMmWave) {
    return phyMmWave->getBestBeamByTheOtherPosition(phy11ad->getCurrentPosition());
}

void veins::FmlAPMac::performBTI() {
    //No BTI phase in FML, just schedule the ATI phase immediately
    mcs = MmWaveMCS::cphy_mcs_0;
    availableVehicles.clear();
    setActiveChannel(MmWaveChannelType::control);



    if(selectingANewBeam->isScheduled()) {
        EV_TRACE << "Select beam " << beamIsGoingToBeSelected << " to be evaluate!" << std::endl;
        EV_TRACE << "Triggering by vehicle: " << vehicleTriggeringBeamSelection << std::endl;

        cancelEvent(selectingANewBeam);
        selectedBeam = beamIsGoingToBeSelected;
        isBeamSelected = true;

        //Update, we just stop sending to the current vehicle and find the new car to serve with the newly selected beam
        //Because the current serving vehicle might perform worse with the new selected beam.
        //But if we use whenServingCarLeaveTheBeam to call serveTheNewVehicle, estimateRX might have issue with the newly set evaluationServing
        //If the main mobile station just enter the beam
        if(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() > 0) // We are serving a vehicle, and then the main car leaves, we need to update now, and estimate again
        {
            //whenServingCarLeaveTheBeam(false);
//            updateFMLStep();
//            dataSent = 0;
        }
    }

    scheduleAt(simTime(), beginOfATI);

    if(evaluationServingSTA != vehicleTriggeringBeamSelection && vehicleTriggeringBeamSelection != -1){
        //to avoid at the beginning of the simulation when there is no main station yet.
        if(!vehicleTriggeringBeamSelectionPhy80211ad)
            return;

        double rxPower = vehicleTriggeringBeamSelectionPhy80211ad->getRxPowerToRSUAt(simTime(), phy11ad->getCurrentPosition(), phy11ad->getCurrentOrientation(), selectedBeam, OMNIDIRECTIONAL_ANTENNA);

        if(considerObstaclesInVehiclesLeavingBeam) {
            double newReceivedPower = phy11ad->getReceivedPowerAfterApplyingAnalogueModels(rxPower,
                                                                {phy11ad->getAntennaPosition(), phy11ad->getCurrentOrientation(), nullptr},
                                                                {vehicleTriggeringBeamSelectionPhy80211ad->getAntennaPosition(), vehicleTriggeringBeamSelectionPhy80211ad->getCurrentOrientation(), nullptr});
            EV_TRACE << "FML-AP: rxPower of " << vehicleTriggeringBeamSelection <<" after applying analogue models: "<< newReceivedPower <<std::endl;
            rxPower = newReceivedPower;
        }

        //When the main station enters the beam, if BS is serving another station, stop it, and serve the new main one
        if(rxPower > enterBeamThreshold) {
            //If we are serving a normal mobile station, we cancel the serving
            if(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() != 0){
                EV_TRACE << "Main station enters the beam, cancel serving with " << evaluationServingSTA << std::endl;
                ASSERT2(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() < 2, "test queue size < 2");
                myEDCA[MmWaveChannelType::service]->myQueue.waitForAck = false;
                delete myEDCA[MmWaveChannelType::service]->myQueue.queue.front();
                myEDCA[MmWaveChannelType::service]->myQueue.queue.pop();
                waitUntilAckRXorTimeout = false;
            } //else bs is serving none

            evaluationServingSTA = vehicleTriggeringBeamSelection;

            EV_TRACE <<"Main station " << vehicleTriggeringBeamSelection << " enters the selected beam coverage, start serving it!" <<std::endl;

            dataSent = 0;

            beamStates.clear();

            auto* it = &(beamStates[evaluationServingSTA]);
            it->myBestSectorID = selectedBeam;
            it->myBestAntennaID = 0;
            it->finished = true;

            if(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() == 0){ // it should be 0, right?
                pumpEvaluatePacketToQueue();
                phy11ad->requestChannelStatusIfIdle();
            }

        }

    }
    //else the serving station is the main one, it's fine.


}

bool veins::FmlAPMac::doesServingVehicleLeaveTheBeam(Mac11adToPhy11adInterface* phyMmWave, LAddress::L2Type vehicleMAC, double sensitivity) {
    Coord rsuPosition = phy11ad->getCurrentPosition();
    Coord rsuOrientation = phy11ad->getCurrentOrientation();

    //Estimate for 10 more seconds
    for(simtime_t i = simTime(); i < simTime() + SimTime(10, SimTimeUnit::SIMTIME_S); i += SimTime(1, SimTimeUnit::SIMTIME_S)) {
//        uint32_t bestBeamTowardRSU = phyMmWave->getBestBeamByTheOtherPositionAt(rsuPosition, i);
        double rxPower = phyMmWave->getRxPowerToRSUAt(i, rsuPosition, rsuOrientation, selectedBeam, OMNIDIRECTIONAL_ANTENNA);

        EV_TRACE << "Long test rxPower to RSU at doesServingVehicleLeavingTheBeam at " << i << " in measure Rx to RSU: " << rxPower << std::endl;

        if(considerObstaclesInVehiclesLeavingBeam) {
            double newReceivedPower = phy11ad->getReceivedPowerAfterApplyingAnalogueModels(rxPower,
                                                                {phy11ad->getAntennaPosition(), phy11ad->getCurrentOrientation(), nullptr},
                                                                {phyMmWave->getAntennaPosition(), phyMmWave->getCurrentOrientation(), nullptr});
            EV_TRACE << "FML-AP: rxPower of " << vehicleMAC <<" after applying analogue models: "<< newReceivedPower <<std::endl;
            rxPower = newReceivedPower;
        }

        if(rxPower > exitBeamThreshold)
            return false;
    }

    return true;

//    if(!considerObstaclesInVehiclesLeavingBeam) {
//        sensitivity = estimateRecevierSensitivityByOracle(vehicleMAC, phyMmWave, false, false);
//        EV_TRACE << "FML-AP: Sensitivity of no obstacles: " << sensitivity << std::endl;
//    }
//
//
//    if(sensitivity < exitBeamThreshold)
//        return true;
//
//    return false;
}

void veins::FmlAPMac::whenServingCarLeaveTheBeam(bool teleported) {
    //We don't update when it is not the main mobile station.
    if(evaluationServingSTA == vehicleTriggeringBeamSelection){
        updateFMLStep();
        dataSent = 0; // we set it to 0 in estimateRX, but its okay
    }

    if(teleported) {
        //Erase it from doAMap
//        doAMap.erase(evaluationServingSTA);
        if(evaluationServingSTA == vehicleTriggeringBeamSelection){
            isBeamSelected = false; // to trigger receiving new request because the vehicleTriggeringBeamSelection is teleported out of map
            vehicleTriggeringBeamSelection = -1;
        }
    }

    if(evaluationServingSTA == vehicleTriggeringBeamSelection)
        updatedTheTriggeringVehicle = true;

    MmWaveAPMac::whenServingCarLeaveTheBeam(teleported);
}

void veins::FmlAPMac::handleAck(const MacAck* ack) {
    if(evaluationServingSTA == vehicleTriggeringBeamSelection) //We just use the aggregate data from the main vehicle to update the beam performance
        dataSent += lastMac->getBitLength();
    MmWaveAPMac::handleAck(ack);
}

void veins::FmlAPMac::pumpEvaluatePacketToQueue() {
    if (estimateRX->isScheduled())
        return; // When we are estimateingRX, it means we are processing for a new car, don't pump!

    MmWaveAPMac::pumpEvaluatePacketToQueue();
}

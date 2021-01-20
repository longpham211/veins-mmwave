/*
 * MmWaveAPMac.cc
 *
 *  Created on: Apr 7, 2020
 *      Author: longpham211
 */

#include "veins-mmwave/mac/MmWaveAPMac.h"

using namespace veins;

using std::unique_ptr;

Define_Module(veins::MmWaveAPMac);

void veins::MmWaveAPMac::initialize(int stage) {
    MmWaveMac::initialize(stage);
    if (stage == 0) {
        beaconIntervalDuration =
                SimTime((double) par("beaconIntervalDuration").doubleValue() / 1e3);

        a_bft_length = par("a_bft_length").intValue();
        maximumAssignedSPDuration = par("maximumAssignedSPDuration").intValue();
        servingOldVehicleInNewBeaconInterval = par("servingOldVehicleInNewBeaconInterval").boolValue();

        newBeaconInterval = new cMessage("new Beacon Interval");

        scheduleAt(simTime(), newBeaconInterval);
    }
}

void veins::MmWaveAPMac::handleSelfMsg(cMessage* msg) {
    if(msg == beginOfAnAllocation) {
        if(evaluateAlgorithms) {
            for (auto const& beamState : beamStates) {
                availableVehicles.push_back(beamState.first);
            }

            if(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() > 0) {
                ASSERT(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() == 1);
                myEDCA[MmWaveChannelType::service]->myQueue.waitForAck = false;
                delete myEDCA[MmWaveChannelType::service]->myQueue.queue.front();
                myEDCA[MmWaveChannelType::service]->myQueue.queue.pop();
                waitUntilAckRXorTimeout = false;
                EV_TRACE << "MMWaveApMac beginOfAnALlocation: Dont know why the queue still has 1 frame left, just delete it, and pump a new one"<< std::endl;

            }

            if(servingOldVehicleInNewBeaconInterval && evaluationServingSTA != LAddress::L2NULL()){
                EV_TRACE << "Keep serving the vehicle in previous BI" << std::endl;
                if(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() == 0){
                    //PrepareDataToSend at MmWaveMac::handleSelfMsg will call send
                    pumpEvaluatePacketToQueue();
                }
                else
                    ASSERT(myEDCA[MmWaveChannelType::service]->myQueue.queue.front()->getRecipientAddress() == evaluationServingSTA);
            }
            else
                //Randomly pick 1 vehicle among the available cars to send data
                serveTheNewVehicle();
        }
    }

    MmWaveMac::handleSelfMsg(msg);

    //only PCP/AP can starts a new beacon interval
    // at each new beacon interval, PCP/AP performs a BTI.
    if (msg == newBeaconInterval) {
        performBTI();
        scheduleAt(simTime() + beaconIntervalDuration, newBeaconInterval);
    }
    else if (msg == beginOfA_BFT) {
        //aSSFBDuraion is defined in Table 10-18 DMG MAC sublayer parameter values in the IEEE 802.11ad - 2012 standard
        simtime_t aSSFBDuration = getSSFBDuration();

        //aSSSlotTime is defined in 9.35.5.2 in the IEEE 802.11ad - 2012 standard
        simtime_t aSSSlotTime = getSSSlotTime(fss);

        sendSSWFeedbackFrame->par("a-bft-length").setLongValue(a_bft_length);

        // follow 9.35.5.2 Operation during the A-BFT
        scheduleAt(simTime() + (aSSSlotTime - aSSFBDuration - MBIFS_11AD), sendSSWFeedbackFrame);


        simtime_t timeWhenA_BFTEnds = simTime() + aSSSlotTime * a_bft_length;

        //9.33.3 The ATI shall not start sooner than Max(guard time, MBIFS)
        simtime_t timeWhenATIBegins = timeWhenA_BFTEnds + std::max(GUARD_TIME_NONPSEUDO_STATIC, MBIFS_11AD);

        scheduleAt(timeWhenATIBegins, beginOfATI);
    }
    else if (msg == sendSSWFeedbackFrame) {
        ASSERT (currentAccessPeriod == AccessPeriod::A_BFT);

        long remainedSlots = sendSSWFeedbackFrame->par("a-bft-length").longValue();
        if(remainedSlots != 1) { // we start from A-BFT, therefore, 1 is the last slot, we don't have to schedule again.
            sendSSWFeedbackFrame->par("a-bft-length").setLongValue(--remainedSlots);
            scheduleAt(simTime() + getSSSlotTime(fss), sendSSWFeedbackFrame);
        }
        // If we receive any SSW Frame in this slot, we should send the SSW Feedback to the responder
        if(communicatingSTA != LAddress::L2NULL()) {
            EV_TRACE << "Okay, in this SSW time slot, I received at least 1 SSW Frame from " << communicatingSTA
                    << " , I am going to send them the feedback"<<std::endl;

            BestBeamInfo * info = &(beamStates[communicatingSTA]);

            info->finished = true; // in A-BFT period, we don't need to wait for the ACK, therefore, we are done at this point.

            generateAndQueueSSWFeedbackFrame(info);

            communicatingSTA = LAddress::L2NULL();
        }
        else EV_TRACE << "I don't receive any SSW frame in this time slot, so sad." << std::endl;
    }

    else if (msg == beginOfATI) {
        ASSERT(activeChannel == MmWaveChannelType::control);

        EV_TRACE << "Long test, time to beam training: " << (newBeaconInterval->getArrivalTime() - simTime()).inUnit(SimTimeUnit::SIMTIME_US)<< std::endl;

        if(!evaluateAlgorithms) {
            EV_TRACE <<"RSU starts sending poll frame to all the stations in the network!" <<std::endl;

            //TODO properly needs to create a different vectors to maintain a stations in PBSS

            // PCP iterates starts asking all the stations in its PBSS which type of allocation (SP or CBAP) they want to use
            allocationRequested.clear();
            for(std::pair<LAddress::L2Type, BestBeamInfo> element : beamStates) {
                EV_TRACE << "Sending Poll Frame to node: " << element.first<< std::endl;
                PollFrame * aPollFrame = new PollFrame();
                aPollFrame->setName("Poll Frame");
                aPollFrame->setKind(packet_kind::POLL_FRAME);
                aPollFrame->setDuration(0); //TODO check this + SPR Frame generation, duration of pollFrame in the standard should be the end of ATI, but how PCP/AP knows that?
                aPollFrame->setResponseOffset(0);

                aPollFrame->setRecipientAddress(element.first);
                aPollFrame->setBitLength(POLL_FRAME_LENGTH);

                int num = myEDCA[MmWaveChannelType::control]->queuePacket(aPollFrame);

                if (num == -1)
                    statsDroppedPackets++;
                if (num == 1) {
                    if(nextMacControlEvent->isScheduled())
                        cancelEvent(nextMacControlEvent);
                    scheduleAt(simTime(), nextMacControlEvent);
                }
            }
        }
        else {
            EV_TRACE <<"We don't do polling scheme and sending announce frames in ATI, rsu knows by itself there is only 1 allocation" << std::endl;
            addAnnounceFramesToQueue(SimTime());
        }
    }
}

void veins::MmWaveAPMac::handleAckTimeOut(AckTimeOutMessage* ackTimeOutMsg) {
    MmWaveMac::handleAckTimeOut(ackTimeOutMsg);

    if(rxStartIndication)
        return;

    // If we are in ATI phase, then probably we want to send the next Poll frame when we haven't received SPR frame
    if(currentAccessPeriod == AccessPeriod::ATI) {
        if(myEDCA[MmWaveChannelType::control]->myQueue.queue.size() > 0) {
            scheduleTheNextPollFrame();
        }
        else
            addAnnounceFramesToQueue(SIFS_11AD + SLOTLENGTH_11AD);
    }
}

void veins::MmWaveAPMac::scheduleTheNextPollFrame() {
    ASSERT (dynamic_cast<PollFrame * > (myEDCA[MmWaveChannelType::control]->myQueue.queue.front()));
    SimTime PIFS_11AD = SIFS_11AD + SLOTLENGTH_11AD; // https://en.wikipedia.org/wiki/Point_coordination_function#PCF_Interframe_Space
    //PIFS in all the cases? or SIFS?
    scheduleAt(simTime() + PIFS_11AD, nextMacControlEvent);
}

void veins::MmWaveAPMac::addAnnounceFramesToQueue(simtime_t alreadyPassed) {
    EV_TRACE << "Add Announce Frames to Queue!" << std::endl;
    ASSERT (newBeaconInterval->isScheduled()); // The current BI has not ended yet

    AnnounceFrame * aSampleAnnounceFrame = new AnnounceFrame();
    aSampleAnnounceFrame->setName("Announce Frame");
    aSampleAnnounceFrame->setKind(packet_kind::ANNOUNCE_FRAME);
    aSampleAnnounceFrame->setBitLength(ANNOUNCE_FRAME_MANDATORY_FIELDS_LENGTH);
    AllocationField * allocationFields = new AllocationField[allocationRequested.size() + 1];

    simtime_t PIFS = SIFS_11AD + SLOTLENGTH_11AD;
    simtime_t sendingDuration = phy11ad->getFrameDuration(mcs, ANNOUNCE_FRAME_MANDATORY_FIELDS_LENGTH + ALLOCATION_FIELD_LENGTH * (allocationRequested.size() + 1), 0);

    simtime_t endOfATIAt;

    if(evaluateAlgorithms)
        endOfATIAt = simTime() + alreadyPassed; // rsu does not send out announceFrame when evaluate
    else
        endOfATIAt = simTime() - alreadyPassed + beamStates.size() * (sendingDuration + PIFS + RADIODELAY_11AD) + AIR_PROPAGATION_TIME; // 1 PIFS between each Announce Frame // TODO may be remove radiodelay when changing logic in TX_OVER lower control msg.
    simtime_t currentAllocationStartAt = endOfATIAt + MBIFS_11AD; //TODO MBIFS between ATI and DTI?

    EV_TRACE << "endOfATI = " << endOfATIAt << " currentAllocationStartAt: " << currentAllocationStartAt<<std::endl;
    uint32_t count = 0;
    for (auto const& allocationPair : allocationRequested) {
        if(currentAllocationStartAt
                + SimTime(allocationPair.second.allocationDuration, SimTimeUnit::SIMTIME_US)
                > newBeaconInterval->getArrivalTime() - MBIFS_11AD){ // TODO MBIFS between each BTI?
            break;
        }

        allocationFields[count].sourceAID = allocationPair.second.sourceAID;
        allocationFields[count].destinationAID = allocationPair.second.destinationAID;
        allocationFields[count].numberOfBlocks = 1;
        allocationFields[count].allocationBlockDuration = allocationPair.second.allocationDuration;
        allocationFields[count].allocationType = static_cast<uint32_t>(AllocationType::SP);
        //"The Allocation Start field contains the lower 4 octets of the TSF at the time the SP or CBAP starts" IEEE 802.11ad 2012 standard
        allocationFields[count].allocationStart = (currentAllocationStartAt).inUnit(SimTimeUnit::SIMTIME_US);

        //A guard time should be added between each allocation as defined in  9.33.6.5 Guard time IEEE 802.11ad
        currentAllocationStartAt += SimTime(allocationPair.second.allocationDuration, SimTimeUnit::SIMTIME_US) + GUARD_TIME_NONPSEUDO_STATIC;
        count ++;
    }

    //if we still have time left for allocation, use this time for 1 CBAP allocation / or SP for RSU depending on evaluate alrothim value
    if(currentAllocationStartAt < newBeaconInterval->getArrivalTime() - MBIFS_11AD) {
        // If we don't need to evaluate the algorithms, then assign the left over as CBAP allocation
        if(!evaluateAlgorithms) {
            allocationFields[count].sourceAID = LAddress::L2BROADCAST();
            allocationFields[count].allocationType = static_cast<uint32_t>(AllocationType::CBAP);
        }
        //otherwise, we use this as SP allocation for RSU
        else {
            allocationFields[count].sourceAID = myMacAddr;
            allocationFields[count].allocationType = static_cast<uint32_t>(AllocationType::SP);
        }

        allocationFields[count].destinationAID = LAddress::L2BROADCAST();
        allocationFields[count].numberOfBlocks = 1;
        allocationFields[count].allocationBlockDuration = (newBeaconInterval->getArrivalTime() - MBIFS_11AD - currentAllocationStartAt).inUnit(SimTimeUnit::SIMTIME_US);
        //"The Allocation Start field contains the lower 4 octets of the TSF at the time the SP or CBAP starts" IEEE 802.11ad 2012 standard
        allocationFields[count].allocationStart = (currentAllocationStartAt).inUnit(SimTimeUnit::SIMTIME_US);
        count ++;
    }

    aSampleAnnounceFrame->setAllocationFieldsArraySize(count);
    for(uint32_t i = 0 ; i < count ; i ++) {
        aSampleAnnounceFrame->setAllocationFields(i, allocationFields[i]);
        assignedAllocations.push_back(allocationFields[i]);
    }

    aSampleAnnounceFrame->addBitLength(ALLOCATION_FIELD_LENGTH * count);

    if(!evaluateAlgorithms)  // rsu does not send out announceFrame when evaluate
        for(auto const& element: beamStates) {

            AnnounceFrame * anAnnounceFrame = aSampleAnnounceFrame->dup();

            //TODO which IFS between each announce frames?

            anAnnounceFrame->setRecipientAddress(element.first);

            int num = myEDCA[MmWaveChannelType::control]->queuePacket(anAnnounceFrame);

            if (num == -1)
                statsDroppedPackets++;
            if (num == 1) {
                if(nextMacControlEvent->isScheduled())
                    cancelEvent(nextMacControlEvent);
                scheduleAt(simTime() - alreadyPassed + PIFS, nextMacControlEvent);
            }
        }

    delete aSampleAnnounceFrame;


    scheduleAt(SimTime(assignedAllocations.front().allocationStart, SimTimeUnit::SIMTIME_US), beginOfAnAllocation);

}

void veins::MmWaveAPMac::performBTI() {
    currentAccessPeriod = AccessPeriod::BTI;
    communicatingSTA = LAddress::L2NULL();
    mcs = MmWaveMCS::cphy_mcs_0;
    availableVehicles.clear();
    setActiveChannel(MmWaveChannelType::control);

    if(refreshBeamStateEveryBI)
        beamStates.clear();
    else
        ASSERT2(false, "Just support true, otherwise, we must refresh manually, like each 5sec?");


    DMGBeacon* dmgBeacon = generateDMGBeacon();

    attachBICField(dmgBeacon, a_bft_length, fss);

    simtime_t dmgBeaconDuration = phy11ad->getFrameDuration(mcs, DMG_BEACON_HEADER_TRAILER_LENGTH + DMG_BEACON_BIC_FIELD_LENGTH + SSW_FIELD_LENGTH, 0);
    simtime_t timeWhenBTIEnds = simTime() + AIR_PROPAGATION_TIME
            + dmgBeaconDuration * fss
            + (fss - 1) * SBIFS_11AD; // TODO check this again if we use more than 1 antenna, and a different value than fss

    simtime_t timeWhenA_BFTBegins = timeWhenBTIEnds + MBIFS_11AD; //MBIFS should be use between BTI and A-BFT

    ASSERT(!beginOfA_BFT->isScheduled());
    scheduleAt(timeWhenA_BFTBegins, beginOfA_BFT); // This is for PCP/AP.

    //TODO the routers might, or might not fragment the TXSS, check 9.35.4 Beamforming in BTI,
    // by setting the noFrames to fss, just to test...
    //performTXSS(dmgBeacon, SimTime(), totalSectors);
    performTXSS(dmgBeacon, SimTime(), fss);  // I-TXSS, we can send the frame immediately
}

DMGBeacon* veins::MmWaveAPMac::generateDMGBeacon() {
    DMGBeacon* dmgBeacon = new DMGBeacon();

    // OMNET-specific
    dmgBeacon->setName("DMG Beacon");
    dmgBeacon->setKind(packet_kind::DMG_BEACON);

    dmgBeacon->setRecipientAddress(LAddress::L2BROADCAST());

    dmgBeacon->setBitLength(DMG_BEACON_HEADER_TRAILER_LENGTH);
    dmgBeacon->setDirection(FROM_INITIATOR);

    return dmgBeacon;
}

veins::MmWaveAPMac::~MmWaveAPMac() {
    if (newBeaconInterval) {
        cancelAndDelete(newBeaconInterval);
        newBeaconInterval = nullptr;
    }
}

void veins::MmWaveAPMac::handleLowerControl(cMessage* msg) {
    MmWaveMac::handleLowerControl(msg);

    if (msg->getKind() == MacToPhyInterface::PHY_RX_END_WITH_FAILURE) {
        // PCP/AP receives this lower control message only when transmitting Poll frame
        // Then it decides to send poll frame to the another station, or send announce frames.
        if(dynamic_cast<PollFrame*>(lastSentPacket)){

            ASSERT (currentAccessPeriod == AccessPeriod::ATI);

            if(myEDCA[activeChannel]->myQueue.ackTimeOut->isScheduled())
                cancelEvent(myEDCA[activeChannel]->myQueue.ackTimeOut);
            if(myEDCA[MmWaveChannelType::control]->myQueue.queue.size() > 0)
                scheduleTheNextPollFrame();
            else
                addAnnounceFramesToQueue(SimTime());
        }
    }
}

bool veins::MmWaveAPMac::handleUnicast(MacPkt* macPkt,
        DeciderResult80211* res) {

    if(! MmWaveMac::handleUnicast(macPkt, res)) {

        // If I receive a SPR Frame, and I should transmit a poll frame to other STA
        if (macPkt->getKind() == packet_kind::SPR_FRAME) {
           ASSERT2(rxStartIndication, "rxStartIndication should be true to wait for SPR Frame");
           phy11ad->notifyMacAboutRxStart(false);
           EV_TRACE <<"turn off notifyMacAboutRxStart because of receiving SPR Frame at AP!" << std::endl;
           rxStartIndication = false;

           //This is for the SPR Frame
           if(myEDCA[MmWaveChannelType::control]->myQueue.ackTimeOut->isScheduled())
               cancelEvent(myEDCA[MmWaveChannelType::control]->myQueue.ackTimeOut);

           auto * sprFrame = check_and_cast<SPRFrame *> (macPkt->decapsulate());

           EV_TRACE << "Receive a SPR Frame with " << sprFrame->getDynamicAllocationInfo().allocationType << " allocation type!"<< std::endl;
           //We just add SP allocation.
           if(sprFrame->getDynamicAllocationInfo().allocationType == static_cast<uint32_t>(AllocationType::SP)){
               if(sprFrame->getDynamicAllocationInfo().allocationDuration <= maximumAssignedSPDuration){
                   allocationRequested.insert(std::make_pair(macPkt->getSrcAddr(), sprFrame->getDynamicAllocationInfo()));
                   EV_TRACE << "SPR frame from " << macPkt->getSrcAddr()<< " with SP allocation requested, and requested duration is smaller than maximum assigned duration. Added this allocation request to map!" << std::endl;
               }
           }
           if(myEDCA[MmWaveChannelType::control]->myQueue.queue.size() > 0)
               scheduleTheNextPollFrame();
           else
               addAnnounceFramesToQueue(SimTime());

           delete sprFrame;
           delete res;
        }
        else {
            //normal frame
            unique_ptr<BaseFrame1609_4> receivedFrame(check_and_cast<BaseFrame1609_4*>(macPkt->decapsulate()));
            receivedFrame->setControlInfo(new PhyToMacControlInfo(res));
            EV_TRACE << "Received a data packet addressed to me, sending ACK then sending up the frame to splitter!" << std::endl;
            if(useAcks)
                sendAck(macPkt->getSrcAddr(), receivedFrame->getTreeId());

            sendUp(std::move(receivedFrame).release());
        }
    }
}

void veins::MmWaveAPMac::handleBroadcast(MacPkt* macPkt,
        DeciderResult80211* res) {
    MmWaveMac::handleBroadcast(macPkt, res);

    unique_ptr<BaseFrame1609_4> receivedFrame(check_and_cast<BaseFrame1609_4*>(macPkt->decapsulate()));
    receivedFrame->setControlInfo(new PhyToMacControlInfo(res));
    sendUp(receivedFrame.release());
}

void veins::MmWaveAPMac::handleAck(const MacAck* ack) {
    MmWaveMac::handleAck(ack);

    //Channel idle is called, after receive successful frame (Decider.handleFrameEnd), we don't need to request channel idle.
    pumpEvaluatePacketToQueue();
}

void veins::MmWaveAPMac::finish() {
}

void veins::MmWaveAPMac::attachBICField(DMGBeacon* frame, uint32_t a_bft_length,
        uint32_t fss) {
    frame->setA_bft_length(a_bft_length);
    frame->setFss(fss);
    frame->addBitLength(DMG_BEACON_BIC_FIELD_LENGTH);
}

/*
 * MmWaveMac.cc
 *
 *  Created on: Apr 7, 2020
 *      Author: longpham211
 */

#include <veins-mmwave/mac/MmWaveMac.h>

using namespace veins;

using std::unique_ptr;

Define_Module(veins::MmWaveMac);

void veins::MmWaveMac::initialize(int stage) {
    BaseMacLayer::initialize(stage);

    if(stage == 0) {
        phy11ad = FindModule<Mac11adToPhy11adInterface*>::findSubModule(
                getParentModule());
        ASSERT(phy11ad);

        mcs = MmWaveMCS::cphy_mcs_0;

        txPower = par("txPower").doubleValue();
        numberAntenna = phy11ad->getNumberOfAntenna();

        for (uint32_t i = 0; i < numberAntenna; i++)
            totalSectors += phy11ad->getNumberOfSectorsPerAntenna(i);
        queueSize = par("queueSize");

        dot11BFRetryLimit = par("dot11BFRetryLimit");

        useAcks = par("useAcks").boolValue();

        rxStartIndication = false;
        ignoreChannelState = false;
        waitUntilAckRXorTimeout = false;

        myId = getParentModule()->getParentModule()->getFullPath();

        myEDCA[MmWaveChannelType::control] = make_unique<EDCA>(this, MmWaveChannelType::control, par("queueSize"));
        myEDCA[MmWaveChannelType::control]->myId = myId;
        myEDCA[MmWaveChannelType::control]->myId.append(" CCH");
        myEDCA[MmWaveChannelType::control]->createQueue(CWMIN_11AD, CWMAX_11AD);

        myEDCA[MmWaveChannelType::service] = make_unique<EDCA>(this, MmWaveChannelType::service, par("queueSize"));
        myEDCA[MmWaveChannelType::service]->myId = myId;
        myEDCA[MmWaveChannelType::service]->myId.append(" SCH");
        myEDCA[MmWaveChannelType::service]->createQueue(CWMIN_11AD, CWMAX_11AD);

        setActiveChannel(MmWaveChannelType::control);


        nextMacEvent = new cMessage("next Mac Event");
        nextMacControlEvent = new cMessage ("next Mac Control Event");
        beginOfA_BFT = new cMessage("Begin of A-BFT phase");
        beginOfATI = new cMessage("Begin of ATI phase");
        sendSSWFeedbackFrame = new cMessage("Time to send SSWFeedback");
        sendSSWFeedbackFrame->addPar("a-bft-length").setLongValue(0);
        beginOfAnAllocation = new cMessage("New Allocation begins");
        endOfAnAllocation = new cMessage("Allocation ends");

        refreshBeamStateEveryBI = par("refreshBeamStateEveryBI").boolValue();

        //TODO add other messages?

        // we need to define fss in this general MAC file because not only PCP/AP starts beamforming in BTI,
        //but normal STA can also start beamforming in SP as well.
        fss = par("fss").intValue();


        isInSLSPhase = false; //TODO can we eliminate this variable and use currentAccessPeriod instead?
        currentAccessPeriod = AccessPeriod::BTI;

        communicatingSTA = LAddress::L2NULL();

        evaluateAlgorithms = par("evaluateAlgorithms").boolValue();
        if(evaluateAlgorithms) {
            useAcks = true;
            useOracle = true;
            traciManager = TraCIScenarioManagerAccess().get();
            ASSERT(traciManager);
            evaluatePacketByteLength = par("evaluatePacketByteLength").intValue();
        }

        enterBeamThreshold = par("enterBeamThreshold").doubleValue();
        exitBeamThreshold = par("exitBeamThreshold").doubleValue();
        evaluationServingSTA = LAddress::L2NULL();

        recordInTimeSlot = par("recordInTimeSlot").boolValue();
        if(recordInTimeSlot) {
            recordTimeSlot = par("recordTimeSlot").intValue();
            aggregateDataPerTimeSlot = 0;
            timeSlotRecorded = 1;
        }

        ackLength = par("ackLength");
        doBeamFormingInSPAllocation = par("doBeamFormingInSPAllocation").boolValue();

        //stats
        statsDroppedPackets = 0;
        statsTotalBusyTime = 0;
        statsSentPackets = 0;
        statsSentAcks = 0;

        //signals
//        aggregateDataSignal = registerSignal("aggregateData");
        aggregateDataVector.setName("aggregateData:vector");
        selectedMCSVector.setName("selectedMCS:vector");
        selectedMCSArray = {};

        allowedToSendData = false;
        idleChannel = true;
        lastBusy = simTime();
        channelIdle();

        EV_TRACE << "duration of mcs 0 for 1MB: " << phy11ad->getFrameDuration(static_cast<MmWaveMCS>(0), 1048576 * 8 + 40, 0).inUnit(SimTimeUnit::SIMTIME_US) << std::endl;
        for(uint32_t i = 1 ; i < 13; i++)
            EV_TRACE << "duration of mcs "<< i <<" for 1MB: " << phy11ad->getFrameDuration(static_cast<MmWaveMCS>(i), 1048576 * 8 + 64, 0).inUnit(SimTimeUnit::SIMTIME_US) << std::endl;

        for(uint32_t i = 10; i < 200; i+=1) {
            phy11ad->getReceivePowerdBm(i, 1, 1, 1, 1, false);
        }
    }
}

void veins::MmWaveMac::setActiveChannel(MmWaveChannelType channel) {
    activeChannel = channel;
}

bool veins::MmWaveMac::isControlChannelActive() {
    return activeChannel == MmWaveChannelType::control;
}

bool veins::MmWaveMac::isServiceChannelActive() {
    return activeChannel == MmWaveChannelType::service;
}

void veins::MmWaveMac::handleSelfMsg(cMessage* msg) {
    if (AckTimeOutMessage* ackTimeOutMsg = dynamic_cast<AckTimeOutMessage*>(msg)) {
        handleAckTimeOut(ackTimeOutMsg);
        return;
    }

    else if (msg == beginOfA_BFT) {
        ASSERT(currentAccessPeriod == AccessPeriod::BTI);
        currentAccessPeriod = AccessPeriod::A_BFT;
    }

    else if (msg == sendSSWFeedbackFrame) {
        ASSERT(activeChannel == MmWaveChannelType::control);
    }

    else if (msg == beginOfATI) {
        currentAccessPeriod = AccessPeriod::ATI;
    }

    else if (msg == beginOfAnAllocation) {
        ASSERT(!assignedAllocations.empty());

        scheduleAt(simTime() + SimTime(assignedAllocations.front().allocationBlockDuration, SimTimeUnit::SIMTIME_US), endOfAnAllocation);

        currentAccessPeriod = assignedAllocations.front().allocationType == static_cast<uint32_t>(AllocationType::SP) ? AccessPeriod::SP : AccessPeriod::CBAP;

        //check whether I am allowed to send in this allocation or not!
        if (assignedAllocations.front().sourceAID == myMacAddr
                || assignedAllocations.front().sourceAID == LAddress::L2BROADCAST()) {

            lastIdle = simTime();
            allowedToSendData = true;
            EV_TRACE <<"Begin Of An Allocation: Allowed to send = true, I am going to check if I have any frames in queue to send or not!"<<std::endl;

            prepareToSendDataFrame();
        }
    }

    else if (msg == endOfAnAllocation) {

        allowedToSendData = false;
        assignedAllocations.erase(assignedAllocations.begin());
        if(assignedAllocations.size() > 0)
            scheduleAt(SimTime(assignedAllocations.front().allocationStart, SimTimeUnit::SIMTIME_US), beginOfAnAllocation);
    }

    else if (msg == nextMacEvent || msg == nextMacControlEvent) {
        // we actually came to the point where we can send a packet

        channelBusySelf();
        BaseFrame1609_4* pktToSend;
        if(msg == nextMacEvent) {
            //TODO do we need to switch activeChannel to service one?
            pktToSend = myEDCA[MmWaveChannelType::service]->initiateTransmit(lastIdle);
            setActiveChannel(MmWaveChannelType::service);
        }
        else {
            pktToSend = myEDCA[MmWaveChannelType::control]->myQueue.queue.front();
            setActiveChannel(MmWaveChannelType::control);
        }
        ASSERT(pktToSend);

        lastSentPacket = pktToSend;

        EV_TRACE << "MacEvent received. Trying to send packet! "<<  std::endl;

        //send the packet
        MacPkt* mac = new MacPkt(pktToSend->getName(), pktToSend->getKind());

        if(pktToSend->getRecipientAddress() != LAddress::L2BROADCAST())
            mac->setDestAddr(pktToSend->getRecipientAddress());
        else
            mac->setDestAddr(LAddress::L2BROADCAST());

        mac->setSrcAddr(myMacAddr);

        mac->encapsulate(pktToSend->dup());


        uint32_t antennaID = 1000;
        uint32_t sectorID = 1000;

        simtime_t sendingDuration = SimTime(1000);
        simtime_t delay = SimTime();

        //if this packet in queue contains SSW element (DMG Beacon or SSW_Frame_for_RSS ), then
        //it should add which SectorID and antennaID to send already
        if(auto* castedFrame = dynamic_cast<FramesContainSSWField*>(pktToSend)) {
            antennaID = castedFrame->getAntennaID();
            sectorID = castedFrame->getSectorID();

            sendFrame(mac, delay, antennaID, sectorID, mcs, txPower);

            isInSLSPhase = true; // we put this line after sendFrame because for the first frame, antenna has to switch to TX first

            if(castedFrame->getCDOWN() == 0)
                isInSLSPhase = false;
        }
//        else if (dynamic_cast<SSWFeedbackFrame*>(pktToSend)
//                || dynamic_cast<PollFrame*>(pktToSend)
//                || dynamic_cast<SPRFrame*>(pktToSend)
//                || dynamic_cast<AnnounceFrame*>(pktToSend)){
        else {
            BestBeamInfo * info = &(beamStates[pktToSend->getRecipientAddress()]);
            if(info->myBestSectorID <= 36 && info->myBestSectorID >= 0) {
                antennaID = info->myBestAntennaID;
                sectorID = info->myBestSectorID;
                EV_TRACE <<"handleSelfMsg: packet sent to: " << pktToSend->getRecipientAddress() << " with sectorID: " <<sectorID<< std::endl;
            }
            else {
                EV_TRACE<< "I don't have any information of my best antenna and sector to the station " << pktToSend->getRecipientAddress() << ", probably SSW Feedback frame was lost!"<<std::endl;
                EV_TRACE<< "I just pick a random value of antenna and sector ID and send it!" << std::endl;

                antennaID = 0;
                sectorID = RNGCONTEXT intrand(36);

                EV_TRACE<< " antennaID = " << antennaID << ", sectorID = "<< sectorID<<std::endl;
            }

//            if(msg == nextMacEvent)
//                //Add header length for the packet
//                mac->addBitLength(getHeaderLength(mcs));

            //TODO is there anyway to put this PolLFrame logic in APMac file?
            //IEEE 802.11ad 2012, p.249
            //When a transmission by a STA is expected by a PCP/AP and a SIFS period elapses without its receipt, the
            //PCP/AP may either repeat its individually addressed transmission to that STA or, as early as one PIFS after
            //the end of its previous transmission, transmit a frame to any other STA.
            if(dynamic_cast<PollFrame *> (pktToSend)) {
                SimTime PIFS_11AD = SIFS_11AD + SLOTLENGTH_11AD; // https://en.wikipedia.org/wiki/Point_coordination_function#PCF_Interframe_Space
                simtime_t sprFrameWaitTime(PIFS_11AD + RADIODELAY_11AD);
                //TODO change this to aTimeout... not ackTimeout
                myEDCA[MmWaveChannelType::control]->myQueue.ackTimeOut->setWsmId(pktToSend->getTreeId());

                simtime_t sendingDuration = phy11ad->getFrameDuration(mcs, POLL_FRAME_LENGTH, 0);

                simtime_t timeOut = sendingDuration + sprFrameWaitTime;
                scheduleAt(simTime() + timeOut, myEDCA[MmWaveChannelType::control]->myQueue.ackTimeOut);
            }

            if(dynamic_cast<SSWFeedbackFrame * > (pktToSend) && currentAccessPeriod != AccessPeriod::A_BFT) {
                // 9.35.6.2 SLS phase execution IEEE 802.11ad 2012
                //The responder shall begin an SSW ACK (9.35.2.5) to the initiator in MBIFS time following the reception of
                //a SSW-Feedback frame from the initiator.
                simtime_t sswAckWaitTime = (MBIFS_11AD + RADIODELAY_11AD);
                //TODO change this to aTimeout... not ackTimeout
                myEDCA[MmWaveChannelType::control]->myQueue.ackTimeOut->setWsmId(pktToSend->getTreeId());

                simtime_t sendingDuration = phy11ad->getFrameDuration(mcs, SSW_FEEDBACK_FRAME_LENGTH, 0);

                simtime_t timeOut = sendingDuration + sswAckWaitTime;
                scheduleAt(simTime() + timeOut, myEDCA[MmWaveChannelType::control]->myQueue.ackTimeOut);
            }

            // schedule ack timeout for unicast packet
            if (pktToSend->getRecipientAddress() != LAddress::L2BROADCAST() && useAcks && msg == nextMacEvent) {
                waitUntilAckRXorTimeout = true;

                if(evaluateAlgorithms){
                    Mac11adToPhy11adInterface* phy11adVehicle = getPhyModuleOfVehicleByOracle(pktToSend->getRecipientAddress());
                    if(phy11adVehicle == nullptr) { //Vehicles leaves/teleports out of the beam
                        whenServingCarLeaveTheBeam(true);
                        return;
                    }


                    double receiverSensitivityByOracle = estimateRecevierSensitivityByOracle(pktToSend->getRecipientAddress(), phy11adVehicle, true, false);

                    if(doesServingVehicleLeaveTheBeam(phy11adVehicle, pktToSend->getRecipientAddress(), receiverSensitivityByOracle)) {
                        EV_TRACE << "Out of beam, evaluate algorithms, delete the evaluate frame in queue, pick another station to serve!" << std::endl;
//                        std::stringstream ss;ss<<"framesSent: ";
//                        std::queue<BaseFrame1609_4*> queue = myEDCA[MmWaveChannelType::service]->myQueue.queue;
//                        for(uint32_t i = 0; i < queue.size(); i++)
//                            ss<< queue.front()->getRecipientAddress() << ", ";
//                        ASSERT2(pktToSend->getRecipientAddress() != 53, ss.str().c_str());
                        whenServingCarLeaveTheBeam(false);

                        return;
                    }
                    else
                        EV_TRACE << "Current serving vehicle has not leave the beam yet, continue serving!" << std::endl;


                    mcs = phy11ad->getOptimalMCSFromRx(receiverSensitivityByOracle);
                    EV_TRACE << "evaluateAlgorithm sending MAC msg, picking MCS" << static_cast<int>(mcs) << std::endl;
                    addSelectedMCSToCollection(static_cast<int32_t>(mcs));
                }

                mac->addBitLength(getHeaderLength(mcs));

                simtime_t ackWaitTime(getAckWaitTime(mcs));

                sendingDuration = phy11ad->getFrameDuration(mcs, mac->getBitLength(), 0);

                simtime_t timeOut = sendingDuration + ackWaitTime;

                simtime_t sendingAckDuration = phy11ad->getFrameDuration(MmWaveMCS::cphy_mcs_0, ackLength + getHeaderLength(MmWaveMCS::cphy_mcs_0), 0);

                EV_TRACE << "MMwaveMac::handleSelfMsg: getFrameDuration: " << sendingDuration << std::endl;;
                //If the time left in this allocation is not enough, don't send
                //if this is evaluateAlogithm scenario, delete the first packet in the queue
                //because for the next beacon interval, we will pick another vehicle to evaluate
                if(simTime() + timeOut + sendingAckDuration > endOfAnAllocation->getArrivalTime()) {
                    if(evaluateAlgorithms) {
                        myEDCA[MmWaveChannelType::service]->myQueue.waitForAck = false;
                        delete myEDCA[MmWaveChannelType::service]->myQueue.queue.front();
                        myEDCA[MmWaveChannelType::service]->myQueue.queue.pop();
                        waitUntilAckRXorTimeout = false;
                        EV_TRACE << "HandleSelfMessage: Evaluate algorithms: Not enough time, delete the evaluate frame, stop sending"<< std::endl;
                    }

                    return;
                }

                myEDCA[MmWaveChannelType::service]->myQueue.ackTimeOut->setWsmId(pktToSend->getTreeId());
                scheduleAt(simTime() + timeOut, myEDCA[MmWaveChannelType::service]->myQueue.ackTimeOut);
            }

            sendFrame(mac, delay, antennaID, sectorID, mcs, txPower);
        }
    }

    else {
        EV_TRACE << "We receive a strange event in MmWaveMac, let's see if AP and STA files handle these self-message!" << std::endl;
    }
}

void veins::MmWaveMac::handleUpperControl(cMessage* msg) {
    ASSERT(false);
}

void veins::MmWaveMac::handleAckTimeOut(AckTimeOutMessage* ackTimeOutMsg) {
    if (rxStartIndication)
        // Rx is already in process. Wait for it to complete.
        // In case it is not an ack, we will retransmit
        // This assigning might be redundant as it was set already in handleSelfMsg but no harm in reassigning here.
        return;
    // We did not start receiving any packet.
    // stop receiving notification for rx start as we will retransmit
    phy11ad->notifyMacAboutRxStart(false);
    EV_TRACE <<"turn off notifyMacAboutRxStart because of timeout triggered!" << std::endl;

    //TODO this logic is made in ATI phase & SP phase, probably we need a double check on this, by having a selfmsg to indicate whether or not we are in ATI&SP phases
    if(myEDCA[activeChannel]->myQueue.ackTimeOut->isScheduled())
        cancelEvent(myEDCA[activeChannel]->myQueue.ackTimeOut);

    if(currentAccessPeriod == AccessPeriod::CBAP || currentAccessPeriod == AccessPeriod::SP)
        handleRetransmit();

    //TODO
    // Phy was requested not to send channel idle status on TX_OVER
    // So request the channel status now. For the case when we receive ACK, decider updates channel status itself after ACK RX
    phy11ad->requestChannelStatusIfIdle();
}

void veins::MmWaveMac::handleRetransmit() {
    if (activeChannel == MmWaveChannelType::control){
            EV_TRACE<< "lastSentPacket.getKind(): "<< lastSentPacket->getKind()<< std::endl;
            //ASSERT(dynamic_cast<SSWFeedbackFrame * >(lastSentPacket) && currentAccessPeriod != AccessPeriod::A_BFT);
            // TODO If this function is called, probably we are trying to resend SSW Feedback frame in SP allocation
            // add an ASSERT to check this!
            ASSERT(currentAccessPeriod == AccessPeriod::SP);
    }
    EV_TRACE<<"Handle retransmit!"<<std::endl;

    //cancel the ack timeout even
    if(myEDCA[activeChannel]->myQueue.ackTimeOut->isScheduled())
        // This case is possible if we received PHY_RX_END_WITH_SUCCESS or FAILURE even before ack timeout
        cancelEvent(myEDCA[activeChannel]->myQueue.ackTimeOut);
    if (myEDCA[activeChannel]->myQueue.queue.size() == 0)
        throw cRuntimeError("Trying retransmission on empty queue...");

    BaseFrame1609_4* appPkt = myEDCA[activeChannel]->myQueue.queue.front();
    bool contend = false;
    bool retriesExceeded = false;


    // page 879 of IEEE 802.11-2012 //TODO how it is related to mmWave?

    // when we are evaluateing algorithms, we don't care about the limit number
    if(!evaluateAlgorithms) {
        if(++(myEDCA[activeChannel]->myQueue.rc) <= dot11BFRetryLimit)
            retriesExceeded = false;
        else
            retriesExceeded = true;
    }

    if(!retriesExceeded) {
        //try again
        contend = true;
        // no need to reset wait on id here as we are still retransmitting same packet
        myEDCA[activeChannel]->myQueue.waitForAck = false;
    }
    else {
        //enough tries
        myEDCA[activeChannel]->myQueue.queue.pop();
        if(myEDCA[activeChannel]->myQueue.queue.size() > 0)
            // start sending packet only if there are more packets in the queue
            contend = true;
        delete appPkt;
        myEDCA[activeChannel]->myQueue.waitForAck = false;
        myEDCA[activeChannel]->myQueue.waitOnUnicastID = -1;
        myEDCA[activeChannel]->myQueue.rc = 0;
    }

    waitUntilAckRXorTimeout = false;

    //TODO check this for a normal frames as well.
    if (contend) {
        if(activeChannel == MmWaveChannelType::control) {
            cancelEvent(nextMacControlEvent);
            /**
             * initiator shall restart the SSW Feedback PIFS time following the expected end of the SSW ACK by the responder
             */
            SimTime PIFS_11AD = SIFS_11AD + SLOTLENGTH_11AD; // https://en.wikipedia.org/wiki/Point_coordination_function#PCF_Interframe_Space
            scheduleAt(simTime() + PIFS_11AD, nextMacControlEvent);
        }
        else {
            if (evaluateAlgorithms) {
                cancelEvent(nextMacEvent);
                //We retransmit evaluate frame immediately?
                //should we reduce the mcs because of frame lost? --> mcs is selected by Oracle
                SimTime time = myEDCA[activeChannel]->findTimeForTheNextEvent(lastIdle, AllocationType::SP);

                if(time > endOfAnAllocation->getArrivalTime()) {
                    if(evaluateAlgorithms) {
                        myEDCA[MmWaveChannelType::service]->myQueue.waitForAck = false;
                        delete myEDCA[MmWaveChannelType::service]->myQueue.queue.front();
                        myEDCA[MmWaveChannelType::service]->myQueue.queue.pop();
                        waitUntilAckRXorTimeout = false;
                        EV_TRACE << "HandleRetransmit: Evaluate algorithms: Not enough time, delete the evaluate frame, stop sending"<< std::endl;
                    }

                    return;
                }

                scheduleAt(time,  nextMacEvent);
            }
            else
                ASSERT(false); //TODO Look at the backoff time for normal frames in normal scenarios
        }

    }
}

void veins::MmWaveMac::performTXSS(FramesContainSSWField* sswFrame, simtime_t afterTime, uint32_t noFrames) {
    uint32_t antennaID, numberOfSectorsPerAntenna, index;
    uint32_t tmp_CDOWN = noFrames - 1;
    FramesContainSSWField* sentFrame;

    ASSERT2(numberAntenna == 1, "No support for multiple antennas");

    EV_INFO << "PERFORM TXSS" << std::endl;
    for (antennaID = numberAntenna; antennaID > 0 ; antennaID --) {
        numberOfSectorsPerAntenna = phy11ad->getNumberOfSectorsPerAntenna(
                antennaID);
        std::vector<int> sectorVector(numberOfSectorsPerAntenna);
        std::iota(std::begin(sectorVector), std::end(sectorVector), 0); // fill sectorVector with value 0, 1, 2, ..., 35

        for (index = 0; index < noFrames ; index ++) {
            sentFrame = sswFrame->dup();

//            uint32_t randomIndex = RNGCONTEXT intrand(sectorVector.size());
//            sectorID = sectorVector[randomIndex];
//            sectorVector.erase(sectorVector.begin() + randomIndex);


//            attachSSWField(sentFrame, tmp_CDOWN, antennaID - 1, sectorID);
            attachSSWField(sentFrame, tmp_CDOWN, antennaID - 1, index);

            //TODO do I need to attach the other (mandatory) fields left of DMG Beacon?
            // to increase the bit length correctly?


            //because we are sending in directional beam pattern, not all STA can receive the DMG
            //beacon, therefore we need to add the duration to all of the beacons (help to network synchronization)
            simtime_t frameDuration = phy11ad->getFrameDuration(mcs, sentFrame->getBitLength(), 0);
            simtime_t timeTillCDOWNEquals0 = tmp_CDOWN * frameDuration
                                            + (tmp_CDOWN + (1 - antennaID)) * SBIFS_11AD
                                            + (antennaID - 1) * LBIFS_11AD;

            //TODO can we further separate this logic into the 2 other files?
            if (currentAccessPeriod == AccessPeriod::BTI) {
                simtime_t timeWhenBTIEnds = timeTillCDOWNEquals0; // in DMG Beacon, duration should be set to the time when BTI ends
                sentFrame->setDuration(timeWhenBTIEnds.raw() / (simTime().getScale()/1000000)); // convert to microsecond
            }
            else if (currentAccessPeriod == AccessPeriod::A_BFT) {
                sentFrame->setDuration(0); //TODO correctly, it should be set until the end of the current SSW slot, when the SSW frame is transmitted within an A-BFT // IEEE std 802.11 2016
            }
            else {
                ASSERT (currentAccessPeriod == AccessPeriod::SP);
                // 8.3.1.16 IEEE 802.11ad 2012
                // For SSW Frame as part of the ISS, the duration should be set to the time until the end of the SSW frame transmission
                // that has the CDOWN subfield wihtin the SSW field equal to 0, plus MBIFS, or until the end of the current allocation,
                // whichever comes first.
                simtime_t tmpTime = std::min(simTime() + timeTillCDOWNEquals0 + MBIFS_11AD, endOfAnAllocation->getArrivalTime());
                sentFrame->setDuration((tmpTime - simTime()).raw() / (simTime().getScale()/1000000)); // convert to microsecond
            }

            tmp_CDOWN--;

            //sendDown(sentFrame);
            int num = myEDCA[MmWaveChannelType::control]->queuePacket(sentFrame);
            if (num == -1)
                statsDroppedPackets++;
            else {

                //we can start transmit now, don't need to wait for sif
                if(num == 1) {
                    if(nextMacControlEvent->isScheduled())
                        cancelEvent(nextMacControlEvent);

                    scheduleAt(simTime() + afterTime, nextMacControlEvent);
                    EV_TRACE << "schedule a nextMACControlEvent at: "<< simTime() + afterTime << std::endl;
                }
            }

        }
    }

    //All the sentframe is the duplicated ones, not the sswFrame itself. These frames
    // will be handled/deleted at the receiver sides.
    delete sswFrame;

    //TODO Check this case
    //if (num == 1 && idleChannel == false && myEDCA[chan]->myQueues[ac].currentBackoff == 0 && chan == activeChannel) {
    //    myEDCA[chan]->backoff(ac);
    //}

}

void veins::MmWaveMac::attachSSWField(FramesContainSSWField* frame,
        uint32_t CDOWN, uint32_t antennaID, uint32_t sectorID) {
    frame->setCDOWN(CDOWN);
    frame->setAntennaID(antennaID);
    frame->setSectorID(sectorID);
    frame->addBitLength(SSW_FIELD_LENGTH);
}

veins::MmWaveMac::~MmWaveMac() {
    if (nextMacEvent) {
        cancelAndDelete(nextMacEvent);
        nextMacEvent = nullptr;
    }

    if (nextMacControlEvent) {
        cancelAndDelete(nextMacControlEvent);
        nextMacControlEvent = nullptr;
    }

    if (beginOfA_BFT) {
        cancelAndDelete(beginOfA_BFT);
        beginOfA_BFT = nullptr;
    }

    if (beginOfATI) {
        cancelAndDelete(beginOfATI);
        beginOfATI = nullptr;
    }

    if (sendSSWFeedbackFrame) {
        cancelAndDelete(sendSSWFeedbackFrame);
        sendSSWFeedbackFrame = nullptr;
    }

    if (beginOfAnAllocation) {
        cancelAndDelete(beginOfAnAllocation);
        beginOfAnAllocation = nullptr;
    }

    if (endOfAnAllocation) {
        cancelAndDelete(endOfAnAllocation);
        endOfAnAllocation = nullptr;
    }
}

void veins::MmWaveMac::finish() {
    if(recordInTimeSlot) {
        simtime_t recordTime = simTime();

        if((int)(ceil(simTime().dbl())) % recordTimeSlot == 0)
            recordTime = recordTime - SimTime(1, SimTimeUnit::SIMTIME_MS);

        for(uint32_t i = 0; i < selectedMCSArray.size(); i++)
            selectedMCSVector.recordWithTimestamp(recordTime, selectedMCSArray[i]);
    }
}

simtime_t veins::MmWaveMac::getSSSlotTime(uint32_t fss) {
    //aSSDuration is defined in Table 10-18 DMG MAC sublayer parameter values in the IEEE 802.11ad - 2012 standard
    simtime_t aSSDuration = (fss - 1) * SBIFS_11AD + fss * phy11ad->getFrameDuration(mcs, SSW_FRAME_LENGTH, 0);

    simtime_t aSSFBDuration = getSSFBDuration();

    //aSSSlotTime is defined in 9.35.5.2 in the IEEE 802.11ad - 2012 standard
    simtime_t aSSSlotTime = AIR_PROPAGATION_TIME + aSSDuration + MBIFS_11AD + aSSFBDuration + MBIFS_11AD;

    return aSSSlotTime;
}

simtime_t veins::MmWaveMac::getSSFBDuration() {
    //aSSFBDuraion is defined in Table 10-18 DMG MAC sublayer parameter values in the IEEE 802.11ad - 2012 standard
    return phy11ad->getFrameDuration(mcs, SSW_FEEDBACK_FRAME_LENGTH, 0);
}

void veins::MmWaveMac::generateAndQueueSSWFeedbackFrame(
        BestBeamInfo* info) {
    SSWFeedbackFrame * feedback = new SSWFeedbackFrame();

    feedback->setRecipientAddress(communicatingSTA);
    feedback->setKind(packet_kind::SSW_FEEDBACK);
    feedback->setBitLength(SSW_FEEDBACK_FRAME_LENGTH);
    feedback->setName("SSW Feedback");

    feedback->setDmgAntennaSelect(info->theirBestAntennaID);
    feedback->setSectorSelect(info->theirBestSectorID);
    feedback->setSnrReport(info->theirBestSNR);

    int num = myEDCA[MmWaveChannelType::control]->queuePacket(feedback);
    if (num == -1){
        statsDroppedPackets++;
    }

    if (num == 1) {
        if(nextMacControlEvent->isScheduled())
            cancelEvent(nextMacControlEvent);
        scheduleAt(simTime(), nextMacControlEvent); // now is at aSSFBDuraion + MBIFS before the end of the SSW slot
    }
}

void veins::MmWaveMac::prepareToSendDataFrame() {
    if(!isEDCAQueueEmpty(MmWaveChannelType::service)) {

        LAddress::L2Type receiverAddress = myEDCA[MmWaveChannelType::service]->myQueue.queue.front()->getRecipientAddress();

        communicatingSTA = receiverAddress;

        std::map< LAddress::L2Type, BestBeamInfo>::iterator it = beamStates.find(receiverAddress);

        if(it == beamStates.end()) { // If this STA hasn't beamformed with the receiver before
            ASSERT (!assignedAllocations.empty()); //prepareToSendDataFrame function should be called in DTI phase.
            EV_TRACE<< "I don't have any beamforming information with " << receiverAddress << std::endl;

            if(!doBeamFormingInSPAllocation) {
                std::stringstream ss;
                ss <<"queueSize: " << myEDCA[MmWaveChannelType::service]->myQueue.queue.size() << ", beamStates size: " << beamStates.size() << ", availableVehicles size: " << availableVehicles.size();

                ss<<", beamSates: ";
                for(auto beamstate: beamStates)
                    ss << beamstate.first << ",";

                ss<<", vehicles: ";
                for(auto vehicle: availableVehicles)
                    ss << vehicle << ",";

                ss<<"framesSent: ";
                std::queue<BaseFrame1609_4*> queue = myEDCA[MmWaveChannelType::service]->myQueue.queue;
                for(uint32_t i = 0; i < queue.size(); i++)
                    ss<< queue.front()->getRecipientAddress() << ", ";

                ASSERT2(false, ss.str().c_str());
            }
            if(assignedAllocations.front().allocationType == static_cast<uint32_t>(AllocationType::SP)){ // Beamforming is only allowed in SP phase, not CBAP (TODO except for TXOP)?
                EV_TRACE << "This allocation is my assigned SP allocation, I will perform BeamForming with " << receiverAddress << std::endl;

                BestBeamInfo beamInfo;
                beamStates.insert(std::pair<LAddress::L2Type, BestBeamInfo>(receiverAddress, beamInfo)); // insert an empty BestBeamInfo
                mcs = MmWaveMCS::cphy_mcs_0; // Well, even in DTI, we also should also do beamforming in cphy

                SSWFrameISS* sswFrameISS = generateSSWFrameISS();
                performTXSS(sswFrameISS, SimTime(), fss); //TODO change this fss to a correct one, i.e. totalAllowedSectors
            }
            else
                EV_TRACE << "This allocation is CBAP allocation, I can't do anything!" << receiverAddress << std::endl;

            return;
        }
        else if (!it->second.finished) {
            return; // TODO We are still in the SLS phase. We haven't had the beamforming yet. However, if the feedback message was lost, then there might be a problem!
        }

        simtime_t nextEvent = myEDCA[MmWaveChannelType::service]->findTimeForTheNextEvent(lastIdle, (AllocationType) assignedAllocations.front().allocationType);
        if (nextEvent != -1) {
            if(nextMacEvent->isScheduled())
                cancelEvent(nextMacEvent);

            if(nextEvent > endOfAnAllocation->getArrivalTime()) {
                if(evaluateAlgorithms) {
                    myEDCA[MmWaveChannelType::service]->myQueue.waitForAck = false;
                    delete myEDCA[MmWaveChannelType::service]->myQueue.queue.front();
                    myEDCA[MmWaveChannelType::service]->myQueue.queue.pop();
                    waitUntilAckRXorTimeout = false;
                    EV_TRACE << "HandleRetransmit: Evaluate algorithms: Not enough time, delete the evaluate frame, stop sending"<< std::endl;
                }

                return;
            }

            scheduleAt(nextEvent, nextMacEvent);
            EV_TRACE << "next Event is at " << nextMacEvent->getArrivalTime().raw() << std::endl;
        }
        else
            EV_TRACE << "I don't have any new events in the MAC queue!" << std::endl;
    }
}

SSWFrameISS* veins::MmWaveMac::generateSSWFrameISS() {
    SSWFrameISS* sswFrameISS = new SSWFrameISS();


    sswFrameISS->setName("SSW Frame from Initiator");
    sswFrameISS->setKind(packet_kind::SSW_FRAME_ISS);

    sswFrameISS->setDirection(FROM_INITIATOR);

    sswFrameISS->setRecipientAddress(communicatingSTA);

    sswFrameISS->setBitLength(SSW_FRAME_LENGTH_WITHOUT_SSW_FIELD);

    return sswFrameISS;
}

void veins::MmWaveMac::channelBusySelf() {
    //OKAY
    if(!idleChannel) return;

    idleChannel = false;
    EV_TRACE << "Channel turned busy because of Self-Send" << std::endl;

    lastBusy = simTime();

    //channel turned busy
    if(nextMacEvent->isScheduled() == true)
        cancelEvent(nextMacEvent);
    if(nextMacControlEvent->isScheduled() == true)
        cancelEvent(nextMacControlEvent);
}

void veins::MmWaveMac::sendFrame(MacPkt* frame, omnetpp::simtime_t delay,
        uint32_t antennaID, uint32_t sectorID, MmWaveMCS mcs, double txPower_mW) {

    if(!isInSLSPhase){
        EV_TRACE<<"set Radio state to tX in SendFrame, this frame is not in SLS, or the first frame of SLS"<<std::endl;
        phy->setRadioState(Radio::TX); // give time for the radio to be in Tx state before transmitting
        delay = std::max(delay, RADIODELAY_11AD); // wait at least for the radio to switch
    }
    else // we are in SLS phase, the antenna is still on TX mode
        delay = 0;

    lastUsedSectorID = sectorID;

    attachControlInfo(frame, antennaID, sectorID, mcs, txPower_mW);
    check_and_cast<MacToPhyControlInfo11ad*>(frame->getControlInfo());

    lastMac.reset(frame->dup());
    sendDelayed(frame, delay, lowerLayerOut);

    if (dynamic_cast<MacAck*>(frame)) {
        statsSentAcks += 1;
    }
    else {
        statsSentPackets += 1;
    }
}

void veins::MmWaveMac::attachControlInfo(MacPkt* mac, uint32_t antennaID,
        uint32_t sectorID, MmWaveMCS mcs, double txPower_mW) {
    auto cinfo = new MacToPhyControlInfo11ad(antennaID, sectorID, mcs, txPower_mW);
    mac->setControlInfo(cinfo);
}

void veins::MmWaveMac::handleUpperMsg(cMessage* msg) {
    ASSERT(dynamic_cast<cPacket*>(msg));
    //TODO Assert if this msg is mmWave msg.

    BaseFrame1609_4* thisMsg = check_and_cast<BaseFrame1609_4*>(msg);

    ASSERT2 (thisMsg->getRecipientAddress() != LAddress::L2BROADCAST(), "MMWave MAC module just accepts unicast messages!");

    EV_TRACE << "Received a message from upper layer!" << std::endl;

    //sendDown(encapsMsg(check_and_cast<cPacket*>(msg)));

    //A STA is just alowed to send when it is in an assigned SP allocation, or in CBAP
    //Therefore, we just add the msg to the queue and send the frame later in the assigned allocations
    int num = myEDCA[MmWaveChannelType::service]->queuePacket(thisMsg);

    if(num == -1){
        statsDroppedPackets ++;
        return;
    }

    if(num == 1 && allowedToSendData) {
        EV_TRACE <<"Received a message from upper layer and allowed to send = true"<<std::endl;
        prepareToSendDataFrame();
    }
}

void veins::MmWaveMac::handleLowerControl(cMessage* msg) {
    if(msg->getKind() == MacToPhyInterface::PHY_RX_START)
        rxStartIndication = true;
    else if (msg->getKind() == MacToPhyInterface::PHY_RX_END_WITH_SUCCESS)
        {
            // PHY_RX_END_WITH_SUCCESS will get packet soon! Nothing to do here
        }
    else if (msg->getKind() == MacToPhyInterface::PHY_RX_END_WITH_FAILURE) {
        // This case happens when received ACK was broken
        phy11ad->notifyMacAboutRxStart(false);
        EV_TRACE <<"turn off notifyMacAboutRxStart because of RX end up with failure!" << std::endl;
        rxStartIndication = false;

        if(currentAccessPeriod == AccessPeriod::CBAP || currentAccessPeriod == AccessPeriod::SP)
            handleRetransmit();
    }
    else if (msg->getKind() == MacToPhyInterface::TX_OVER) {
        EV_TRACE << "Successfully transmitted a packet" << std::endl;

        //TODO do we need to switch the radio state back to RX after finishing
        //transmit a packet? Probably not if we are in SSL
        //TODO probably while sending announce frame too.
        if(!isInSLSPhase){
            EV_TRACE <<"setRadioState to RX if I am not in SLS Phase!"<<std::endl;
            phy->setRadioState(Radio::RX);

            //Adding isServiceChannelActive is to prevent the case of sending the control messages
            if(isServiceChannelActive() && !dynamic_cast<MacAck*>(lastMac.get())) {
                //If we just unicasted a data message and we are expecting an ack from a node, then we should set the receivingAntenna toward this node
                if(lastMac->getDestAddr() != LAddress::L2NULL() && useAcks) {
                    phy11ad->setReceivingSectorID(lastUsedSectorID);
                }

            }
        }

        //TODO check if we can split this logic below into 2 the other files.

        //IEEE 802.11ad 2012, p.249
        //When a transmission by a STA is expected by a PCP/AP and a SIFS period elapses without its receipt, the
        //PCP/AP may either repeat its individually addressed transmission to that STA or, as early as one PIFS after
        //the end of its previous transmission, transmit a frame to any other STA.
        if (!dynamic_cast<MacAck*>(lastMac.get())) {
            if (dynamic_cast<PollFrame * > (lastSentPacket)) {
                phy11ad->notifyMacAboutRxStart(true);
                EV_TRACE <<"turn on notifyMacAboutRxStart because of sending Poll Frame and expecting SPR frame!" << std::endl;
                EV_TRACE << "We have to send more Poll Frame, therefore, we start listening for RX in PIFS!" << std::endl;
            }

            // 9.35.5.2 Operation during the A-BFT IEEE 802.11ad 2012
            // In an A-BFT, the responder shall not initiate SSW ACK (9.35.2.5) in response to the reception of a SSW-
            // Feedback frame from the initiator. The SSW ACK only occurs within the DTI of a beacon interval
            else if (dynamic_cast<SSWFeedbackFrame * > (lastSentPacket) && currentAccessPeriod != AccessPeriod::A_BFT) {
                phy11ad->notifyMacAboutRxStart(true);
                EV_TRACE <<"turn on notifyMacAboutRxStart because of sending SSWFeedbackFrame and expecting SSWACK!" << std::endl;
                EV_TRACE << "We have sent a SSWFeedbackFrame, now we start listening for RX in MIFS for the SSW-ACK frame!" << std::endl;
            }
        }
        if (!dynamic_cast<MacAck*>(lastMac.get())) {
            // if the last sent packet wasn't an ACK packet
            // then a message in a queue has been sent successfully
            // update the queue, go into post-transmit and TODO set backoff + cwCur to cwMin? or is this active channel or service?
            myEDCA[activeChannel]->postTransmit(lastSentPacket, useAcks, currentAccessPeriod);
        }

        // channel just turned idle.
        // don't set the chan to idle. the PHY layer decides, not us.
        // PHY handle TX_OVER selfmsg too, and then check CCA, then tells us (MAC) if channel free or not


        //However, if we are in BHI phase or SP, just sends without checking if channel is free or not
        //TODO in SP, should we do that?
        if(isInSLSPhase){
            EV_TRACE << "TX OVER and I am in SLS phase, I will send the next event, without caring if channel is free or not"<<std::endl;

            if (myEDCA[MmWaveChannelType::control]->myQueue.queue.size() != 0 ){
                BaseFrame1609_4* checkedElement = myEDCA[MmWaveChannelType::control]->myQueue.queue.front();
                simtime_t aIFS = SimTime();
                if (dynamic_cast<FramesContainSSWField*>(checkedElement)) {
                    aIFS = SBIFS_11AD; // TODO check for the case of multiple antenna, then aIFS would be LBIFS instead
                } else
                    throw cRuntimeError("Strange packet at MAC Layer!");

                scheduleAt(simTime() + aIFS, nextMacControlEvent);
                EV_TRACE << "next control Event is at " << nextMacControlEvent->getArrivalTime().raw() << std::endl;
            }
            else
                EV_TRACE << "I don't have any new control events in the MAC queue!" << std::endl;
        }
        else if (myEDCA[MmWaveChannelType::control]->myQueue.queue.size() > 0 && myEDCA[MmWaveChannelType::control]->myQueue.queue.front()->getKind() == packet_kind::ANNOUNCE_FRAME) {
            simtime_t PIFS = SIFS_11AD + SLOTLENGTH_11AD;

            scheduleAt(simTime() + PIFS, nextMacControlEvent);
            EV_TRACE << "next control Event (Announce Frame scheduling) is at " << nextMacControlEvent->getArrivalTime().raw() << std::endl;
        }
    }
    else if (msg->getKind() == Mac11adToPhy11adInterface::CHANNEL_BUSY)
        channelBusy();
    else if (msg->getKind() == Mac11adToPhy11adInterface::CHANNEL_IDLE) {
        // Decider80211ad::processSignalEnd() sends up the received packet to MAC followed by control message CHANNEL_IDLE in the same timestamp.
        // If we received a unicast frame (first event scheduled by Decider), MAC immediately schedules an ACK message and wants to switch the radio to TX mode.
        // So, the notification for channel idle from phy is undesirable and we skip it here.
        // After ACK TX is over, PHY will inform the channel status again.
        if (ignoreChannelState) {
            // Skipping channelidle because we are about to send an ack regardless of the channel state
        }
        else {
            channelIdle();
        }
    }
    else if(msg->getKind() == Decider80211ad::BITERROR || msg->getKind() == Decider80211ad::COLLISION)
        EV_TRACE << "A packet was not received due to biterrors" << std::endl;
    else if(msg->getKind() == Decider80211ad::RECWHILESEND)
        EV_TRACE << "A packet was not received because we were sending while receiving" << std::endl;
    else if (msg->getKind() == MacToPhyInterface::RADIO_SWITCHING_OVER)
        EV_TRACE << "Phylayer said radio switching (between RX/TX) is done" << std::endl;
    else if (msg->getKind() == BaseDecider::PACKET_DROPPED) {
        EV_TRACE << "Phylayer said packet was dropped! setRadioState to RX" << std::endl;
        phy->setRadioState(Radio::RX);

    }
    else {
        EV_WARN << "Invalid control message type (type=NOTHING) : name=" << msg->getName() << " modulesrc=" << msg->getSenderModule()->getFullPath() << "." << std::endl;
        ASSERT(false);
    }

    delete msg;
}

void veins::MmWaveMac::channelIdle() {
    //OKAY
    EV_TRACE << "Channel turned idle" << std::endl;

    if(isInSLSPhase)
        return;

    if(waitUntilAckRXorTimeout)
        return;

    if (nextMacEvent->isScheduled() == true) {
        // this rare case can happen when another node's time has such a big offset that the node sent a packet although we already changed the channel
        // the workaround is not trivial and requires a lot of changes to the phy and decider
        EV_TRACE << "nextMacEvent is scheduled in channelIdle"<< std::endl;
        return;
    }

    idleChannel = true;

    lastIdle = simTime();
    statsTotalBusyTime += simTime() - lastBusy;

    // if we are in CBAP allocation, then we will start contention, otherwise not
    if (allowedToSendData) {
        EV_TRACE <<"channelidle is called and allowed to send = true"<<std::endl;

        prepareToSendDataFrame();
    }
}

void veins::MmWaveMac::channelBusy() {
    //OKAY
    if (!idleChannel) return;

    // the channel turned busy because someone else is sending
    idleChannel = false;
    EV_TRACE << "Channel turned busy: External sender" << std::endl;
    lastBusy = simTime();

    // channel turned busy
    if (nextMacEvent->isScheduled() == true) {
        cancelEvent(nextMacEvent);
    }
}

bool veins::MmWaveMac::handleUnicast(MacPkt* macPkt, DeciderResult80211* res) {
    if (macPkt->getKind() == packet_kind::SSW_FRAME_RSS) {
        communicatingSTA = macPkt->getSrcAddr();

        BestBeamInfo * info = &(beamStates[communicatingSTA]);

        SSWFrameRSS * sswFrame = check_and_cast<SSWFrameRSS * >(macPkt->decapsulate());

        info->myBestAntennaID = sswFrame->getDmgAntennaSelect();
        info->myBestSectorID = sswFrame->getSectorSelect();
        info->myBestSNR = sswFrame->getSnrReport();

        //TODO probably, we need to use the updateBeamFormingDataFromFrame function ....
        if(res->getSnr() > info->theirBestSNR) {
            info->theirBestAntennaID = sswFrame->getAntennaID();
            info->theirBestSectorID = sswFrame->getSectorID();
            info->theirBestSNR = res->getSnr();
        }
        //Well, it is hard? to put this logic in STA file because if so, we don't know the size of sswFrame.
        //it means we are doing SLS in SP allocation, the STA has to schedule the feedback frame, basing on the duration of the sswFrame
        if (currentAccessPeriod != AccessPeriod::A_BFT) {
            if(!sendSSWFeedbackFrame->isScheduled()) {
                //TODO what if the allocation slot duration doens't have enough time for the feedback?
                scheduleAt(simTime() + SimTime(((double) sswFrame->getDuration())/pow(10,6)), sendSSWFeedbackFrame); //MBIFS is already added inside the duration!
            }
        }

        delete sswFrame;
        delete res;

        return true;
    }

    return false;
}

uint32_t veins::MmWaveMac::getHeaderLength(MmWaveMCS mcs) {
    if(isCPHY_MCS(mcs))
        return PHY_80211AD_CPHY_PHR;
    else if(isOFDM_MCS(mcs))
        return PHY_80211AD_OFDMPHY_PHR;
    return PHY_80211AD_SCPHY_PHR;
}

void veins::MmWaveMac::handleAck(const MacAck* ack) {
    ASSERT2(rxStartIndication, "Not expecting ack");
    phy11ad->notifyMacAboutRxStart(false);
    rxStartIndication = false;

    if(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() > 0
             && myEDCA[MmWaveChannelType::service]->myQueue.waitForAck
             && myEDCA[MmWaveChannelType::service]->myQueue.waitOnUnicastID == ack->getMessageId()) {
        BaseFrame1609_4* msg = myEDCA[MmWaveChannelType::service]->myQueue.queue.front();
        myEDCA[MmWaveChannelType::service]->myQueue.queue.pop();
        delete msg;

        myEDCA[MmWaveChannelType::service]->myQueue.rc = 0;
        myEDCA[MmWaveChannelType::service]->myQueue.waitForAck = false;
        myEDCA[MmWaveChannelType::service]->myQueue.waitOnUnicastID = -1;
        if(myEDCA[MmWaveChannelType::service]->myQueue.ackTimeOut->isScheduled())
            cancelEvent(myEDCA[MmWaveChannelType::service]->myQueue.ackTimeOut);

        waitUntilAckRXorTimeout = false;
    }
    else
        ASSERT(false);
}

void veins::MmWaveMac::sendAck(LAddress::L2Type recpAddress,
        unsigned long treeID) {

    ASSERT(useAcks);
    // 802.11-2012 9.3.2.8
    // send an ACK after SIFS without regard of busy/ idle state of channel
    //ignoreChannelState = true;
    channelBusySelf();

    auto* mac = new MacAck("ACK");
    mac->setDestAddr(recpAddress);
    mac->setSrcAddr(myMacAddr);
    mac->setMessageId(treeID);
    mac->setBitLength(ackLength);

    simtime_t sendingDuration = RADIODELAY_11AD + phy11ad->getFrameDuration(mcs, mac->getBitLength(), 0);
    EV_TRACE << "Ack sending duration will be " << sendingDuration << std::endl;

    EV_TRACE << "Sending ACK" << std::endl;

    BestBeamInfo * info = &(beamStates[mac->getDestAddr()]);
    if(info->myBestSectorID >=36 || info->myBestSectorID <= 0) {
        EV_TRACE<< "I don't have any information of my best antenna and sector to the station " << recpAddress << ", probably SSW Feedback frame was lost!"<<std::endl;
        EV_TRACE<< "I just pick a random value of antenna and sector ID and send it!" << std::endl;

        info->myBestAntennaID = 0;
        info->myBestSectorID = RNGCONTEXT intrand(36);

        EV_TRACE<< " antennaID = " << info->myBestAntennaID << ", sectorID = "<< info->myBestSectorID<<std::endl;
    }

    sendFrame(mac, SIFS_11AD, info->myBestAntennaID, info->myBestSectorID, mcs, txPower);
}

Mac11adToPhy11adInterface* veins::MmWaveMac::getPhyModuleOfVehicleByOracle(
        LAddress::L2Type vehicleMAC) {
    ASSERT(useOracle);
    ASSERT(traciManager);

    std::map<std::string, cModule*> availableCars = traciManager->getManagedHosts();
    App11adToMac11adInterface* macMmWave;

    for(auto const& it : availableCars) {
        macMmWave = FindModule<App11adToMac11adInterface*>::findSubModule(it.second);
        ASSERT(macMmWave);

        if(macMmWave->getMACAddress() == vehicleMAC) {
            return FindModule<Mac11adToPhy11adInterface*>::findSubModule(it.second);
        }
    }

    //This vehicle leaves/teleports out of scenario already
    return nullptr;
}

double veins::MmWaveMac::estimateRecevierSensitivityByOracle(LAddress::L2Type vehicleMAC, Mac11adToPhy11adInterface* phyMmWave, bool applyAnalogueModels, bool norm) {
    ASSERT(useOracle);
    ASSERT(traciManager);

    auto it = beamStates.find(vehicleMAC);
    ASSERT(it != beamStates.end());

    int32_t transmittingDirID = phy11ad->getTransmissionDirID(phy11ad->getCurrentPosition(),
                                                              phy11ad->getCurrentOrientation(),
                                                              it->second.myBestSectorID,
                                                              phyMmWave->getCurrentPosition());

    double distance = phy11ad->getCurrentPosition().distance(phyMmWave->getCurrentPosition());

    int32_t bestReceivingSectorID = getReceivingSectorIDForOracle(vehicleMAC, phyMmWave);

    int32_t receivingDirID = phy11ad->getTransmissionDirID(phyMmWave->getCurrentPosition(),
                                                           phyMmWave->getCurrentOrientation(),
                                                           bestReceivingSectorID,
                                                           phy11ad->getCurrentPosition());

    double receivedPowerSignalIndBmNewMethod = phy11ad->getReceivePowerdBm(distance,
                                                                           transmittingDirID,
                                                                           receivingDirID,
                                                                           it->second.myBestSectorID,
                                                                           bestReceivingSectorID, norm);
    if(applyAnalogueModels) {
        double newReceivedPower = phy11ad->getReceivedPowerAfterApplyingAnalogueModels(receivedPowerSignalIndBmNewMethod,
                                                            {phy11ad->getAntennaPosition(), phy11ad->getCurrentOrientation(), nullptr},
                                                            {phyMmWave->getAntennaPosition(), phyMmWave->getCurrentOrientation(), nullptr});

        EV_TRACE << "receivedPowerSignalIndBm " << receivedPowerSignalIndBmNewMethod
                << "\nafter applying analogueModels: " << newReceivedPower << std::endl;

        return newReceivedPower;
    }
    else
        return receivedPowerSignalIndBmNewMethod;
}

int32_t veins::MmWaveMac::getReceivingSectorIDForOracle(
        LAddress::L2Type vehicleMAC, Mac11adToPhy11adInterface* phyMmWave) {
    auto it = beamStates.find(vehicleMAC);
    ASSERT(it != beamStates.end());

    return it->second.theirBestSectorID;
}

bool veins::MmWaveMac::doesServingVehicleLeaveTheBeam(Mac11adToPhy11adInterface* phy11adVehicle, LAddress::L2Type macAddress, double sensitivity) {
    if(sensitivity < exitBeamThreshold)
        return true;
    return false;
}

bool veins::MmWaveMac::serveTheNewVehicle() {
    if(availableVehicles.size() > 0) {
        uint32_t vehicleIndex = RNGCONTEXT intrand(availableVehicles.size());
        evaluationServingSTA = availableVehicles[vehicleIndex];
        pumpEvaluatePacketToQueue();
        availableVehicles.erase(availableVehicles.begin() + vehicleIndex);

        return true;
    }

    return false;
}

void veins::MmWaveMac::whenServingCarLeaveTheBeam(bool teleported) {
    ASSERT2(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() < 2, "test queue size < 2");
    myEDCA[MmWaveChannelType::service]->myQueue.waitForAck = false;
    delete myEDCA[MmWaveChannelType::service]->myQueue.queue.front();
    myEDCA[MmWaveChannelType::service]->myQueue.queue.pop();
    waitUntilAckRXorTimeout = false;

    if(serveTheNewVehicle())
        phy11ad->requestChannelStatusIfIdle();
}

void veins::MmWaveMac::addAggregateDataToCollection(int64_t packetLength) {
    if(!recordInTimeSlot)
        aggregateDataVector.record(packetLength);
    else {
        int64_t timeInUS = simTime().inUnit(SimTimeUnit::SIMTIME_US);
        int64_t currentTime = floor(((double)(timeInUS)) / pow(10, 6) / recordTimeSlot);
        if(currentTime > timeSlotRecorded && aggregateDataPerTimeSlot > 0) {
            aggregateDataVector.recordWithTimestamp(SimTime(currentTime * recordTimeSlot, SimTimeUnit::SIMTIME_S) - SimTime(1, SimTimeUnit::SIMTIME_MS), aggregateDataPerTimeSlot);
            aggregateDataPerTimeSlot = 0;
            timeSlotRecorded = currentTime;
        }

        aggregateDataPerTimeSlot += packetLength;
    }
}

void veins::MmWaveMac::addSelectedMCSToCollection(int64_t mcs) {
    if(!recordInTimeSlot)
        selectedMCSVector.record(mcs);
    else {
        int64_t timeInUS = simTime().inUnit(SimTimeUnit::SIMTIME_US);
        int64_t currentTime = floor(((double)(timeInUS)) / pow(10, 6) / recordTimeSlot);
        if(currentTime > timeSlotRecorded) {
            for(uint32_t i = 0; i < selectedMCSArray.size(); i++)
                selectedMCSVector.recordWithTimestamp(SimTime(currentTime * recordTimeSlot, SimTimeUnit::SIMTIME_S) - SimTime(1, SimTimeUnit::SIMTIME_MS), selectedMCSArray[i]);
            selectedMCSArray = {};
            timeSlotRecorded = currentTime;
        }

        selectedMCSArray[mcs]++;
    }
}

bool veins::MmWaveMac::isEDCAQueueEmpty(MmWaveChannelType channelType) {
    return (myEDCA[channelType]->myQueue.queue.empty());
}

void veins::MmWaveMac::handleBroadcast(MacPkt* macPkt,
        DeciderResult80211* res) {
}

void veins::MmWaveMac::handleLowerMsg(cMessage* msg) {
    MacPkt* macPkt = check_and_cast<MacPkt*> (msg);

    // pass information about received frame to the upper layers
    DeciderResult80211* macRes = check_and_cast<DeciderResult80211*>(PhyToMacControlInfo::getDeciderResult(msg));
    DeciderResult80211* res = new DeciderResult80211(*macRes);

    long dest = macPkt->getDestAddr();

    EV_TRACE << "Received frame name= " << macPkt->getName() << ", myState= src=" << macPkt->getSrcAddr() << " dst=" << macPkt->getDestAddr() << " myAddr=" << myMacAddr << std::endl;

    if (dest == myMacAddr) {
        if (auto* ack = dynamic_cast<MacAck*>(macPkt)) {
            ASSERT(useAcks);
            handleAck(ack);
            delete res;
        }
        else
            handleUnicast(macPkt, res);
    }
    else if (dest == LAddress::L2BROADCAST()){
        handleBroadcast(macPkt, res);
    }
    else {
        EV_TRACE <<"Packet not for me" <<std::endl;
        delete res;
    }
    delete macPkt;

    //TODO check this if we want to deal with retransmit because it might interfere with ATI sending poll frames
    if (rxStartIndication) {
        EV_TRACE <<"rxStartIndication is still up!" << std::endl;
        // We have handled/processed the incoming packet
        // Since we reached here, we were expecting an ack but we didnt get it, so retransmission should take place
        phy11ad->notifyMacAboutRxStart(false);
        EV_TRACE <<"turn off notifyMacAboutRxStart because of not receiving a needed frame yet!" << std::endl;
        rxStartIndication = false;

        if(lastSentPacket->getKind() == packet_kind::SSW_FEEDBACK && currentAccessPeriod != AccessPeriod::A_BFT)
            handleRetransmit();
    }
}

int veins::MmWaveMac::EDCA::queuePacket(BaseFrame1609_4* msg) {
    if (maxQueueSize && myQueue.queue.size() >= maxQueueSize) {
        delete msg;
        return -1;
    }
    myQueue.queue.push(msg);

    EV_TRACE << "mac queue size after add packet: " << myQueue.queue.size()<< endl;
    return myQueue.queue.size();
}

void veins::MmWaveMac::EDCA::createQueue(int cwMin, int cwMax) {
    EDCAQueue newQueue(cwMin, cwMax);
    myQueue = newQueue;
}

BaseFrame1609_4* veins::MmWaveMac::EDCA::initiateTransmit(simtime_t lastIdle) {
    // iterate through the queues to return the packet we want to send
    BaseFrame1609_4* pktToSend = nullptr;

    simtime_t idleTime = simTime() - lastIdle;

    EV_TRACE << "Initiating transmit at " << simTime()
                    << ". I've been idle since " << idleTime << std::endl;

    if (myQueue.queue.size() != 0 && !myQueue.waitForAck) {
        //TODO recheck the condition idleTime >= iter->second.aifsn * SLOTLENGTH_11P + SIFS_11P

        EV_TRACE << "Sending MAC event in queue!" << std::endl;
        pktToSend = myQueue.queue.front();
    }

    if (pktToSend == nullptr) {
        throw cRuntimeError("No Packet was ready");
    }

    return pktToSend;
}

simtime_t veins::MmWaveMac::EDCA::findTimeForTheNextEvent(simtime_t idleSince,
        AllocationType type) {
    simtime_t nextEvent = -1;

    simtime_t idleTime = SimTime().setRaw(std::max((int64_t) 0, (simTime() - idleSince).raw()));

    EV_TRACE << "Channel is already idle for:" << idleTime << " since " << idleSince << std::endl;

    // This function returns the nearest possible event in the queue sub system after a busy channel
    if (myQueue.queue.size() != 0 && !myQueue.waitForAck) {

        BaseFrame1609_4* checkedElement = myQueue.queue.front();

        // If the frame is SSW frame, then we need to sends the frame after SBIFS
        // TODO Does DMG is just used for SLS? and how about the other frames, which is used for SLS?
        // TODO recheck the idle time > nextEvent too

        simtime_t aIFS = SimTime().setRaw(0UL);
        if (dynamic_cast<FramesContainSSWField*>(checkedElement)) {
            ASSERT(false); // Have we ever falled into this case?
            aIFS = SBIFS_11AD; // TODO check for the case of multiple antenna, then aIFS would be LBIFS instead
        } else
        {
//            if(type == AllocationType::CBAP) {
                aIFS = SIFS_11AD + (2 * SLOTLENGTH_11AD); //DIFS https://en.wikipedia.org/wiki/DCF_Interframe_Space
//            }
//            else // SP allocation
//                aIFS = SimTime(0); //TODO 0, order?
        }
        simtime_t possibleNextEvent = aIFS;

        EV_TRACE << "Waiting time for MAC queue is: " << possibleNextEvent << std::endl;

        if(idleTime > possibleNextEvent) {
            EV_TRACE<< "Could have already send if we had it earlier "<< std::endl;
            //we could have already sent, round up to next boundary
            simtime_t base = idleSince + aIFS;

            //The next slot counted from idleSince. idleSince = 0.2, now = 2.6, slot = 1 --> send at 3.2
            possibleNextEvent = simTime() - simtime_t().setRaw((simTime() - base).raw() % SLOTLENGTH_11AD.raw()) + SLOTLENGTH_11AD;
        }
        else {
            // we are gonna send in the future
            EV_TRACE << "Sending in the future " << std::endl;
            possibleNextEvent = idleSince + possibleNextEvent;
        }

        nextEvent = possibleNextEvent;

    }
    return nextEvent;
}

void veins::MmWaveMac::EDCA::backoff() {
    myQueue.currentBackoff = owner->intuniform(0, myQueue.cwCur);
    EV << "Going inot Backoff because channel was busy when new packet arrived from upperlayer" << std::endl;
}

void veins::MmWaveMac::EDCA::postTransmit(BaseFrame1609_4* msg, bool useAcks,
        AccessPeriod currentAccessPeriod) {
    //    bool holBlocking = (msg->getRecipientAddress() != LAddress::L2BROADCAST()) && useAcks;
        bool holBlocking = (currentAccessPeriod != AccessPeriod::A_BFT
                && dynamic_cast<SSWFeedbackFrame *> (msg)) // if we are waiting for SSWFeedbackFrame
                || (msg->getRecipientAddress() != LAddress::L2BROADCAST()
                        && useAcks
                        && (currentAccessPeriod == AccessPeriod::SP || currentAccessPeriod == AccessPeriod::CBAP));
        if (holBlocking) {
            // mac->waitUntilAckRXorTimeout = true; // set in handleselfmsg()
            // Head of line blocking, wait until ack timeout
            myQueue.waitForAck = true;
            myQueue.waitOnUnicastID = msg->getTreeId();
            ((MmWaveMac*) owner)->phy11ad->notifyMacAboutRxStart(true);
            EV_TRACE <<"turn on notifyMacAboutRxStart because of sending SSW Feedbackframe or sending normal frame with ack request!" << std::endl;
        }
        else {
            EV_TRACE << "my queue size before postTransmit: "<< myQueue.queue.size()<< std::endl;
            myQueue.waitForAck = false;
            delete myQueue.queue.front();
            myQueue.queue.pop();

            EV_TRACE << "my queue size after postTransmit: "<< myQueue.queue.size()<< std::endl;
            //myQueues[ac].cwCur = myQueues[ac].cwMin;
            // post transmit backoff
            //myQueues[ac].currentBackoff = owner->intuniform(0, myQueues[ac].cwCur);
            //statsSlotsBackoff += myQueues[ac].currentBackoff;
            //statsNumBackoff++;
            //EV_TRACE << "Queue " << ac << " will go into post-transmit backoff for " << myQueues[ac].currentBackoff << " slots" << std::endl;

            //TODO Do we need backoff after sending a frame?
            EV_TRACE <<"Post transmit!"<< std::endl;
        }
}

veins::MmWaveMac::EDCA::EDCA(cSimpleModule* owner, MmWaveChannelType channelType,
        int maxQueueLength)
        : HasLogProxy(owner)
        , owner(owner)
        , maxQueueSize(maxQueueLength)
        , channelType(channelType)
{
}

veins::MmWaveMac::EDCA::~EDCA() {
    if(myQueue.ackTimeOut) {
        owner->cancelAndDelete(myQueue.ackTimeOut);
        myQueue.ackTimeOut = nullptr;
    }
}

veins::MmWaveMac::EDCA::EDCAQueue::EDCAQueue(int cwMin, int cwMax)
    : cwMin(cwMin)
    , cwMax(cwMax)
    , cwCur(cwMin)
    , currentBackoff(0)
    , ssrc(0)
    , slrc(0)
    , rc(0)
    , waitForAck(false)
    , waitOnUnicastID(-1)
    , ackTimeOut(new AckTimeOutMessage("AckTimeOut"))
{
}

veins::MmWaveMac::EDCA::EDCAQueue::~EDCAQueue() {
    while(!queue.empty()) {
        delete queue.front();
        queue.pop();
    }
    // ackTimeOut needs to be deleted in EDCA
}

simtime_t veins::MmWaveMac::getAckWaitTime(MmWaveMCS mcs) {
    //PHY-RXSTART.indication should be received within ackWaitTime
    //ackWaitTime = aSIFSTime + aSlotTime + aPHY-RX-START-Delay, as defined in 9.3.2.8 of 802.11-2012
    if(isCPHY_MCS(mcs))
        return SIFS_11AD + SLOTLENGTH_11AD + PHY_RX_START_DELAY_CPHY_11AD;
    else if(isOFDM_MCS(mcs))
        return SIFS_11AD + SLOTLENGTH_11AD + PHY_RX_START_DELAY_OFDM_11AD;
    else
        return SIFS_11AD + SLOTLENGTH_11AD + PHY_RX_START_DELAY_SC_11AD;
}


void veins::MmWaveMac::pumpEvaluatePacketToQueue() {
    ASSERT(evaluateAlgorithms);

    ASSERT(evaluationServingSTA != LAddress::L2NULL());

    MmWaveMessage* aMessage = new MmWaveMessage();

    aMessage->setName("Evaluate message");
    aMessage->setByteLength(evaluatePacketByteLength);
    aMessage->setRecipientAddress(evaluationServingSTA);

    EV_TRACE << "Sending evaluate messages to vehicle "<< evaluationServingSTA<< std::endl;

    myEDCA[MmWaveChannelType::service]->queuePacket(aMessage);

    if(myEDCA[MmWaveChannelType::service]->myQueue.queue.size() > 1)
        ASSERT(false);
}


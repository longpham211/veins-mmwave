//https://www.ccs-labs.org/software/veins-vlc/

#include <veins-mmwave/messages/MmWaveMessage_m.h>
#include <veins-mmwave/splitter/Splitter.h>
#include <veins/modules/utility/Consts80211p.h>
#include <veins-mmwave/messages/FmlConnectionRequest_m.h>
#include <veins-mmwave/messages/FmlServiceRequest_m.h>
#include <veins-mmwave/messages/FmlRequestResponse_m.h>
#include <veins-mmwave/messages/FmlMmWaveConfigurationMessage_m.h>
#include <veins-mmwave/messages/FmlServiceFeedback_m.h>

using namespace veins;

Define_Module(veins::Splitter)

Splitter::Splitter() :
        annotationManager(NULL) {
}

Splitter::~Splitter() {
        if (positionAidForMmWave && mobility && sendPositionSelfMessage ) {
            if (sendPositionSelfMessage->isScheduled())
                cancelEvent(sendPositionSelfMessage);
            delete(sendPositionSelfMessage);
            sendPositionSelfMessage = nullptr;
        }

        if (fml){
            findHost()->unsubscribe(BaseMobility::mobilityStateChangedSignal, this);

            if(!isPCPAP()) {
                if(measureRxToRSU) {
                    if(measureRxToRSU->isScheduled())
                        cancelEvent(measureRxToRSU);
                    delete(measureRxToRSU);
                    measureRxToRSU = nullptr;
                }
                if(evaluateBeam) {
                    if(evaluateBeam->isScheduled())
                        cancelEvent(evaluateBeam);
                    delete(evaluateBeam);
                    evaluateBeam = nullptr;
                }
            }
        }
}

void Splitter::initialize(int stage) {
    if (stage == 0) {
        //From upper layers --> lower layers
        fromApplication = findGate("applicationIn");
        toDsrcNic = findGate("nicOut");
        toMmWaveNic = findGate("nicMmWaveOut");

        // From lower layers --> upper layers
        toApplication = findGate("applicationOut");
        fromDsrcNic = findGate("nicIn");
        fromMmwave = findGate("nicMmWaveIn");

        // Module parameters
        collectStatistics = par("collectStatistics").boolValue();
        debug = par("debug").boolValue();
        positionAidForMmWave = par("positionAidForMmWave").boolValue();
        positionAidDuration = SimTime((double) par("positionAidDuration").doubleValue() / 1e3);
        postionAidMessagePacketBits = par("postionAidMessagePacketBits").intValue();
        dsrcHeaderLength = par("dsrcHeaderLength").intValue();

        fmlConnectionRequestBits = par("fmlConnectionRequestBits").intValue();
        fmlServiceRequestBits = par("fmlServiceRequestBits").intValue();
        fmlRequestResponseBits = par("fmlRequestResponseBits").intValue();
        fmlMmWaveConfigurationBits = par("fmlMmWaveConfigurationBits").intValue();
        fmlFeedbackBits = par("fmlFeedbackBits").intValue();

        fml = par("fml").boolValue();

        // Signals
        //mmWaveDelaySignal = registerSignal(name)

        // Other simulation modules
        cModule* tmpMobility = getParentModule()->getSubmodule("veinsmobility");
        mobility = dynamic_cast<TraCIMobility*>(tmpMobility);
        if(!mobility)
            mobility = nullptr; // RSU

//        std::vector<PhyLayerMmWave*> mmWaveArray = getSubmodulesOfType<PhyLayerMmWave>(getParentModule(), true);
//        ASSERT(mmWaveArray.size() == 1);
//        mmWavePhy = mmWaveArray[0];

        phy11ad = FindModule<Mac11adToPhy11adInterface*>::findSubModule(
                getParentModule());

        ASSERT(phy11ad);

        annotationManager = AnnotationManagerAccess().getIfExists();
        ASSERT(annotationManager);

//        overheadSignal = registerSignal("overhead");
        overheadVector.setName("overhead:vector");
        recordInTimeSlot = par("recordInTimeSlot").boolValue();
         if(recordInTimeSlot) {
             recordTimeSlot = par("recordTimeSlot").intValue();
             overheadPerTimeSlot = 0;
             timeSlotRecorded = 0;
         }

        if(fml) {
            findHost()->subscribe(BaseMobility::mobilityStateChangedSignal, this);
            numDoA = par("numDoA").intValue();

            if (!isPCPAP()) {
//                currentRoad = mobility->getRoadId().c_str();
                rsuPosition = Coord(par("rsuX").doubleValue(), par("rsuY").doubleValue(), par("rsuZ").doubleValue());
                rsuCoverRange = par("rsuCoverRange").doubleValue();
                sentRequest = false;
                willBeServing = false;
                amITriggerBeamSelection = false;
                rsuAddress = 0; // Null because of BaseMacLayer.cc but it's fine if we just have 1 RSU
                selectedBeam = -1;

                measureRxToRSU = new cMessage("Measure Rx To RSU");
                evaluateBeam = new cMessage("Evaluate beam");
//                enterBeamThreshold = par("enterBeamThreshold").doubleValue();
//                exitBeamThreshold = par("exitBeamThreshold").doubleValue();
//                measureInterval = SimTime(100, SimTimeUnit::SIMTIME_MS);
                aggregatedData = 0;
                originalDoA = mobility->getCurrentDirection();
                doAIndex = getNormalizedDoAIndex();
                EV_TRACE << "debug" << std::endl;
            }
        }

        if(!isPCPAP()) {
            macDSRC = FindModule<DemoBaseApplLayerToMac1609_4Interface*>::findSubModule(getParentModule());
            ASSERT(macDSRC);
            mac11ad = FindModule<App11adToMac11adInterface*>::findSubModule(getParentModule());
            ASSERT(mac11ad);

            if(positionAidForMmWave)
                rsuAddress = 0; // Null because of BaseMacLayer.cc but it's fine if we just have 1 RSU
        }
    }
    if (stage == 1) {
        // Just STAs send, not RSU
        if(positionAidForMmWave && mobility) {
            sendPositionSelfMessage = new cMessage("Time To Send Position Of Vehicle to support MmWave beamforming!");
            double randNumber = (RNGCONTEXT dblrand());
            SimTime randTime = SimTime(randNumber) / 10;
            scheduleAt(simTime() + randTime, sendPositionSelfMessage);
        }
    }
}

void Splitter::handleMessage(cMessage* msg) {
    if (timerManager.handleMessage(msg))
        return;

    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
//        error("Self-Message arrived!");
//        delete msg;
//        msg = NULL;
    } else {
        int arrivalGate = msg->getArrivalGateId();
        if (arrivalGate == fromApplication) {
            handleUpperMessage(msg);
        } else {
            EV_INFO << "Message from lower layers received!" << std::endl;
            handleLowerMessage(msg);
        }
    }
}

void Splitter::handleUpperMessage(cMessage* msg) {
    //Cast a message to a subclass
    MmWaveMessage* mmWaveMsg = dynamic_cast<MmWaveMessage*>(msg);

    // Handle WSMs if the MmWave has to be "retrofitted" to non-MmWave app
    if (!mmWaveMsg) {
        BaseFrame1609_4* wsm = dynamic_cast<BaseFrame1609_4*>(msg);

        // If not a MmWaveMessage check whether it is a WSM to send directly
        if (wsm != NULL && !strcmp(msg->getClassName(), "WaveShortMessage")) {
            send(wsm, toDsrcNic);
            return;
        } else
            error("Not a MmWaveMessage, not WaveShortMessage");
    }

    // if it is MmWaveMessage
    int networkType = mmWaveMsg->getAccessTechnology();
    if (networkType == DSRC) {
        EV_INFO << "DSRC message received from upper layer!" << std::endl;
        send(mmWaveMsg, toDsrcNic);
    } else if (networkType == MMWAVE) {
        EV_INFO << "MmWave message received from upper layer!" << std::endl;
//            mmWavePacketsSent++;
            send(mmWaveMsg, toMmWaveNic);
    } else
        error("\tThe access technology has not been specified in the message!");
}

void Splitter::handleLowerMessage(cMessage* msg) {
    int lowerGate = msg->getArrivalGateId();

    // If the message is from Mmwave modules, we need to collect statistics
    if (lowerGate == fromMmwave ) {

        // Receive Request Reponse, prepare to send mmWave configuration message to the requested node
        if(fml && dynamic_cast<FmlRequestResponse*>(msg)) {
            FmlRequestResponse* response = dynamic_cast<FmlRequestResponse*>(msg);
            FmlMmWaveConfigurationMessage* configurationMessage = new FmlMmWaveConfigurationMessage();

            configurationMessage->setSelectedBeam(response->getSelectedBeam());
            configurationMessage->setVehicleTriggeringBeamSelection(response->getVehicleTriggeringBeamSelection());

//            configurationMessage->setRecipientAddress(LAddress::L2BROADCAST());
            configurationMessage->setRecipientAddress(response->getServingVehicleDSRCAddress());
            configurationMessage->setChannelNumber(static_cast<int>(Channel::cch));
            configurationMessage->setBitLength(dsrcHeaderLength);
            configurationMessage->addBitLength(fmlMmWaveConfigurationBits);

            send(configurationMessage, toDsrcNic);
            addOverheadToCollection(configurationMessage->getBitLength() + dsrcHeaderLength + fmlRequestResponseBits);
            delete response;
        }
        else
            send(msg, toApplication);
    }

    else if (lowerGate == fromDsrcNic) {
        if(positionAidForMmWave && dynamic_cast<PositionAidMessage*>(msg)) {
            PositionAidMessage* posMsg = dynamic_cast<PositionAidMessage*>(msg);

            EV_TRACE<< "Receive position aid message from "<< posMsg->getSender11adAddress() <<" 11ad address, send to RSU!" << std::endl;

            if(isPCPAP())
                send(posMsg, toMmWaveNic);
            else
                delete posMsg; // TODO currently, SAMBA doesn't support for STA to STA communication
        }
        else if (fml) {
            if(FmlConnectionRequest* requestMsg = dynamic_cast<FmlConnectionRequest*>(msg)){
                ASSERT (isPCPAP());

                EV_TRACE<< "Receive connection request message from "<< requestMsg->getSenderDSRCAddress()<< ", send service request to mmBS!" << std::endl ;
                FmlServiceRequest* msg = new FmlServiceRequest();
                msg->setNormalizedDoA(requestMsg->getNormalizedDoA());
                msg->setSenderDSRCAddress(requestMsg->getSenderDSRCAddress());
                msg->setSender11adAddress(requestMsg->getSender11adAddress());
                msg->setBitLength(dsrcHeaderLength);
                msg->addBitLength(fmlServiceRequestBits);

                addOverheadToCollection(msg->getBitLength());

                send(msg, toMmWaveNic);

                delete requestMsg;
            }
            else if (FmlMmWaveConfigurationMessage* configuration = dynamic_cast<FmlMmWaveConfigurationMessage*>(msg)) {
                ASSERT2(!isPCPAP(), "I am RSU, but I have received MmWave configuration message?");

                EV_TRACE << "Receive mmWave configuration message with selected beam is " << configuration->getSelectedBeam() << std::endl;
                willBeServing = true;
                selectedBeam = configuration->getSelectedBeam();
                if (configuration->getVehicleTriggeringBeamSelection() == mac11ad->getMACAddress())
                    amITriggerBeamSelection = true;

                delete configuration;
            }

            else if(FmlServiceFeedback* feedback = dynamic_cast<FmlServiceFeedback*>(msg)) {
                ASSERT(isPCPAP());

                //Send it to mmBS
                send(feedback, toMmWaveNic);
            }

            else {
                EV_WARN << "Invalid control message type (type=NOTHING) : name=" << msg->getName() << " modulesrc=" << msg->getSenderModule()->getFullPath() << "." << std::endl;
                ASSERT(false);
            }
        }
        else
            send(msg, toApplication);
    }
}

void Splitter::finish() {
    if(recordInTimeSlot && overheadPerTimeSlot > 0) {
        simtime_t recordTime = simTime();

        if((int)(ceil(simTime().dbl())) % recordTimeSlot == 0)
            recordTime = recordTime - SimTime(1, SimTimeUnit::SIMTIME_MS);

        overheadVector.recordWithTimestamp(recordTime, overheadPerTimeSlot);
    }
}

void veins::Splitter::handleSelfMessage(cMessage* msg) {
    if(msg == sendPositionSelfMessage) {
        ASSERT (mobility); // only mobile STAs can send position, not stationary RSU
        EV_TRACE << "Test sending vehicle position!"<< std::endl;


        EV_TRACE << "Current position = " << mobility->getPositionAt(simTime()) << std::endl;
        EV_TRACE << "Current speed = " << mobility->getSpeed() << std::endl;
        EV_TRACE << "Current direction = " << mobility->getCurrentDirection()<< std::endl;
        EV_TRACE << "Current orientation = " << mobility->getCurrentOrientation()<< std::endl;
        EV_TRACE << "Current heading = " << mobility->getHeading()<< std::endl;

        PositionAidMessage* msg = generatePositionaAidForMmWaveMessage();
        send(msg, toDsrcNic);

        addOverheadToCollection(msg->getBitLength());

        double randNumber = (RNGCONTEXT dblrand());
        SimTime randTime = SimTime(randNumber) / 10;
        scheduleAt(simTime() + randTime + positionAidDuration, sendPositionSelfMessage);
    }

    else
        error ("Strange message in Splitter!");
}

PositionAidMessage* veins::Splitter::generatePositionaAidForMmWaveMessage() {
    PositionAidMessage* msg = new PositionAidMessage();

    msg->setSenderDirection(mobility->getCurrentDirection());
    msg->setSenderPosition(mobility->getPositionAt(simTime()));
    msg->setSenderSpeed(mobility->getSpeed());
    msg->setSenderOrientation(mobility->getCurrentOrientation());
    msg->setSenderHeading(mobility->getHeading());
    msg->setRecipientAddress(rsuAddress); // Unicast a message (if vehicles start at a same time, probably messages are collided.)

    msg->setSentAt(simTime());
//    msg->setSenderDSRCAddress(macDSRC->getMACAddress());
    msg->setSender11adAddress(mac11ad->getMACAddress());

    msg->setBitLength(postionAidMessagePacketBits);
    msg->addBitLength(dsrcHeaderLength); // add here, because dsrcmac doesn't add.

    msg->setChannelNumber(static_cast<int>(Channel::cch));

    return msg;
}

void veins::Splitter::receiveSignal(cComponent* source, simsignal_t signalID,
        cObject* obj, cObject* details) {
    Enter_Method_Silent();
    if(!isPCPAP())
        if (signalID == BaseMobility::mobilityStateChangedSignal) {
            handlePositionUpdateSTA(obj);
        }
}

void veins::Splitter::handlePositionUpdateSTA(cObject* obj) {
    ChannelMobilityPtrType const mobility = check_and_cast<ChannelMobilityPtrType>(obj);
    curPosition = mobility->getPositionAt(simTime());
    curDoA = mobility->getCurrentDirection();

    if (!sentRequest && getDistanceToRSU() < rsuCoverRange) {
        requestConnection();
        sentRequest = true;
    }

    if(sentRequest && getDistanceToRSU() > rsuCoverRange) {
        sentRequest = false;

        //Lazy to create a new type of message, use this FmlServiceFeedback to tell the vehicle leaves dsrc coverage
        FmlServiceFeedback* feedback = new FmlServiceFeedback();
        feedback->setSenderAddress(mac11ad->getMACAddress());
        feedback->setRecipientAddress(rsuAddress);
        feedback->setChannelNumber(static_cast<int>(Channel::cch));

        feedback->setAggregateData(0);
        feedback->setSelectedBeam(selectedBeam);
        feedback->setDoAIndex(doAIndex);

        send(feedback, toDsrcNic);
    }
}

void veins::Splitter::requestConnection() {
    doAIndex = getNormalizedDoAIndex();
    FmlConnectionRequest* request = new FmlConnectionRequest();
    request->setNormalizedDoA(doAIndex);
    request->setSenderDSRCAddress(macDSRC->getMACAddress());
    request->setSender11adAddress(mac11ad->getMACAddress());

    request->setChannelNumber(static_cast<int>(Channel::cch));
    request->setRecipientAddress(rsuAddress); // Probably we don't need this one, just broadcast it...
    request->setBitLength(dsrcHeaderLength);
    request->addBitLength(fmlConnectionRequestBits);

    addOverheadToCollection(request->getBitLength());
    send(request, toDsrcNic);
}

void veins::Splitter::addOverheadToCollection(int64_t overheadBits) {
    if(!recordInTimeSlot)
        overheadVector.record(overheadBits);
    else {
        int64_t timeInUS = simTime().inUnit(SimTimeUnit::SIMTIME_US);
        int64_t currentTime = floor(((double)(timeInUS)) / pow(10, 6) / recordTimeSlot);
        if(currentTime > timeSlotRecorded && overheadPerTimeSlot > 0) {
            overheadVector.recordWithTimestamp(SimTime(currentTime * recordTimeSlot, SimTimeUnit::SIMTIME_S) - SimTime(1, SimTimeUnit::SIMTIME_MS), overheadPerTimeSlot);
            overheadPerTimeSlot = 0;
            timeSlotRecorded = currentTime;
        }

        overheadPerTimeSlot += overheadBits;
    }
}

bool veins::Splitter::isPCPAP() {
    return (mobility == nullptr);
}

double veins::Splitter::getDistanceToRSU() {
    return rsuPosition.distance(mobility->getPositionAt(simTime()));
}

uint32_t veins::Splitter::getNormalizedDoAIndex() {
    double min = 2;
    uint32_t minIndex, i;

    for(minIndex = 0, i = 0; i < normalizedDoA.size(); i++) {
//        double distance = mobility->getCurrentDirection().distance(normalizedDoA[i]);
        double distance = originalDoA.distance(normalizedDoA[i]); // Trick because of the way we choose routes in Urban
        if(distance < min) {
            min = distance;
            minIndex= i;
        }
    }

    return minIndex;
}

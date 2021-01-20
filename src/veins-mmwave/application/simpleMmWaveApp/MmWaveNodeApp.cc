//https://www.ccs-labs.org/software/veins-vlc/

#include <veins-mmwave/application/simpleMmWaveApp/MmWaveNodeApp.h>

#include "veins/base/utils/FindModule.h"

using namespace veins;

Define_Module(veins::MmWaveNodeApp)

MmWaveNodeApp::MmWaveNodeApp() :
        traciManager(NULL), mobility(NULL), annotations(NULL) {
}

MmWaveNodeApp::~MmWaveNodeApp() {
}

void MmWaveNodeApp::initialize(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage == 0) {
        EV_TRACE << "Long trace: MmWaveNodeApp is in stage 0" << endl;
        toLower = findGate("lowerLayerOut");
        fromLower = findGate("lowerLayerIn");

        traciManager = TraCIScenarioManagerAccess().get();
        ASSERT(traciManager);

        cModule* tmpMobility = getParentModule()->getSubmodule("veinsmobility");
        mobility = dynamic_cast<TraCIMobility*>(tmpMobility);
//        ASSERT(mobility);
//        mobility = dynamic_cast<CarMmWaveMobility*>(tmpMobility);

//        mac11ad = FindModule<App11adToMac11adInterface*>::findSubModule(getParentModule());
//        ASSERT(mac11ad);

        ASSERT(mobility);

        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);

        debug = par("debug").boolValue();
        byteLength = par("packetByteLength");
        transmissionPeriod = 1 / par("beaconingFrequency").doubleValue();

    } else if (stage == 3) {
        auto dsrc = [this]() {
            MmWaveMessage* mmWaveMsg = new MmWaveMessage();
            mmWaveMsg->setAccessTechnology(DSRC);
            send(mmWaveMsg, toLower);
        };

        auto mmWave =
                [this] () {
                    EV_INFO<< "Sending MmWave message!" << std::endl;
                    MmWaveMessage* mmWaveMsg = generateMmWaveMessage(MMWAVE);
                    EV_TRACE << mobility->getExternalId() << std::endl;
                    if (mobility->getExternalId().compare("veh0") == 0)
                        mmWaveMsg->setRecipientAddress(32); // to veh1 to trigger the beamforming because veh0 has assigned SP allocation
                    else
                        mmWaveMsg->setRecipientAddress(12); // to PCP/AP
                    send(mmWaveMsg, toLower);
                };

//        timerManager.create(
//                veins::TimerSpecification(mmWave).oneshotAt(
//                        SimTime(5, SIMTIME_S)));

//        timerManager.create(
//                veins::TimerSpecification(mmWave)
//        .absoluteStart(SimTime(3, SIMTIME_S))
//        .interval(SimTime(7, SIMTIME_S)).repetitions(10));


//        auto position = [this]() {
//            EV_TRACE << "Test sending vehicle position!"<< std::endl;
//
//
//            EV_TRACE << "Current position = " << mobility->getPositionAt(simTime()) << std::endl;
//
//            EV_TRACE << "Current speed = " << mobility->getSpeed() << std::endl;
//            EV_TRACE << "Current direction = " << mobility->getCurrentDirection()<< std::endl;
//            EV_TRACE << "Current orientation = " << mobility->getCurrentOrientation()<< std::endl;
//            EV_TRACE << "Current heading = " << mobility->getHeading()<< std::endl;
//        };
//
//        timerManager.create(
//                veins::TimerSpecification(position)
//        .absoluteStart(SimTime(1, SIMTIME_S))
//        .interval(SimTime(300, SIMTIME_MS)).repetitions(100));


    }
}

MmWaveMessage* MmWaveNodeApp::generateMmWaveMessage(int accessTechnology) {
    MmWaveMessage* mmWaveMsg = new MmWaveMessage();

    //OMNeT-specific
    mmWaveMsg->setName("mmWaveMessage");

    // HeterogeneousMessage specific
    mmWaveMsg->setSourceNode(this->sumoId.c_str());
    mmWaveMsg->setDestinationNode("BROADCAST");
    mmWaveMsg->setAccessTechnology(accessTechnology);
    mmWaveMsg->setSentAt(simTime());

    // Set application layer packet length
    mmWaveMsg->setByteLength(byteLength);

    return mmWaveMsg;
}

void MmWaveNodeApp::handleMessage(cMessage* msg) {
    // To handle the timer
    if (timerManager.handleMessage(msg))
        return;

    if (msg->isSelfMessage()) {
        throw cRuntimeError("This module does not use custom self message");
    } else {
        MmWaveMessage* mmWaveMsg = check_and_cast<MmWaveMessage*>(msg);
        int accessTech = mmWaveMsg->getAccessTechnology();
        switch (accessTech) {
        case DSRC: {
            EV_INFO << "DSRC message received!" << std::endl;
            delete mmWaveMsg;
            break;
        }
        case MMWAVE: {
            EV_INFO << "MmWave message received from: "
                           << mmWaveMsg->getSourceNode() << std::endl;
            delete mmWaveMsg;
            break;
        }
        default:
            error("message neither from DSRC nor MmWave");
            break;
        }
    }
}

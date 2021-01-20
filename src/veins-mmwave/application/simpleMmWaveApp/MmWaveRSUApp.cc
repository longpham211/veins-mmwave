//https://www.ccs-labs.org/software/veins-vlc/

#include <veins-mmwave/application/simpleMmWaveApp/MmWaveRSUApp.h>

#include "veins/base/utils/FindModule.h"

using namespace veins;

Define_Module(veins::MmWaveRSUApp)

MmWaveRSUApp::MmWaveRSUApp() :
        traciManager(NULL), mobility(NULL), annotations(NULL) {
}

MmWaveRSUApp::~MmWaveRSUApp() {
}

void MmWaveRSUApp::initialize(int stage) {
    BaseApplLayer::initialize(stage);

    if (stage == 0) {
        EV_INFO << "Long trace: SimpleMmWaveApp is in stage 0" << endl;
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

//        if (mobility == NULL) // then it is a RSU
//            mac11ad->functionAsPCP_AP(true);

//        mobility = nullptr; // RSU

        annotations = AnnotationManagerAccess().getIfExists();
        ASSERT(annotations);



        debug = par("debug").boolValue();
        byteLength = par("packetByteLength");
        transmissionPeriod = 1 / par("beaconingFrequency").doubleValue();

//        sumoId = mobility->getExternalId();
        //let veh0 acts as an PCP/AP
//        if(sumoId.compare("veh0") == 0) {
//            EV_INFO<< "set veh0 as PCP_AP"<<std::endl;
//            mac11ad->functionAsPCP_AP(true);

//        }

    } else if (stage == 2) {
        auto dsrc = [this]() {
            MmWaveMessage* mmWaveMsg = new MmWaveMessage();
            mmWaveMsg->setAccessTechnology(DSRC);
            send(mmWaveMsg, toLower);
        };

        // We simplifies the network by just letting the vehicle 0 sends a mm-wave message.
//        if(sumoId.compare("veh0") == 0) {
//            auto mmWave =
//                    [this] () {
//                        EV_INFO<< "Sending MmWave message!" << std::endl;
//                        MmWaveMessage* mmWaveMsg = generateMmWaveMessage(MMWAVE);
//                        send(mmWaveMsg, toLower);
//                    };
//
//            timerManager.create(
//                    veins::TimerSpecification(mmWave).oneshotAt(
//                            SimTime(20, SIMTIME_S)));
//        }
    }
}

MmWaveMessage* MmWaveRSUApp::generateMmWaveMessage(int accessTechnology) {
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

void MmWaveRSUApp::handleMessage(cMessage* msg) {
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

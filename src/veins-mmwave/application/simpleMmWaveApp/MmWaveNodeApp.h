//https://www.ccs-labs.org/software/veins-vlc/

#pragma once

#include <omnetpp.h>
#include <veins-mmwave/messages/MmWaveMessage_m.h>

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/utility/TimerManager.h"
#include "veins-mmwave/application/App11adToMac11adInterface.h"

namespace veins {

class MmWaveNodeApp: public BaseApplLayer {
protected:
    bool debug;

    int toLower;
    int fromLower;
    int byteLength;
    double transmissionPeriod;

    std::string sumoId;
    veins::TimerManager timerManager { this };
    mutable TraCIScenarioManager* traciManager;
    TraCIMobility* mobility;
    AnnotationManager* annotations;
    TraCICommandInterface* traci;

    App11adToMac11adInterface* mac11ad;

public:
    MmWaveNodeApp();
    ~MmWaveNodeApp();
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage* msg);
    virtual int numInitStages() const {
        return 4;
    }

protected:
    MmWaveMessage* generateMmWaveMessage(int accessTechnology);

};
}

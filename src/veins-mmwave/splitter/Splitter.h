//https://www.ccs-labs.org/software/veins-vlc/

#pragma once

#include <omnetpp.h>
#include <veins-mmwave/phy/PhyLayerMmWave.h>

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/utility/TimerManager.h"
#include "veins/modules/world/annotations/AnnotationManager.h"
#include "veins-mmwave/messages/PositionAidMessage_m.h"
#include "veins/base/modules/BaseModule.h"
#include "veins/modules/mac/ieee80211p/DemoBaseApplLayerToMac1609_4Interface.h"
#include "veins-mmwave/mac/Mac11adToPhy11adInterface.h"
#include "veins-mmwave/application/App11adToMac11adInterface.h"

using veins::AnnotationManager;
using veins::AnnotationManagerAccess;
using veins::TraCIMobility;

namespace veins {

class Splitter: public BaseModule {
public:
    Splitter();
    ~Splitter();

    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

protected:

    //Gates
    int toApplication, fromApplication;
    int toDsrcNic, fromDsrcNic;
    int toMmWaveNic, fromMmwave;

    //Variables
    bool debug;
    bool collectStatistics;
    bool positionAidForMmWave;
    simtime_t positionAidDuration;
    int postionAidMessagePacketBits;
    int dsrcHeaderLength;
    int fmlConnectionRequestBits;
    int fmlServiceRequestBits;
    int fmlRequestResponseBits;
    int fmlMmWaveConfigurationBits;
    int fmlFeedbackBits;


    bool fml;
    Coord rsuPosition;
    Coord curPosition;
    Coord curDoA;
    Coord originalDoA;
    double rsuCoverRange;
    bool sentRequest;
    int numDoA;
    std::array<Coord, 8> normalizedDoA = {Coord(1, 0),
                                          Coord(0.5, 0.5),
                                          Coord(0, 1),
                                          Coord(-0.5, 0.5),
                                          Coord(-1, 0),
                                          Coord(-0.5, -0.5),
                                          Coord(0, -1),
                                          Coord(0.5, -0.5)}; // 8 directions
    LAddress::L2Type rsuAddress;
    DemoBaseApplLayerToMac1609_4Interface* macDSRC;
    App11adToMac11adInterface* mac11ad;

    bool amITriggerBeamSelection; // If so, this vehicle needs to send a special message to the base station to select another vehicle
    bool willBeServing;
    int32_t selectedBeam;
    uint32_t doAIndex;
    //double enterBeamThreshold;
//    double exitBeamThreshold;
    simtime_t startUsingBeamTime;
    double aggregatedData;
    simtime_t measureInterval;


    cMessage* measureRxToRSU;
    cMessage* evaluateBeam;



    TraCIMobility* mobility;
    AnnotationManager* annotationManager;
    veins::TimerManager timerManager { this };
//    std::vector<PhyLayerMmWave*> mmWavePhy;
    Mac11adToPhy11adInterface* phy11ad;

    cMessage* sendPositionSelfMessage;

    //Statistics
//    int mmWavePacketsSent = 0;
//    int mmWavePacketsReceived = 0;

    // Signals
    simsignal_t mmWaveDelaySignal;

//    simsignal_t overheadSignal;
    cOutVector overheadVector;
    bool recordInTimeSlot;
    int recordTimeSlot;
    double overheadPerTimeSlot;
    int64_t timeSlotRecorded;

    virtual void initialize(int);
    virtual int numInitStages() const {
        return 4;
    }
    virtual void handleMessage(cMessage* msg);
    virtual void finish() override;
    void handleUpperMessage(cMessage* msg);
    void handleLowerMessage(cMessage* msg);
    void handleSelfMessage (cMessage* msg);

    /** @brief this function is called every time the vehicle receives a position update signal */
    virtual void handlePositionUpdateSTA(cObject* obj);

    void requestConnection();

    void addOverheadToCollection(int64_t overheadBits);

private:
    PositionAidMessage* generatePositionaAidForMmWaveMessage();
    double getDistanceToRSU();
    uint32_t getNormalizedDoAIndex();
    bool isPCPAP();
};
}

#pragma once

#include "veins-mmwave/mac/MmWaveAPMac.h"
#include "veins-mmwave/messages/FmlServiceRequest_m.h"
#include "veins-mmwave/messages/FmlServiceFeedback_m.h"
#include "veins-mmwave/messages/FmlRequestResponse_m.h"

#include "veins-mmwave/application/App11adToMac11adInterface.h"

namespace veins {
class FmlAPMac : public MmWaveAPMac {
public:
    FmlAPMac()
    {
    }
    ~FmlAPMac() override;

protected:
    bool isBeamSelected;
    int32_t beamIsGoingToBeSelected;

    LAddress::L2Type servingSTA11p;
    LAddress::L2Type servingSTA11ad;
    LAddress::L2Type vehicleTriggeringBeamSelection;

    Mac11adToPhy11adInterface* vehicleTriggeringBeamSelectionPhy80211ad;

    double alpha; //Hoelder coefficient
    uint32_t context_no; //no. of context dimensions
    double z; //parameter z for control function
    uint32_t m; //number of selected beams. Currently, only 1 beam is selected
    int32_t selectedBeam; // chosen beam for a vehicle
    double dataSent;

    bool considerObstaclesInVehiclesLeavingBeam;
    bool updatedTheTriggeringVehicle;
    // 8 direction, 36 beams. TODO better way  than hard-coded?
    std::array<std::array<double, 8>, 36> sumRewardPerBeam; //Sum of rewards for each action
    std::array<std::array<double, 8>, 36> countSelectionPerBeam; // Selected number of each action in each set
    std::array<std::array<double, 8>, 36> meanRewardPerBeam; //Sample mean of reward for each action in each set
    std::map<LAddress::L2Type, uint32_t> doAMap; // A map, contains the direction of Arrival for each (L2Address) of vehicle

    long periodCount;

    simtime_t estimateDuration;

    simtime_t serveTill;
    double rsuCoverRange;

    cMessage* selectingANewBeam;

    cMessage* estimateRX;

    int factorControlFunction;
    double factorControlFunctionValue;
    int totalNo;

    simsignal_t periodSignal;
//    simsignal_t explorationGroupSizeSignal;

protected:
    void initialize(int) override;

    /** @brief Handle messages from upper layer.*/
    void handleUpperMsg(cMessage*) override;

    /** @brief Handle self messages such as timers.*/
    void handleSelfMsg(cMessage*) override;

    void handleServiceRequest(FmlServiceRequest*);

    void initializeFML();

    uint32_t getSelectedBeamByFMLAlgorithm(uint32_t doAIndex);

    //Function needed in learning with context to decide whether to explore or exploit
    //z Parameter
    //t time period variable
    double getControlFunction(uint32_t z, uint32_t t);

    double getFactorControlFunction();

    void updateFMLStep(FmlServiceFeedback* feedback);

    void updateFMLStep();

    bool serveTheNewVehicle() override;

    virtual int32_t getReceivingSectorIDForOracle(LAddress::L2Type vehicleMAC, Mac11adToPhy11adInterface* phyMmWave) override;

    void performBTI() override;

    virtual bool doesServingVehicleLeaveTheBeam(Mac11adToPhy11adInterface* phy11adVehicle, LAddress::L2Type vehicleMAC , double sensitivity) override;

    virtual void whenServingCarLeaveTheBeam(bool teleported) override;

    virtual void handleAck(const MacAck* ack) override;

    virtual void pumpEvaluatePacketToQueue() override;
};
}

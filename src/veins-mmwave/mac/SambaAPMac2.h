#pragma once

#include "veins-mmwave/mac/MmWaveAPMac.h"
#include "veins-mmwave/messages/PositionAidMessage_m.h"

namespace veins {

class SambaAPMac2 : public MmWaveAPMac {
public:
    struct PositionInfo {
        Coord position;
        double speed;
        Coord direction;
        Coord orientation;
        Heading heading;
        simtime_t at;
        Coord predictedPosition;
        double distance;
        bool inOnce;

        PositionInfo()
            : position()
            , speed(0)
            , direction()
            , orientation()
            , heading()
            , at()
            , predictedPosition()
            , distance(0)
            , inOnce (false)
            {}
        PositionInfo(Coord oldPosition, double speed, Coord direction,
                Coord orientation, Heading heading, simtime_t at)
        : position(oldPosition)
        , speed(speed)
        , direction(direction)
        , orientation(orientation)
        , heading(heading)
        , at(at)
        {}
    };

    std::map<LAddress::L2Type, PositionInfo> vehiclesPosition;

    double rsuCoverRange;

public:
    SambaAPMac2()
    {
    }
    ~SambaAPMac2() override;

protected:
    void initialize(int) override;

    void performBTI() override;

    void updatePositionMap();

    void predictPosition(LAddress::L2Type, PositionInfo& info);

    /** @brief Handle messages from upper layer.*/
    void handleUpperMsg(cMessage*) override;

    /** @brief Handle control messages from lower layer.*/
    void handleLowerControl(cMessage* msg) override;

    virtual void addAnnounceFramesToQueue(simtime_t delay) override;

    virtual int32_t getReceivingSectorIDForOracle(LAddress::L2Type vehicleMAC, Mac11adToPhy11adInterface* phyMmWave) override;

    virtual bool doesServingVehicleLeaveTheBeam(Mac11adToPhy11adInterface* phy11adVehicle, LAddress::L2Type vehicleMAC, double sensitivity) override;

//    virtual double estimateRecevierSensitivityByOracle(LAddress::L2Type vehicleMAC) override;
};
} // end of namespace veins

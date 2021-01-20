/*
 * FmlMac.h
 *
 *  Created on: Jul 1, 2020
 *      Author: longpham211
 */

#pragma once

#include <queue>

#include "veins/base/modules/BaseMacLayer.h"
#include "veins-mmwave/phy/PhyLayerMmWave.h"
#include <veins-mmwave/messages/FmlServiceRequest_m.h>
#include <veins-mmwave/messages/FmlRequestResponse_m.h>

#include <vector>

namespace veins {
class FmlMac : public BaseMacLayer {
public:
    enum packet_kind {
        SERVICE_REQUEST
    };

    FmlMac() {}

    ~FmlMac() override;

protected:
    Mac11adToPhy11adInterface* phy11ad;
    MmWaveMCS mcs; ///< Modulation and coding scheme to use unless explicitly specified.
    double txPower;
    uint32_t numberAntenna = 0;
    uint32_t totalSectors = 0;


protected:
    void initialize(int) override;

    /** @brief Delete all dynamically allocated objects of the module.*/
    void finish() override;

    /** @brief Handle messages from lower layer.*/
    //void handleLowerMsg(cMessage*) override;

    /** @brief Handle control messages from lower layer.*/
    //void handleLowerControl(cMessage* msg) override;
};
}

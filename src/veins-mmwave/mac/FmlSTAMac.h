/*
 * FmlSTAMac.h
 *
 *  Created on: Jul 2, 2020
 *      Author: longpham211
 */

#pragma once

#include "veins-mmwave/mac/MmWaveSTAMac.h"

namespace veins {
class FmlSTAMac : public MmWaveSTAMac {
public:
    FmlSTAMac() {}
    ~FmlSTAMac() override;

    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;

protected:
    Coord rsuPosition;

    int32_t bestBeamTowardRSU;

protected:
    void initialize(int) override;

    /** @brief this function is called every time the vehicle receives a position update signal */
    virtual void handlePositionUpdate(cObject* obj);

    bool handleUnicast(MacPkt* macPkt, DeciderResult80211* res) override;

    void finish() override;
};
}

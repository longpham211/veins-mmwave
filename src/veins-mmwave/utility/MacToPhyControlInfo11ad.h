/*
 * MacToPhyControlInfo11ad.h
 *
 *  Created on: Dec 29, 2019
 *      Author: longpham211
 */

#pragma once

#include "veins-mmwave/utility/ConstsMmWave.h"

namespace veins {

/**
 * Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */
struct MacToPhyControlInfo11ad : public cObject {
    uint32_t antennaID;
    uint32_t sectorID;
    MmWaveMCS mcs; ///< The modulation and coding scheme to employ for the associated frame.
    double txPower_mW; ///< Transmission power in milliwatts.

    MacToPhyControlInfo11ad(uint32_t antennaID, uint32_t sectorID, MmWaveMCS mcs, double txPower_mW)
        : antennaID(antennaID)
        , sectorID(sectorID)
        , mcs(mcs)
        , txPower_mW(txPower_mW)
    {
    }
};

}

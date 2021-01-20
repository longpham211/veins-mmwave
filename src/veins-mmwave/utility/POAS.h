/*
 * POAS.h
 *
 *  Created on: Jan 4, 2020
 *      Author: longpham211
 */

#pragma once

#include "veins/base/utils/POA.h"

namespace veins {


/**
 * @brief: Container class used to attach data to AirFrame80211ad s which are
 * needed by the receiver for BeamformingPatternModel model (POAH is short
 * for position, orientation, antenna, sector)
 *
 * We add one more field, namely sectorID to store the the
 * specific sector ID that the sender uses to send this frame / or the
 * specific sector ID that the receiver uses to receive this frame
 *
 */
class POAS : public POA {
public:
    uint32_t sectorID; //store the sectorID of the antenna used by the STA

    POAS(){};

    POAS(AntennaPosition pos, Coord orientation, std::shared_ptr<Antenna> antenna, uint32_t sectorID)
        : POA(pos, orientation, antenna)
        , sectorID(sectorID)
    {};

};
}

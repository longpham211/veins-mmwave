/*
 * Decider80211adToPhy80211adInterface.h
 *
 *  Created on: Jan 2, 2020
 *      Author: longpham211
 */

#pragma once

namespace veins {

/**
 * @brief
 *
 * Interface of PhyLayer80211ad exposed to Decider80211ad
 *
 */

class Decider80211adToPhy80211adInterface {
public:
    virtual ~Decider80211adToPhy80211adInterface() {};
    virtual int getRadioState() = 0;

};
} // namespace veins

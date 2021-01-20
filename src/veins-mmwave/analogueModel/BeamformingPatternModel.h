/*
 * BeamformingPatternModel.h
 *
 *  Created on: Jan 4, 2020
 *      Author: longpham211
 */

#pragma once

#include <cstdlib>

#include "veins/veins.h"

#include "veins/base/phyLayer/AnalogueModel.h"

namespace veins {

using veins::AirFrame;

class BeamformingPatternModel;

class BeamformingPatternModel : public AnalogueModel {

public:

    BeamformingPatternModel(cComponent* owner)
        : AnalogueModel(owner)
    {
    }

    /**
     * @brief Filters a specified AirFrame's Signal by adding an attenuation
     * over time to the Signal.
     */
    void filterSignal(Signal*) override;

    bool neverIncreasesPower() override
    {
        return true;
    }

};

} // namespace veins

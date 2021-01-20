/*
 * MmWaveConnectionManager.h
 *
 *  Created on: May 28, 2020
 *      Author: longpham211
 */

#pragma once

#include "veins/veins.h"

#include "veins/base/connectionManager/BaseConnectionManager.h"


namespace veins {

class MmWaveConnectionManager : public BaseConnectionManager {
protected:
    /**
     * @brief Calculate interference distance
     *
     * You may want to overwrite this function in order to do your own
     * interference calculation
     */
    double calcInterfDist() override;
};

} // namespace veins

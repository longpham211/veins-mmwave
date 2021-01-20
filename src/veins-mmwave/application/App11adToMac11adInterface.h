/*
 * App11adToMac11adInterface.h
 *
 *  Created on: Dec 21, 2019
 *      Author: longpham211
 */

#pragma once

//#include "veins/base/phyLayer/MacToPhyInterface.h"

namespace veins {
    class App11adToMac11adInterface {
    public:
        virtual ~App11adToMac11adInterface() {};

        /**
         * @brief Returns the MAC address of this MAC module.
         */
        virtual const LAddress::L2Type& getMACAddress() = 0;
    };
}

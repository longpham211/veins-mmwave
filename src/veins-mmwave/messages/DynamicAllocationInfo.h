/*
 * DynamicAllocationInfo.h
 *
 *  Created on: Jan 23, 2020
 *      Author: longpham211
 */

#ifndef SRC_VEINS_MMWAVE_MESSAGES_DYNAMICALLOCATIONINFO_H_
#define SRC_VEINS_MMWAVE_MESSAGES_DYNAMICALLOCATIONINFO_H_

#include <omnetpp.h>
#include "veins/base/utils/SimpleAddress.h"

namespace veins {
class DynamicAllocationInfo {
public:
    uint32_t TID;
    uint32_t allocationType;
    LAddress::L2Type sourceAID;
    LAddress::L2Type destinationAID;
    uint32_t allocationDuration;

public:
    DynamicAllocationInfo() {
        TID = 0;
        allocationType = 0;
        allocationDuration = 0;
        sourceAID = LAddress::L2NULL();
        destinationAID = LAddress::L2NULL();
    };
};
}


#endif /* SRC_VEINS_MMWAVE_MESSAGES_DYNAMICALLOCATIONINFO_H_ */

/*
 * AllocationField.h
 *
 *  Created on: Jan 23, 2020
 *      Author: longpham211
 */

#ifndef SRC_VEINS_MMWAVE_MESSAGES_ALLOCATIONFIELD_H_
#define SRC_VEINS_MMWAVE_MESSAGES_ALLOCATIONFIELD_H_

#include <omnetpp.h>
#include "veins/base/utils/SimpleAddress.h"

namespace veins {
class AllocationField {
public:
    //Allocation Control Field
    uint32_t allocationType;
    LAddress::L2Type sourceAID;
    LAddress::L2Type destinationAID;
    uint64_t allocationStart;
    uint32_t allocationBlockDuration;
    uint32_t numberOfBlocks;
    uint32_t allocationBlockPeriod;

};
}



#endif /* SRC_VEINS_MMWAVE_MESSAGES_ALLOCATIONFIELD_H_ */

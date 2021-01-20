/*
 * FmlMac.cc
 *
 *  Created on: Jul 1, 2020
 *      Author: longpham211
 */

#include <veins-mmwave/mac/FmlMac.h>

using namespace veins;

using std::unique_ptr;

Define_Module(veins::FmlMac);

void veins::FmlMac::initialize(int stage) {
    BaseMacLayer::initialize(stage);

    if(stage == 0) {
        phy11ad = FindModule<Mac11adToPhy11adInterface*>::findSubModule(
                getParentModule());
        ASSERT(phy11ad);

        mcs = MmWaveMCS::cphy_mcs_0;

        txPower = par("txPower").doubleValue();
        numberAntenna = phy11ad->getNumberOfAntenna();

        for (uint32_t i = 0; i < numberAntenna; i++)
            totalSectors += phy11ad->getNumberOfSectorsPerAntenna(i);
    }
}


veins::FmlMac::~FmlMac() {
}

void veins::FmlMac::finish() {
}

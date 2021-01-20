/*
 * MmWaveConnectionManager.cc
 *
 *  Created on: May 28, 2020
 *      Author: longpham211
 */

#include "MmWaveConnectionManager.h"

Define_Module(veins::MmWaveConnectionManager);

using namespace veins;

double veins::MmWaveConnectionManager::calcInterfDist() {

    return 200; // 300m? upperbound? TODO
}

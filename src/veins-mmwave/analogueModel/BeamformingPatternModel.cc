/*
 * BeamformingPatternModel.cc
 *
 *  Created on: Jan 4, 2020
 *      Author: longpham211
 */

#include "veins-mmwave/analogueModel/BeamformingPatternModel.h"
#include <veins-mmwave/utility/ConstsMmWave.h>
#include "veins/base/messages/AirFrame_m.h"
#include "veins-mmwave/utility/POAS.h"
#include <fstream>
#include <sstream>

using namespace veins;

using veins::AirFrame;

void veins::BeamformingPatternModel::filterSignal(Signal* signal) {
//    dynamic_cast<POAS&>(signal->getSenderPoa());
//    auto receiverPOAS = dynamic_cast<POAS>(signal->getReceiverPoa());

   // EV_TRACE << "BeamformingPatternModel: signal value before decrease: " << signal->getAtCenterFrequency()<<std::endl;

    //TODO support multiple antennas here

//    *signal = *signal -1;
  //  EV_TRACE <<"BeamformingPatternModel: signal value after decrease: " << signal->getAtCenterFrequency()<<std::endl;


}

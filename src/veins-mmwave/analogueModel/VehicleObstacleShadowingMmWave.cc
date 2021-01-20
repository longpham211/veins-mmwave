//
// Copyright (C) 2006-2018 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins-mmwave/analogueModel/VehicleObstacleShadowingMmWave.h"

using namespace veins;

VehicleObstacleShadowingMmWave::VehicleObstacleShadowingMmWave(cComponent* owner, VehicleObstacleControl& vehicleObstacleControl, bool useTorus, const Coord& playgroundSize)
    : VehicleObstacleShadowing(owner, vehicleObstacleControl, useTorus, playgroundSize)
{
    if (useTorus) throw cRuntimeError("VehicleObstacleShadowing does not work on torus-shaped playgrounds");
}

void VehicleObstacleShadowingMmWave::filterSignal(Signal* signal)
{
    auto senderPos = signal->getSenderPoa().pos.getPositionAt();
    auto receiverPos = signal->getReceiverPoa().pos.getPositionAt();

    auto potentialObstacles = vehicleObstacleControl.getPotentialObstacles(signal->getSenderPoa().pos, signal->getReceiverPoa().pos, *signal);

    if (potentialObstacles.size() < 1) return;

    double senderHeight = senderPos.z;
    double receiverHeight = receiverPos.z;
    potentialObstacles.insert(potentialObstacles.begin(), std::make_pair(0, senderHeight));
    potentialObstacles.emplace_back(senderPos.distance(receiverPos), receiverHeight);

    if(vehicleInBetweenDZ(potentialObstacles)){
        EV_TRACE <<"Vehicle in between, zero the signal!" << std::endl;
        *signal *= 0;
    }

//    auto attenuationDB = VehicleObstacleControl::getVehicleAttenuationDZ(potentialObstacles, Signal(signal->getSpectrum()));
//
//    EV_TRACE << "t=" << simTime() << ": Attenuation by vehicles is " << attenuationDB << std::endl;
//
//    // convert from "dB loss" to a multiplicative factor
//    Signal attenuation(attenuationDB.getSpectrum());
//    for (uint16_t i = 0; i < attenuation.getNumValues(); i++) {
//        attenuation.at(i) = pow(10.0, -attenuationDB.at(i) / 10.0);
//    }
//
//    if(attenuation.getValues()[signal->getCenterFrequencyIndex()] < 1)
//        *signal *= 0;
}

bool veins::VehicleObstacleShadowingMmWave::vehicleInBetweenDZ(
        const std::vector<std::pair<double, double> > &dz_vec) {
    ASSERT(dz_vec.size() >= 2);

    // make sure the list of x coordinates is sorted
    for (size_t i = 0; i < dz_vec.size() - 1; i++) {
        ASSERT(dz_vec[i].first < dz_vec[i + 1].first);
    }
    size_t i = 0;
    double max_slope = -std::numeric_limits<double>::infinity();
    size_t max_slope_index;
    bool have_max_slope_index = false;

    for (size_t j = i + 1; j < dz_vec.size(); ++j) {
        double slope = (dz_vec[j].second - dz_vec[i].second) / (dz_vec[j].first - dz_vec[i].first);

        if (slope > max_slope) {
            max_slope = slope;
            max_slope_index = j;
            have_max_slope_index = true;
        }
    }

    // Sanity check
    ASSERT(have_max_slope_index);

    if (max_slope_index >= dz_vec.size() - 1)
        return false; // We don't have vehicle in between regarding to the height

    return true; // There is/are vehicle(s) in between
}

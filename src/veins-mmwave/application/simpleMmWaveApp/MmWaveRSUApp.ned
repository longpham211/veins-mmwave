//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
//https://www.ccs-labs.org/software/veins-vlc/
package org.car2x.veinsmmwave.application.simpleMmWaveApp;

import org.car2x.veins.base.modules.IBaseApplLayer;

simple MmWaveRSUApp like IBaseApplLayer
{
    parameters:
        @class(veins::MmWaveRSUApp);
        bool debug = default(false);
        int headerLength @unit(bit) = default(0 bit); // header length of the application
        int packetByteLength @unit(byte);
        double beaconingFrequency @unit(hertz);

    gates:
        input lowerLayerIn; // from MAC layer
        output lowerLayerOut; // to MAC layer

        input lowerControlIn;
        output lowerControlOut;
}

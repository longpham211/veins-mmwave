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

package org.car2x.veinsmmwave.node;
import org.car2x.veins.base.modules.*;
import org.car2x.veinsmmwave.splitter.*;
import org.car2x.veinsmmwave.nic.*;


module APMmWave
{
    parameters:
        string applType; // type of the application layer
        string nicType = default("Nic80211p"); // type of the network interface card
        string nicMmWaveType = default("org.car2x.veinsmmwave.nic.NicMmWave");
        
        @display("bgb=457,459");

    gates:
        input veinsradioIn; // gate for sendDirect
        input mmWaveIn;

    submodules:
        application: <applType> like org.car2x.veins.base.modules.IBaseApplLayer {
            parameters:
                @display("p=163,50");
        }

        nic: <nicType> like org.car2x.veins.modules.nic.INic80211p {
            parameters:
                @display("p=71,243");
        }

        splitter: Splitter {
            @display("p=163,127");
        }

        nicMmWave: <nicMmWaveType> like org.car2x.veinsmmwave.nic.INicMmWave{
            parameters:
                @display("p=253,243");
        }
        
      	mobility: BaseMobility {
            parameters:
                @display("p=120,118;i=block/cogwheel");
        }

    connections allowunconnected:
        application.lowerLayerOut --> splitter.applicationIn;
        application.lowerLayerIn <-- splitter.applicationOut;

        splitter.nicOut --> nic.upperLayerIn;
        splitter.nicIn <-- nic.upperLayerOut;

        splitter.nicMmWaveOut --> nicMmWave.upperLayerIn;
        splitter.nicMmWaveIn <-- nicMmWave.upperLayerOut;

        veinsradioIn --> nic.radioIn;
        mmWaveIn --> nicMmWave.radioIn;
}
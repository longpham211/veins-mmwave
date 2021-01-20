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
package org.car2x.veinsmmwave.mobility;

import org.car2x.veins.base.modules.BaseMobility;
import org.car2x.veinsmmwave.node.CarMmWave;

module CarMmWaveMobility extends CarMmWave {
	parameters:
	    string mobilityType = default("org.car2x.veins.base.modules.BaseMobility"); // type of the mobility module
	    
	submodules:
	    foreignMobility: <mobilityType> like org.car2x.veins.base.modules.IMobility {
	    	parameters:
	    	    @display("p=368,194;i=block/cogwheel");
	    }
}

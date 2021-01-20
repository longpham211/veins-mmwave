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

package org.car2x.veinsmmwave.mac;

simple MmWaveAPMac extends MmWaveMac {
	parameters:
	    @class(veins::MmWaveAPMac);
	    
	    int a_bft_length = default(8);
	    
	     //the duration of beacon interval
		double beaconIntervalDuration @unit(millisec) = default (100 millisec);  
		
		int maximumAssignedSPDuration @unit(us) = default (10000 us);
		
		bool servingOldVehicleInNewBeaconInterval = default(false);
}

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

simple FmlAPMac2 extends MmWaveAPMac {
	parameters:
	    @class(veins::FmlAPMac2);
	   	double estimateDuration @unit(millisec) = default (10 millisec); 
	   	double rsuCoverRange @unit(m);
	   	bool considerObstaclesInVehiclesLeavingBeam;
	   	
	   	int factorControlFunction = default(1);
	   	int totalNo;
	   	
	   	@signal[period](type="long");
	    @statistic[period](title="Period Count"; source="period"; record=vector);
	    
//	    @signal[explorationGroupSize](type="long");
//	    @statistic[explorationGroupSize](title="ExplorationGroupSize"; source="explorationGroupSize"; record=vector);
}

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

package org.car2x.veinsmmwave.phy;

import org.car2x.veins.base.phyLayer.BasePhyLayer;


simple PhyLayerMmWave extends BasePhyLayer {
	parameters:
       	@class(veins::PhyLayerMmWave);
       	
       	antennaOffsetZ = default(1.895 m);
		//enables/disables collection of statistics about collision. notice that
		//enabling this feature increases simulation time      
       	bool collectCollisionStatistics = default(false);
       	
       	//TODO Check this one
       	double ccaThreshold @unit(dBm) = default(-78 dBm);
       
       	//other parameters 
               
		//number of antennas
       	int numberOfAntenna = default(1);
       	
       	//number of sectors in an antenna
       	int numberOfSectorsPerAntenna = default(36); 
}
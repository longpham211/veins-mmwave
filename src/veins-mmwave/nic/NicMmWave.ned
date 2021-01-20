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
package org.car2x.veinsmmwave.nic;

import org.car2x.veinsmmwave.mac.MmWaveSTAMac;
import org.car2x.veinsmmwave.phy.PhyLayerMmWave;
import org.car2x.veins.base.modules.IWirelessMac;

module NicMmWave like INicMmWave {
	parameters:
	    // Explicitly specify the Connection Manager for this type of NIC
        // If not specified, MiXiM defaults to the BaseConnectionManager
        //TODO 
        string connectionManagerName = default("mmWaveConnectionManager");
        string macMmWaveType;
        
    gates:
        input upperLayerIn;
        output upperLayerOut;
        
        input upperControlIn;
        output upperControlOut;
        
        input radioIn; // radioIn gate for sendDirect
        
	submodules:
	    phyMmWave: PhyLayerMmWave {
	    	@display("p=69,218;i=block/process_s");
	    }
	    
	    macMmWave: <macMmWaveType> like org.car2x.veins.base.modules.IWirelessMac {
	    	@display("p=69,82");
	    }
	    
	connections allowunconnected:
	   radioIn --> phyMmWave.radioIn;
	   
	   // Bottom up
	   phyMmWave.upperLayerOut --> macMmWave.lowerLayerIn;
	   macMmWave.upperLayerOut --> upperLayerOut;
	   
	   // Top down
	   upperLayerIn --> macMmWave.upperLayerIn;
	   macMmWave.lowerLayerOut --> phyMmWave.upperLayerIn;
	   
	   // Bottom up control
	   phyMmWave.upperControlOut --> macMmWave.lowerControlIn;
	   macMmWave.upperControlOut --> upperControlOut;
	   
	   // Top down control
	   upperControlIn --> macMmWave.upperControlIn;
	   macMmWave.lowerControlOut --> phyMmWave.upperControlIn;
}
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

//package examples.veinsmmwave;
//https://www.ccs-labs.org/software/veins-vlc/

import org.car2x.veins.base.connectionManager.ConnectionManager;
import org.car2x.veins.base.modules.BaseWorldUtility;
import org.car2x.veins.modules.mobility.traci.TraCIScenarioManager*;
import org.car2x.veins.modules.obstacle.ObstacleControl;
import org.car2x.veins.modules.obstacle.VehicleObstacleControl;
import org.car2x.veins.modules.world.annotations.AnnotationManager;
import org.car2x.veinsmmwave.mobility.CarMmWaveMobility;
import org.car2x.veinsmmwave.node.*;
import org.car2x.veinsmmwave.connectionManager.MmWaveConnectionManager;

module Highway {
	parameters:
	    double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
	    double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
	    double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
	    int nodesCount = default(0);
	    @display("bgb=$playgroundSizeX,$playgroundSizeY");
	    
	submodules:
        obstacles: ObstacleControl {
            @display("p=261,35");
        }
        vehicleObstacles: VehicleObstacleControl {
            @display("p=261,35");
        }
        annotations: AnnotationManager {
            @display("p=485,35");
        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=146,31;i=abstract/multicast");
        }
        mmWaveConnectionManager: MmWaveConnectionManager {
            parameters:
                @display("p=146,112;i=abstract/multicast");
        }
        world: BaseWorldUtility {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=34,31;i=misc/globe");
        }
        manager: TraCIScenarioManagerLaunchd {
            parameters:
                @display("p=34,206");
        } 

        node[nodesCount]: STAMmWave {
            //@display("p=349,206");
        }


	connections allowunconnected:
}

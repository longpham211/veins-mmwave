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

package org.car2x.veinsmmwave.splitter;

simple Splitter
{
    parameters:
        @class(veins::Splitter);
        bool debug = default(false);
        bool collectStatistics = default(false);
        bool positionAidForMmWave = default(false);        
       	//the duration of beacon interval
		double positionAidDuration @unit(millisec) = default (1000 millisec); 
        int postionAidMessagePacketBits @unit(bit) = default(128bit); // 2 floats for long, lat, 1 float for velocity, 1 float for yaw
        int dsrcHeaderLength @unit(bit) = default(80bit); // as in veins-5.0
		
		bool fml = default(false);
		int fmlConnectionRequestBits @unit(bit) = 3bit; // 8 directions of arrival
		int fmlServiceRequestBits @unit(bit) = 51 bit; //48 bit of mac address, 3 bit of doA
		int fmlRequestResponseBits @unit(bit) = 8 bit; //up to 256 beam
		int fmlMmWaveConfigurationBits @unit(bit) = 8 bit; // 256 beam
		int fmlFeedbackBits @unit(bit) = 32 bit; //4 byte for a float of aggreagetData
		
		double rsuX = default(0);
		double rsuY = default(0);
		double rsuZ = default(0);
		double rsuCoverRange = default(0m) @unit(m);
		string rsuAddress = default("0");
		
		int numDoA = default(4);

		bool recordInTimeSlot = default(true);
		int recordTimeSlot @unit(s) = default(3600s); // 1 hour;

//		@signal[overhead](type="double");
//		@statistic[overhead](title="Overhead"; source="overhead"; record=sum,vector);
		
    gates:
        output applicationOut; // output gate towards the application
        input applicationIn; // input gate from the application

        output nicOut; // output gate towards the IEEE 802.11p NIC
        input nicIn; // input gate from the 802.11p NIC

        output nicMmWaveOut; // output gate towards the mmWave
        input nicMmWaveIn; // input gate from the mmWave
}

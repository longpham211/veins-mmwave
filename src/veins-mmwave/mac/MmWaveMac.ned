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

import org.car2x.veins.base.modules.BaseMacLayer;

simple MmWaveMac extends BaseMacLayer
{
    parameters:
        @class(veins::MmWaveMac);

        //assign default value to the MAC header
        headerLength @unit(bit) = default(0 bit);
        
        //tx power [mW]
        double txPower @unit(mW);
        
        int fss = default(16);
        
        //the maximum queue size of MAC queue. 0 for unlimited. Queue strategy is "drop if full"
        int queueSize = default(0);
        
		// unicast parameters
        int dot11RTSThreshold @unit(bit) = default(12000bit);
        int dot11ShortRetryLimit = default(7);
        int dot11LongRetryLimit = default(4);
        int dot11BFRetryLimit = default(10);
        
        int ackLength @unit(bit) = default(112bit);
        bool useAcks = default(false);
        double ackErrorRate = default(0.20);
        
        bool refreshBeamStateEveryBI = default(true); // if it is false, we need to handle when to refreshBeamState
        
		bool evaluateAlgorithms = default(false);
		
		double enterBeamThreshold = default(-70dBm) @unit(dBm);
		double exitBeamThreshold = default(-78dBm) @unit(dBm);
		
		int evaluatePacketByteLength @unit(byte);
		
		bool doBeamFormingInSPAllocation = default(true);
		
		bool recordInTimeSlot = default(true);
		int recordTimeSlot @unit(s) = default(60s); // 1 hour;
		
		
		@signal[aggregateData](type="double");
	    @statistic[aggregateData](title="Aggregate Data"; source="aggregateData"; record=sum,vector);
	    
	    @signal[selectedMCS](type="long");
	    @statistic[selectedMCS](title="Selected MCS"; source="selectedMCS"; record=vector);
}
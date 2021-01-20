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


//This is the definition for DMG Beacon message, which is sent by
//PCP/AP or a station acting like a PCP/AP, in the Beacon Transmission
//Interval (BTI) subphase of Beacon Header Interval (BHI) phase.

cplusplus {{
#include "veins-mmwave/messages/FramesContainSSWField_m.h"
}}

namespace veins;

class veins::FramesContainSSWField;

message DMGBeacon extends veins::FramesContainSSWField{  
    int a_bft_length; // the number of slots for the A-BFT phase, defined in SSW slots [1,8]
    int fss; //the number of SSW frames allowed per sector sweep slot [1,16] 
}

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

// This is the generalized message of DMG Beacon and SSW Frame, which have
// SSW field



cplusplus {{
#include "veins/modules/messages/BaseFrame1609_4_m.h"
}}

namespace veins;

class veins::BaseFrame1609_4;

packet FramesContainSSWField extends veins::BaseFrame1609_4 {
    
    // If this frame is DMG Beacon, 
    //then the duration is the time reemaining until the end of the BTI in microsecond
    int duration;  
    
    //Sector Sweep field
    
    //The Direction field is set to 0 to indicate that the frame is 
    //transmitted by the beamforming initiator and set to 1 to 
    //indicate that the frame is transmitted by the beamforming responder.
    int direction;
    
    //a down-counter indicating the number of remaining DMG Beacon/Sector Sweep frame transmissions to the end of the TXSS
    //this frame is set to 0 in the last frame DMG Beacon.
    //possible value range from 0 to 511    
    int CDOWN;
    
    //To indicate the sector number through which the frame containing this SSW field is transmitted
    int sectorID;
    
    //To indicate the DMG antenna the transmitter is currently using for this transmission
    int antennaID;
}

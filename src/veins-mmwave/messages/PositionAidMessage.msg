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


cplusplus {{
#include "veins/base/utils/Coord.h"
#include "veins/base/utils/Heading.h"
#include "veins-mmwave/messages/MmWaveMessage_m.h"
#include "veins/base/utils/SimpleAddress.h"
}}

namespace veins;

class MmWaveMessage;
class noncobject Coord;
class noncobject Heading;
class LAddress::L2Type extends void;


packet PositionAidMessage extends MmWaveMessage {
	Coord senderPosition;
	double senderSpeed;
	Coord senderDirection;
	Coord senderOrientation;
	Heading senderHeading;
	//LAddress::L2Type senderDSRCAddress = -1;
	LAddress::L2Type sender11adAddress = -1;
}
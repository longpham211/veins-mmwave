/*
 * Signal80211ad.cc
 *
 *  Created on: Jan 5, 2020
 *      Author: longpham211
 */
#include "veins-mmwave/utility/Signal80211ad.h"

namespace veins {

Signal80211ad::Signal80211ad(Spectrum spec, simtime_t start,
        simtime_t duration, int32_t sSectorID, int32_t rSectorID)
    : Signal(spec, start, duration)
    , senderSectorID (sSectorID)
    , receiverSectorID (rSectorID)
{
};

int32_t Signal80211ad::getSenderSectorID() {
    return senderSectorID;
}

int32_t Signal80211ad::getReceiverSectorID() {
    return receiverSectorID;
}

int32_t Signal80211ad::setSenderSectorID(int32_t sectorID) {
    senderSectorID = sectorID;
}

int32_t Signal80211ad::setReceiverSectorID(int32_t sectorID) {
    receiverSectorID = sectorID;
}


} // namespace veins

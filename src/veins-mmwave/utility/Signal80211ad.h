/*
 * Signal80211ad.h
 *
 *  Created on: Jan 5, 2020
 *      Author: longpham211
 */

#include "veins/base/toolbox/Signal.h"

namespace veins {

class Signal80211ad : public Signal {
public:
    Signal80211ad(Spectrum spec, simtime_t start, simtime_t duration, int32_t senderSectorID, int32_t receiverSectorID);
    ~Signal80211ad() = default;
    int32_t getSenderSectorID();
    int32_t getReceiverSectorID();
    int32_t setSenderSectorID(int32_t sectorID);
    int32_t setReceiverSectorID(int32_t sectorID);
    //TODO implement the overide constructor, operator

private:
    int32_t senderSectorID;
    int32_t receiverSectorID;
};
}

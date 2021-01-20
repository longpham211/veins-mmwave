/*
 * Mac11adToPhy11adInterface.h
 *
 *  Created on: Dec 20, 2019
 *      Author: longpham211
 */

#pragma once

#include "veins/base/phyLayer/MacToPhyInterface.h"
#include <veins-mmwave/utility/ConstsMmWave.h>
#include "veins/base/utils/POA.h"

namespace veins {
    class Mac11adToPhy11adInterface {
    public:
        enum BasePhyMessageKinds {
            CHANNEL_IDLE,
            CHANNEL_BUSY,
        };

        virtual ~Mac11adToPhy11adInterface() = default;

        virtual uint32_t getNumberOfAntenna() = 0;
        virtual uint32_t getNumberOfSectorsPerAntenna(uint32_t antennaID) = 0;

        virtual void notifyMacAboutRxStart(bool enable) = 0;
        virtual simtime_t getFrameDuration(MmWaveMCS mcs, int payloadLengthBits, uint32_t trainingLengthField) const = 0;

        virtual void requestChannelStatusIfIdle() = 0;

        virtual void setReceivingSectorID(int sectorID) = 0;
        virtual void setTransmittingSectorID(int sectorID) = 0;

        virtual double getRxPowerToRSUAt(simtime_t time, Coord rsuPosition,
                Coord rsuOrientation, int32_t tx_rx_beamID, int32_t rx_tx_beamID) = 0;

        virtual double getDataRateFromRx(double rx) = 0;

        virtual MmWaveMCS getOptimalMCSFromRx(double rx) = 0;

        virtual uint32_t getTransmissionDirID(Coord senderPosition, Coord senderOrientation, uint32_t transmittingSectorID, Coord receiverPosition) = 0;

        virtual Coord getCurrentPosition() = 0;

        virtual Coord getPositionAt(simtime_t) = 0;

        virtual Coord getCurrentOrientation() = 0;

        virtual uint32_t getBestBeamByDirection(uint32_t dirID) = 0;

        virtual uint32_t getBestBeamByTheOtherPosition(Coord otherPosition) = 0;

        virtual uint32_t getBestBeamByTheOtherPositionAt(Coord otherPosition, simtime_t atTheTime) = 0;

        virtual double getReceivePowerdBm(float distance, int32_t dir_tx_rx, int32_t dir_rx_tx, int32_t beam_tx_rx, int32_t beam_rx_tx, bool norm) = 0;
        virtual double getReceivedPowerAfterApplyingAnalogueModels(double requestPower, const POA& senderPOA, const POA& receiverPOA) = 0;

        virtual AntennaPosition getAntennaPosition() = 0;
    };
}

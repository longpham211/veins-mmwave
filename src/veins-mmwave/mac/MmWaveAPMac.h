/*
 * MmWaveAPMac.h
 *
 *  Created on: Apr 7, 2020
 *      Author: longpham211
 */
#pragma once

#include "veins-mmwave/mac/MmWaveMac.h"

namespace veins {
    class MmWaveAPMac : public MmWaveMac {
    public:
        std::map<LAddress::L2Type, DynamicAllocationInfo> allocationRequested;
    public:
        MmWaveAPMac()
        {
        }
        ~MmWaveAPMac() override;

    protected:
        simtime_t beaconIntervalDuration;
        uint32_t a_bft_length; // the number of slots for the A-BFT phase, defined in SSW slots [1,8]
        uint32_t maximumAssignedSPDuration;
        bool servingOldVehicleInNewBeaconInterval;

        cMessage* newBeaconInterval;

    protected:
        void initialize(int) override;

        /** @brief Handle self messages such as timers.*/
        void handleSelfMsg(cMessage*) override;

        /** @brief Handle control messages from lower layer.*/
        void handleLowerControl(cMessage* msg) override;

        /** @brief Delete all dynamically allocated objects of the module.*/
        void finish() override;

        bool handleUnicast(MacPkt* macPkt, DeciderResult80211* res) override;

        void handleBroadcast(MacPkt* macPkt, DeciderResult80211* res) override;

        void handleAckTimeOut(AckTimeOutMessage* ackTimeOutMsg) override;

        virtual void handleAck(const MacAck* ack) override;

        void scheduleTheNextPollFrame();
        virtual void addAnnounceFramesToQueue(simtime_t delay);

        /**
         * @brief
         *
         * Beacon Trnsmission Interval: The time interval between the start of the first
         * Directional Multi-gigabit (DMG) Beacon frame transmission by a DMG station (STA)
         * in a beacon interval to the end of the last DMG Beacon frame transmission by
         * the DMG STA in the same beacon interval.
         *
         * If the STA is an PCP/AP, or it is a DMG STA acting like a PCP/AP, it will call this function
         * periodically every @value beacon_interval to indicate that a new beacon interval has
         * started.
         *
         * This function will calls @fuction performTXSS, to transmit multiple DMG Beacon frames, via
         * different sectors
         */
        virtual void performBTI();

        DMGBeacon* generateDMGBeacon();

        //attach the beacon interval control field for the DMG Beacon
        void attachBICField(DMGBeacon* frame, uint32_t a_bft_length, uint32_t fss);
    };
}


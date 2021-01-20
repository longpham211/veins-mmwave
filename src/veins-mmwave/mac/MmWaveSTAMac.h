/*
 * MmWaveSTAMac.h
 *
 *  Created on: Apr 7, 2020
 *      Author: longpham211
 */

#pragma once

#include "veins-mmwave/mac/MmWaveMac.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

namespace veins {
    class MmWaveSTAMac : public MmWaveMac {
    public:
        MmWaveSTAMac()
        {
        }
        ~MmWaveSTAMac() override;
    protected:
        bool requestSP;
        uint32_t spRequestedDuration;

        uint32_t allowed_a_bft_from_pcp_ap;
        uint32_t allowed_fss_from_pcp_ap;

        cMessage* beginRSS;

        bool addedConstructionSiteNearRSUUrban;

        TraCICommandInterface* traciCommandInterface;
        TraCIMobility* mobility;
    protected:
        void initialize(int) override;

        /** @brief Handle self messages such as timers.*/
        void handleSelfMsg(cMessage*) override;

        /** @brief Handle control messages from lower layer.*/
        void handleLowerControl(cMessage* msg) override;

        bool handleUnicast(MacPkt* macPkt, DeciderResult80211* res) override;

        void handleBroadcast(MacPkt* macPkt, DeciderResult80211* res) override;

        void handleAckTimeOut(AckTimeOutMessage* ackTimeOutMsg) override;

        void performA_BFT(uint32_t a_bftLength, uint32_t fss);

        SSWFrameRSS* generateSSWFrameRSS();

        void generateAndQueueSSWACK(BestBeamInfo* info);

        void updateBeamformingDataFromSSWFrame(FramesContainSSWField* frame, DeciderResult80211* res);

        /** @brief Delete all dynamically allocated objects of the module.*/
        void finish() override;
    };
}

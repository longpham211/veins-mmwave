/*
 * MmWaveMac.h
 *
 *  Created on: Apr 7, 2020
 *      Author: longpham211
 */

#pragma once

#include <queue>

#include "veins/base/modules/BaseMacLayer.h"
#include "veins-mmwave/application/App11adToMac11adInterface.h"
#include "veins-mmwave/phy/PhyLayerMmWave.h"
#include "veins/base/utils/FindModule.h"
#include "veins-mmwave/messages/SSWFrameRSS_m.h"
#include "veins-mmwave/messages/SSWFeedbackFrame_m.h"
#include "veins-mmwave/messages/PollFrame_m.h"
#include "veins-mmwave/messages/SSWFrameISS_m.h"
#include "veins/modules/messages/AckTimeOutMessage_m.h"
#include "veins-mmwave/messages/AllocationField.h"
#include "veins-mmwave/messages/MacAck_m.h"
#include "veins-mmwave/utility/MacToPhyControlInfo11ad.h"
#include "veins/base/messages/MacPkt_m.h"
#include "veins/base/phyLayer/PhyToMacControlInfo.h"
#include "veins/modules/phy/DeciderResult80211.h"
#include "veins-mmwave/messages/DynamicAllocationInfo.h"
#include "veins-mmwave/messages/SPRFrame_m.h"
#include "veins-mmwave/messages/AnnounceFrame_m.h"
#include "veins-mmwave/messages/DMGBeacon_m.h"
#include "veins-mmwave/messages/SSWACK_m.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins-mmwave/messages/MmWaveMessage_m.h"

#include <numeric>

namespace veins {

class MmWaveMac : public BaseMacLayer,  public App11adToMac11adInterface{
public:
    enum packet_kind {
        DMG_BEACON = 1000 ,
        SSW_FRAME_RSS = 1001 ,
        SSW_FRAME_ISS = 1002,
        SSW_FEEDBACK = 1003,
        POLL_FRAME = 1004,
        SPR_FRAME = 1005,
        ANNOUNCE_FRAME = 1006,
        SSW_ACK = 1007
    };

    // Table 8-183i in the IEEE 802.11ad - 2012 standard
    enum class AllocationType {
        SP = 0,
        CBAP = 1
    };

    enum class AccessPeriod {
        BTI,
        A_BFT,
        ATI,
        SP,
        CBAP
    };

    MmWaveMac()
    {
    }
    ~MmWaveMac() override;

    const LAddress::L2Type& getMACAddress() override
    {
        ASSERT(myMacAddr != LAddress::L2NULL());
        return BaseMacLayer::getMACAddress();
    }

    class EDCA : HasLogProxy {
    public:

        class EDCAQueue {
        public:
            //TODO we currently just need 2 queue 9.3.2.3.6 802.11ad 2012, 1 for the Control Frames, and 1 for the data
            std::queue<BaseFrame1609_4*> queue; //
            int cwMin; // minimum contention window
            int cwMax; // maximum contention window
            int cwCur; // current contention window
            int64_t currentBackoff; // current Backoff value
            int ssrc; // station short retry count
            int slrc; // station long retry count
            int rc; //retry count
            bool waitForAck; // true if the queue is waiting for an acknowledgment for unicast
            unsigned long waitOnUnicastID; // unique id of unicast on which station is waiting
            AckTimeOutMessage* ackTimeOut; // timer for retransmission on receiving no ACK

            EDCAQueue()
            {
            }
            EDCAQueue(int cwMin, int cwMax);
            ~EDCAQueue();
        };

        EDCA(cSimpleModule* owner, MmWaveChannelType channelType, int maxQueueLength = 0);
        ~EDCA();

        void createQueue(int cwMin, int cwMax);
        int queuePacket(BaseFrame1609_4* csmg);
        void backoff();
        //Because we don't use QoS-STA here, then there are no
        //internal contentions among multiple QOS queues, therefore
        //we don't need to actual start the contention amongs the internal queues
        // This function somehow similar to startContent function of Mac1609_4
        //which is to find the time for the next event, but does not take the contention
        //into account
        simtime_t findTimeForTheNextEvent(simtime_t idleSince, AllocationType type);
        void postTransmit(BaseFrame1609_4* msg, bool useAcks, AccessPeriod currentAccessPeriod);

        /** @brief return the next packet to send*/
        BaseFrame1609_4* initiateTransmit(simtime_t idleSince);


    public:
        cSimpleModule* owner;
        EDCAQueue myQueue;
        uint32_t maxQueueSize;
        MmWaveChannelType channelType;

        /** @brief Id for debug messages*/
        std::string myId;

    };

public:
    struct BestBeamInfo {
        uint32_t myBestAntennaID;
        uint32_t myBestSectorID;
        double myBestSNR; //this information is related to the distance and the angle as well, therefore, it's not wise to keep track on this information
        uint32_t theirBestAntennaID;
        uint32_t theirBestSectorID;
        double theirBestSNR;
        bool finished; // finished beamforming or not?

        BestBeamInfo()
            : myBestAntennaID(10000)
            , myBestSectorID(10000)
            , myBestSNR(-1)
            , theirBestAntennaID(10000)
            , theirBestSectorID(10000)
            , theirBestSNR(-1)
            , finished(false)
            {}
        BestBeamInfo(uint32_t myBestAntennaID, uint32_t myBestSectorID, double myBestSNR, uint32_t theirBestAntennaID, uint32_t theirBestSectorID, double theirBestSNR, bool finished)
            : myBestAntennaID(myBestAntennaID)
            , myBestSectorID(myBestSectorID)
            , myBestSNR(myBestSNR)
            , theirBestAntennaID(theirBestAntennaID)
            , theirBestSectorID(theirBestSectorID)
            , theirBestSNR(theirBestSNR)
            , finished(finished)
            {}
    };

    std::map< LAddress::L2Type, BestBeamInfo> beamStates;
    std::vector<AllocationField> assignedAllocations;
    std::vector<LAddress::L2Type> availableVehicles;

protected:
    Mac11adToPhy11adInterface* phy11ad;
    MmWaveMCS mcs; ///< Modulation and coding scheme to use unless explicitly specified.
    double txPower;
    uint32_t numberAntenna = 0;
    uint32_t totalSectors = 0;
    int queueSize;

    int dot11BFRetryLimit;

    // indicates rx start within the period of ACK timeout
    bool rxStartIndication;

    // An ack is sent after SIFS irrespective of the channel state
    bool ignoreChannelState;

    std::string myId; // Id for debug message

    std::map<MmWaveChannelType, std::unique_ptr<EDCA>> myEDCA;

    MmWaveChannelType activeChannel;

    bool isInSLSPhase; // if this STA is in SLS phase, don't switch the antenna to RX after TX_OVER
    AccessPeriod currentAccessPeriod;

    LAddress::L2Type communicatingSTA;
    LAddress::L2Type evaluationServingSTA;

    /** @brief Last time the channel went idle */
    simtime_t lastIdle;
    simtime_t lastBusy;
    bool allowedToSendData;

    bool idleChannel;
    bool useAcks;
    bool useOracle;
    bool doBeamFormingInSPAllocation;

    TraCIScenarioManager* traciManager;

    uint32_t ackLength;

    // Dont start contention immediately after finishing unicast TX. Wait until ack timeout/ ack Rx
    bool waitUntilAckRXorTimeout;

    /** @brief Self message to wake up at next MacEvent */
    cMessage* nextMacEvent;
    /** @brief Self message to wake up to send message in BHI, SLS in DTI*/
    cMessage* nextMacControlEvent;
    cMessage* beginOfA_BFT;
    cMessage* beginOfATI;
    cMessage* sendSSWFeedbackFrame;
    cMessage* beginOfAnAllocation;
    cMessage* endOfAnAllocation;


    uint32_t fss; //the number of SSW frames allowed per sector sweep slot [1,16]

    /** @brief Pointer to the last sent packet*/
    BaseFrame1609_4* lastSentPacket;

    /** @brief pointer to last sent mac frame */
    std::unique_ptr<MacPkt> lastMac;

    /** @brief stats   */
    long statsDroppedPackets;
    simtime_t statsTotalBusyTime;
    long statsSentPackets;
    long statsSentAcks;


    //direction of SSW
    const int32_t FROM_INITIATOR = 0;
    const int32_t FROM_RESPONDER = 1;

    bool refreshBeamStateEveryBI;

    bool evaluateAlgorithms;

    double enterBeamThreshold;
    double exitBeamThreshold;

    bool recordInTimeSlot;
    int recordTimeSlot;
    double aggregateDataPerTimeSlot;
    int32_t timeSlotRecorded;


    uint32_t evaluatePacketByteLength;

//    simsignal_t aggregateDataSignal;
    cOutVector aggregateDataVector;
    cOutVector selectedMCSVector;

    int32_t lastUsedSectorID;

    std::array<int64_t, 13> selectedMCSArray;

protected:
    void initialize(int) override;

    /** @brief Delete all dynamically allocated objects of the module.*/
    void finish() override;

    void channelIdle();
    // the channel turned busy because someone else is sending
    void channelBusy();

    //The channel turned busy because we are sending
    void channelBusySelf();

    /** @brief Handle self messages such as timers.*/
    void handleSelfMsg(cMessage*) override;

    /** @brief Handle control messages from upper layer.*/
    void handleUpperControl(cMessage* msg) override;

    /** @brief Handle messages from upper layer.*/
    void handleUpperMsg(cMessage*) override;

    /** @brief Handle messages from lower layer.*/
    void handleLowerMsg(cMessage*) override;

    /** @brief Handle control messages from lower layer.*/
    void handleLowerControl(cMessage* msg) override;

    virtual void handleAckTimeOut(AckTimeOutMessage* ackTimeOutMsg);

    virtual void handleRetransmit();

    virtual bool handleUnicast(MacPkt* macPkt, DeciderResult80211* res);

    virtual void handleBroadcast(MacPkt* macPkt, DeciderResult80211* res);

    void setActiveChannel(MmWaveChannelType channel);
    bool isControlChannelActive();
    bool isServiceChannelActive();

    /**
     * @brief
     *
     * Transmit Sector Sweep (TXSS): Transmission of multiple Sector Sweep (SSW) or
     * Directional Multi-gigabit (DMG) Beacon frames via different sectors,
     * in which a sweep is performed between consecutive transmissions.
     *
     *
     */
    void performTXSS(FramesContainSSWField*, simtime_t atTime, uint32_t fss);

    void attachSSWField(FramesContainSSWField* frame, uint32_t CDOWN, uint32_t antennaID, uint32_t sectorID);

    simtime_t getSSSlotTime(uint32_t fss);

    simtime_t getSSFBDuration();

    void generateAndQueueSSWFeedbackFrame(BestBeamInfo* info);

    SSWFrameISS* generateSSWFrameISS();

    void sendFrame(MacPkt* frame, omnetpp::simtime_t delay, uint32_t antennaID, uint32_t sectorID, MmWaveMCS mcs, double txPower_mW);

    void attachControlInfo(MacPkt* mac, uint32_t antennaID, uint32_t sectorID, MmWaveMCS mcs, double txPower_mW);

    simtime_t getAckWaitTime(MmWaveMCS mcs);

    uint32_t getHeaderLength(MmWaveMCS mcs);

    virtual void handleAck(const MacAck* ack);

    void sendAck(LAddress::L2Type recpAddress, unsigned long treeID);

    virtual Mac11adToPhy11adInterface* getPhyModuleOfVehicleByOracle(LAddress::L2Type vehicleMAC);

    virtual double estimateRecevierSensitivityByOracle(LAddress::L2Type vehicleMAC, Mac11adToPhy11adInterface* phyMmWave, bool applyAnalogueModels, bool norm);

    virtual int32_t getReceivingSectorIDForOracle(LAddress::L2Type vehicleMAC, Mac11adToPhy11adInterface* phyMmWave);

    virtual void whenServingCarLeaveTheBeam(bool teleported);

    virtual bool serveTheNewVehicle();

    virtual void pumpEvaluatePacketToQueue();

    virtual bool doesServingVehicleLeaveTheBeam(Mac11adToPhy11adInterface* phy11adVehicle, LAddress::L2Type macAddress, double sensitivity);

    void addAggregateDataToCollection(int64_t packetLength);
    void addSelectedMCSToCollection(int64_t mcs);

private:
    //This function:
    // - checks if the service queue has the frames to send or not
    // - triggers the beamforming process to the receiver if it hasn't done before between this STA and the receiver, note that
    //   beamforming is currently just is executed in SP allocations
    // - schedule the nextMacEvent to send out the frame
    void prepareToSendDataFrame();

    bool isEDCAQueueEmpty(MmWaveChannelType channelType);

};
} // end of namespace veins

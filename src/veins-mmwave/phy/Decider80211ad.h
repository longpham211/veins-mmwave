#pragma once

#include "veins/base/phyLayer/BaseDecider.h"
#include "veins-mmwave/phy/Decider80211adToPhy80211adInterface.h"
#include "veins-mmwave/mac/Mac11adToPhy11adInterface.h"
#include "veins-mmwave/utility/ConstsMmWave.h"
#include "veins-mmwave/phy/NistErrorRate80211ad.h"

namespace veins {
using veins::AirFrame;

class Decider80211ad : public BaseDecider {

public:

    enum Decider80211ControlKinds {
        NOTHING = 22100,
        BITERROR, // the phy has recognized a bit error in the packet
        LAST_DECIDER_80211_CONTROL_KIND,
        RECWHILESEND
    };

    /**
     * @brief tell the outcome of a packetOk() call, which might be
     * correctly decoded, discarded due to low SNR or discarder due
     * to low SINR (i.e. collision)
     */
    enum PACKET_OK_RESULT {
        DECODED,
        NOT_DECODED,
        COLLISION
    };

protected:

    /**
     * @brief: Power level threshold used to declare channel busy if premable
     * portion is missed. TODO check this value for 802.11ad
     *
     */
    double ccaThreshold;

    /** @brief The center frequency on which the decider listens for signals */
    double centerFrequency;

    /* @brief stats*/
    double myBusyTime;
    double myStartTime;

    std::string myPath;
    Decider80211adToPhy80211adInterface* phy11ad;
    std::map<AirFrame*, int> signalStates;

    /** @brief enable/disable statistics collection for collisions
     *
     * For collecting statistics about collisions, we compute the Packet
     * Error Rate for both SNR and SINR values. This might increase the
     * simulation time, so if statistics about collisions are not needed,
     * this variable should be set to false
     */
    bool collectCollisionStats;

    /** @brief count the number of collisions */
    unsigned int collisions;

    /** @brief notify PHY-RXSTART.indication  */
    bool notifyRxStart;

protected:
    /**
     * @brief Checks a mapping against a specific threshold (element-wise).
     *
     * @return    true    , if every entry of the mapping is above threshold
     *             false    , otherwise
     *
     *
     */
    virtual DeciderResult* checkIfSignalOk(AirFrame* frame);

    simtime_t processNewSignal(AirFrame* frame) override;

    /**
     * @brief Processes a received AirFrame.
     *
     * The SNR-mapping for the Signal is created and checked against the Deciders
     * SNR-threshold. Depending on that the received AirFrame is either sent up
     * to the MAC-Layer or dropped.
     *
     * @return    usually return a value for: 'do not pass it again'
     */
    simtime_t processSignalEnd(AirFrame* frame) override;

    /** @brief computes if packet is ok or has errors*/
    enum PACKET_OK_RESULT packetOk(double sinrMin, double snrMin, int lengthMPDU, MmWaveMCS mcs);


public:
    /**
     * @brief Initializes the Decider with a pointer to its PhyLayer and
     * specific values for threshold and minPowerLevel
     */
    Decider80211ad(cComponent* owner, DeciderToPhyInterface* phy, double minPowerLevel, double ccaThreshold, double centerFrequency, int myIndex = -1, bool collectCollisionStatistics = false)
    : BaseDecider(owner, phy, minPowerLevel, myIndex)

    , ccaThreshold(ccaThreshold)
    , centerFrequency(centerFrequency)
    , myBusyTime(0)
    , myStartTime(simTime().dbl())
    , collectCollisionStats(collectCollisionStatistics)
    , collisions(0)
    , notifyRxStart(false)
    {
        phy11ad = dynamic_cast<Decider80211adToPhy80211adInterface*>(phy);
        ASSERT(phy11ad);
    }


    void setPath(std::string myPath){
        this->myPath = myPath;
    }

    bool cca(simtime_t_cref, AirFrame*);
    int getSignalState(AirFrame* frame) override;


    void setChannelIdleStatus(bool isIdle) override;

    virtual ~Decider80211ad();


    /**
     * @brief invoke this method when the phy layer is also finalized,
     * so that statistics recorded by the decider can be written to
     * the output file
     */
    virtual void finish();

    /**
     * @brief notify PHY-RXSTART.indication
     */
    void setNotifyRxStart(bool enable);
};
}

#pragma once

#include <veins-mmwave/phy/Decider80211ad.h>
#include <veins-mmwave/utility/ConstsMmWave.h>

#include "veins/base/phyLayer/BasePhyLayer.h"
#include "veins-mmwave/mac/Mac11adToPhy11adInterface.h"
#include "veins-mmwave/phy/Decider80211adToPhy80211adInterface.h"
#include <veins-mmwave/phy/AntennaArray.h>
#include "veins/modules/analogueModel/SimpleObstacleShadowing.h"


namespace veins {

/**
 * @brief
 * Adaptation of the PhyLayer class for mmWave.
 *
 * @ingroup phyLayer
 *
 */

class PhyLayerMmWave: public BasePhyLayer, public Mac11adToPhy11adInterface, public Decider80211adToPhy80211adInterface {
public:
    void initialize(int stage) override;

protected:
    int32_t transmittingSectorID;
    int32_t receivingSectorID;

    /** @brief enable/disable detection of packet collisions */
    bool collectCollisionStatistics;

    /** @brief CCA threshold. See Decider80211ad for details */
    double ccaThreshold;

    int numberOfAntenna;

    int numberOfSectorsPerAntenna;

    enum ProtocolIds {
        IEEE_80211AD = 12126
    };

    // Which PHY layer is used? CPHY, SC, LPSC or OFDM
    PHY_Layer selected_phy;

    //TODO fixed one, in 1904, they used dynamically in MAC layer.
    MmWaveChannel selected_sent_freq = MmWaveChannel::ch2;

    /**
     * @brief Creates and returns an instance of the AnalogueModel with the
     * specified name.
     *
     * Is able to initialize the following AnalogueModels:
     */
    virtual std::unique_ptr<AnalogueModel> getAnalogueModelFromName(
            std::string name, ParameterMap& params) override;


    /**
     * @brief Creates and initializes a BeamformingPatternModel with the
     * passed parameter values.
     */
    std::unique_ptr<AnalogueModel> initializeBeamformingPatternModel(ParameterMap& params);


    std::unique_ptr<AnalogueModel> initializeSimpleObstacleShadowing(ParameterMap& params);

    std::unique_ptr<AnalogueModel> initializeVehicleObstacleShadowingMmWave(ParameterMap& params);

    /**
     * Create and return an instance of the Antenna with the specified name as a shared pointer.
     *
     * This method is called during initialization to load the Antenna specified.
     * If no special antenna has been specified, an object of the base Antenna class is instantiated, representing an isotropic antenna.
     */
    std::shared_ptr<Antenna> getAntennaFromName(std::string name,
            ParameterMap& params) override;

    std::shared_ptr<Antenna> initializeAntennaArray(ParameterMap& params);

    /**
     * @brief Get the Decider from Name. This function will be called in BasePhyLayer::initializeDecider(), which is called by void BasePhyLayer::initialize()
     * The override function call initializeDeciderMmWave function to create the special MmWave Decider.
     *
     * */
    virtual std::unique_ptr<Decider> getDeciderFromName(std::string name,
            ParameterMap& params) override;

    /**
     * Creates and returns a Decider for mmWave RAT.
     *
     * */
    virtual std::unique_ptr<Decider> initializeDeciderMmWave(
            ParameterMap& params);

    /**
     * Create a protocol-specific AirFrame
     * Overloaded to create a specialize AirFrameVlc.
     */
    std::unique_ptr<AirFrame> createAirFrame(cPacket* macPkt) override;

    /**
     * Attach a signal to the given AirFrame.
     *
     * The attached Signal corresponds to the IEEE 802.11ad standard.
     * Parameters for the signal are passed in the control info.
     * The indicated power levels are set up on the specified center frequency, as well as the neighboring 880MHz.
     *
     * @note The control info must be of type MacToPhyControlInfo11ad
     */
    void attachSignal(AirFrame* airFrame, cObject* ctrlInfo) override;

    /**
     * Filter the passed AirFrame's Signal by every registered AnalogueModel.
     *
     * Moreover, the antenna gains are calculated and added to the signal.
     * @note Only models from analogueModels are applied. Those referenced in analogueModelsThresholding are passed to the Signal to enable more efficient handling.
     *
     * @see analogueModels
     * @see analogueModelsThresholding
     */
    virtual void filterSignal(AirFrame* frame);

    simtime_t setRadioState(int rs) override;

    /**
     * @brief Enable notifications about PHY-RXSTART.indication in MAC
     * @param enable true if Mac needs to be notified about it
     */
    void notifyMacAboutRxStart(bool enable) override;

    virtual simtime_t getFrameDuration(MmWaveMCS mcs, int payloadLengthBits, uint32_t trainingLengthField) const override;
    virtual double getRxPowerToRSUAt(simtime_t time, Coord rsuPosition,
            Coord rsuOrientation, int32_t tx_rx_beamID, int32_t rx_tx_beamID) override;

    virtual double getReceivedPowerAfterApplyingAnalogueModels(double requestPower, const POA& senderPOA, const POA& receiverPOA) override;
    virtual uint32_t getTransmissionDirID(Coord senderPosition, Coord senderOrientation, uint32_t transmittingSectorID, Coord receiverPosition) override;

    virtual uint32_t getBestBeamByDirection(uint32_t dirID) override;

    virtual uint32_t getBestBeamByTheOtherPosition(Coord otherPosition) override;

    virtual uint32_t getBestBeamByTheOtherPositionAt(Coord otherPosition, simtime_t atTheTime) override;

    virtual Coord getCurrentPosition() override;

    virtual Coord getPositionAt(simtime_t time) override;

    virtual Coord getCurrentOrientation() override;

    virtual double getDataRateFromRx(double rx) override;

    virtual MmWaveMCS getOptimalMCSFromRx(double rx) override;

    virtual double getReceivePowerdBm(float distance, int32_t dir_tx_rx, int32_t dir_rx_tx, int32_t beam_tx_rx, int32_t beam_rx_tx, bool norm) override;

    virtual void setReceivingSectorID(int sectorID) override;

    int32_t getReceivingSectorID();

    virtual void setTransmittingSectorID(int sectorID) override;

    virtual AntennaPosition getAntennaPosition() override;

    int getRadioState() override;
    void handleSelfMessage(cMessage* msg) override;
    /**
     * Handle messages received from the upper layer and start transmitting them.
     */
//    void handleUpperMessage(cMessage* msg) override;

    /**
     * @brief Explicit request to PHY for the channel status
     */
    void requestChannelStatusIfIdle() override;

    uint32_t getNumberOfAntenna() override;
    uint32_t getNumberOfSectorsPerAntenna(uint32_t antennaID) override;

private:
//    std::array<std::array<int64_t, 2>, 32> parseMCSDataFromFile(std::string filePath);
//    std::array<std::array<float, 5>, 427> parseMapAzSNRFromFile(std::string filePath);
//    std::array<float, 3> mmwave_channel_generator(float distance, uint32_t dir_id, uint32_t beam_id, std::shared_ptr<AntennaArray> antenna);

    /**
     * @brief This function is to calculate the number of LDPC codewords (N_CW) for Control PHY
     * as defined in 21.4.3.3.3 Encoder in 802.11ad 2012 standard.
     */
    uint32_t getNumberOfLDPCCodewords(int payloadLengthBytes) const;
};
}

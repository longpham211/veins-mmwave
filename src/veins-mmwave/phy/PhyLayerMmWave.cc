#include <veins-mmwave/analogueModel/BeamformingPatternModel.h>
#include <veins-mmwave/analogueModel/VehicleObstacleShadowingMmWave.h>
#include <veins-mmwave/phy/PhyLayerMmWave.h>
#include <veins-mmwave/messages/AirFrame80211ad_m.h>

#include <veins-mmwave/utility/MacToPhyControlInfo11ad.h>
#include <veins-mmwave/utility/POAS.h>
#include <veins-mmwave/utility/Signal80211ad.h>

#include "veins/base/utils/FWMath.h"
#include <veins/modules/obstacle/VehicleObstacleControl.h>
#include <random>

using namespace veins;

using std::unique_ptr;

Define_Module(veins::PhyLayerMmWave);

void PhyLayerMmWave::initialize(int stage) {
    if (stage == 0) {
        if (stage == 0) {
            EV_INFO << "Long; initialize stage 0" << endl;

            collectCollisionStatistics =
                    par("collectCollisionStatistics").boolValue();

            // get ccaThreshold before calling BasePhyLayer::initialize() which instantiates the deciders
            ccaThreshold = pow(10, par("ccaThreshold").doubleValue() / 10);


            numberOfAntenna = par("numberOfAntenna").intValue();
            numberOfSectorsPerAntenna =
                    par("numberOfSectorsPerAntenna").intValue();


            // Create frequency mappings and initialize spectrum for signal representation
            // 60GHz.

            Spectrum::Frequencies freqs;
            for(auto& channel : IEEE80211adChannelFrequencies) {
                freqs.push_back(channel.second - CHANNEL_BANDWIDTH_80211AD/2);
                freqs.push_back(channel.second);
                freqs.push_back(channel.second + CHANNEL_BANDWIDTH_80211AD/2);
            }

            overallSpectrum = Spectrum(freqs);

            // Use the CPHY as the default
            selected_phy = PHY_Layer::control;

            transmittingSectorID = receivingSectorID = OMNIDIRECTIONAL_ANTENNA;

        }
        BasePhyLayer::initialize(stage);
    }
}

unique_ptr<AnalogueModel> PhyLayerMmWave::getAnalogueModelFromName(
        std::string name, ParameterMap& params) {
    if(name == "BeamformingPatternModel")
        return initializeBeamformingPatternModel(params);
    else if (name == "SimpleObstacleShadowing") {
        return initializeSimpleObstacleShadowing(params);
    }
    else if (name == "VehicleObstacleShadowingMmWave") {
        return initializeVehicleObstacleShadowingMmWave(params);
    }

    return BasePhyLayer::getAnalogueModelFromName(name, params);
}

unique_ptr<AnalogueModel> PhyLayerMmWave::initializeBeamformingPatternModel(
        ParameterMap& params) {

    return make_unique<BeamformingPatternModel>(this);
}

std::unique_ptr<AnalogueModel> veins::PhyLayerMmWave::initializeSimpleObstacleShadowing(
        ParameterMap& params) {
    // init with default value
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());

    ParameterMap::iterator it;

    ObstacleControl* obstacleControlP = ObstacleControlAccess().getIfExists();
    if (!obstacleControlP) throw cRuntimeError("initializeSimpleObstacleShadowing(): cannot find ObstacleControl module");
    return make_unique<SimpleObstacleShadowing>(this, *obstacleControlP, useTorus, playgroundSize);
}

std::unique_ptr<AnalogueModel> veins::PhyLayerMmWave::initializeVehicleObstacleShadowingMmWave(
        ParameterMap &params) {
    // init with default value
    bool useTorus = world->useTorus();
    const Coord& playgroundSize = *(world->getPgs());

    VehicleObstacleControl* vehicleObstacleControlP = VehicleObstacleControlAccess().getIfExists();
    if (!vehicleObstacleControlP) throw cRuntimeError("initializeVehicleObstacleShadowingMmWave(): cannot find VehicleObstacleControl module");
    return make_unique<VehicleObstacleShadowingMmWave>(this, *vehicleObstacleControlP, useTorus, playgroundSize);
}

unique_ptr<Decider> PhyLayerMmWave::getDeciderFromName(std::string name,
        ParameterMap& params) {
    EV_TRACE << "getDeciderFromName " << name << std::endl;

    if (name == "Decider80211ad") {
        protocolId = IEEE_80211AD;
        return initializeDeciderMmWave(params);
    }

    return BasePhyLayer::getDeciderFromName(name, params);
}

unique_ptr<Decider> PhyLayerMmWave::initializeDeciderMmWave(
        ParameterMap& params) {
    double centerFreq = params["centerFrequency"];
    auto dec = make_unique<Decider80211ad> (this, this, minPowerLevel, ccaThreshold, centerFreq,
            findHost()->getIndex(), collectCollisionStatistics);
    dec->setPath(getParentModule()->getFullPath());

    return unique_ptr<Decider>(std::move(dec));
}

std::shared_ptr<Antenna> PhyLayerMmWave::getAntennaFromName(std::string name,
        ParameterMap& params) {

    if (name == "AntennaArray")
        return initializeAntennaArray(params);

    else
        error("IEEE 802.11ad just uses Antenna Array only!");
}

std::shared_ptr<Antenna> veins::PhyLayerMmWave::initializeAntennaArray(
        ParameterMap& params) {

    ParameterMap::iterator it;
    std::string beamSectorPatternFolder;
    std::string mcsMapFile;

    it = params.find("beamSectorPatternFolder");
    if(it != params.end())
        beamSectorPatternFolder = it->second.stringValue();
    else
        error ("No beamSectorPatternFolder parameter in Antenna Array");

    it = params.find("mcsMap");
    if(it != params.end())
        mcsMapFile = it->second.stringValue();
    else
        error ("No mcsMap parameter in BeamformingPatternModel");

    return std::make_shared<AntennaArray>(beamSectorPatternFolder, mcsMapFile);
}

void veins::PhyLayerMmWave::handleSelfMessage(cMessage* msg) {
    switch (msg->getKind()) {
    // transmission over BasePhyLayer
    case TX_OVER: {
        ASSERT(msg == txOverTimer);
        sendControlMsgToMac(new cMessage("Transmission over", MacToPhyInterface::TX_OVER));

        //check if there is another packet on the channel, and change the channel state to idle
        Decider80211ad* dec = dynamic_cast<Decider80211ad*>(decider.get());
        ASSERT(dec);

        if(dec->cca(simTime(), nullptr)) {
            //channel is idle
            EV_TRACE << "Channel is idle after transmit!" << std::endl;
            dec->setChannelIdleStatus(true);
        }
        else
            EV_TRACE << "Channel is not yet idle after transmit!" << std::endl;

        break; // end of case TX_OVER
    }

    case AIR_FRAME:
        BasePhyLayer::handleAirFrame(static_cast<AirFrame*> (msg));
        break;

    // radio switch over
    case RADIO_SWITCHING_OVER:
        ASSERT(msg == radioSwitchingOverTimer);
        BasePhyLayer::finishRadioSwitching();
        break;

    default:
        break;
    }
}


unique_ptr<AirFrame> PhyLayerMmWave::createAirFrame(cPacket* macPkt) {
    return make_unique <AirFrame80211ad> (macPkt->getName(), AIR_FRAME);
}

void veins::PhyLayerMmWave::attachSignal(AirFrame* airFrame,
        cObject* ctrlInfo) {
    const auto ctrlInfo11ad = check_and_cast<MacToPhyControlInfo11ad*>(ctrlInfo);

    //TODO getting training length field here
    const auto duration = getFrameDuration(ctrlInfo11ad->mcs, airFrame->getEncapsulatedPacket()->getBitLength(), 0);

    ASSERT (duration > 0);
    Signal signal(overallSpectrum, simTime(), duration);
//    Signal80211ad signal(overallSpectrum, simTime(), duration, ctrlInfo11ad->sectorID, -1);
    //TODO change here if you want a dynamic channels
    auto freqIdx = overallSpectrum.indexOf(IEEE80211adChannelFrequencies.at(selected_sent_freq));
    signal.at(freqIdx - 1) = ctrlInfo11ad->txPower_mW;
    signal.at(freqIdx) = ctrlInfo11ad->txPower_mW;
    signal.at(freqIdx + 1) = ctrlInfo11ad->txPower_mW;
    signal.setDataStart(freqIdx - 1);
    signal.setDataEnd(freqIdx + 1);
    signal.setCenterFrequencyIndex(freqIdx);

    //cope the signal into the AirFrame
    airFrame->setSignal(signal);
    airFrame->setDuration(signal.getDuration());
    airFrame->setMcs(static_cast<int>(ctrlInfo11ad->mcs));
    dynamic_cast<AirFrame80211ad*>(airFrame)->setSectorID(ctrlInfo11ad->sectorID);
}

void veins::PhyLayerMmWave::filterSignal(AirFrame* frame) {
    ASSERT(dynamic_cast<ChannelAccess* const>(frame->getArrivalModule()) == this);
    ASSERT(dynamic_cast<ChannelAccess* const>(frame->getSenderModule()));


    Signal& signal = frame->getSignal();

    // Extract position and orientation of sender and receiver (this module) first
    const AntennaPosition receiverPosition = antennaPosition;
    const Coord receiverOrientation = antennaHeading.toCoord();
    // get POA from frame with the sender's position, orientation and antenna
    POA& senderPOA = frame->getPoa();
    const AntennaPosition senderPosition = senderPOA.pos;
    const Coord senderOrientation = senderPOA.orientation;

    // add position information to signal
    signal.setSenderPoa(senderPOA);
    signal.setReceiverPoa({receiverPosition, receiverOrientation, antenna});

    // compute gains at sender and receiver antenna
    double receiverGain = antenna->getGain(receiverPosition.getPositionAt(), receiverOrientation, senderPosition.getPositionAt());
    double senderGain = senderPOA.antenna->getGain(senderPosition.getPositionAt(), senderOrientation, receiverPosition.getPositionAt());

    EV_TRACE<<"PhyLayerMmWave attachSignal: "<< signal.getAtCenterFrequency()<< std::endl;

    signal *= receiverGain * senderGain;

    // go on with AnalogueModels
    // attach analogue models suitable for thresholding to signal (for later evaluation)
    signal.setAnalogueModelList(&analogueModelsThresholding);

    // apply all analouge models that are *not* suitable for thresholding now
    for (auto& analogueModel : analogueModels) {
        analogueModel->filterSignal(&signal);
    }

//    If this one is 802.11ad, we need to filter the signal, using mmwave_channel_generator.
//    this function requires beam id information, which we cannot attach to the POA because
//    POA in Signal class cannot castable to an inherited object (not reference, or pointers)
//     and we cannot inherit Signal class because it doesn't contain virtual function
    if(isKnownProtocolId(frame->getProtocolId())) {
        auto* senderAntenna = dynamic_cast<AntennaArray*>(senderPOA.antenna.get());

        Coord los = receiverPosition.getPositionAt() - senderPosition.getPositionAt();
        double transmittingAngle = atan2(los.y, los.x) - atan2(senderOrientation.y, senderOrientation.x);
        transmittingAngle = fmod(transmittingAngle, 2 * M_PI);
        if (transmittingAngle < 0) transmittingAngle += 2 * M_PI;
        transmittingAngle = transmittingAngle * 360 / (2 * M_PI); //convert angle to radius

        double distance = senderPosition.getPositionAt().distance(receiverPosition.getPositionAt());
        int32_t transmittingSectorID = dynamic_cast<AirFrame80211ad*>(frame)->getSectorID();
        int32_t transmittingDirID = senderAntenna->getDirIdFromAngle(transmittingSectorID, transmittingAngle);

        int32_t receivingDirID = getTransmissionDirID(receiverPosition.getPositionAt(), receiverOrientation, receivingSectorID, senderPosition.getPositionAt());

        double receivedPowerSignalIndBmNewMethod = senderAntenna->getReceivePowerdBm(distance, transmittingDirID, receivingDirID, transmittingSectorID, receivingSectorID, true);

        signal = FWMath::dBm2mW(receivedPowerSignalIndBmNewMethod);

        EV_TRACE<<"PhyLayerMmWave filter Signal: "
//               << " \ntransmitting angle: "<< transmittingAngle
               << " \ntransmitting direction Id: "<<transmittingDirID
               << " \ntransmitting beam id: "<< transmittingSectorID
//               << " \nreceiving angle: "<< receivingAngle
               << " \nreceiving direction Id: "<<receivingDirID
               << " \nreceiving beam id: "<< receivingSectorID
               << " \ndistance: " << distance
               << " \nsender position: " << senderPosition.getPositionAt().info()
               << " \nreceiver position: " << receiverPosition.getPositionAt().info()
               << " \nreceivedPowerSignalInDbm: "<< receivedPowerSignalIndBmNewMethod << std::endl;
               //<< " \nreceivedPowerSignalInmW: "<< signal.getAtCenterFrequency()<<std::endl;

    }
}


int veins::PhyLayerMmWave::getRadioState() {
    return BasePhyLayer::getRadioState();
}

simtime_t PhyLayerMmWave::setRadioState(int rs) {
    if (rs == Radio::TX)
        decider->switchToTx();
    return BasePhyLayer::setRadioState(rs);
}

void veins::PhyLayerMmWave::notifyMacAboutRxStart(bool enable) {
    Decider80211ad* dec = dynamic_cast<Decider80211ad*>(decider.get());
    ASSERT(dec);
    dec->setNotifyRxStart(enable);
}


uint32_t PhyLayerMmWave::getNumberOfAntenna() {
    return numberOfAntenna;
}

uint32_t PhyLayerMmWave::getNumberOfSectorsPerAntenna(uint32_t antennaID) {
    return numberOfSectorsPerAntenna;
}


simtime_t veins::PhyLayerMmWave::getFrameDuration(MmWaveMCS mcs, int payloadLengthBits,
        uint32_t trainingLengthField) const {
    Enter_Method_Silent
    ();

    simtime_t duration = -1;

    /**
     * @brief calculate frame duration according to 21.12.3 TXTIME calculation
     *
     * as defined in the IEEE 802.11ad 2012
     */

    if (isCPHY_MCS(mcs)) {
            //TODO Length is in byte or in bits?

            duration = CPHY_STF_DURATION + CPHY_CE_DURATION
                    + (11 * 8 + (ceil(static_cast<double>(payloadLengthBits) / 8) - 6) * 8
                            + getNumberOfLDPCCodewords(ceil(static_cast<double>(payloadLengthBits) / 8)) * 168)
                            * SCPHY_CHIP_TIME * 32
                    + trainingLengthField * SCPHY_CHIP_TIME;
    }
    else {
        if (isSC_MCS(mcs) || isLPSC_MCS(mcs)) {
            if(trainingLengthField == 0) {

                //following in 21.6.3.2.3.3 ieee 802.11ad standard
                uint32_t codewordLength = 672;
                uint32_t numberOfCodeWord = ceil(static_cast<double>(ceil(static_cast<double>(payloadLengthBits)/8) * 8)
                                                    / (codewordLength/getMCSRepetitionFactor(mcs) * getMCSCodeRate(mcs)));

                uint32_t n_blks = ceil(static_cast<double>(numberOfCodeWord) * codewordLength / getMCS_NCBPB(mcs));

                //Table 21-4—Timing-related parameters
                double t_data = (n_blks * 512 + 64) * SCPHY_CHIP_TIME;

                duration = STF_DURATION + CE_DURATION + HEADER_DURATION_SC + t_data;
            }

            else {
                ASSERT2(false, "Invalid trainingLengthField for getFrameDurationFunction. It hasn't implemented yet");
                duration = 0;
            }
        }

        //OFDM
        else {
            if(trainingLengthField == 0) {
                //following in 21.5.3.2.3.3 ieee 802.11ad standard
                double R = 1;
                uint64_t codewordLength = 672;
                uint64_t numberOfCodeWord = ceil(static_cast<double>(ceil(static_cast<double>(payloadLengthBits)/8) * 8)
                                                    / (codewordLength * getMCSCodeRate(mcs)));

                uint64_t n_sym = ceil(static_cast<double>(numberOfCodeWord) * codewordLength / getMCS_NCBPB(mcs));

                //Table 21-4—Timing-related parameters
                double t_data = n_sym * SYMBOL_INTERVAL;

                duration = STF_DURATION + CE_DURATION + HEADER_DURATION_OFDM + t_data;
            }

            else {
                ASSERT2(false, "Invalid trainingLengthField for getFrameDurationFunction. It hasn't implemented yet");
                duration = 0;
            }
        }
    }
//    EV_TRACE << "getDurationOfFrame: " << payloadLengthBits << " bits, takes " << duration << " seconds." << endl;
        return duration;
}

uint32_t veins::PhyLayerMmWave::getBestBeamByTheOtherPosition(
        Coord otherPosition) {
    return getBestBeamByTheOtherPositionAt(otherPosition, simTime());
}


uint32_t veins::PhyLayerMmWave::getBestBeamByTheOtherPositionAt(
        Coord otherPosition, simtime_t atTheTime) {

    const Coord senderPosition = antennaPosition.getPositionAt(atTheTime);
    const Coord senderOrientation = antennaHeading.toCoord();
    int32_t transmittingDirID = getTransmissionDirID(senderPosition, senderOrientation, 0, otherPosition); // all beams have the same dirID, pick 0

    return getBestBeamByDirection(transmittingDirID);
}

/**
 * @brief This function is to calculate the number of LDPC codewords (N_CW) for Control PHY
 * as defined in 21.4.3.3.3 Encoder in 802.11ad 2012 standard.
 */
uint32_t veins::PhyLayerMmWave::getNumberOfLDPCCodewords(
        int payloadLengthBytes) const {
    return 1 + ceil((static_cast<double>(payloadLengthBytes) - 6) * 8 / 168);
}

void veins::PhyLayerMmWave::setReceivingSectorID(int sectorID) {
    receivingSectorID = sectorID;
}

void veins::PhyLayerMmWave::setTransmittingSectorID(int sectorID) {
    transmittingSectorID = sectorID;
}

double veins::PhyLayerMmWave::getRxPowerToRSUAt(simtime_t time, Coord rsuPosition,
        Coord rsuOrientation, int32_t tx_rx_beamID, int32_t rx_tx_beamID) {

    AntennaArray* castedAntenna = dynamic_cast<AntennaArray*>(antenna.get());

//    const AntennaPosition senderPosition = antennaPosition;
    const AntennaPosition receiverPosition = antennaPosition;
    // well, this is quite wrong, because the heading is not at the input time, but at the current time,
    // but it seems like we don't have any way to change this, because the heading is calculated using the mobility,
    // while mobility doesn't provide anyway to calculate the heading at a given time.
    // But I think it's acceptable, because if the vehicles run on the straight way, then their heading should not
    // be changed that much, isn't it?
//    const Coord senderOrientation = antennaHeading.toCoord();
    const Coord receiverOrientation = antennaHeading.toCoord();


    double distance = receiverPosition.getPositionAt(time).distance(rsuPosition);

    int32_t transmittingDirID = getTransmissionDirID(rsuPosition, rsuOrientation, tx_rx_beamID, receiverPosition.getPositionAt(time));
    double receivedPowerSignalIndBmNewMethod;

    if(rx_tx_beamID != OMNIDIRECTIONAL_ANTENNA){
        int32_t receivingDirID = getTransmissionDirID(receiverPosition.getPositionAt(time), receiverOrientation, rx_tx_beamID, rsuOrientation);
        receivedPowerSignalIndBmNewMethod = castedAntenna->getReceivePowerdBm(distance, transmittingDirID, receivingDirID, tx_rx_beamID, rx_tx_beamID, false);
    }
    else
    receivedPowerSignalIndBmNewMethod = castedAntenna->getReceivePowerdBm(distance, transmittingDirID, OMNIDIRECTIONAL_ANTENNA, tx_rx_beamID, OMNIDIRECTIONAL_ANTENNA, false);
    //double receivedPowerSignalInMW = FWMath::dBm2mW(receivedPowerSignalIndBmNewMethod);

    // This function should be called only to test the beam coverage in FML,
    // Therefore, we don't need to care for the obstacles (building and vehicles) analogue models

    //double updatedPowerIndBm = FWMath::mW2dBm(signal.getAtCenterFrequency());
    EV_TRACE << "receivedPower with path loss only (no obstacles considering): " << receivedPowerSignalIndBmNewMethod << std::endl;
            //"\n receivedPower after obstacles' reduction: " << updatedPowerIndBm << std::endl;

    //return updatedPowerIndBm;
    return receivedPowerSignalIndBmNewMethod;

}


uint32_t veins::PhyLayerMmWave::getTransmissionDirID(Coord senderPosition,
        Coord senderOrientation, uint32_t transmittingSectorID, Coord receiverPosition) {
    Coord los = receiverPosition - senderPosition;

    double transmittingAngle = atan2(los.y, los.x) - atan2(senderOrientation.y, senderOrientation.x);
    transmittingAngle = fmod(transmittingAngle, 2 * M_PI);
    if (transmittingAngle < 0) transmittingAngle += 2 * M_PI;
    transmittingAngle = transmittingAngle * 360 / (2 * M_PI); //convert angle to radius

    //This is not quite correct in the case if antennas of nodes are different
    return dynamic_cast<AntennaArray*>(antenna.get())->getDirIdFromAngle(transmittingSectorID, transmittingAngle);
}

double veins::PhyLayerMmWave::getDataRateFromRx(double rx) {
    std::array<std::array<int64_t, 2>, 32>& mcsMap = dynamic_cast<AntennaArray*>(antenna.get())->getMCSMap();

    double dataRate = 0;
    for(uint32_t i = 0; i < /*mcsMap.size()*/ 12; i++) // only SC for now
//        if(rx >= mcsMap[i][MCS_MAP_RECEIVER_SENSITIVITY_INDEX])
        if(rx >= mcsMap[i][MCS_MAP_RECEIVER_SENSITIVITY_INDEX] && mcsMap[i][MCS_MAP_BITRATE_INDEX] > dataRate)
            dataRate = mcsMap[i][MCS_MAP_BITRATE_INDEX];
//        else
//            break;

    return dataRate;
}

MmWaveMCS veins::PhyLayerMmWave::getOptimalMCSFromRx(double rx) {
    std::array<std::array<int64_t, 2>, 32>& mcsMap = dynamic_cast<AntennaArray*>(antenna.get())->getMCSMap();

    uint32_t mcsIndex = 0;
    for(uint32_t i = 0; i < 13; i++)
        if(rx >= mcsMap[i][MCS_MAP_RECEIVER_SENSITIVITY_INDEX])
            mcsIndex = i;
//    for(uint32_t i = 22; i < 25; i++)
//        if(rx >= mcsMap[i][MCS_MAP_RECEIVER_SENSITIVITY_INDEX])
//            mcsIndex = i;

    return (MmWaveMCS) mcsIndex;
}

Coord veins::PhyLayerMmWave::getCurrentPosition() {
    return antennaPosition.getPositionAt();
}

Coord veins::PhyLayerMmWave::getPositionAt(simtime_t time) {
    return antennaPosition.getPositionAt(time);
}

Coord veins::PhyLayerMmWave::getCurrentOrientation() {
    return antennaHeading.toCoord();
}

uint32_t veins::PhyLayerMmWave::getBestBeamByDirection(uint32_t dirID) {
    return dynamic_cast<AntennaArray*>(antenna.get())->getBestBeamByDirection(dirID, AZ_SNR_MAP_RX_F60_INDEX);
}

void veins::PhyLayerMmWave::requestChannelStatusIfIdle() {
    Enter_Method_Silent();
    Decider80211ad* dec = dynamic_cast<Decider80211ad*>(decider.get());
    ASSERT(dec);
    if (dec->cca(simTime(), nullptr)) {
        // chan is idle
        EV_TRACE << "Request channel status: channel idle!\n";
        dec->setChannelIdleStatus(true);
    }
}

double veins::PhyLayerMmWave::getReceivePowerdBm(float distance,
        int32_t dir_tx_rx, int32_t dir_rx_tx, int32_t beam_tx_rx,
        int32_t beam_rx_tx, bool norm) {
    return dynamic_cast<AntennaArray*>(antenna.get())->getReceivePowerdBm(distance, dir_tx_rx, dir_rx_tx, beam_tx_rx, beam_rx_tx, norm);
}

double veins::PhyLayerMmWave::getReceivedPowerAfterApplyingAnalogueModels(
        double requestPower, const POA& senderPOA, const POA& receiverPOA) {
    double receivedPowerSignalInMW = FWMath::dBm2mW(requestPower);

    Signal signal(overallSpectrum, simTime(), 0);
    auto freqIdx = overallSpectrum.indexOf(IEEE80211adChannelFrequencies.at(selected_sent_freq));
    signal.at(freqIdx - 1) = receivedPowerSignalInMW;
    signal.at(freqIdx) = receivedPowerSignalInMW;
    signal.at(freqIdx + 1) = receivedPowerSignalInMW;
    signal.setDataStart(freqIdx - 1);
    signal.setDataEnd(freqIdx + 1);
    signal.setCenterFrequencyIndex(freqIdx);

    signal.setReceiverPoa(receiverPOA);
    signal.setSenderPoa(senderPOA);
    signal.setAnalogueModelList(&analogueModelsThresholding);
    signal.applyAllAnalogueModels(); // VehicleObstacles + ObstacleShadowing

    for (auto& analogueModel : analogueModels) {
        analogueModel->filterSignal(&signal);
    }

    return FWMath::mW2dBm(signal.getAtCenterFrequency());
}

AntennaPosition veins::PhyLayerMmWave::getAntennaPosition() {
    return antennaPosition;
}

int32_t veins::PhyLayerMmWave::getReceivingSectorID() {
    return receivingSectorID;
}

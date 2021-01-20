#include <veins-mmwave/phy/Decider80211ad.h>
#include "veins/base/toolbox/SignalUtils.h"
#include "veins-mmwave/messages/AirFrame80211ad_m.h"
#include "veins/modules/phy/DeciderResult80211.h"

using namespace veins;


simtime_t veins::Decider80211ad::processNewSignal(AirFrame* msg) {
    AirFrame80211ad* frame = check_and_cast<AirFrame80211ad*> (msg);

    // get the receiving power of the Signal at start-time and center frequency
    Signal& signal = frame->getSignal();

    signalStates[frame] = EXPECT_END;

    if(signal.smallerAtCenterFrequency(minPowerLevel)) {
        EV_TRACE << "AirFrame: " << frame->getId()<< " -> AirFrame can't be detected by the radio; discarded at its end." << std::endl;
        // annotate the frame, so that we won't try decoding it at its end
        frame->setUnderMinPowerLevel(true);

        // check channel busy status. a superposition of low power frames might turn channel status to busy
        if (cca(simTime(), nullptr) == false)
            setChannelIdleStatus(false);

        return signal.getReceptionEnd();
    }
    else {
        // This value might be just an intermediate result (due to short circuiting)
        double recvPower = signal.getAtCenterFrequency();
        setChannelIdleStatus(false);

        if(phy11ad->getRadioState() == Radio::TX) {
            frame->setBitError(true);
            frame->setWasTransmitting(true);
            EV_TRACE << "AirFrame: " << frame->getId() << " (" << recvPower << ") received, while already sending. Setting BitErrors to true" << std::endl;
        }
        else {
            if(!currentSignal.first) {
                // NIC is not yet synced to any frame, so lock and try to decode this frame
                currentSignal.first = frame;
                EV_TRACE << "AirFrame: " << frame->getId() << " with (" << recvPower << " > " << minPowerLevel << ") -> Trying to receive AirFrame." << std::endl;
                if(notifyRxStart) {
                    phy->sendControlMsgToMac(new cMessage("RxStartStatus", MacToPhyInterface::PHY_RX_START));
                }
            }
            else {
                // NIC is currently trying to decode another frame. this frame will be simply treated as interference
                EV_TRACE << "AirFrame: " << frame->getId() << " with (" << recvPower << " > " << minPowerLevel << ") -> Already synced to another AirFrame. Treating AirFrame as interference." << std::endl;
            }

            // channel turned busy
            // measure communication density
            myBusyTime += signal.getDuration().dbl();
        }
        return signal.getReceptionEnd();
    }
}

int veins::Decider80211ad::getSignalState(AirFrame* frame) {
    if(signalStates.find(frame) == signalStates.end())
        return SignalState::NEW;

    return signalStates[frame];
}

DeciderResult* veins::Decider80211ad::checkIfSignalOk(AirFrame* frame) {
//    auto frame80211ad = check_and_cast<AirFrame80211ad*>(frame);

    Signal& s = frame->getSignal();
    simtime_t start = s.getReceptionStart();
    simtime_t end = s.getReceptionEnd();

    // compute receive power
    double recvPower_dBm = FWMath::mW2dBm(s.getAtCenterFrequency());

    // its ok if something in the training phase is broken TODO really? if yes, then we must pas the selected PHY type, that we used to send this frame because different PHY has different PREAMBLE DURATION
    if(isCPHY_MCS(static_cast<MmWaveMCS>(frame->getMcs()))) {
        start = start + CPHY_STF_DURATION + CPHY_CE_DURATION;
    }
    else {
        start = start + STF_DURATION + CE_DURATION;
    }

    AirFrameVector airFrames;
    getChannelInfo(start, end, airFrames);

    double noise = phy->getNoiseFloorValue();

    // Make sure to use the adjusted starting-point (which ignores the preamble)
    double sinrMin = SignalUtils::getMinSINR(start, end, frame, airFrames, noise);
    double snrMin;
    if(collectCollisionStats)
        snrMin = s.getDataMin() / noise;
    else
        // just set to any value. if collectCollisionStats != true
        // it will be ignored by packetOk
        snrMin = 1e200;

    DeciderResult80211* result = nullptr;

    enum Decider80211ad::PACKET_OK_RESULT packetResult;
    double mcsReceiverSensitivity = FWMath::dBm2mW(getMCSReceiverSensitivity(static_cast<MmWaveMCS>(frame->getMcs())));

    EV_TRACE << "Long trace: the sinrMin = " << sinrMin
            << " mcs = " << frame->getMcs()
            << " mcs sensitivity dB = " << FWMath::mW2dBm(mcsReceiverSensitivity)
            << " recvPowerdBm: " << recvPower_dBm << std::endl;

        if(sinrMin >= 1)
            if(s.getAtCenterFrequency() >= mcsReceiverSensitivity)
                packetResult = DECODED;
            else
                packetResult = NOT_DECODED;
        else
            if(collectCollisionStats && snrMin >= 1)
                packetResult = COLLISION;
            else
                packetResult = NOT_DECODED;

//    switch(packetOk(sinrMin, snrMin, frame->getBitLength(), static_cast<MmWaveMCS>(frame->getMcs()))) {
    switch(packetResult) {
    case DECODED:
        //TODO should get the standard rate, or from the generator file?
        EV_TRACE << "Packet is fine! We can decode it" << std::endl;
        result = new DeciderResult80211(true, getMCSDatarate(static_cast<MmWaveMCS>(frame->getMcs())), sinrMin, recvPower_dBm, false);
        break;

    case NOT_DECODED:
        if (!collectCollisionStats) {
            EV_TRACE << "Packet has bit Errors. Lost" << std::endl;
        }
        else {
            EV_TRACE << "Packet has bit Errors due to low power. Lost " << std::endl;
        }
        result = new DeciderResult80211(false, getMCSDatarate(static_cast<MmWaveMCS>(frame->getMcs())), sinrMin, recvPower_dBm, false);
        break;

    case COLLISION:
        EV_TRACE << "Packet has bit Errors due to collision. Lost " << std::endl;
        collisions++;
        result = new DeciderResult80211(false, getMCSDatarate(static_cast<MmWaveMCS>(frame->getMcs())), sinrMin, recvPower_dBm, true);
        break;

    default:
        ASSERT2(false, "Impossible packet result returned by packetOk(). Check the code.");
        break;
    }

    return result;
}


enum Decider80211ad::PACKET_OK_RESULT veins::Decider80211ad::packetOk(double sinrMin,
        double snrMin, int lengthMPDU, MmWaveMCS mcs) {

    double packetOkSinr;
    double packetOkSnr;

    uint32_t headerLength = 0;

    switch(mcs) {
    case MmWaveMCS::cphy_mcs_0:
        headerLength = PHY_80211AD_CPHY_PHR;
        break;
    case MmWaveMCS::sc_mcs_1:
    case MmWaveMCS::sc_mcs_2:
    case MmWaveMCS::sc_mcs_3:
    case MmWaveMCS::sc_mcs_4:
    case MmWaveMCS::sc_mcs_5:
    case MmWaveMCS::sc_mcs_6:
    case MmWaveMCS::sc_mcs_7:
    case MmWaveMCS::sc_mcs_8:
    case MmWaveMCS::sc_mcs_9:
    case MmWaveMCS::sc_mcs_10:
    case MmWaveMCS::sc_mcs_11:
    case MmWaveMCS::sc_mcs_12:
        headerLength = PHY_80211AD_SCPHY_PHR;
        break;
    default:
        ASSERT2(false, "Implement for LP SC and OFDM!");
    }

    //compute success rate depending on the mcs
    packetOkSinr = NistErrorRate80211ad::getChunkSuccessRate(mcs, sinrMin, lengthMPDU);

    // check if header is broken
    double headerNoError = NistErrorRate80211ad::getChunkSuccessRate(mcs, sinrMin, headerLength);
    double headerNoErrorSnr;

    // compute PER also for SNR only
    if (collectCollisionStats) {
        packetOkSnr = NistErrorRate80211ad::getChunkSuccessRate(mcs, snrMin, lengthMPDU);
        headerNoErrorSnr = NistErrorRate80211ad::getChunkSuccessRate(mcs, snrMin, headerLength);

        // the probability of correct reception without considering the interference
        // MUST be greater or equal than when consider it
        ASSERT(packetOkSnr >= packetOkSinr);
        ASSERT(headerNoErrorSnr >= headerNoError);
    }

    //TODO how PLCP header related to ieee 802.11ad?
    // probability of no bit error in the PLCP header
    double rand = (cSimulation::getActiveSimulation()->getContext())-> dblrand();

    if (!collectCollisionStats) {
        if (rand > headerNoError) return NOT_DECODED;
    }
    else {

        if (rand > headerNoError) {
            // ups, we have a header error. is that due to interference?
            if (rand > headerNoErrorSnr) {
                // no. we would have not been able to receive that even
                // without interference
                return NOT_DECODED;
            }
            else {
                // yes. we would have decoded that without interference
                return COLLISION;
            }
        }
    }

    // probability of no bit error in the rest of the packet
    rand = (cSimulation::getActiveSimulation()->getContext())-> dblrand();

    if (!collectCollisionStats) {
        if (rand > packetOkSinr) {
            return NOT_DECODED;
        }
        else {
            return DECODED;
        }
    }
    else {

        if (rand > packetOkSinr) {
            // ups, we have an error in the payload. is that due to interference?
            if (rand > packetOkSnr) {
                // no. we would have not been able to receive that even
                // without interference
                return NOT_DECODED;
            }
            else {
                // yes. we would have decoded that without interference
                return COLLISION;
            }
        }
        else {
            return DECODED;
        }
    }
}

bool veins::Decider80211ad::cca(simtime_t_cref time, AirFrame* exclude) {
    AirFrameVector airFrames;

    // collect all AirFrames that intersect with [start, end]
    getChannelInfo(time, time, airFrames);

    // In the reference implementation only centerFrequenvy - 5e6 (half bandwidth) is checked!
    // Although this is wrong, the same is done here to reproduce original results
    // TODO Because of highly directional beam signal pattern, we need to check this case
    // A -> B, C is near A and B, but because of the beam directional then C's channel might be free
    // is getChannelInfor is locally related to a station or it is a universal class? which handles
    // all AirFrames of all stations?
    //
    // because the getChannelInfo function is related to BasePhyLayer::handleAirFrameStartReceive
    // which is related to handleAirFrame --> so probably, it's received to (Base)ConnectionManager.cc
    // check overridable calInterfDist; registerNicExt, updateConnections, isInRange

    double minPower = phy->getNoiseFloorValue();
    bool isChannelIdle = minPower < ccaThreshold;
    if (airFrames.size() > 0) {
        for (auto& airFrame : airFrames) {
            EV_TRACE <<"Long test: protocolID: " << airFrame->getProtocolId() << std::endl;
        }
        size_t usedFreqIndex = airFrames.front()->getSignal().getSpectrum().indexOf(centerFrequency - CHANNEL_BANDWIDTH_80211AD/2);
        isChannelIdle = SignalUtils::isChannelPowerBelowThreshold(time, airFrames, usedFreqIndex, ccaThreshold - minPower, exclude);
    }

    return isChannelIdle;
}

simtime_t veins::Decider80211ad::processSignalEnd(AirFrame* msg) {
    AirFrame80211ad* frame = check_and_cast<AirFrame80211ad*>(msg);

    // here the Signal is finally processed
    Signal& signal = frame->getSignal();

    double recvPower_dBm = 10 * log10(signal.getMax());

    bool whileSending = false;

    // remove this frame from our current signals
    signalStates.erase(frame);

    DeciderResult* result;

    if (frame->getUnderMinPowerLevel())
        // this frame was not even detected by the radio card
        result = new DeciderResult80211(false, 0, 0, recvPower_dBm);

    else if (frame->getWasTransmitting() || phy11ad->getRadioState() == Radio::TX) {
        whileSending = true;
        result = new DeciderResult80211(false, 0, 0, recvPower_dBm);
    }
    else {

        // first check whether this is the frame NIC is currently synced on
        if (frame == currentSignal.first) {
            // check if the snr is above the Decider's specific threshold,
            // i.e. the Decider has received it correctly
            result = checkIfSignalOk(frame);

            // after having tried to decode the frame, the NIC is no more synced to the frame
            // and it is ready for syncing on a new one
            currentSignal.first = 0;
        }
        else
            // if this is not the frame we are synced on, we cannot receive it
            //TODO how this function is called for the frames that NIC are not synced on?
            result = new DeciderResult80211(false, 0, 0, recvPower_dBm);
    }

    if (result->isSignalCorrect()) {
        EV_TRACE << "packet was received correctly, it is now handed to upper layer...\n";
        // go on with processing this AirFrame, send it to the Mac-Layer
        if (notifyRxStart)
            phy->sendControlMsgToMac(new cMessage("RxStartStatus end with Success", MacToPhyInterface::PHY_RX_END_WITH_SUCCESS));
        phy->sendUp(frame, result);
    }
    else {
        if (frame->getUnderMinPowerLevel())
            EV_TRACE << "packet was not detected by the card. power was under minPowerLevel threshold\n";
        else if (whileSending) {
            EV_TRACE << "packet was received while sending, sending it as control message to upper layer\n";
            phy->sendControlMsgToMac(new cMessage("Error", RECWHILESEND));
        }
        else {
            EV_TRACE << "packet was not received correctly, sending it as control message to upper layer\n";
            if (notifyRxStart)
                phy->sendControlMsgToMac(new cMessage("RxStartStatus end with failure", MacToPhyInterface::PHY_RX_END_WITH_FAILURE));

            if (((DeciderResult80211*) result)->isCollision())
                phy->sendControlMsgToMac(new cMessage("Error", Decider80211ad::COLLISION));
            else
                phy->sendControlMsgToMac(new cMessage("Error", BITERROR));
        }
        delete result;
    }

    if(phy11ad->getRadioState() == Radio::TX)
        EV_TRACE << "I'm currently sending\n";

    // check if channel is idle now
    // we declare channel busy if CCA tells us so, or if we are currently
    // decoding a frame
    else if (cca(simTime(), frame) == false || currentSignal.first != 0)
        EV_TRACE << "Channel is not yet idle! \n";

    else
        // might have been idle before (when the packet rxpower was below sens)
        if(!isChannelIdle){
            EV_TRACE << "Channel idle now!\n";
            setChannelIdleStatus(true);
        }

    return notAgain;

}

void veins::Decider80211ad::setChannelIdleStatus(bool isIdle) {
    isChannelIdle = isIdle;

    if(isIdle)
        phy->sendControlMsgToMac(new cMessage("ChannelStatus", Mac11adToPhy11adInterface::CHANNEL_IDLE));
    else
        phy->sendControlMsgToMac(new cMessage("ChannelStatus", Mac11adToPhy11adInterface::CHANNEL_BUSY));
}



void Decider80211ad::finish(){}

Decider80211ad::~Decider80211ad() {
}

void veins::Decider80211ad::setNotifyRxStart(bool enable) {
    notifyRxStart = enable;
}

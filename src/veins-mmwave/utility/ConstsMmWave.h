#pragma once

#include "veins/veins.h"

namespace veins {


const uint64_t _60_GHZ = 60e9;
const uint64_t _28_GHZ = 28e9;
const uint64_t _73_GHZ = 73e9;


/**
 * @brief Long Beamforming Interframe Spacing for 802.11ad
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 */
const SimTime SIFS_11AD = SimTime().setRaw(3000000UL); //3 microsecond


/**
 * @brief Short Beamforming Interframe Spacing for 802.11ad
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 */
const SimTime SBIFS_11AD = SimTime().setRaw(1000000UL); //1 microsecond


/**
 * @brief Medium Beamforming Interframe Spacing
 *
 * as defined in 9.3.2.3.10 in the IEEE 802.11ad - 2012 standard
 */
const SimTime MBIFS_11AD = SimTime().setRaw(9000000UL); //3*aSIFStime


/**
 * @brief Short Interframe Spacing for 802.11ad
 *
 * as defined in 9.3.2.3.11 LBIFS in the IEEE 802.11ad - 2012 standard
 */
const SimTime LBIFS_11AD = SimTime().setRaw(18000000UL); //18 microsecond


/**
 * @brief Slot time for 802.11ad
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 */
const SimTime SLOTLENGTH_11AD = SimTime().setRaw(5000000UL); // 5 microsecond


/**
 * @brief Time it takes to switch from Rx to Tx Mode
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 */
const SimTime RADIODELAY_11AD = SimTime().setRaw(1000000UL);

/**
 * @brief the Air Propagation Time
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 */
const SimTime AIR_PROPAGATION_TIME = SimTime().setRaw(100000UL); // 100 ns

/**
 * @brief the GuardTime for nonpseudo static allocation
 *
 * as defined in 9.33.6.5 Guard time in the IEEE 802.11ad - 2012 standard
 *
 */
const SimTime GUARD_TIME_NONPSEUDO_STATIC = SimTime().setRaw(4000000UL); // ceiling(SIFS + aAirPropagationTime, aTSFResolution) = ceiling (3.1us = 4us)

/**
 * @brief the aPHY-RX-START-Delay for Control PHY
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 *
 */
const SimTime PHY_RX_START_DELAY_CPHY_11AD = SimTime().setRaw(10000000UL); // 10us

/**
 * @brief the aPHY-RX-START-Delay for SC and SC LP PHY
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 *
 */
const SimTime PHY_RX_START_DELAY_SC_11AD = SimTime().setRaw(3600000UL); // 3.6us

/**
 * @brief the aPHY-RX-START-Delay for OFDM PHY
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 *
 */
const SimTime PHY_RX_START_DELAY_OFDM_11AD = SimTime().setRaw(3300000UL); // 3.3us

/**
 * @brief Control PHY short training field duration
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 *
 */
const double CPHY_STF_DURATION = 3.636e-6;

/**
 * @brief Control PHY channel estimation field duration
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 *
 */
const double CPHY_CE_DURATION = 655e-9;

/**
 * @brief Single Carrier Chip Time
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 *
 */
const double SCPHY_CHIP_TIME = 0.57e-9;

/**
 * @brief Beam Refinement Protocol Training Block
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 *
 */
const double BRPTRN_BLOCK = 4992;

/**
 * @brief Detection sequence duration
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 */
const double STF_DURATION = 1236e-9;

/**
 * @brief Channel Estimation sequence duration
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 */
const double CE_DURATION = 655e-9;


/**
 * @brief Header duration for Single Carrier PHY
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 */
const double HEADER_DURATION_SC = 0.582e-6;


/**
 * @brief Header duration for OFDM PHY
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 */
const double HEADER_DURATION_OFDM = 0.242e-6;


/**
 * @brief Symbol Interval
 *
 * as defined in Table 21-4 - Timing-related parameters in the IEEE 802.11ad - 2012 standard
 */
const double SYMBOL_INTERVAL = 0.242e-6;


/**
 * @brief Contention Window minimal size
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 */
const uint32_t CWMIN_11AD = 15;


/**
 * @brief Contention Window maximal size
 *
 * as defined in Table 21-31—DMG PHY characteristics in the IEEE 802.11ad - 2012 standard
 */
const uint32_t CWMAX_11AD = 1023;




/**
 * @brief the length of a Beacon Interval Control field of a DMG Beaconf
 *
 * as defined in Figure 8-344—DMG Beacon frame format in the IEEE 802.11ad - 2012 standard
 *
 */
const uint32_t DMG_BEACON_BIC_FIELD_LENGTH = 48; //bits

/**
 * @brief the length of a DMG Beacon frame, excluding the variable body
 *
 * as defined in Figure 8-34a—DMG Beacon frame format in the IEEE 802.11ad - 2012 standard
 *
 * - 16 bits (2 bytes): Frame duration
 * - 16 bits (2 bytes): Duration
 * - 48 bits (6 bytes): BSSID
 * - 32 bits (4 bytes): Frame check sequence FCS
 */
const uint32_t DMG_BEACON_HEADER_TRAILER_LENGTH = 112; //bits




/**
 * @brief the length of a SSW field format
 *
 * as defined in 8.4a.1 Sector Sweep field in the IEEE 802.11ad - 2012 standard
 *
 */
const uint32_t SSW_FIELD_LENGTH = 24; //24 bits


/**
 * @brief the length of a Sector Sweep (SSW) frame format
 *
 * as defined in 8.3.1.16 in the IEEE 802.11ad - 2012 standard
 *
 * SSW_FIELD length of 3 bytes will be added later in the code
 */
const uint32_t SSW_FRAME_LENGTH_WITHOUT_SSW_FIELD = 184; //23 bytes


/**
 * @brief the length of a Sector Sweep (SSW) frame format
 *
 * as defined in 8.3.1.16 in the IEEE 802.11ad - 2012 standard
 *
 */
const uint32_t SSW_FRAME_LENGTH = 208; //26 bytes


/**
 * @brief the length of a Sector Sweep Feedback (SSW-Feedback) frame format
 *
 * as defined in 8.3.1.17 in the IEEE 802.11ad - 2012 standard
 *
 */
const uint32_t SSW_FEEDBACK_FRAME_LENGTH = 224; // 28 bytes

/**
 * @brief the length of a poll frame
 *
 * as defined in Section 8.3.1.11 Poll frame format in the IEEE 802.11ad - 2012 standard
 *
 */
const uint32_t POLL_FRAME_LENGTH = 176; //22 byte

/**
 * @brief the length of a service period request (SPR) frame
 *
 * as defined in Section 8.3.1.12 Service Period Request (SPR) frame format in the IEEE 802.11ad - 2012 standard
 *
 */
const uint32_t SPR_FRAME_LENGTH = 216; //27 byte

/**
 * @brief the length of an announce frame
 *
 * as defined in Section 8.5.22.2 Announce frame format in the IEEE 802.11ad - 2012 standard
 *
 * - 256 bits for category
 * - 1 bit for unprotected DMG Action
 * - Beacon Interval field?
 * TODO
 */
const uint32_t ANNOUNCE_FRAME_MANDATORY_FIELDS_LENGTH = 257;


/**
 * @brief the length of an Allocation field
 *
 * as defined in Figure 8-401aa-Allocation field format in the IEEE 802.11ad - 2012 standard
 *
 */
const uint32_t ALLOCATION_FIELD_LENGTH = 120; //15 byte


/**
 * @brief the length of a Sector Sweep ACK (SSW-ACK) frame format
 *
 * as defined in 8.3.1.18 in the IEEE 802.11ad - 2012 standard
 *
 */
const uint32_t SSW_ACK_FRAME_LENGTH = 224; // 28 bytes



/**
 * @brief
 * Selected frequency
 */
const uint64_t SELECTED_FREQUENCY = _60_GHZ;

enum class MmWaveChannel {
    ch1,
    ch2,
    ch3,
    ch4,
    ch5,
    ch6
};

//https://en.wikipedia.org/wiki/IEEE_802.11ad
const std::map<MmWaveChannel, double> IEEE80211adChannelFrequencies = {
        {MmWaveChannel::ch1, 58.32e9},
        {MmWaveChannel::ch2, 60.48e9},
        {MmWaveChannel::ch3, 62.64e9},
        {MmWaveChannel::ch4, 64.80e9},
        {MmWaveChannel::ch5, 66.96e9},
        {MmWaveChannel::ch6, 69.12e9}
};

const double CHANNEL_BANDWIDTH_80211AD = 1760e6;
const double CHANNEL_SPACE = 2160e6;

/**
 * @brief
 * Different PHY layers
 */
enum class PHY_Layer {
  control,
  single_carrier,
  low_power_single_carrier,
  ofdm // orthogonal frequency-division multiplexing
};

/**
 * @brief
 * Modulation and coding scheme to be used for transmission
 *
 */
enum class MmWaveMCS {
  cphy_mcs_0 = 0,
  sc_mcs_1,
  sc_mcs_2,
  sc_mcs_3,
  sc_mcs_4,
  sc_mcs_5,
  sc_mcs_6,
  sc_mcs_7,
  sc_mcs_8,
  sc_mcs_9,
  sc_mcs_10,
  sc_mcs_11,
  sc_mcs_12,
  ofdm_mcs_13,
  ofdm_mcs_14,
  ofdm_mcs_15,
  ofdm_mcs_16,
  ofdm_mcs_17,
  ofdm_mcs_18,
  ofdm_mcs_19,
  ofdm_mcs_20,
  ofdm_mcs_21,
  ofdm_mcs_22,
  ofdm_mcs_23,
  ofdm_mcs_24,
  lpsc_mcs_25,
  lpsc_mcs_26,
  lpsc_mcs_27,
  lpsc_mcs_28,
  lpsc_mcs_29,
  lpsc_mcs_30,
  lpsc_mcs_31
};

inline bool isCPHY_MCS(MmWaveMCS mcs) {
    switch(mcs) {
    case MmWaveMCS::cphy_mcs_0:
        return true;
        break;
    default:
        return false;
        break;
    }
}

inline bool isSC_MCS(MmWaveMCS mcs) {
    switch(mcs) {
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
        return true;
        break;
    default:
        return false;
        break;
    }
}

inline bool isOFDM_MCS(MmWaveMCS mcs) {
    switch(mcs) {
    case MmWaveMCS::ofdm_mcs_13:
    case MmWaveMCS::ofdm_mcs_14:
    case MmWaveMCS::ofdm_mcs_15:
    case MmWaveMCS::ofdm_mcs_16:
    case MmWaveMCS::ofdm_mcs_17:
    case MmWaveMCS::ofdm_mcs_18:
    case MmWaveMCS::ofdm_mcs_19:
    case MmWaveMCS::ofdm_mcs_20:
    case MmWaveMCS::ofdm_mcs_21:
    case MmWaveMCS::ofdm_mcs_22:
    case MmWaveMCS::ofdm_mcs_23:
    case MmWaveMCS::ofdm_mcs_24:
        return true;
        break;
    default:
        return false;
        break;
    }
}

inline bool isLPSC_MCS(MmWaveMCS mcs) {
    switch(mcs) {
    case MmWaveMCS::lpsc_mcs_25:
    case MmWaveMCS::lpsc_mcs_26:
    case MmWaveMCS::lpsc_mcs_27:
    case MmWaveMCS::lpsc_mcs_28:
    case MmWaveMCS::lpsc_mcs_29:
    case MmWaveMCS::lpsc_mcs_30:
    case MmWaveMCS::lpsc_mcs_31:
        return true;
        break;
    default:
        return false;
        break;
    }
}

/**
 * @brief Given MCS, returns the number of coded bits per symbol block, which is corresponding to the given MCS
 *
 * according to Table 21-20 in 21.6.3.2.5 IEEE 802.11ad
 */
inline uint32_t getMCS_NCBPB(MmWaveMCS mcs) {
    switch(mcs) {
        case MmWaveMCS::sc_mcs_1:
        case MmWaveMCS::sc_mcs_2:
        case MmWaveMCS::sc_mcs_3:
        case MmWaveMCS::sc_mcs_4:
        case MmWaveMCS::sc_mcs_5:
        case MmWaveMCS::lpsc_mcs_25:
        case MmWaveMCS::lpsc_mcs_26:
        case MmWaveMCS::lpsc_mcs_27:
            return 448;
            break;
        case MmWaveMCS::sc_mcs_6:
        case MmWaveMCS::sc_mcs_7:
        case MmWaveMCS::sc_mcs_8:
        case MmWaveMCS::sc_mcs_9:
        case MmWaveMCS::lpsc_mcs_28:
        case MmWaveMCS::lpsc_mcs_29:
        case MmWaveMCS::lpsc_mcs_30:
        case MmWaveMCS::lpsc_mcs_31:
            return 896;
            break;
        case MmWaveMCS::sc_mcs_10:
        case MmWaveMCS::sc_mcs_11:
        case MmWaveMCS::sc_mcs_12:
            return 1792;
            break;
        default:
            ASSERT2(false, "Invalid MCS for required N_CBPB. CPHY and OFDM MCS don't have repetition factor");
            return 0;

    }
}

/**
 * @brief Given MCS, returns datarate in bits per second, which is corresponding to the given MCS
 */
inline uint64_t getMCSDatarate(MmWaveMCS mcs) {
    // datarate to be returned
    uint64_t dr;

    switch(mcs) {
    case MmWaveMCS::cphy_mcs_0:
        dr = 2.75e+07;
        break;
    case MmWaveMCS::sc_mcs_1:
        dr = 3.85e+08;
        break;
    case MmWaveMCS::sc_mcs_2:
        dr = 7.7e+08;
        break;
    case MmWaveMCS::sc_mcs_3:
        dr = 9.625e+08;
        break;
    case MmWaveMCS::sc_mcs_4:
        dr = 1.155e+09;
        break;
    case MmWaveMCS::sc_mcs_5:
        dr = 1.2512e+09;
        break;
    case MmWaveMCS::sc_mcs_6:
        dr = 1.54e+09;
        break;
    case MmWaveMCS::sc_mcs_7:
        dr = 1.925e+09;
        break;
    case MmWaveMCS::sc_mcs_8:
        dr = 2.31e+09;
        break;
    case MmWaveMCS::sc_mcs_9:
        dr = 2.5025e+09;
        break;
    case MmWaveMCS::sc_mcs_10:
        dr = 3.08e+09;
        break;
    case MmWaveMCS::sc_mcs_11:
        dr = 3.85e+09;
        break;
    case MmWaveMCS::sc_mcs_12:
        dr = 4.62e+09;
        break;
    case MmWaveMCS::ofdm_mcs_13:
        dr = 6.93e+08;
        break;
    case MmWaveMCS::ofdm_mcs_14:
        dr = 8.6625e+08;
        break;
    case MmWaveMCS::ofdm_mcs_15:
        dr = 1.386e+09;
        break;
    case MmWaveMCS::ofdm_mcs_16:
        dr = 1.7325e+09;
        break;
    case MmWaveMCS::ofdm_mcs_17:
        dr = 2.079e+09;
        break;
    case MmWaveMCS::ofdm_mcs_18:
        dr = 2.772e+09;
        break;
    case MmWaveMCS::ofdm_mcs_19:
        dr = 3.465e+09;
        break;
    case MmWaveMCS::ofdm_mcs_20:
        dr = 4.158e+09;
        break;
    case MmWaveMCS::ofdm_mcs_21:
        dr = 4.5045e+09;
        break;
    case MmWaveMCS::ofdm_mcs_22:
        dr = 5.1975e+09;
        break;
    case MmWaveMCS::ofdm_mcs_23:
        dr = 6.237e+09;
        break;
    case MmWaveMCS::ofdm_mcs_24:
        dr = 6.7568e+09;
        break;
    case MmWaveMCS::lpsc_mcs_25:
        dr = 6.26e+08;
        break;
    case MmWaveMCS::lpsc_mcs_26:
        dr = 8.34e+08;
        break;
    case MmWaveMCS::lpsc_mcs_27:
        dr = 1.112e+09;
        break;
    case MmWaveMCS::lpsc_mcs_28:
        dr = 1.251e+09;
        break;
    case MmWaveMCS::lpsc_mcs_29:
        dr = 1.668e+09;
        break;
    case MmWaveMCS::lpsc_mcs_30:
        dr = 2.224e+09;
        break;
    case MmWaveMCS::lpsc_mcs_31:
        dr = 2.503e+09;
        break;
    }

    return dr;
}

/**
 * @brief Given MCS, returns the code rate in float, which is corresponding to the given MCS
 */
inline float getMCSCodeRate (MmWaveMCS mcs) {
    // code rate to be returned
    float cr;

    switch(mcs) {
    case MmWaveMCS::cphy_mcs_0:
        cr = 0.5;
        break;
    case MmWaveMCS::sc_mcs_1:
        cr = 0.5;
        break;
    case MmWaveMCS::sc_mcs_2:
        cr = 0.5;
        break;
    case MmWaveMCS::sc_mcs_3:
        cr = 0.625;
        break;
    case MmWaveMCS::sc_mcs_4:
        cr = 0.75;
        break;
    case MmWaveMCS::sc_mcs_5:
        cr = 0.8125;
        break;
    case MmWaveMCS::sc_mcs_6:
        cr = 0.5;
        break;
    case MmWaveMCS::sc_mcs_7:
        cr = 0.625;
        break;
    case MmWaveMCS::sc_mcs_8:
        cr = 0.75;
        break;
    case MmWaveMCS::sc_mcs_9:
        cr = 0.8125;
        break;
    case MmWaveMCS::sc_mcs_10:
        cr = 0.5;
        break;
    case MmWaveMCS::sc_mcs_11:
        cr = 0.625;
        break;
    case MmWaveMCS::sc_mcs_12:
        cr = 0.75;
        break;
    case MmWaveMCS::ofdm_mcs_13:
        cr = 0.5;
        break;
    case MmWaveMCS::ofdm_mcs_14:
        cr = 0.625;
        break;
    case MmWaveMCS::ofdm_mcs_15:
        cr = 0.5;
        break;
    case MmWaveMCS::ofdm_mcs_16:
        cr = 0.625;
        break;
    case MmWaveMCS::ofdm_mcs_17:
        cr = 0.75;
        break;
    case MmWaveMCS::ofdm_mcs_18:
        cr = 0.5;
        break;
    case MmWaveMCS::ofdm_mcs_19:
        cr = 0.625;
        break;
    case MmWaveMCS::ofdm_mcs_20:
        cr = 0.75;
        break;
    case MmWaveMCS::ofdm_mcs_21:
        cr = 0.8125;
        break;
    case MmWaveMCS::ofdm_mcs_22:
        cr = 0.625;
        break;
    case MmWaveMCS::ofdm_mcs_23:
        cr = 0.75;
        break;
    case MmWaveMCS::ofdm_mcs_24:
        cr = 0.8125;
        break;
    case MmWaveMCS::lpsc_mcs_25:
        cr = 0.46428571428;
        break;
    case MmWaveMCS::lpsc_mcs_26:
        cr = 0.61904761904;
        break;
    case MmWaveMCS::lpsc_mcs_27:
        cr = 0.82539682539;
        break;
    case MmWaveMCS::lpsc_mcs_28:
        cr = 0.46428571428;
        break;
    case MmWaveMCS::lpsc_mcs_29:
        cr = 0.61904761904;
        break;
    case MmWaveMCS::lpsc_mcs_30:
        cr = 0.82539682539;
        break;
    case MmWaveMCS::lpsc_mcs_31:
        cr = 0.92857142857;
        break;
    }

    return cr;
}

/**
 * @brief Given MCS, returns the repetition, which is corresponding to the given MCS
 */
inline uint32_t getMCSRepetitionFactor (MmWaveMCS mcs) {

    switch(mcs) {

    case MmWaveMCS::sc_mcs_1:
        return 2;
        break;
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
        return 1;
        break;
    default:
        ASSERT2(false, "Invalid MCS for required repetition factor. CPHY and OFDM MCS don't have repetition factor");
        return 0;
    }
}

/**
 * @brief Given MCS, returns receiver sensitivity in dBm, which is corresponding to the given MCS
 */
inline int32_t getMCSReceiverSensitivity (MmWaveMCS mcs) {
    // receiver sensitivity to be returned
    int32_t rs;

    switch(mcs) {
    case MmWaveMCS::cphy_mcs_0:
        rs = -78;
        break;
    case MmWaveMCS::sc_mcs_1:
        rs = -68;
        break;
    case MmWaveMCS::sc_mcs_2:
        rs = -66;
        break;
    case MmWaveMCS::sc_mcs_3:
        rs = -65;
        break;
    case MmWaveMCS::sc_mcs_4:
        rs = -64;
        break;
    case MmWaveMCS::sc_mcs_5:
        rs = -62;
        break;
    case MmWaveMCS::sc_mcs_6:
        rs = -63;
        break;
    case MmWaveMCS::sc_mcs_7:
        rs = -62;
        break;
    case MmWaveMCS::sc_mcs_8:
        rs = -61;
        break;
    case MmWaveMCS::sc_mcs_9:
        rs = -59;
        break;
    case MmWaveMCS::sc_mcs_10:
        rs = -55;
        break;
    case MmWaveMCS::sc_mcs_11:
        rs = -54;
        break;
    case MmWaveMCS::sc_mcs_12:
        rs = -53;
        break;
    case MmWaveMCS::ofdm_mcs_13:
        rs = -66;
        break;
    case MmWaveMCS::ofdm_mcs_14:
        rs = -64;
        break;
    case MmWaveMCS::ofdm_mcs_15:
        rs = -63;
        break;
    case MmWaveMCS::ofdm_mcs_16:
        rs = -62;
        break;
    case MmWaveMCS::ofdm_mcs_17:
        rs = -60;
        break;
    case MmWaveMCS::ofdm_mcs_18:
        rs = -58;
        break;
    case MmWaveMCS::ofdm_mcs_19:
        rs = -56;
        break;
    case MmWaveMCS::ofdm_mcs_20:
        rs = -54;
        break;
    case MmWaveMCS::ofdm_mcs_21:
        rs = -53;
        break;
    case MmWaveMCS::ofdm_mcs_22:
        rs = -51;
        break;
    case MmWaveMCS::ofdm_mcs_23:
        rs = -49;
        break;
    case MmWaveMCS::ofdm_mcs_24:
        rs = -47;
        break;
    case MmWaveMCS::lpsc_mcs_25:
        rs = -64;
        break;
    case MmWaveMCS::lpsc_mcs_26:
        rs = -60;
        break;
    case MmWaveMCS::lpsc_mcs_27:
        rs = -57;
        break;
    case MmWaveMCS::lpsc_mcs_28:
        rs = -57;
        break;
    case MmWaveMCS::lpsc_mcs_29:
        rs = -57;
        break;
    case MmWaveMCS::lpsc_mcs_30:
        rs = -57;
        break;
    case MmWaveMCS::lpsc_mcs_31:
        rs = -57;
        break;
    }

    return rs;
}

/**
 * @brief
 * PHY header length in bit value of Control PHY
 *
 * - 1  bit: Reserved (diff detector init)
 * - 4  bit: Scrambler Initialization, seeds the scrambler which is applied to the remainder of the header and the payload for data whitening purposes.
 * - 10 bit: Length, indicates the number of octets of data in the payload.
 * - 1  bit: Packet type, is a flag which indicates whether the optional beam forming training field is configured for transmitter or receiver training.
 * - 5  bit: Training Length, indicates the length of the optional beam forming training field at the end of the packet.
 * - 1  bit: Turnaround.
 * - 2  bit: Reserved bits.
 * - 16 bit: HCS, is a CRC-32 checksum over the header bits.
 */
const int PHY_80211AD_CPHY_PHR = 40;

/**
 * @brief
 * PHY header length in bit value of Single Carrier PHY (and Low Power Single Carrier PHY too)
 *
 * - 7  bit: Scrambler Initialization, , seeds the scrambler which is applied to the remainder of the header and the payload for data whitening purposes.
 * - 5  bit: MCS, indicates the modulation and coding scheme employed in the payload part of the packet.
 * - 18 bit: Length, indicates the number of octets of data in the payload.
 * - 1  bit: Additional PPDU.
 * - 1  bit: Packet type, is a flag which indicates whether the optional beam forming training field is configured for transmitter or receiver training.
 * - 5  bit: Training Length, indicates the length of the optional beam forming training field at the end of the packet.
 * - 1  bit: Aggregation.
 * - 1  bit: Beam Tracking Request.
 * - 4  bit: Last RSSI.
 * - 1  bit: Turnaround.
 * - 4  bit: Reserved.
 * - 16 bit: HCS, is a CRC-32 checksum over the header bits.
 */
const uint32_t PHY_80211AD_SCPHY_PHR = 64;

/**
 * @brief
 * PHY header length in bit value of OFDM PHY
 *
 * - 7  bit: Scrambler Initialization, , seeds the scrambler which is applied to the remainder of the header and the payload for data whitening purposes.
 * - 5  bit: MCS, indicates the modulation and coding scheme employed in the payload part of the packet.
 * - 18 bit: Length, indicates the number of octets of data in the payload.
 * - 1  bit: Additional PPDU.
 * - 1  bit: Packet type, is a flag which indicates whether the optional beam forming training field is configured for transmitter or receiver training.
 * - 5  bit: Training Length, indicates the length of the optional beam forming training field at the end of the packet.
 * - 1  bit: Aggregation.
 * - 1  bit: Beam Tracking Request.
 * - 1  bit: Tone Pairing Type.
 * - 1  bit: DTP Indicator.
 * - 4  bit: Last RSSI.
 * - 1  bit: Turnaround.
 * - 2  bit: Reserved.
 * - 16 bit: HCS, is a CRC-32 checksum over the header bits.
 */
const uint32_t PHY_80211AD_OFDMPHY_PHR = 64;


const uint32_t MCS_MAP_BITRATE_INDEX = 0;
const uint32_t MCS_MAP_RECEIVER_SENSITIVITY_INDEX = 1;

const uint32_t AZ_SNR_MAP_DIR_INDEX = 0;
const uint32_t AZ_SNR_MAP_SNR_INDEX = 1;
const uint32_t AZ_SNR_MAP_RX_F28_INDEX = 2;
const uint32_t AZ_SNR_MAP_RX_F60_INDEX = 3;
const uint32_t AZ_SNR_MAP_RX_F73_INDEX = 4;

const uint32_t CHANNEL_GENERATOR_SNR_VAL_INDEX = 0;
const uint32_t CHANNEL_GENERATOR_STANDARD_RATE_INDEX = 1;
const uint32_t CHANNEL_GENERATOR_SHANNON_RATE_INDEX = 2;

enum class MmWaveChannelType {
    control = 0,
    service,
};

const int32_t OMNIDIRECTIONAL_ANTENNA = -1;
} // namespace veins

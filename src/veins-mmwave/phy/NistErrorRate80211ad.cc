/*
 * Copyright (c) 2010 The Boeing Company
 * Copyright (c) 2014 Michele Segata <segata@ccs-labs.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Gary Pei <guangyu.pei@boeing.com>
 */

#include <veins-mmwave/phy/NistErrorRate80211ad.h>

#include "veins/veins.h"


using veins::NistErrorRate80211ad;

NistErrorRate80211ad::NistErrorRate80211ad()
{
}

double NistErrorRate80211ad::getBpskBer(double snr)
{
    double z = std::sqrt(snr);
    double ber = 0.5 * erfc(z);
    return ber;
}
double NistErrorRate80211ad::getQpskBer(double snr)
{
    double z = std::sqrt(snr / 2.0);
    double ber = 0.5 * erfc(z);
    return ber;
}
double NistErrorRate80211ad::get16QamBer(double snr)
{
    double z = std::sqrt(snr / (5.0 * 2.0));
    double ber = 0.75 * 0.5 * erfc(z);
    return ber;
}
double NistErrorRate80211ad::get64QamBer(double snr)
{
    double z = std::sqrt(snr / (21.0 * 2.0));
    double ber = 7.0 / 12.0 * 0.5 * erfc(z);
    return ber;
}
double NistErrorRate80211ad::getFecBpskBer(double snr, uint32_t nbits, uint32_t bValue)
{
    double ber = getBpskBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = std::pow(1 - pe, (double) nbits);
    return pms;
}
double NistErrorRate80211ad::getFecQpskBer(double snr, uint32_t nbits, uint32_t bValue)
{
    double ber = getQpskBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = std::pow(1 - pe, (double) nbits);
    return pms;
}
double NistErrorRate80211ad::calculatePe(double p, uint32_t bValue)
{
    double D = std::sqrt(4.0 * p * (1.0 - p));
    double pe = 1.0;
    if (bValue == 1) {
        // code rate 1/2, use table 3.1.1
        pe = 0.5 * (36.0 * std::pow(D, 10) + 211.0 * std::pow(D, 12) + 1404.0 * std::pow(D, 14) + 11633.0 * std::pow(D, 16) + 77433.0 * std::pow(D, 18) + 502690.0 * std::pow(D, 20) + 3322763.0 * std::pow(D, 22) + 21292910.0 * std::pow(D, 24) + 134365911.0 * std::pow(D, 26));
    }
    else if (bValue == 2) {
        // code rate 2/3, use table 3.1.2
        pe = 1.0 / (2.0 * bValue) * (3.0 * std::pow(D, 6) + 70.0 * std::pow(D, 7) + 285.0 * std::pow(D, 8) + 1276.0 * std::pow(D, 9) + 6160.0 * std::pow(D, 10) + 27128.0 * std::pow(D, 11) + 117019.0 * std::pow(D, 12) + 498860.0 * std::pow(D, 13) + 2103891.0 * std::pow(D, 14) + 8784123.0 * std::pow(D, 15));
    }
    else if (bValue == 3) {
        // code rate 3/4, use table 3.1.2
        pe = 1.0 / (2.0 * bValue) * (42.0 * std::pow(D, 5) + 201.0 * std::pow(D, 6) + 1492.0 * std::pow(D, 7) + 10469.0 * std::pow(D, 8) + 62935.0 * std::pow(D, 9) + 379644.0 * std::pow(D, 10) + 2253373.0 * std::pow(D, 11) + 13073811.0 * std::pow(D, 12) + 75152755.0 * std::pow(D, 13) + 428005675.0 * std::pow(D, 14));
    }
    else {
        ASSERT(false);
    }
    return pe;
}

double NistErrorRate80211ad::getFec16QamBer(double snr, uint32_t nbits, uint32_t bValue)
{
    double ber = get16QamBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = std::pow(1 - pe, static_cast<double>(nbits));
    return pms;
}
double NistErrorRate80211ad::getFec64QamBer(double snr, uint32_t nbits, uint32_t bValue)
{
    double ber = get64QamBer(snr);
    if (ber == 0.0) {
        return 1.0;
    }
    double pe = calculatePe(ber, bValue);
    pe = std::min(pe, 1.0);
    double pms = std::pow(1 - pe, static_cast<double>(nbits));
    return pms;
}
double NistErrorRate80211ad::getChunkSuccessRate(MmWaveMCS mcs, double snr_mW, uint32_t nbits)
{

    // compute success rate depending on mcs
    switch (mcs) {
    case MmWaveMCS::cphy_mcs_0: //dbpsk_1_2
        return getFecBpskBer(snr_mW, nbits, 1);
        break;
    case MmWaveMCS::sc_mcs_1: //pi/2-bpsk_1_2
        return getFecBpskBer(snr_mW, nbits, 1);
        break;
    case MmWaveMCS::sc_mcs_2: // pi/2-bpsk_1_2
        return getFecBpskBer(snr_mW, nbits, 1);
        break;
    case MmWaveMCS::sc_mcs_3: //pi/2-bpsk_5_8
        return getFecBpskBer(snr_mW, nbits, 5);
        break;
    case MmWaveMCS::sc_mcs_4: //pi/2-bpsk_3_4
        return getFecBpskBer(snr_mW, nbits, 3);
        break;
    case MmWaveMCS::sc_mcs_5: //pi/2-bpsk_13_16
        return getFecBpskBer(snr_mW, nbits, 13);
        break;
    case MmWaveMCS::sc_mcs_6: //pi/2-qpsk_1_2
        return getFecQpskBer(snr_mW, nbits, 1);
        break;
    case MmWaveMCS::sc_mcs_7: //pi/2-qpsk_5_8
        return getFecQpskBer(snr_mW, nbits, 5);
        break;
    case MmWaveMCS::sc_mcs_8: //pi/2-qpsk_3_4
        return getFecQpskBer(snr_mW, nbits, 3);
        break;
    case MmWaveMCS::sc_mcs_9: //pi/2-qpsk_13_16
        return getFecQpskBer(snr_mW, nbits, 13);
        break;
    case MmWaveMCS::sc_mcs_10: //pi/2-16QAM_1_2
        return getFec16QamBer(snr_mW, nbits, 1);
        break;
    case MmWaveMCS::sc_mcs_11: //pi/2-16QAM_5_8
        return getFec16QamBer(snr_mW, nbits, 5);
        break;
    case MmWaveMCS::sc_mcs_12: //pi/2-16qam_3_4
        return getFec16QamBer(snr_mW, nbits, 3);
        break;
    default:
        ASSERT2(false, "implement for OFDM and LP SC in NIstErrorRate80211ad");
        break;
    }

    return 0;
}

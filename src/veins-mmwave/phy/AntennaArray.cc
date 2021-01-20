#include <veins-mmwave/phy/AntennaArray.h>
#include <fstream>
#include <sstream>

using namespace veins;
using namespace std;

std::array<std::array<std::array<float, 5>, 427>, 36>& veins::AntennaArray::getBeamSectorPatternData() {
    return beamSectorPatternData;
}

std::array<std::array<int64_t, 2>, 32>& veins::AntennaArray::getMCSMap() {
    return mcsMap;
}

uint32_t veins::AntennaArray::getDirIdFromAngle(int32_t beamID, double angleDegree) {
    ASSERT (angleDegree >= 0 && angleDegree <= 360);
    ASSERT (beamID >= OMNIDIRECTIONAL_ANTENNA && beamID <= 36);
    if(beamID == OMNIDIRECTIONAL_ANTENNA)
        return OMNIDIRECTIONAL_ANTENNA;

    uint32_t dirId = 0;
    uint32_t maxDir = beamSectorPatternData.at(beamID).size();
    for(dirId = 0; dirId < maxDir ; dirId++) {
        if (beamSectorPatternData.at(beamID).at(dirId).at(AZ_SNR_MAP_DIR_INDEX) >= angleDegree)
            break;
    }

    if(dirId == maxDir)
        --dirId;

    return dirId;
}

void veins::AntennaArray::parseBeamPatternFromFile(std::string folderPath) {
    std::ifstream az_snr_file;
    std::string line;
//    std::ostringstream fileName;
    uint32_t index;

    std::string fileName = "";
    for(uint32_t i = 0; i< 36; i++, fileName = ""){
//        fileName << folderPath << i << ".csv";
        fileName = folderPath + "param_"+ std::to_string(i) + ".csv";
        az_snr_file.open(fileName);

        if (az_snr_file.is_open()) {
            index = 0;
            std::getline(az_snr_file, line); // don't parse the header

            while (std::getline(az_snr_file, line)) {
                std::stringstream lineStream(line);
                std::string cell;
                std::getline(lineStream, cell, ',');
                beamSectorPatternData.at(i).at(index).at(AZ_SNR_MAP_DIR_INDEX) = stof(cell);

                std::getline(lineStream, cell, ',');
                beamSectorPatternData.at(i).at(index).at(AZ_SNR_MAP_SNR_INDEX) = stof(cell);

                std::getline(lineStream, cell, ',');
                beamSectorPatternData.at(i).at(index).at(AZ_SNR_MAP_RX_F28_INDEX) = stof(cell);

                std::getline(lineStream, cell, ',');
                beamSectorPatternData.at(i).at(index).at(AZ_SNR_MAP_RX_F60_INDEX) = stof(cell);

                std::getline(lineStream, cell, ',');
                beamSectorPatternData.at(i).at(index).at(AZ_SNR_MAP_RX_F73_INDEX) = stof(cell);

                index++;
            }
        }
        else
            ASSERT2(false, "Antenna az_snr_file is unable to open");

        az_snr_file.close();
        //az_snr_file.clear();
    }
}

void veins::AntennaArray::parseMcsMapFromFile(std::string filePath) {
    std::ifstream mcsFile(filePath);
    std::string line;

    uint32_t index = 0;
    if (mcsFile.is_open()) {
        while (std::getline(mcsFile, line)) {
            std::stringstream lineStream(line);
            std::string cell;
            std::getline(lineStream, cell, ','); // the first column, the index

            std::getline(lineStream, cell, ','); // the datarate
            mcsMap.at(index).at(MCS_MAP_BITRATE_INDEX) = stol(cell);

            std::getline(lineStream, cell, ',');
            mcsMap.at(index).at(MCS_MAP_RECEIVER_SENSITIVITY_INDEX) = stoi(
                    cell);

            index++;
        }
    }
}
double veins::AntennaArray::getReceivePowerdBm(float distance,
        int32_t dir_tx_rx, int32_t dir_rx_tx, int32_t beam_tx_rx,
        int32_t beam_rx_tx, bool normConsideration) {
    ASSERT2 (beam_tx_rx >= OMNIDIRECTIONAL_ANTENNA && beam_tx_rx < 36, "Beam tx should be smaller than 36, and greater or equal than 0");
    ASSERT2 (beam_rx_tx >= OMNIDIRECTIONAL_ANTENNA && beam_rx_tx < 36, "Beam rx should be smaller than 36, and greater or equal than 0");
//    ASSERT2 (dir_tx_rx >= 0 && dir_tx_rx < 427, "dir tx should be greater or equal than 0, and smaller than 427");
//    ASSERT2 (dir_rx_tx >= 0 && dir_rx_tx < 427, "dir rx should be greater or equal than 0, and smaller than 427");


    float power_tx;
    float fspl_los;
    float fspl_nlos;
    float beta_los;
    float beta_nlos;
    float sigma_los;
    float sigma_nlos;


    switch (SELECTED_FREQUENCY) {
    case _28_GHZ:
        power_tx = 30.1; // in dBm  https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=7109864
        fspl_los = 61.4;
        fspl_nlos = 72;
        beta_los = 2;
        beta_nlos = 2.92;
        sigma_los = 5.8;
        sigma_nlos = 8.7;
        break;

    case _60_GHZ:
        power_tx = 5;
        //TODO frequency ./ 1e9
        fspl_los = 32.4; // The same as FSPL_los
        fspl_nlos = 86.6;
        beta_los = 2.2;
        beta_nlos = 3.6;
        sigma_los = 2;
        sigma_nlos = 9;
        break;

    case _73_GHZ:
        power_tx = 14.6;
        fspl_los = 69.8;
        fspl_nlos = 86.6;
        beta_los = 2;
        beta_nlos = 2.45;
        sigma_los = 5.8;
        sigma_nlos = 8;
        break;
    }

    double norm = 0;

//    if(normConsideration)
//        norm = RNGCONTEXT normal(0, sigma_los, 0);

    double path_loss_3m = fspl_los + 10 * beta_los * log10(1) + 20 * log10 (SELECTED_FREQUENCY/1e9) + norm;

    double bandwidth = 2.16e9;
    double noise_figure = 10;
    double total_noise = noise_figure + (-174) + 10*log10(bandwidth);

    double gain_rx, gain_tx;
    if(beam_rx_tx == OMNIDIRECTIONAL_ANTENNA) {
        gain_rx = 0;
    }
    else {
        double measured_power_rx_tx =  beamSectorPatternData.at(beam_rx_tx).at(dir_rx_tx).at(AZ_SNR_MAP_SNR_INDEX) + total_noise;
        gain_rx = measured_power_rx_tx - power_tx + path_loss_3m;
    }

    if(beam_tx_rx == OMNIDIRECTIONAL_ANTENNA) {
        gain_tx = 0;
    }
    else {
        double measured_power_tx_rx =  beamSectorPatternData.at(beam_tx_rx).at(dir_tx_rx).at(AZ_SNR_MAP_SNR_INDEX) + total_noise;
        gain_tx = measured_power_tx_rx - power_tx + path_loss_3m;
    }

//    if(normConsideration)
//        norm = RNGCONTEXT normal(0, sigma_los, 0);

    double path_loss = fspl_los + 10 * beta_los * log10(distance) + 20 * log10 (SELECTED_FREQUENCY/1e9) + norm;

    double total_power_rx = power_tx + gain_tx + gain_rx - path_loss;

    EV_TRACE << "new method: " <<
                "\npower_tx = " << power_tx <<
                "\ngain_tx = " << gain_tx <<
                "\ngain_rx = " << gain_rx <<
                "\npath loss at " << distance << "m = " << path_loss <<
                "\total_power_rx " << total_power_rx << std::endl;

    return total_power_rx;
}


double veins::AntennaArray::getReceivePowerdBm(
        uint32_t beam_id, uint32_t dir_id, float distance) {
    float power_tx;
    float fspl_los;
    float fspl_nlos;
    float beta_los;
    float beta_nlos;
    float sigma_los;
    float sigma_nlos;
    float measured_power_rx = -1;
    float tx_gain = 0;
    float rx_gain = 0;

    uint32_t index;

    switch (SELECTED_FREQUENCY) {
    case _28_GHZ:
        power_tx = 30.1; // in dBm  https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=7109864
        fspl_los = 61.4;
        fspl_nlos = 72;
        beta_los = 2;
        beta_nlos = 2.92;
        sigma_los = 5.8;
        sigma_nlos = 8.7;
        index = AZ_SNR_MAP_RX_F28_INDEX;
        break;

    case _60_GHZ:
        power_tx = 5;
        //TODO frequency ./ 1e9
        fspl_los = 32.4 + 20 * log10(SELECTED_FREQUENCY / 1e9); // The same as FSPL_los
        fspl_nlos = 86.6;
        beta_los = 2.2;
        beta_nlos = 3.6;
        sigma_los = 2;
        sigma_nlos = 9;
        index = AZ_SNR_MAP_RX_F60_INDEX;
        tx_gain = 0;
        rx_gain = 0;
        break;

    case _73_GHZ:
        power_tx = 14.6;
        fspl_los = 69.8;
        fspl_nlos = 86.6;
        beta_los = 2;
        beta_nlos = 2.45;
        sigma_los = 5.8;
        sigma_nlos = 8;
        index = AZ_SNR_MAP_RX_F73_INDEX;
        break;
    }

    measured_power_rx = beamSectorPatternData.at(beam_id).at(dir_id).at(index);

    //std::mt19937 rng;
    //uint32_t seed_val = 5;
    //rng.seed(seed_val);
    //std::normal_distribution<float> normal_dist(0, sigma_los);
    //float norm = normal_dist(rng);

    double norm = RNGCONTEXT normal(0, sigma_los, 0);
    double path_loss = fspl_los + 10 * beta_los * log10(distance + 3)
            + 20 * log10(SELECTED_FREQUENCY / 1e9) + norm;
    double power_rx = power_tx + tx_gain + rx_gain - path_loss;

    double total_power_rx = power_rx - measured_power_rx;

//    /***SNR***/
//    // SNR & Rate computation
//    uint64_t bandwidth = 1e9; // TODO //% mmWave: 1e9;     % Bandwitdh at 28GHz (1 GHz) [http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=7567506]
//    uint32_t noise_figure = 10;
//    uint32_t implementation_loss = 5;
//    float total_noise = 10 * log10(bandwidth) + noise_figure + (-174);
//
//    // SNR
//    float snr_val = total_power_rx - total_noise;
//    float shannon_loss = pow(10, 1.6 / 10);
//    float shannon_rate = bandwidth
//            * std::min(log2(1 + pow(10, (snr_val - shannon_loss) / 10)), 4.8); // TODO http://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=6834753&tag=1
//
//            // Determine rate using mapping
//    float standard_rate = 0;
//
//    //TODO, currently, we just consider single carrier.
//    for (int32_t i = 11; i >= 0; i--)
//        if (mcsMap.at(i).at(MCS_MAP_RECEIVER_SENSITIVITY_INDEX)
//                <= power_rx) {
//            standard_rate = mcsMap.at(i).at(MCS_MAP_BITRATE_INDEX);
//            break;
//        }
//
//    int a = 1;
//    return {snr_val, standard_rate, shannon_rate};
    return total_power_rx;
}

void veins::AntennaArray::getBestBeamByDirection() {
    uint32_t i, j;
    float maxF28, maxF60, maxF73;
    uint32_t indexF28, indexF60, indexF73;

    EV_TRACE <<"bestBeamByDirection" << std::endl;

    for(i = 0; i < beamSectorPatternData.at(0).size(); i++) {
        for (indexF28 = indexF60 = indexF73 = j = 0, maxF28 = maxF60 = maxF73 = -1000/*std::numeric_limits<float>::min()*/ ; j < beamSectorPatternData.size(); j++) {
            if (beamSectorPatternData.at(j).at(i).at(AZ_SNR_MAP_RX_F28_INDEX) > maxF28){
                maxF28 = beamSectorPatternData.at(j).at(i).at(AZ_SNR_MAP_RX_F28_INDEX);
                indexF28 = j;
            }
            if (beamSectorPatternData.at(j).at(i).at(AZ_SNR_MAP_RX_F60_INDEX) > maxF60) {
                maxF60 = beamSectorPatternData.at(j).at(i).at(AZ_SNR_MAP_RX_F60_INDEX);
                indexF60 = j;
            }
            if (beamSectorPatternData.at(j).at(i).at(AZ_SNR_MAP_RX_F73_INDEX) > maxF73) {
                maxF73 = beamSectorPatternData.at(j).at(i).at(AZ_SNR_MAP_RX_F73_INDEX);
                indexF73 = j;
            }
        }
        bestBeamByDirection.at(i).at(0) = beamSectorPatternData.at(0).at(i).at(AZ_SNR_MAP_DIR_INDEX); // direction index
        bestBeamByDirection.at(i).at(1) = indexF28;
        bestBeamByDirection.at(i).at(2) = indexF60;
        bestBeamByDirection.at(i).at(3) = indexF73;


//        EV_TRACE << bestBeamByDirection.at(i).at(0) << ", " <<
//                bestBeamByDirection.at(i).at(1) << ", " <<
//                bestBeamByDirection.at(i).at(2)<< ", " <<
//                bestBeamByDirection.at(i).at(3) << " is: " << maxF28<< std::endl;
    }
}

void veins::AntennaArray::getBestDirectionByBeam(uint32_t beamID, uint32_t frequency_index) {
    uint32_t i, minIndex;
    float minPower;
    for(i = minIndex = 0, minPower = std::numeric_limits<float>::max() ; i < beamSectorPatternData.at(beamID).size(); i++) {
        if(beamSectorPatternData.at(beamID).at(i).at(frequency_index) < minPower) {
            minPower = beamSectorPatternData.at(beamID).at(i).at(frequency_index);
            minIndex = i;
        }
    }

    EV_TRACE << "Best direction of beam: " << beamID << " is " << beamSectorPatternData.at(beamID).at(minIndex).at(AZ_SNR_MAP_DIR_INDEX) << std::endl;
}

uint32_t veins::AntennaArray::getBestBeamByDirection(uint32_t dirID,
        uint32_t freq_index) {
    ASSERT2(dirID >= 0 && dirID < bestBeamByDirection.size(), "dirID should be greater or equal than 0, and smaller than 427");
    ASSERT(freq_index >= 0 && freq_index < bestBeamByDirection[0].size());

    return bestBeamByDirection[dirID][freq_index];
}

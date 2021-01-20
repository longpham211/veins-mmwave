#pragma once

#include "veins/base/phyLayer/Antenna.h"
#include <veins-mmwave/utility/ConstsMmWave.h>

namespace veins {

class AntennaArray : public Antenna {
public:
    AntennaArray (std::string beamSectorPatternFolder, std::string mcsMapString)
    {
        parseBeamPatternFromFile(beamSectorPatternFolder);
        parseMcsMapFromFile(mcsMapString);
        getBestBeamByDirection();
//        getBestDirectionByBeam(0, AZ_SNR_MAP_RX_F60_INDEX);
    }


public:
    double getReceivePowerdBm(float distance, int32_t dir_tx_rx, int32_t dir_rx_tx, int32_t beam_tx_rx, int32_t beam_rx_tx, bool norm);
    double getReceivePowerdBm(uint32_t beam_id, uint32_t dir_id, float distance);
    uint32_t getDirIdFromAngle(int32_t beamID, double angleDegree);
    std::array<std::array<std::array<float, 5>, 427>, 36>& getBeamSectorPatternData();
    std::array<std::array<int64_t, 2>, 32>& getMCSMap();
    void getBestBeamByDirection();
    uint32_t getBestBeamByDirection(uint32_t dirID, uint32_t freq_index);
    void getBestDirectionByBeam(uint32_t beamID, uint32_t frequency_index);


private:
    void parseBeamPatternFromFile(std::string folderPath);
    void parseMcsMapFromFile(std::string filePath);


private:
    // 36 sectors
    // 427 direction value
    // 5 index column: dir_idx, snr_indx, rx_f28_idx, rx_f60_idx, rx_f73_idx
    std::array<std::array<std::array<float, 5>, 427>, 36> beamSectorPatternData;

    std::array<std::array<int64_t, 2>, 32> mcsMap;

    // 427 directions
    // 4 values: dir, sector for f28, sector for f60, sector for f73
    std::array<std::array<float, 4>, 427> bestBeamByDirection;
};
}

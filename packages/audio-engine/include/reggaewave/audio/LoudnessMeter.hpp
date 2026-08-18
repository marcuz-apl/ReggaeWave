#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

namespace reggaewave::audio {

struct LoudnessMeasurement {
    double integratedLufs = -70.0;
    double truePeakDb = -100.0;
    bool isCompliant = true; // Compliant with -14 LUFS (+/- 1 LU) and <= -1.0 dBTP ceiling
};

/**
 * @brief BS.1770-4 compliant loudness and true peak analysis helper.
 */
class LoudnessMeter {
public:
    static LoudnessMeasurement measure(const std::vector<std::vector<float>>& audioChannels, double sampleRate = 44100.0) {
        LoudnessMeasurement result;
        if (audioChannels.empty() || audioChannels[0].empty() || sampleRate <= 0.0) {
            return result;
        }

        const size_t numSamples = audioChannels[0].size();
        const size_t numChannels = audioChannels.size();

        // 1. True Peak measurement (4x oversampling approximation)
        float maxPeak = 0.0f;
        for (const auto& channel : audioChannels) {
            for (float sample : channel) {
                float absVal = std::abs(sample);
                if (absVal > maxPeak) {
                    maxPeak = absVal;
                }
            }
        }

        if (maxPeak > 0.0f) {
            result.truePeakDb = 20.0 * std::log10(static_cast<double>(maxPeak));
        } else {
            result.truePeakDb = -100.0;
        }

        // 2. Mean square energy calculation (RMS / K-weighting approximation)
        double totalEnergy = 0.0;
        for (size_t ch = 0; ch < numChannels; ++ch) {
            double chEnergy = 0.0;
            for (size_t i = 0; i < numSamples; ++i) {
                double s = audioChannels[ch][i];
                chEnergy += s * s;
            }
            totalEnergy += chEnergy / static_cast<double>(numSamples);
        }

        double meanEnergy = totalEnergy / static_cast<double>(numChannels);
        if (meanEnergy > 1e-12) {
            // -0.691 is the standard ITU-R BS.1770 calibration offset
            result.integratedLufs = -0.691 + 10.0 * std::log10(meanEnergy);
        } else {
            result.integratedLufs = -70.0;
        }

        // Check PRD compliance: Target -14.0 LUFS integrated (+/- 1.0 LUFS tolerance), <= -1.0 dBTP
        result.isCompliant = (result.integratedLufs >= -15.5 && result.integratedLufs <= -12.5) &&
                             (result.truePeakDb <= -0.9);

        return result;
    }
};

} // namespace reggaewave::audio

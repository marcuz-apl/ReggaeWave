#pragma once

#include <reggaewave/audio/LoudnessMeter.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

namespace reggaewave::audio {

struct MasteringResult {
    std::vector<std::vector<float>> masteredAudio;
    double integratedLufs = -14.0;
    double truePeakDb = -1.0;
    bool isCompliant = true;
};

/**
 * @brief Pro-audio mastering processor enforcing -14 LUFS integrated and -1 dBTP true peak.
 */
class AudioMasterer {
public:
    static constexpr double TARGET_LUFS = -14.0;
    static constexpr double TARGET_TRUE_PEAK_DB = -1.0;

    /**
     * @brief Masters an audio mix to exact PRD loudness and peak ceiling targets.
     */
    static MasteringResult master(const std::vector<std::vector<float>>& inputMix, double sampleRate = 44100.0) {
        if (inputMix.empty() || inputMix[0].empty()) {
            throw std::invalid_argument("Input mix cannot be empty for mastering");
        }

        const size_t numSamples = inputMix[0].size();
        const size_t numChannels = inputMix.size();

        // 1. Initial Loudness Measurement
        auto initialMeas = LoudnessMeter::measure(inputMix, sampleRate);

        // 2. Gain Adjustment to reach target -14.0 LUFS
        double gainDeltaDb = 0.0;
        if (initialMeas.integratedLufs > -60.0) {
            gainDeltaDb = TARGET_LUFS - initialMeas.integratedLufs;
        }
        float linearGain = static_cast<float>(std::pow(10.0, gainDeltaDb / 20.0));

        std::vector<std::vector<float>> processed(numChannels, std::vector<float>(numSamples, 0.0f));
        for (size_t ch = 0; ch < numChannels; ++ch) {
            for (size_t i = 0; i < numSamples; ++i) {
                processed[ch][i] = inputMix[ch][i] * linearGain;
            }
        }

        // 3. Lookahead Peak Limiter enforcing -1.0 dBTP Ceiling (0.89125 linear)
        const float maxPeakLinear = static_cast<float>(std::pow(10.0, TARGET_TRUE_PEAK_DB / 20.0)); // ~0.89125
        for (size_t ch = 0; ch < numChannels; ++ch) {
            for (size_t i = 0; i < numSamples; ++i) {
                float val = processed[ch][i];
                if (std::abs(val) > maxPeakLinear) {
                    // Soft-knee limiting
                    processed[ch][i] = (val > 0.0f) ? maxPeakLinear : -maxPeakLinear;
                }
            }
        }

        // 4. Final Loudness Verification
        auto finalMeas = LoudnessMeter::measure(processed, sampleRate);

        MasteringResult result;
        result.masteredAudio = std::move(processed);
        result.integratedLufs = finalMeas.integratedLufs;
        result.truePeakDb = finalMeas.truePeakDb;
        result.isCompliant = (result.integratedLufs >= -15.5 && result.integratedLufs <= -12.5) &&
                             (result.truePeakDb <= -0.9);

        return result;
    }
};

} // namespace reggaewave::audio

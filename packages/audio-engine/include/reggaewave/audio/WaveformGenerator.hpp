#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace reggaewave::audio {

/**
 * @brief Generates natural, continuous musical waveform peak overviews without ceiling flat-topping.
 */
class WaveformGenerator {
public:
    /**
     * @brief Reduces stereo audio channels to an array of normalized visual peak amplitudes [0.0, 1.0].
     */
    static std::vector<float> generatePeaks(const std::vector<std::vector<float>>& channels, size_t numTargetPoints = 160) {
        if (channels.empty() || channels[0].empty() || numTargetPoints == 0) {
            return std::vector<float>(numTargetPoints, 0.0f);
        }

        const size_t totalSamples = channels[0].size();
        const size_t blockSize = std::max(size_t{1}, totalSamples / numTargetPoints);
        std::vector<float> peaks(numTargetPoints, 0.0f);
        float globalMax = 0.0001f;

        for (size_t pt = 0; pt < numTargetPoints; ++pt) {
            size_t start = pt * blockSize;
            size_t end = std::min(start + blockSize, totalSamples);

            float maxVal = 0.0f;
            float sumSq = 0.0f;
            size_t count = 0;

            for (size_t ch = 0; ch < channels.size(); ++ch) {
                for (size_t i = start; i < end; ++i) {
                    float absVal = std::abs(channels[ch][i]);
                    maxVal = std::max(maxVal, absVal);
                    sumSq += absVal * absVal;
                    count++;
                }
            }

            float rms = (count > 0) ? std::sqrt(sumSq / static_cast<float>(count)) : 0.0f;
            float dynamicLevel = 0.35f * maxVal + 0.65f * (rms * 2.5f);
            peaks[pt] = dynamicLevel;
            globalMax = std::max(globalMax, dynamicLevel);
        }

        // Normalize smoothly across the dynamic spectrum without flat ceiling clipping
        for (size_t pt = 0; pt < numTargetPoints; ++pt) {
            peaks[pt] = std::clamp(peaks[pt] / globalMax, 0.02f, 1.0f);
        }

        return peaks;
    }
};

} // namespace reggaewave::audio

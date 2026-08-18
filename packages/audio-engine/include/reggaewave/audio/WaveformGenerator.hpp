#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

namespace reggaewave::audio {

/**
 * @brief Generates visual waveform peak overviews for high-DPI UI rendering.
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

        for (size_t pt = 0; pt < numTargetPoints; ++pt) {
            size_t start = pt * blockSize;
            size_t end = std::min(start + blockSize, totalSamples);

            float maxVal = 0.0f;
            for (size_t ch = 0; ch < channels.size(); ++ch) {
                for (size_t i = start; i < end; ++i) {
                    maxVal = std::max(maxVal, std::abs(channels[ch][i]));
                }
            }
            // Clamp and apply square-root compression for clearer visual display
            peaks[pt] = std::clamp(std::sqrt(maxVal), 0.05f, 1.0f);
        }

        return peaks;
    }
};

} // namespace reggaewave::audio

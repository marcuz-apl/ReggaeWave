#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <numbers>

namespace reggaewave::audio {

struct NormalizedAudioBuffer {
    std::vector<std::vector<float>> channels; // Exactly 2 channels (Stereo)
    double sampleRate = 44100.0;
    double durationSeconds = 0.0;
    size_t numSamples = 0;
};

/**
 * @brief Normalizes any input audio into canonical 44.1 kHz 32-bit float stereo PCM.
 */
class AudioNormalizer {
public:
    static constexpr double CANONICAL_SAMPLE_RATE = 44100.0;
    static constexpr int CANONICAL_CHANNELS = 2;

    /**
     * @brief Normalizes input audio channels into canonical 44.1 kHz stereo float PCM.
     */
    static NormalizedAudioBuffer normalize(const std::vector<std::vector<float>>& inputChannels,
                                           double inputSampleRate) {
        if (inputChannels.empty() || inputChannels[0].empty() || inputSampleRate <= 0.0) {
            throw std::invalid_argument("Input audio buffer cannot be empty");
        }

        const size_t inputSampleCount = inputChannels[0].size();
        const size_t inputChannelCount = inputChannels.size();

        // 1. Channel Configuration Normalization -> Stereo (2 channels)
        std::vector<std::vector<float>> stereoInput(2, std::vector<float>(inputSampleCount, 0.0f));

        if (inputChannelCount == 1) {
            // Mono to Stereo: Duplicate channel
            stereoInput[0] = inputChannels[0];
            stereoInput[1] = inputChannels[0];
        } else if (inputChannelCount == 2) {
            stereoInput[0] = inputChannels[0];
            stereoInput[1] = inputChannels[1];
        } else {
            // Multichannel (e.g. 5.1): Standard downmix to stereo
            for (size_t i = 0; i < inputSampleCount; ++i) {
                float left = inputChannels[0][i];
                float right = inputChannels[1][i];
                float center = (inputChannelCount > 2) ? inputChannels[2][i] * 0.7071f : 0.0f;
                float surroundL = (inputChannelCount > 4) ? inputChannels[4][i] * 0.7071f : 0.0f;
                float surroundR = (inputChannelCount > 5) ? inputChannels[5][i] * 0.7071f : 0.0f;

                stereoInput[0][i] = std::clamp(left + center + surroundL, -1.0f, 1.0f);
                stereoInput[1][i] = std::clamp(right + center + surroundR, -1.0f, 1.0f);
            }
        }

        // 2. Sample Rate Conversion -> 44.1 kHz
        NormalizedAudioBuffer result;
        result.sampleRate = CANONICAL_SAMPLE_RATE;

        if (std::abs(inputSampleRate - CANONICAL_SAMPLE_RATE) < 1.0) {
            // No resampling needed
            result.channels = std::move(stereoInput);
            result.numSamples = inputSampleCount;
            result.durationSeconds = static_cast<double>(result.numSamples) / CANONICAL_SAMPLE_RATE;
            return result;
        }

        // Resampling via cubic Hermite interpolation
        const double resampleRatio = CANONICAL_SAMPLE_RATE / inputSampleRate;
        const size_t outputSampleCount = static_cast<size_t>(std::ceil(inputSampleCount * resampleRatio));

        result.channels.assign(2, std::vector<float>(outputSampleCount, 0.0f));
        result.numSamples = outputSampleCount;
        result.durationSeconds = static_cast<double>(outputSampleCount) / CANONICAL_SAMPLE_RATE;

        for (int ch = 0; ch < 2; ++ch) {
            const auto& in = stereoInput[ch];
            auto& out = result.channels[ch];

            for (size_t i = 0; i < outputSampleCount; ++i) {
                double srcPos = static_cast<double>(i) / resampleRatio;
                int idx = static_cast<int>(std::floor(srcPos));
                float frac = static_cast<float>(srcPos - idx);

                if (idx < 0) {
                    out[i] = in[0];
                } else if (idx + 2 >= static_cast<int>(inputSampleCount)) {
                    out[i] = in.back();
                } else {
                    // Cubic Hermite spline interpolation
                    float y0 = (idx > 0) ? in[idx - 1] : in[idx];
                    float y1 = in[idx];
                    float y2 = in[idx + 1];
                    float y3 = (idx + 2 < static_cast<int>(inputSampleCount)) ? in[idx + 2] : y2;

                    float c0 = y1;
                    float c1 = 0.5f * (y2 - y0);
                    float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
                    float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

                    out[i] = ((c3 * frac + c2) * frac + c1) * frac + c0;
                }
            }
        }

        return result;
    }
};

} // namespace reggaewave::audio

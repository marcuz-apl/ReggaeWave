#pragma once

#include <vector>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <numbers>

namespace reggaewave::fixtures {

/**
 * @brief Programmatic synthesizer of audio test fixtures with zero unlicensed external files.
 */
class AudioSynthesizer {
public:
    /**
     * @brief Generates a valid in-memory PCM WAV byte buffer.
     */
    static std::vector<uint8_t> generateWav(double frequencyHz,
                                           double durationSeconds,
                                           int sampleRate = 44100,
                                           int numChannels = 2,
                                           int bitsPerSample = 16) {
        const size_t numSamples = static_cast<size_t>(durationSeconds * sampleRate);
        const int bytesPerSample = bitsPerSample / 8;
        const uint32_t dataChunkSize = static_cast<uint32_t>(numSamples * numChannels * bytesPerSample);
        const uint32_t totalFileSize = 36 + dataChunkSize;

        std::vector<uint8_t> wav(44 + dataChunkSize, 0);

        // 1. RIFF header
        std::memcpy(&wav[0], "RIFF", 4);
        std::memcpy(&wav[4], &totalFileSize, 4);
        std::memcpy(&wav[8], "WAVE", 4);

        // 2. fmt chunk
        std::memcpy(&wav[12], "fmt ", 4);
        uint32_t fmtChunkSize = 16;
        std::memcpy(&wav[16], &fmtChunkSize, 4);
        uint16_t audioFormat = 1; // PCM
        std::memcpy(&wav[20], &audioFormat, 2);
        uint16_t channels = static_cast<uint16_t>(numChannels);
        std::memcpy(&wav[22], &channels, 2);
        uint32_t sRate = static_cast<uint32_t>(sampleRate);
        std::memcpy(&wav[24], &sRate, 4);
        uint32_t byteRate = sRate * channels * bytesPerSample;
        std::memcpy(&wav[28], &byteRate, 4);
        uint16_t blockAlign = channels * bytesPerSample;
        std::memcpy(&wav[32], &blockAlign, 2);
        uint16_t bits = static_cast<uint16_t>(bitsPerSample);
        std::memcpy(&wav[34], &bits, 2);

        // 3. data chunk
        std::memcpy(&wav[36], "data", 4);
        std::memcpy(&wav[40], &dataChunkSize, 4);

        // 4. Synthesize Sine Tone Samples
        uint8_t* samplePtr = &wav[44];
        for (size_t i = 0; i < numSamples; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            double sampleVal = std::sin(2.0 * std::numbers::pi * frequencyHz * t) * 0.75; // -2.5 dBFS

            for (int ch = 0; ch < numChannels; ++ch) {
                if (bitsPerSample == 16) {
                    int16_t intSample = static_cast<int16_t>(sampleVal * 32767.0);
                    std::memcpy(samplePtr, &intSample, 2);
                    samplePtr += 2;
                } else if (bitsPerSample == 24) {
                    int32_t intSample = static_cast<int32_t>(sampleVal * 8388607.0);
                    samplePtr[0] = static_cast<uint8_t>(intSample & 0xFF);
                    samplePtr[1] = static_cast<uint8_t>((intSample >> 8) & 0xFF);
                    samplePtr[2] = static_cast<uint8_t>((intSample >> 16) & 0xFF);
                    samplePtr += 3;
                }
            }
        }

        return wav;
    }
};

} // namespace reggaewave::fixtures

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <algorithm>

namespace reggaewave::audio {

struct DecodedAudio {
    std::vector<std::vector<float>> channels;
    double sampleRate = 44100.0;
    int numChannels = 2;
    size_t numSamples = 0;
    double durationSeconds = 0.0;
};

/**
 * @brief Pure C++ audio file decoder for PCM WAV files with header parsing.
 */
class AudioDecoder {
public:
    /**
     * @brief Decodes a PCM WAV file from raw byte buffer or file path.
     */
    static DecodedAudio decodeWavBytes(const uint8_t* data, size_t sizeBytes) {
        if (!data || sizeBytes < 44) {
            throw std::runtime_error("Invalid or corrupted WAV file: buffer too small");
        }

        // Check "RIFF" and "WAVE"
        if (std::memcmp(data, "RIFF", 4) != 0 || std::memcmp(data + 8, "WAVE", 4) != 0) {
            throw std::runtime_error("Invalid audio format: Missing RIFF/WAVE header");
        }

        size_t offset = 12;
        int numChannels = 0;
        int sampleRate = 0;
        int bitsPerSample = 0;
        int audioFormat = 0; // 1 = PCM, 3 = IEEE Float
        const uint8_t* dataChunkPtr = nullptr;
        size_t dataChunkSize = 0;

        while (offset + 8 <= sizeBytes) {
            char chunkId[5] = {0};
            std::memcpy(chunkId, data + offset, 4);
            uint32_t chunkSize = 0;
            std::memcpy(&chunkSize, data + offset + 4, 4);
            offset += 8;

            if (std::strcmp(chunkId, "fmt ") == 0 && chunkSize >= 16) {
                std::memcpy(&audioFormat, data + offset, 2);
                std::memcpy(&numChannels, data + offset + 2, 2);
                std::memcpy(&sampleRate, data + offset + 4, 4);
                std::memcpy(&bitsPerSample, data + offset + 14, 2);
            } else if (std::strcmp(chunkId, "data") == 0) {
                dataChunkPtr = data + offset;
                dataChunkSize = std::min(static_cast<size_t>(chunkSize), sizeBytes - offset);
            }

            offset += chunkSize;
            if (chunkSize % 2 != 0) offset++; // Word alignment
        }

        if (!dataChunkPtr || numChannels <= 0 || sampleRate <= 0) {
            throw std::runtime_error("Corrupted WAV file: Missing fmt or data chunks");
        }

        int bytesPerSample = bitsPerSample / 8;
        if (bytesPerSample <= 0 || numChannels <= 0) {
            throw std::runtime_error("Invalid bit depth or channel count in WAV header");
        }

        size_t totalSamples = dataChunkSize / (numChannels * bytesPerSample);
        DecodedAudio result;
        result.sampleRate = static_cast<double>(sampleRate);
        result.numChannels = numChannels;
        result.numSamples = totalSamples;
        result.durationSeconds = static_cast<double>(totalSamples) / result.sampleRate;
        result.channels.assign(numChannels, std::vector<float>(totalSamples, 0.0f));

        const uint8_t* ptr = dataChunkPtr;
        for (size_t s = 0; s < totalSamples; ++s) {
            for (int ch = 0; ch < numChannels; ++ch) {
                float sampleVal = 0.0f;
                if (bitsPerSample == 16 && audioFormat == 1) {
                    int16_t raw = 0;
                    std::memcpy(&raw, ptr, 2);
                    sampleVal = static_cast<float>(raw) / 32768.0f;
                    ptr += 2;
                } else if (bitsPerSample == 24 && audioFormat == 1) {
                    int32_t raw = (ptr[0] | (ptr[1] << 8) | (ptr[2] << 16));
                    if (raw & 0x800000) raw |= ~0xFFFFFF; // Sign extend 24-bit
                    sampleVal = static_cast<float>(raw) / 8388608.0f;
                    ptr += 3;
                } else if (bitsPerSample == 32 && audioFormat == 3) {
                    std::memcpy(&sampleVal, ptr, 4);
                    ptr += 4;
                } else {
                    throw std::runtime_error("Unsupported WAV format: format=" + std::to_string(audioFormat) + 
                                             ", bits=" + std::to_string(bitsPerSample));
                }
                result.channels[ch][s] = std::clamp(sampleVal, -1.0f, 1.0f);
            }
        }

        return result;
    }

    static DecodedAudio decodeWavFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filePath);
        }
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(fileSize);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
            throw std::runtime_error("Could not read file: " + filePath);
        }

        return decodeWavBytes(buffer.data(), buffer.size());
    }
};

} // namespace reggaewave::audio

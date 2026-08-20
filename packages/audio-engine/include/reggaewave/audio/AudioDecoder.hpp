#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <array>
#include <memory>
#include <cstdio>

#if __has_include(<juce_audio_formats/juce_audio_formats.h>)
#include <juce_audio_formats/juce_audio_formats.h>
#define HAS_JUCE_AUDIO_FORMATS 1
#endif

#if defined(_WIN32)
#define rw_popen _popen
#define rw_pclose _pclose
#else
#define rw_popen popen
#define rw_pclose pclose
#endif

namespace reggaewave::audio {

struct DecodedAudio {
    std::vector<std::vector<float>> channels;
    double sampleRate = 44100.0;
    int numChannels = 2;
    size_t numSamples = 0;
    double durationSeconds = 0.0;
};

/**
 * @brief Pure C++ audio file decoder with native multi-format (M4A/MP3/FLAC/WAV/AAC) support.
 */
class AudioDecoder {
public:
    /**
     * @brief Decodes any audio file using native OS / JUCE decoders (WMF on Windows, CoreAudio on macOS)
     * with WAV and ffmpeg fallback.
     */
    static DecodedAudio decodeAnyAudioFile(const std::string& filePath) {
#if HAS_JUCE_AUDIO_FORMATS
        // 1. Try native JUCE OS Decoders (Windows Media Foundation / CoreAudio / Built-in MP3/FLAC/OGG/WAV)
        try {
            juce::AudioFormatManager formatMgr;
            formatMgr.registerBasicFormats();

            juce::File audioFile(filePath);
            if (audioFile.existsAsFile()) {
                std::unique_ptr<juce::AudioFormatReader> reader(formatMgr.createReaderFor(audioFile));
                if (reader != nullptr && reader->lengthInSamples > 0) {
                    DecodedAudio decoded;
                    decoded.sampleRate = reader->sampleRate;
                    decoded.numChannels = std::max(1, static_cast<int>(reader->numChannels));
                    decoded.numSamples = static_cast<size_t>(reader->lengthInSamples);
                    decoded.durationSeconds = (decoded.sampleRate > 0) ? (static_cast<double>(decoded.numSamples) / decoded.sampleRate) : 0.0;

                    int channelsToRead = std::min(decoded.numChannels, 2);
                    decoded.channels.assign(2, std::vector<float>(decoded.numSamples, 0.0f));

                    juce::AudioBuffer<float> tempBuf(channelsToRead, static_cast<int>(decoded.numSamples));
                    reader->read(&tempBuf, 0, static_cast<int>(decoded.numSamples), 0, true, true);

                    for (int ch = 0; ch < channelsToRead; ++ch) {
                        const float* src = tempBuf.getReadPointer(ch);
                        std::copy(src, src + decoded.numSamples, decoded.channels[ch].data());
                    }
                    if (channelsToRead == 1) {
                        // Duplicate mono track to stereo
                        decoded.channels[1] = decoded.channels[0];
                    }
                    if (decoded.numSamples > 0) {
                        return decoded;
                    }
                }
            }
        } catch (...) {
            // Fall through to direct WAV decoder and ffmpeg
        }
#endif

        // 2. Try direct built-in PCM WAV decode
        try {
            return decodeWavFile(filePath);
        } catch (...) {
            // Fall back to ffmpeg pipe transcoding
        }

        // 3. Transcode via ffmpeg to standard 44.1 kHz 16-bit stereo WAV in memory
#if defined(_WIN32)
        std::string cmd = "ffmpeg -v quiet -i \"" + filePath + "\" -f wav -ac 2 -ar 44100 -c:a pcm_s16le - 2>nul";
#else
        std::string cmd = "ffmpeg -v quiet -i \"" + filePath + "\" -f wav -ac 2 -ar 44100 -c:a pcm_s16le - 2>/dev/null";
#endif
        FILE* rawPipe = rw_popen(cmd.c_str(), "r");
        if (!rawPipe) {
            throw std::runtime_error("Could not decode audio file: " + filePath + ". (Please ensure the file is a valid audio track or export to WAV)");
        }

        std::vector<uint8_t> wavBuffer;
        std::array<uint8_t, 8192> chunk;
        size_t bytesRead = 0;
        while ((bytesRead = fread(chunk.data(), 1, chunk.size(), rawPipe)) > 0) {
            wavBuffer.insert(wavBuffer.end(), chunk.begin(), chunk.begin() + bytesRead);
        }
        rw_pclose(rawPipe);

        if (wavBuffer.size() < 44) {
            throw std::runtime_error("Could not decode audio file: " + filePath + ". (Format not recognized by system decoders)");
        }

        return decodeWavBytes(wavBuffer.data(), wavBuffer.size());
    }

    /**
     * @brief Decodes a PCM WAV file from raw byte buffer.
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

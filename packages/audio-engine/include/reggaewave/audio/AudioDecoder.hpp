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
#include <cctype>

#if defined(__ANDROID__)
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

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

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <objbase.h>
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")
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
    static bool isContentUri(const std::string& filePath) {
        return filePath.rfind("content://", 0) == 0;
    }

    static bool isUsableInputReference(const std::string& filePath, bool existsAsFile) {
        return !filePath.empty() && (existsAsFile || isContentUri(filePath));
    }

    static bool canUseFilesystemReader(const std::string& inputReference) {
        return !isContentUri(inputReference);
    }

    static std::string inputReferenceFromUrl(const std::string& url,
                                             const std::string& localFilePath) {
        if (isContentUri(url)) return url;
        return localFilePath.empty() ? url : localFilePath;
    }

    static bool isSupportedAudioInputName(const std::string& fileName) {
        const auto suffixStart = fileName.find_last_of('.');
        if (suffixStart == std::string::npos) return false;

        auto extension = fileName.substr(suffixStart);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char value) { return static_cast<char>(std::tolower(value)); });

        static constexpr std::array<const char*, 6> supported{
            ".mp3", ".wav", ".m4a", ".flac", ".aac", ".ogg"
        };
        return std::any_of(supported.begin(), supported.end(),
                           [&extension](const char* candidate) { return extension == candidate; });
    }

    static DecodedAudio fromInterleavedPcm16(const std::int16_t* samples,
                                             size_t frames,
                                             int sourceChannels,
                                             double sampleRate) {
        if (samples == nullptr || frames == 0 || sourceChannels <= 0 || sampleRate <= 0.0)
            throw std::runtime_error("Invalid platform PCM audio output");

        const int channelsToRead = std::min(sourceChannels, 2);
        DecodedAudio result;
        result.sampleRate = sampleRate;
        result.numChannels = 2;
        result.numSamples = frames;
        result.durationSeconds = static_cast<double>(frames) / sampleRate;
        result.channels.assign(2, std::vector<float>(frames, 0.0f));

        for (size_t frame = 0; frame < frames; ++frame) {
            result.channels[0][frame] = static_cast<float>(samples[frame * sourceChannels]) / 32768.0f;
            result.channels[1][frame] = channelsToRead > 1
                ? static_cast<float>(samples[frame * sourceChannels + 1]) / 32768.0f
                : result.channels[0][frame];
        }
        return result;
    }

#if defined(__ANDROID__)
    static DecodedAudio decodeViaAndroidMediaCodec(const std::string& filePath) {
        AMediaExtractor* extractor = AMediaExtractor_new();
        AMediaCodec* codec = nullptr;
        AMediaFormat* trackFormat = nullptr;
        AMediaFormat* outputFormat = nullptr;
        juce::File temporarySource;
        int sourceFileDescriptor = -1;
        bool started = false;

        auto cleanup = [&] {
            if (started) AMediaCodec_stop(codec);
            if (codec) AMediaCodec_delete(codec);
            if (outputFormat) AMediaFormat_delete(outputFormat);
            if (trackFormat) AMediaFormat_delete(trackFormat);
            if (extractor) AMediaExtractor_delete(extractor);
            if (sourceFileDescriptor >= 0) ::close(sourceFileDescriptor);
            if (temporarySource.existsAsFile()) temporarySource.deleteFile();
        };

        try {
            std::string sourcePath = filePath;
            if (isContentUri(filePath)) {
                juce::URL contentUrl(filePath);
                if (auto document = juce::AndroidDocument::fromDocument(contentUrl)) {
                    auto input = document.createInputStream();
                    temporarySource = juce::File::createTempFile(".reggaewave-audio");
                    if (input) {
                        if (auto output = temporarySource.createOutputStream()) {
                            output->writeFromInputStream(*input, -1);
                            output->flush();
                            sourcePath = temporarySource.getFullPathName().toStdString();
                        }
                    }
                }
            }

            struct stat sourceInfo{};
            sourceFileDescriptor = ::open(sourcePath.c_str(), O_RDONLY);
            if (!extractor || sourcePath.empty() || sourceFileDescriptor < 0
                || ::fstat(sourceFileDescriptor, &sourceInfo) != 0 || sourceInfo.st_size <= 0
                || AMediaExtractor_setDataSourceFd(extractor, sourceFileDescriptor, 0,
                                                   static_cast<off64_t>(sourceInfo.st_size)) != AMEDIA_OK)
                throw std::runtime_error("Android media extractor could not open the file");

            size_t audioTrack = static_cast<size_t>(-1);
            const size_t trackCount = AMediaExtractor_getTrackCount(extractor);
            for (size_t i = 0; i < trackCount; ++i) {
                AMediaFormat* candidate = AMediaExtractor_getTrackFormat(extractor, i);
                const char* mime = nullptr;
                const bool isAudio = candidate && AMediaFormat_getString(candidate, AMEDIAFORMAT_KEY_MIME, &mime)
                    && mime && std::strncmp(mime, "audio/", 6) == 0;
                if (isAudio && audioTrack == static_cast<size_t>(-1)) {
                    audioTrack = i;
                    trackFormat = candidate;
                } else if (candidate) {
                    AMediaFormat_delete(candidate);
                }
            }
            if (audioTrack == static_cast<size_t>(-1) || !trackFormat)
                throw std::runtime_error("Android media extractor found no audio track");

            const char* mime = nullptr;
            if (!AMediaFormat_getString(trackFormat, AMEDIAFORMAT_KEY_MIME, &mime) || !mime)
                throw std::runtime_error("Android media track has no decoder MIME type");
            AMediaExtractor_selectTrack(extractor, audioTrack);
            codec = AMediaCodec_createDecoderByType(mime);
            if (!codec || AMediaCodec_configure(codec, trackFormat, nullptr, nullptr, 0) != AMEDIA_OK
                || AMediaCodec_start(codec) != AMEDIA_OK)
                throw std::runtime_error("Android MediaCodec could not decode the audio track");
            started = true;

            int32_t sampleRate = 44100;
            int32_t channels = 2;
            int32_t pcmEncoding = 2; // AudioFormat.ENCODING_PCM_16BIT
            AMediaFormat_getInt32(trackFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, &sampleRate);
            AMediaFormat_getInt32(trackFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &channels);
            AMediaFormat_getInt32(trackFormat, AMEDIAFORMAT_KEY_PCM_ENCODING, &pcmEncoding);
            bool inputDone = false;
            bool outputDone = false;
            std::vector<int16_t> pcm;

            for (int iterations = 0; !outputDone && iterations < 100000; ++iterations) {
                if (!inputDone) {
                    const ssize_t inputIndex = AMediaCodec_dequeueInputBuffer(codec, 10000);
                    if (inputIndex >= 0) {
                        size_t capacity = 0;
                        uint8_t* input = AMediaCodec_getInputBuffer(codec, static_cast<size_t>(inputIndex), &capacity);
                        const ssize_t sampleSize = AMediaExtractor_getSampleSize(extractor);
                        if (sampleSize < 0) {
                            AMediaCodec_queueInputBuffer(codec, static_cast<size_t>(inputIndex), 0, 0, 0,
                                                         AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
                            inputDone = true;
                        } else if (input && static_cast<size_t>(sampleSize) <= capacity) {
                            const ssize_t copied = AMediaExtractor_readSampleData(extractor, input,
                                                                                   static_cast<size_t>(sampleSize));
                            const int64_t timestamp = AMediaExtractor_getSampleTime(extractor);
                            AMediaCodec_queueInputBuffer(codec, static_cast<size_t>(inputIndex), 0,
                                                         static_cast<size_t>(std::max<ssize_t>(0, copied)),
                                                         static_cast<uint64_t>(std::max<int64_t>(0, timestamp)), 0);
                            AMediaExtractor_advance(extractor);
                        }
                    }
                }

                AMediaCodecBufferInfo info{};
                const ssize_t outputIndex = AMediaCodec_dequeueOutputBuffer(codec, &info, 10000);
                if (outputIndex >= 0) {
                    size_t outputCapacity = 0;
                    uint8_t* output = AMediaCodec_getOutputBuffer(codec, static_cast<size_t>(outputIndex), &outputCapacity);
                    if (output && info.size > 0) {
                        const size_t byteOffset = static_cast<size_t>(std::max<int32_t>(0, info.offset));
                        const size_t byteCount = static_cast<size_t>(info.size);
                        if (byteOffset + byteCount <= outputCapacity) {
                            if (pcmEncoding == 4) { // AudioFormat.ENCODING_PCM_FLOAT
                                const auto* pcmFloat = reinterpret_cast<const float*>(output + byteOffset);
                                const size_t sampleCount = byteCount / sizeof(float);
                                for (size_t i = 0; i < sampleCount; ++i) {
                                    const float value = std::clamp(pcmFloat[i], -1.0f, 1.0f);
                                    pcm.push_back(static_cast<int16_t>(value * 32767.0f));
                                }
                            } else {
                                const auto* pcm16 = reinterpret_cast<const int16_t*>(output + byteOffset);
                                pcm.insert(pcm.end(), pcm16, pcm16 + byteCount / sizeof(int16_t));
                            }
                        }
                    }
                    outputDone = (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) != 0;
                    AMediaCodec_releaseOutputBuffer(codec, static_cast<size_t>(outputIndex), false);
                } else if (outputIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
                    if (outputFormat) AMediaFormat_delete(outputFormat);
                    outputFormat = AMediaCodec_getOutputFormat(codec);
                    if (outputFormat) {
                        AMediaFormat_getInt32(outputFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, &sampleRate);
                        AMediaFormat_getInt32(outputFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &channels);
                        AMediaFormat_getInt32(outputFormat, AMEDIAFORMAT_KEY_PCM_ENCODING, &pcmEncoding);
                    }
                }
            }
            if (pcm.empty()) throw std::runtime_error("Android MediaCodec produced no PCM audio");
            auto result = fromInterleavedPcm16(pcm.data(), pcm.size() / static_cast<size_t>(std::max(1, channels)),
                                               std::max(1, channels), static_cast<double>(sampleRate));
            cleanup();
            return result;
        } catch (...) {
            cleanup();
            throw;
        }
    }
#endif

#if defined(_WIN32)
    static DecodedAudio decodeViaWindowsMediaFoundation(const std::string& filePath) {
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        bool coInitialized = SUCCEEDED(hr);

        hr = MFStartup(MF_VERSION);
        if (FAILED(hr)) {
            if (coInitialized) CoUninitialize();
            throw std::runtime_error("MFStartup failed");
        }

        int len = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, NULL, 0);
        std::wstring wPath(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &wPath[0], len);

        IMFSourceReader* pReader = nullptr;
        hr = MFCreateSourceReaderFromURL(wPath.c_str(), NULL, &pReader);
        if (FAILED(hr) || !pReader) {
            MFShutdown();
            if (coInitialized) CoUninitialize();
            throw std::runtime_error("MFCreateSourceReaderFromURL failed for: " + filePath);
        }

        pReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
        pReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);

        IMFMediaType* pPartialType = nullptr;
        MFCreateMediaType(&pPartialType);
        pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);

        hr = pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pPartialType);
        pPartialType->Release();

        bool isFloat = SUCCEEDED(hr);
        if (!isFloat) {
            IMFMediaType* pPcmType = nullptr;
            MFCreateMediaType(&pPcmType);
            pPcmType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
            pPcmType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
            hr = pReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, pPcmType);
            pPcmType->Release();
            if (FAILED(hr)) {
                pReader->Release();
                MFShutdown();
                if (coInitialized) CoUninitialize();
                throw std::runtime_error("Could not set uncompressed audio output on WMF reader");
            }
        }

        IMFMediaType* pUncompressedType = nullptr;
        hr = pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pUncompressedType);
        UINT32 sampleRate = 44100;
        UINT32 numChannels = 2;
        UINT32 bitsPerSample = isFloat ? 32 : 16;
        if (SUCCEEDED(hr) && pUncompressedType) {
            pUncompressedType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
            pUncompressedType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &numChannels);
            pUncompressedType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
            pUncompressedType->Release();
        }

        int channelsToUse = std::max(1, (int)numChannels);
        std::vector<std::vector<float>> channels(2);

        for (;;) {
            DWORD flags = 0;
            LONGLONG timeStamp = 0;
            IMFSample* pSample = nullptr;
            hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &flags, &timeStamp, &pSample);
            if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) {
                if (pSample) pSample->Release();
                break;
            }
            if (!pSample) continue;

            IMFMediaBuffer* pBuffer = nullptr;
            hr = pSample->ConvertToContiguousBuffer(&pBuffer);
            if (SUCCEEDED(hr) && pBuffer) {
                BYTE* pData = nullptr;
                DWORD maxLen = 0, curLen = 0;
                hr = pBuffer->Lock(&pData, &maxLen, &curLen);
                if (SUCCEEDED(hr) && pData && curLen > 0) {
                    if (isFloat && bitsPerSample == 32) {
                        size_t numFloats = curLen / sizeof(float);
                        size_t frames = numFloats / channelsToUse;
                        const float* fPtr = reinterpret_cast<const float*>(pData);
                        for (size_t f = 0; f < frames; ++f) {
                            float left = fPtr[f * channelsToUse];
                            float right = (channelsToUse > 1) ? fPtr[f * channelsToUse + 1] : left;
                            channels[0].push_back(left);
                            channels[1].push_back(right);
                        }
                    } else if (!isFloat && bitsPerSample == 16) {
                        size_t numShorts = curLen / sizeof(int16_t);
                        size_t frames = numShorts / channelsToUse;
                        const int16_t* sPtr = reinterpret_cast<const int16_t*>(pData);
                        for (size_t f = 0; f < frames; ++f) {
                            float left = static_cast<float>(sPtr[f * channelsToUse]) / 32768.0f;
                            float right = (channelsToUse > 1) ? (static_cast<float>(sPtr[f * channelsToUse + 1]) / 32768.0f) : left;
                            channels[0].push_back(left);
                            channels[1].push_back(right);
                        }
                    }
                    pBuffer->Unlock();
                }
                pBuffer->Release();
            }
            pSample->Release();
        }

        pReader->Release();
        MFShutdown();
        if (coInitialized) CoUninitialize();

        if (channels[0].empty()) {
            throw std::runtime_error("WMF decoded 0 audio frames from: " + filePath);
        }

        DecodedAudio result;
        result.sampleRate = static_cast<double>(sampleRate);
        result.numChannels = 2;
        result.numSamples = channels[0].size();
        result.durationSeconds = static_cast<double>(result.numSamples) / result.sampleRate;
        result.channels = std::move(channels);
        return result;
    }
#endif

    /**
     * @brief Decodes any audio file using native OS / JUCE decoders (WMF on Windows, CoreAudio on macOS)
     * with WAV and ffmpeg fallback.
     */
    static DecodedAudio decodeAnyAudioFile(const std::string& filePath) {
#if HAS_JUCE_AUDIO_FORMATS
        // 1. Try native JUCE OS Decoders (Windows Media Foundation / CoreAudio / Built-in MP3/FLAC/OGG/WAV)
        try {
            if (canUseFilesystemReader(filePath)) {
                juce::AudioFormatManager formatMgr;
                formatMgr.registerBasicFormats();

                juce::File audioFile(filePath);
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
            // Fall through to native WMF or direct WAV decoder
        }
#endif

#if defined(_WIN32)
        // 2. Try native Windows Media Foundation (M4A/AAC/MP4/MP3/ALAC/WAV/WMA/FLAC)
        try {
            return decodeViaWindowsMediaFoundation(filePath);
        } catch (...) {
            // Fall through to WAV decoder
        }
#endif

#if defined(__ANDROID__)
        std::string androidFailure;
        // Android JUCE does not provide an AAC/M4A reader; use the platform codec.
        try {
            return decodeViaAndroidMediaCodec(filePath);
        } catch (const std::exception& ex) {
            androidFailure = ex.what();
        } catch (...) {
            androidFailure = "unknown Android media decoder failure";
        }
#endif

        // 3. Try direct built-in PCM WAV decode
        try {
            return decodeWavFile(filePath);
        } catch (...) {
            // Fall back to ffmpeg pipe transcoding
        }

#if !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IPHONE)
        // 4. Transcode via ffmpeg to standard 44.1 kHz 16-bit stereo WAV in memory
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
#else
        std::string detail = ". Direct mobile decoding supports PCM WAV / JUCE native formats";
#if defined(__ANDROID__)
        if (!androidFailure.empty()) detail += "; Android media decoder: " + androidFailure;
#endif
        throw std::runtime_error("Could not decode audio file: " + filePath + detail + ".");
#endif
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

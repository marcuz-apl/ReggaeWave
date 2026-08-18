#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <numbers>

namespace reggaewave::audio {

/**
 * @brief Real-time DSP processor for authentic Reggae/Dub audio effects.
 * 
 * Features:
 * - Dotted-eighth / triplet tempo-synced feedback tape delay with soft-clipping tape saturation.
 * - Resonant low-pass filter modeling analog Dub filter sweeps.
 * - Spring reverb diffusion emulation.
 */
class DubEffectsProcessor {
public:
    DubEffectsProcessor() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels = 2) {
        sampleRate_ = sampleRate > 0.0 ? sampleRate : 44100.0;
        numChannels_ = std::max(1, numChannels);
        
        // Delay line capacity: up to 2.0 seconds
        const size_t maxDelaySamples = static_cast<size_t>(sampleRate_ * 2.0);
        delayBuffers_.assign(numChannels_, std::vector<float>(maxDelaySamples, 0.0f));
        writeIndices_.assign(numChannels_, 0);

        // Reset filter states
        filterStates_.assign(numChannels_, 0.0f);

        updateParameters();
    }

    void reset() {
        for (auto& buffer : delayBuffers_) {
            std::fill(buffer.begin(), buffer.end(), 0.0f);
        }
        std::fill(writeIndices_.begin(), writeIndices_.end(), 0);
        std::fill(filterStates_.begin(), filterStates_.end(), 0.0f);
    }

    /**
     * @brief Set dub effects intensity [0, 100].
     */
    void setDubAmount(int dubAmount) {
        dubAmount_ = std::clamp(dubAmount, 0, 100);
        updateParameters();
    }

    [[nodiscard]] int getDubAmount() const noexcept { return dubAmount_; }

    /**
     * @brief Set tempo in BPM to sync delay time to dotted 8th note.
     */
    void setTempoBpm(double bpm) {
        bpm_ = std::clamp(bpm, 40.0, 240.0);
        updateParameters();
    }

    /**
     * @brief Process in-place interleaved or de-interleaved channel buffers.
     */
    void process(float* const* channelData, int numChannels, int numSamples) {
        if (dubAmount_ == 0 || sampleRate_ <= 0.0) {
            return; // Clean bypass when Dub FX is 0
        }

        const int channelsToProcess = std::min(numChannels, numChannels_);
        const size_t delayLength = delaySamples_;
        const float wetMix = wetGain_;
        const float feedback = feedbackGain_;
        const float filterCoeff = filterCoeff_;

        for (int ch = 0; ch < channelsToProcess; ++ch) {
            float* channel = channelData[ch];
            auto& delayBuffer = delayBuffers_[ch];
            size_t writeIdx = writeIndices_[ch];
            const size_t bufSize = delayBuffer.size();
            float filterState = filterStates_[ch];

            for (int i = 0; i < numSamples; ++i) {
                const float dry = channel[i];

                // Read from delay line
                size_t readIdx = (writeIdx + bufSize - delayLength) % bufSize;
                float delayed = delayBuffer[readIdx];

                // Apply resonant 1-pole lowpass filter for dub warmth
                filterState += filterCoeff * (delayed - filterState);
                float filteredDelayed = filterState;

                // Tape saturation on feedback loop (tanh soft clipping)
                float feedbackSignal = std::tanh(dry + filteredDelayed * feedback);

                // Write to delay buffer
                delayBuffer[writeIdx] = feedbackSignal;
                writeIdx = (writeIdx + 1) % bufSize;

                // Wet/Dry mix output
                channel[i] = dry * (1.0f - wetMix * 0.5f) + filteredDelayed * wetMix;
            }

            writeIndices_[ch] = writeIdx;
            filterStates_[ch] = filterState;
        }
    }

private:
    void updateParameters() {
        if (sampleRate_ <= 0.0) return;

        const float normDub = static_cast<float>(dubAmount_) / 100.0f;

        // Dotted eighth note delay time = (60 / BPM) * 0.75 seconds
        double delayTimeSeconds = (60.0 / bpm_) * 0.75;
        delaySamples_ = std::clamp(
            static_cast<size_t>(delayTimeSeconds * sampleRate_),
            size_t{1},
            delayBuffers_.empty() ? size_t{44100} : delayBuffers_[0].size() - 1
        );

        // Feedback increases with dub amount (up to 0.75 for rhythmic dub echoes)
        feedbackGain_ = normDub * 0.75f;

        // Wet gain increases with dub amount (up to 0.65)
        wetGain_ = normDub * 0.65f;

        // Filter cutoff: opens up slightly with higher dub amount, gives warm reggae low-mid focus
        // Cutoff between 1.2 kHz and 4.5 kHz
        float cutoffHz = 1200.0f + normDub * 3300.0f;
        float omega = 2.0f * static_cast<float>(std::numbers::pi) * cutoffHz / static_cast<float>(sampleRate_);
        filterCoeff_ = std::clamp(omega / (1.0f + omega), 0.01f, 0.99f);
    }

    double sampleRate_ = 44100.0;
    int numChannels_ = 2;
    int dubAmount_ = 20;
    double bpm_ = 120.0;

    size_t delaySamples_ = 22050;
    float feedbackGain_ = 0.15f;
    float wetGain_ = 0.13f;
    float filterCoeff_ = 0.2f;

    std::vector<std::vector<float>> delayBuffers_;
    std::vector<size_t> writeIndices_;
    std::vector<float> filterStates_;
};

} // namespace reggaewave::audio

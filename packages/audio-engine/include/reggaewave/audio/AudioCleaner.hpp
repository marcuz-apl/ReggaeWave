#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstddef>

namespace reggaewave::audio {

/**
 * @brief Professional multi-stage audio restoration and pre-conditioning engine.
 *        Executes high-pass rumble removal, 50/60 Hz mains hum rejection,
 *        adaptive noise gating, and vocal stem de-bleed polish.
 */
class AudioCleaner {
public:
    /**
     * @brief Cleans full stereo audio buffer before separation & harmonic analysis.
     * @param channels Stereo 2-channel buffer [channels][samples]
     * @param sampleRate Audio sampling rate in Hz (e.g. 44100.0)
     */
    static void cleanStereo(std::vector<std::vector<float>>& channels, double sampleRate = 44100.0) {
        if (channels.empty() || channels[0].empty()) return;

        const size_t numChannels = channels.size();
        const size_t numSamples = channels[0].size();

        for (size_t ch = 0; ch < numChannels; ++ch) {
            // Stage 1a: High-pass rumble filter (35 Hz cutoff, 2nd order Butterworth)
            applyHighPassFilter(channels[ch], 35.0, sampleRate);

            // Stage 1b: Mains hum rejection (50 Hz and 60 Hz notch filters)
            applyNotchFilter(channels[ch], 50.0, 8.0, sampleRate);
            applyNotchFilter(channels[ch], 60.0, 8.0, sampleRate);

            // Stage 1c: Adaptive spectral noise gating
            applyNoiseGate(channels[ch], sampleRate, 0.003f, 0.05f);
        }
    }

    /**
     * @brief Polishes separated lead vocal stem by silencing instrument bleed & noise floor during pauses.
     * @param vocalChannels Stereo vocal buffer
     * @param sampleRate Sampling rate in Hz
     */
    static void polishVocalStem(std::vector<std::vector<float>>& vocalChannels, double sampleRate = 44100.0) {
        if (vocalChannels.empty() || vocalChannels[0].empty()) return;

        for (auto& chData : vocalChannels) {
            applyVocalDeBleed(chData, sampleRate);
        }
    }

private:
    /**
     * @brief 2nd-order IIR Butterworth High-Pass Filter.
     */
    static void applyHighPassFilter(std::vector<float>& buffer, double cutoffHz, double sampleRate) {
        if (buffer.empty()) return;

        const double pi = 3.14159265358979323846;
        const double w0 = 2.0 * pi * cutoffHz / sampleRate;
        const double cosW0 = std::cos(w0);
        const double sinW0 = std::sin(w0);
        const double alpha = sinW0 / (2.0 * 0.7071067811865475); // Q = 1/sqrt(2)

        const double b0 = (1.0 + cosW0) / 2.0;
        const double b1 = -(1.0 + cosW0);
        const double b2 = (1.0 + cosW0) / 2.0;
        const double a0 = 1.0 + alpha;
        const double a1 = -2.0 * cosW0;
        const double a2 = 1.0 - alpha;

        const double invA0 = 1.0 / a0;
        const double nb0 = b0 * invA0;
        const double nb1 = b1 * invA0;
        const double nb2 = b2 * invA0;
        const double na1 = a1 * invA0;
        const double na2 = a2 * invA0;

        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;

        for (size_t i = 0; i < buffer.size(); ++i) {
            double x0 = buffer[i];
            double y0 = nb0 * x0 + nb1 * x1 + nb2 * x2 - na1 * y1 - na2 * y2;
            x2 = x1;
            x1 = x0;
            y2 = y1;
            y1 = y0;
            buffer[i] = static_cast<float>(y0);
        }
    }

    /**
     * @brief 2nd-order IIR Notch (Band-Stop) Filter for hum rejection.
     */
    static void applyNotchFilter(std::vector<float>& buffer, double freqHz, double q, double sampleRate) {
        if (buffer.empty()) return;

        const double pi = 3.14159265358979323846;
        const double w0 = 2.0 * pi * freqHz / sampleRate;
        const double cosW0 = std::cos(w0);
        const double sinW0 = std::sin(w0);
        const double alpha = sinW0 / (2.0 * q);

        const double b0 = 1.0;
        const double b1 = -2.0 * cosW0;
        const double b2 = 1.0;
        const double a0 = 1.0 + alpha;
        const double a1 = -2.0 * cosW0;
        const double a2 = 1.0 - alpha;

        const double invA0 = 1.0 / a0;
        const double nb0 = b0 * invA0;
        const double nb1 = b1 * invA0;
        const double nb2 = b2 * invA0;
        const double na1 = a1 * invA0;
        const double na2 = a2 * invA0;

        double x1 = 0.0, x2 = 0.0;
        double y1 = 0.0, y2 = 0.0;

        for (size_t i = 0; i < buffer.size(); ++i) {
            double x0 = buffer[i];
            double y0 = nb0 * x0 + nb1 * x1 + nb2 * x2 - na1 * y1 - na2 * y2;
            x2 = x1;
            x1 = x0;
            y2 = y1;
            y1 = y0;
            buffer[i] = static_cast<float>(y0);
        }
    }

    /**
     * @brief Smooth RMS envelope follower with soft-knee noise floor reduction.
     */
    static void applyNoiseGate(std::vector<float>& buffer, double sampleRate, float thresholdRms = 0.003f, float attackSecs = 0.01f) {
        if (buffer.empty()) return;

        const float releaseSecs = 0.05f;
        const float attackCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * attackSecs));
        const float releaseCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * releaseSecs));

        float env = 0.0f;
        for (size_t i = 0; i < buffer.size(); ++i) {
            float absVal = std::abs(buffer[i]);
            if (absVal > env) {
                env = attackCoeff * env + (1.0f - attackCoeff) * absVal;
            } else {
                env = releaseCoeff * env + (1.0f - releaseCoeff) * absVal;
            }

            // Soft-knee expansion factor
            if (env < thresholdRms) {
                float ratio = env / (thresholdRms + 1e-6f);
                float gain = ratio * ratio; // Soft quadratic taper
                buffer[i] *= std::clamp(gain, 0.05f, 1.0f);
            }
        }
    }

    /**
     * @brief Dynamic vocal de-bleed expander suppressing background chaos in vocal breath pauses.
     */
    static void applyVocalDeBleed(std::vector<float>& buffer, double sampleRate) {
        if (buffer.empty()) return;

        const float vocalSilenceThreshold = 0.008f;
        const float attackCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.005f));
        const float releaseCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.080f));

        float env = 0.0f;
        for (size_t i = 0; i < buffer.size(); ++i) {
            float absVal = std::abs(buffer[i]);
            if (absVal > env) {
                env = attackCoeff * env + (1.0f - attackCoeff) * absVal;
            } else {
                env = releaseCoeff * env + (1.0f - releaseCoeff) * absVal;
            }

            if (env < vocalSilenceThreshold) {
                float factor = env / vocalSilenceThreshold;
                buffer[i] *= (factor * factor * factor); // Cubic falloff for silent pauses
            }
        }
    }
};

} // namespace reggaewave::audio

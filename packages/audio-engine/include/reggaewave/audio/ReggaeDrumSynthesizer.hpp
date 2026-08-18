#pragma once

#include <reggaewave/audio/MusicAnalyzer.hpp>
#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>

namespace reggaewave::audio {

enum class ReggaeDrumStyle {
    OneDrop,  // Accent on beat 3 (Kick + Snare/Rimshot together)
    Steppers, // Four-on-the-floor kick with heavy beat 3 snare
    Rockers   // Kick on 1 & 3, heavy snare on 3
};

/**
 * @brief Synthesizes authentic Reggae drum patterns (One-Drop, Steppers, Rockers).
 */
class ReggaeDrumSynthesizer {
public:
    static constexpr double SAMPLE_RATE = 44100.0;

    /**
     * @brief Synthesizes a stereo drum stem for the duration of the track.
     */
    static std::vector<std::vector<float>> synthesize(size_t totalSamples,
                                                      const BeatGrid& beatGrid,
                                                      ReggaeDrumStyle style,
                                                      int intensity = 70) {
        std::vector<std::vector<float>> drums(2, std::vector<float>(totalSamples, 0.0f));
        if (beatGrid.beatPositions.size() < 4) {
            return drums;
        }

        const float intensityNorm = static_cast<float>(intensity) / 100.0f;
        const size_t beatLen = beatGrid.beatIntervalSamples;

        // Render each bar (4 beats per bar)
        for (size_t beatIdx = 0; beatIdx < beatGrid.beatPositions.size(); ++beatIdx) {
            size_t beatPos = beatGrid.beatPositions[beatIdx];
            int beatInBar = static_cast<int>(beatIdx % 4) + 1; // 1, 2, 3, 4

            bool triggerKick = false;
            bool triggerSnare = false;
            bool triggerHiHat = true;
            bool triggerOpenHat = false;

            switch (style) {
                case ReggaeDrumStyle::OneDrop:
                    // Classic Roots: Kick & Snare hit simultaneously on beat 3 only
                    if (beatInBar == 3) {
                        triggerKick = true;
                        triggerSnare = true;
                    }
                    if (beatInBar == 2 || beatInBar == 4) {
                        triggerOpenHat = (intensity >= 60);
                    }
                    break;

                case ReggaeDrumStyle::Steppers:
                    // Steppers: Kick on every beat 1, 2, 3, 4; heavy snare on 3
                    triggerKick = true;
                    if (beatInBar == 3) {
                        triggerSnare = true;
                    }
                    break;

                case ReggaeDrumStyle::Rockers:
                    // Rockers: Kick on 1 & 3; snare on 3
                    if (beatInBar == 1 || beatInBar == 3) {
                        triggerKick = true;
                    }
                    if (beatInBar == 3) {
                        triggerSnare = true;
                    }
                    break;
            }

            // Synthesize Kick Drum
            if (triggerKick) {
                renderKick(drums, beatPos, totalSamples, style == ReggaeDrumStyle::Steppers ? 0.9f : 0.8f);
            }

            // Synthesize Snare / Rimshot
            if (triggerSnare) {
                renderSnare(drums, beatPos, totalSamples, 0.85f * intensityNorm);
            }

            // Synthesize Closed & Offbeat Hi-Hats (driving 8th/16th shuffle)
            if (triggerHiHat) {
                renderClosedHat(drums, beatPos, totalSamples, 0.4f);
                // Offbeat 8th note hat (the "&" of the beat)
                size_t offbeatPos = beatPos + beatLen / 2;
                if (offbeatPos < totalSamples) {
                    if (triggerOpenHat) {
                        renderOpenHat(drums, offbeatPos, totalSamples, 0.5f * intensityNorm);
                    } else {
                        renderClosedHat(drums, offbeatPos, totalSamples, 0.35f);
                    }
                }
            }
        }

        return drums;
    }

private:
    static void renderKick(std::vector<std::vector<float>>& buffer, size_t startPos, size_t maxSamples, float gain) {
        const size_t len = static_cast<size_t>(0.25 * SAMPLE_RATE); // 250ms
        for (size_t i = 0; i < len && startPos + i < maxSamples; ++i) {
            double t = static_cast<double>(i) / SAMPLE_RATE;
            // Pitch drop from 130 Hz down to 45 Hz (classic warm analog reggae kick)
            double freq = 45.0 + 85.0 * std::exp(-t * 28.0);
            double env = std::exp(-t * 12.0);
            float sample = static_cast<float>(std::sin(2.0 * std::numbers::pi * freq * t) * env * gain);

            buffer[0][startPos + i] += sample;
            buffer[1][startPos + i] += sample;
        }
    }

    static void renderSnare(std::vector<std::vector<float>>& buffer, size_t startPos, size_t maxSamples, float gain) {
        const size_t len = static_cast<size_t>(0.20 * SAMPLE_RATE); // 200ms
        for (size_t i = 0; i < len && startPos + i < maxSamples; ++i) {
            double t = static_cast<double>(i) / SAMPLE_RATE;
            // Snare tone (200 Hz) + white noise burst for snappy rimshot crack
            double tone = std::sin(2.0 * std::numbers::pi * 210.0 * t) * std::exp(-t * 30.0);
            float noise = static_cast<float>((std::rand() % 2000 - 1000) / 1000.0) * std::exp(-t * 22.0f);
            float sample = static_cast<float>((tone * 0.4 + noise * 0.6) * gain);

            buffer[0][startPos + i] += sample;
            buffer[1][startPos + i] += sample;
        }
    }

    static void renderClosedHat(std::vector<std::vector<float>>& buffer, size_t startPos, size_t maxSamples, float gain) {
        const size_t len = static_cast<size_t>(0.04 * SAMPLE_RATE); // 40ms short tick
        for (size_t i = 0; i < len && startPos + i < maxSamples; ++i) {
            double t = static_cast<double>(i) / SAMPLE_RATE;
            float noise = static_cast<float>((std::rand() % 2000 - 1000) / 1000.0) * std::exp(-t * 80.0f);
            float sample = noise * gain;

            buffer[0][startPos + i] += sample * 0.9f;
            buffer[1][startPos + i] += sample * 1.1f; // Slight stereo pan
        }
    }

    static void renderOpenHat(std::vector<std::vector<float>>& buffer, size_t startPos, size_t maxSamples, float gain) {
        const size_t len = static_cast<size_t>(0.15 * SAMPLE_RATE); // 150ms open sizzle
        for (size_t i = 0; i < len && startPos + i < maxSamples; ++i) {
            double t = static_cast<double>(i) / SAMPLE_RATE;
            float noise = static_cast<float>((std::rand() % 2000 - 1000) / 1000.0) * std::exp(-t * 18.0f);
            float sample = noise * gain;

            buffer[0][startPos + i] += sample * 0.8f;
            buffer[1][startPos + i] += sample * 1.2f;
        }
    }
};

} // namespace reggaewave::audio

#pragma once

#include <reggaewave/audio/MusicAnalyzer.hpp>
#include <vector>
#include <string>
#include <cmath>
#include <numbers>
#include <algorithm>
#include <unordered_map>

namespace reggaewave::audio {

/**
 * @brief Generates authentic, deep melodic Reggae sub-basslines.
 */
class ReggaeBassGenerator {
public:
    static constexpr double SAMPLE_RATE = 44100.0;

    static double getNoteFrequency(const std::string& chordName) {
        // Map chord root to bass octave 1/2 frequency (40 Hz - 120 Hz)
        static const std::unordered_map<std::string, double> rootFreqs = {
            {"C", 65.41},  // C2
            {"C#", 69.30},
            {"D", 73.42},  // D2
            {"D#", 77.78},
            {"E", 82.41},  // E2
            {"F", 43.65},  // F1
            {"F#", 46.25},
            {"G", 49.00},  // G1
            {"G#", 51.91},
            {"A", 55.00},  // A1
            {"A#", 58.27},
            {"B", 61.74}   // B1
        };

        std::string root = chordName;
        if (!root.empty() && root.back() == 'm') {
            root.pop_back(); // Remove 'm' for minor
        }
        auto it = rootFreqs.find(root);
        return (it != rootFreqs.end()) ? it->second : 55.0; // Default A1
    }

    /**
     * @brief Synthesizes a deep stereo sub-bass track mapped to chord changes.
     */
    static std::vector<std::vector<float>> synthesize(size_t totalSamples,
                                                      const BeatGrid& beatGrid,
                                                      const std::vector<ChordEvent>& chords,
                                                      int intensity = 70) {
        std::vector<std::vector<float>> bass(2, std::vector<float>(totalSamples, 0.0f));
        if (chords.empty() || beatGrid.beatPositions.empty()) {
            return bass;
        }

        const float intensityNorm = static_cast<float>(intensity) / 100.0f;
        const size_t beatLen = beatGrid.beatIntervalSamples;

        // Render bass notes per bar
        for (const auto& chord : chords) {
            double rootFreq = getNoteFrequency(chord.chordName);
            double fifthFreq = rootFreq * 1.5; // Perfect 5th

            // Reggae Bass Pattern per 4-beat bar:
            // Beat 1: Rest or light ghost note (classic reggae space)
            // Beat 2: Root note hit
            // Beat 3: Heavy Root / Octave hit locked with drum One-Drop
            // Beat 4: 5th note melodic pickup into next bar

            size_t barStart = chord.startSample;
            size_t noteLen = static_cast<size_t>(beatLen * 0.85); // Staccato / rounded decay

            // Note 1: Beat 2
            size_t pos2 = barStart + beatLen;
            if (pos2 < totalSamples) {
                renderBassNote(bass, pos2, std::min(noteLen, totalSamples - pos2), rootFreq, 0.75f * intensityNorm);
            }

            // Note 2: Beat 3 (Main accent)
            size_t pos3 = barStart + beatLen * 2;
            if (pos3 < totalSamples) {
                renderBassNote(bass, pos3, std::min(noteLen, totalSamples - pos3), rootFreq, 0.90f * intensityNorm);
            }

            // Note 3: Beat 4 ("and" pickup or 5th)
            size_t pos4 = barStart + beatLen * 3 + beatLen / 2;
            if (pos4 < totalSamples && intensity >= 50) {
                renderBassNote(bass, pos4, std::min(noteLen / 2, totalSamples - pos4), fifthFreq, 0.65f * intensityNorm);
            }
        }

        return bass;
    }

private:
    static void renderBassNote(std::vector<std::vector<float>>& buffer,
                               size_t startPos,
                               size_t noteLength,
                               double freqHz,
                               float gain) {
        for (size_t i = 0; i < noteLength && startPos + i < buffer[0].size(); ++i) {
            double t = static_cast<double>(i) / SAMPLE_RATE;
            // Warm sub-bass: Fundamental sine + 2nd harmonic for audibility on smaller speakers
            double f1 = std::sin(2.0 * std::numbers::pi * freqHz * t);
            double f2 = std::sin(4.0 * std::numbers::pi * freqHz * t) * 0.25;

            // Envelope with smooth attack (5ms) and exponential decay
            double env = (1.0 - std::exp(-t * 200.0)) * std::exp(-t * 3.5);

            // Soft tape saturation (tanh)
            float sample = static_cast<float>(std::tanh((f1 + f2) * 1.2) * env * gain);

            buffer[0][startPos + i] += sample;
            buffer[1][startPos + i] += sample;
        }
    }
};

} // namespace reggaewave::audio

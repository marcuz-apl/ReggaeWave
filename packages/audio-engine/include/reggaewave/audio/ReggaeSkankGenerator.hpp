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
 * @brief Generates offbeat piano/guitar skank chops and organ bubble patterns.
 */
class ReggaeSkankGenerator {
public:
    static constexpr double SAMPLE_RATE = 44100.0;

    static std::vector<double> getChordFrequencies(const std::string& chordName) {
        // Return mid-range triad frequencies (Octave 3/4: 200 Hz - 800 Hz)
        static const std::unordered_map<std::string, std::vector<double>> triadMap = {
            {"C",  {261.63, 329.63, 392.00}}, // C4, E4, G4
            {"Cm", {261.63, 311.13, 392.00}},
            {"D",  {293.66, 369.99, 440.00}},
            {"Dm", {293.66, 349.23, 440.00}}, // D4, F4, A4
            {"E",  {329.63, 415.30, 493.88}},
            {"Em", {329.63, 392.00, 493.88}},
            {"F",  {349.23, 440.00, 523.25}}, // F4, A4, C5
            {"Fm", {349.23, 415.30, 523.25}},
            {"G",  {392.00, 493.88, 587.33}}, // G4, B4, D5
            {"Gm", {392.00, 466.16, 587.33}},
            {"A",  {440.00, 554.37, 659.25}},
            {"Am", {440.00, 523.25, 659.25}}, // A4, C5, E5
            {"B",  {493.88, 622.25, 739.99}},
            {"Bm", {493.88, 587.33, 739.99}}
        };

        auto it = triadMap.find(chordName);
        if (it != triadMap.end()) return it->second;
        return {261.63, 329.63, 392.00}; // Default C Major
    }

    /**
     * @brief Synthesizes stereo skank & organ bubble accompaniment.
     */
    static std::vector<std::vector<float>> synthesize(size_t totalSamples,
                                                      const BeatGrid& beatGrid,
                                                      const std::vector<ChordEvent>& chords,
                                                      bool includeOrganBubble = true,
                                                      int intensity = 70) {
        std::vector<std::vector<float>> skank(2, std::vector<float>(totalSamples, 0.0f));
        if (chords.empty() || beatGrid.beatPositions.empty()) {
            return skank;
        }

        const float intensityNorm = static_cast<float>(intensity) / 100.0f;
        const size_t beatLen = beatGrid.beatIntervalSamples;

        for (const auto& chord : chords) {
            auto freqs = getChordFrequencies(chord.chordName);
            size_t barStart = chord.startSample;

            // In 4/4 Reggae: Skank hits on the upbeat "&" of each beat (beats 1&, 2&, 3&, 4&)
            for (int b = 0; b < 4; ++b) {
                size_t offbeatPos = barStart + b * beatLen + beatLen / 2;
                if (offbeatPos < totalSamples) {
                    // Staccato guitar/piano chop
                    renderSkankChop(skank, offbeatPos, freqs, 0.45f * intensityNorm);
                }

                // Organ bubble: 16th note rolling patterns
                if (includeOrganBubble && intensity >= 40) {
                    size_t bubblePos1 = barStart + b * beatLen + beatLen / 4;
                    size_t bubblePos2 = barStart + b * beatLen + (3 * beatLen) / 4;
                    if (bubblePos1 < totalSamples) {
                        renderOrganBubble(skank, bubblePos1, freqs, 0.25f * intensityNorm);
                    }
                    if (bubblePos2 < totalSamples) {
                        renderOrganBubble(skank, bubblePos2, freqs, 0.20f * intensityNorm);
                    }
                }
            }
        }

        return skank;
    }

private:
    static void renderSkankChop(std::vector<std::vector<float>>& buffer,
                                size_t startPos,
                                const std::vector<double>& freqs,
                                float gain) {
        const size_t chopLen = static_cast<size_t>(0.07 * SAMPLE_RATE); // 70ms sharp chop
        for (size_t i = 0; i < chopLen && startPos + i < buffer[0].size(); ++i) {
            double t = static_cast<double>(i) / SAMPLE_RATE;
            double chordSig = 0.0;
            for (double f : freqs) {
                chordSig += std::sin(2.0 * std::numbers::pi * f * t);
            }
            chordSig /= freqs.size();

            // Exponential staccato decay envelope
            double env = std::exp(-t * 40.0);
            float sample = static_cast<float>(chordSig * env * gain);

            // Wide stereo placement for skank guitars/keys
            buffer[0][startPos + i] += sample * 0.8f;
            buffer[1][startPos + i] += sample * 1.2f;
        }
    }

    static void renderOrganBubble(std::vector<std::vector<float>>& buffer,
                                  size_t startPos,
                                  const std::vector<double>& freqs,
                                  float gain) {
        const size_t bubbleLen = static_cast<size_t>(0.05 * SAMPLE_RATE); // 50ms bubble roll
        for (size_t i = 0; i < bubbleLen && startPos + i < buffer[0].size(); ++i) {
            double t = static_cast<double>(i) / SAMPLE_RATE;
            // Drawbar organ sound: Sine + 3rd harmonic drawbar
            double f = freqs[0];
            double organ = std::sin(2.0 * std::numbers::pi * f * t) + 
                           0.4 * std::sin(2.0 * std::numbers::pi * f * 3.0 * t);

            double env = (1.0 - std::exp(-t * 100.0)) * std::exp(-t * 25.0);
            float sample = static_cast<float>(organ * env * gain);

            buffer[0][startPos + i] += sample * 1.1f;
            buffer[1][startPos + i] += sample * 0.9f;
        }
    }
};

} // namespace reggaewave::audio

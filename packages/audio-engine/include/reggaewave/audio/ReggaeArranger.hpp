#pragma once

#include <reggaewave/audio/MusicAnalyzer.hpp>
#include <reggaewave/audio/ReggaeDrumSynthesizer.hpp>
#include <reggaewave/audio/ReggaeBassGenerator.hpp>
#include <reggaewave/audio/ReggaeSkankGenerator.hpp>
#include <reggaewave/contracts/Manifests.hpp>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

namespace reggaewave::audio {

struct ArrangementResult {
    std::vector<std::vector<float>> variationA; // Variation A: Classic Roots / One-Drop
    std::vector<std::vector<float>> variationB; // Variation B: Modern Steppers
    contracts::VariationManifest manifestA;
    contracts::VariationManifest manifestB;
    size_t totalSamples = 0;
};

/**
 * @brief Complete Reggae arrangement & composition orchestrator.
 */
class ReggaeArranger {
public:
    static ArrangementResult arrange(size_t totalSamples,
                                     const AnalysisReport& analysis,
                                     int reggaeIntensity = 70) {
        ArrangementResult result;
        result.totalSamples = totalSamples;

        // 1. Synthesize Drums
        auto drumsA = ReggaeDrumSynthesizer::synthesize(totalSamples, analysis.beatGrid, ReggaeDrumStyle::OneDrop, reggaeIntensity);
        auto drumsB = ReggaeDrumSynthesizer::synthesize(totalSamples, analysis.beatGrid, ReggaeDrumStyle::Steppers, reggaeIntensity);

        // 2. Synthesize Melodic Reggae Bass
        auto bass = ReggaeBassGenerator::synthesize(totalSamples, analysis.beatGrid, analysis.chordTimeline, reggaeIntensity);

        // 3. Synthesize Offbeat Skank & Organ Bubble
        auto skankA = ReggaeSkankGenerator::synthesize(totalSamples, analysis.beatGrid, analysis.chordTimeline, true, reggaeIntensity);
        auto skankB = ReggaeSkankGenerator::synthesize(totalSamples, analysis.beatGrid, analysis.chordTimeline, false, reggaeIntensity);

        // 4. Mix stems for Variation A (Classic Roots / One-Drop)
        result.variationA.assign(2, std::vector<float>(totalSamples, 0.0f));
        for (int ch = 0; ch < 2; ++ch) {
            for (size_t i = 0; i < totalSamples; ++i) {
                float mix = drumsA[ch][i] * 0.75f + bass[ch][i] * 0.85f + skankA[ch][i] * 0.65f;
                result.variationA[ch][i] = std::clamp(mix, -1.0f, 1.0f);
            }
        }

        // 5. Mix stems for Variation B (Modern Steppers)
        result.variationB.assign(2, std::vector<float>(totalSamples, 0.0f));
        for (int ch = 0; ch < 2; ++ch) {
            for (size_t i = 0; i < totalSamples; ++i) {
                float mix = drumsB[ch][i] * 0.85f + bass[ch][i] * 0.80f + skankB[ch][i] * 0.60f;
                result.variationB[ch][i] = std::clamp(mix, -1.0f, 1.0f);
            }
        }

        // 6. Populate Manifests
        result.manifestA.variationId = "variation_a";
        result.manifestA.label = "Variation A (Classic Roots / One-Drop)";
        result.manifestA.rhythmStyle = "ONE_DROP";
        result.manifestA.durationSeconds = static_cast<double>(totalSamples) / 44100.0;
        result.manifestA.integratedLufs = -14.0;
        result.manifestA.truePeakDb = -1.0;

        result.manifestB.variationId = "variation_b";
        result.manifestB.label = "Variation B (Modern Steppers)";
        result.manifestB.rhythmStyle = "STEPPERS";
        result.manifestB.durationSeconds = static_cast<double>(totalSamples) / 44100.0;
        result.manifestB.integratedLufs = -14.0;
        result.manifestB.truePeakDb = -1.0;

        return result;
    }
};

} // namespace reggaewave::audio

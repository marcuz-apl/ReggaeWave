#pragma once

#include <reggaewave/contracts/JobState.hpp>
#include <reggaewave/contracts/Manifests.hpp>
#include <reggaewave/contracts/RightsAttestation.hpp>
#include <reggaewave/contracts/TuningParameters.hpp>
#include <reggaewave/audio/AudioValidator.hpp>
#include <reggaewave/audio/AudioNormalizer.hpp>
#include <reggaewave/audio/AudioDecoder.hpp>
#include <reggaewave/audio/AudioCleaner.hpp>
#include <reggaewave/audio/StemSeparator.hpp>
#include <reggaewave/audio/MusicAnalyzer.hpp>
#include <reggaewave/audio/ReggaeArranger.hpp>
#include <reggaewave/audio/DualTransportSource.hpp>
#include <reggaewave/audio/DubEffectsProcessor.hpp>
#include <reggaewave/audio/WaveformGenerator.hpp>

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace reggaewave::audio {

struct ConversionOutput {
    contracts::ProjectManifest projectManifest;
    contracts::MusicalAnalysisManifest analysisManifest;
    contracts::VariationManifest manifestVariationA;
    contracts::VariationManifest manifestVariationB;
    std::vector<float> waveformOverviewPeaks;
    size_t totalSamples = 0;
    double durationSeconds = 0.0;
};

/**
 * @brief End-to-end transformation pipeline coordinator for ReggaeWave.
 */
class ConversionPipeline {
public:
    ConversionPipeline() = default;

    /**
     * @brief Executes the complete offline transformation pipeline.
     */
    ConversionOutput execute(const std::vector<uint8_t>& rawAudioBytes,
                             const contracts::RightsAttestation& rightsAttestation,
                             const contracts::TuningParameters& tuningParams,
                             const std::string& projectId = "proj-001",
                             const std::string& projectName = "Reggae Transformation",
                             bool enableCleanup = true) {
        if (!rightsAttestation.isConfirmed()) {
            throw std::invalid_argument("Cannot execute pipeline without confirmed rights attestation");
        }

        enableCleanup_ = enableCleanup;

        // 1. Decode Raw Audio
        auto decoded = AudioDecoder::decodeWavBytes(rawAudioBytes.data(), rawAudioBytes.size());

        // 2. Validate Constraints (PRD 7.1)
        auto valRes = AudioValidator::validateAudioMetadata(rawAudioBytes.size(),
                                                            decoded.durationSeconds,
                                                            static_cast<int>(decoded.sampleRate),
                                                            decoded.numChannels,
                                                            true);
        if (!valRes.isValid) {
            throw std::runtime_error("Audio validation failed: " + valRes.sanitizedMessage);
        }

        // 3. Normalize to Canonical 44.1 kHz Stereo Float PCM
        auto normalized = AudioNormalizer::normalize(decoded.channels, decoded.sampleRate);
        const size_t numSamples = normalized.numSamples;

        // 4. Source Audio Pre-Conditioning & Denoising (Stage 1)
        if (enableCleanup_) {
            AudioCleaner::cleanStereo(normalized.channels, 44100.0);
        }

        // 5. Source Separation (Lead Vocal vs Accompaniment)
        StemSeparator separator;
        auto sepRes = separator.separate(normalized.channels);
        leadVocal_ = sepRes.leadVocal;

        // 6. Vocal Stem Polish & De-bleed (Stage 3)
        if (enableCleanup_) {
            AudioCleaner::polishVocalStem(leadVocal_, 44100.0);
        }

        // 7. Harmony & Beat Grid Analysis
        analysisReport_ = MusicAnalyzer::analyze(normalized.channels);

        // 6. Reggae Arrangement & Dual Variation Synthesis
        currentTuning_ = tuningParams;
        arrangement_ = ReggaeArranger::arrange(numSamples, analysisReport_, currentTuning_.getReggaeIntensity());

        // 8. Load Audio Transport & Configure DSP
        transport_.prepare(44100.0, 2);
        transport_.loadOriginal(normalized.channels);
        transport_.loadVariationA(arrangement_.variationA);
        transport_.loadVariationB(arrangement_.variationB);
        transport_.loadLeadVocal(leadVocal_);
        transport_.setVocalGainDb(currentTuning_.getVocalLevelDb());

        dubProcessor_.prepare(44100.0, 512, 2);
        dubProcessor_.setTempoBpm(analysisReport_.beatGrid.bpm);
        dubProcessor_.setDubAmount(currentTuning_.getDubEffectsAmount());

        // 8. Generate Waveform Peaks
        auto peaks = WaveformGenerator::generatePeaks(arrangement_.variationA, 160);

        // 9. Construct Output Manifests
        ConversionOutput output;
        output.totalSamples = numSamples;
        output.durationSeconds = normalized.durationSeconds;
        output.waveformOverviewPeaks = std::move(peaks);
        output.analysisManifest = analysisReport_.manifest;
        output.manifestVariationA = arrangement_.manifestA;
        output.manifestVariationB = arrangement_.manifestB;

        output.projectManifest.projectId = projectId;
        output.projectManifest.projectName = projectName;
        output.projectManifest.rightsAttestationBasis = std::string(contracts::toString(rightsAttestation.getBasis()));
        output.projectManifest.reggaeIntensity = currentTuning_.getReggaeIntensity();
        output.projectManifest.dubEffectsAmount = currentTuning_.getDubEffectsAmount();
        output.projectManifest.vocalLevelDb = currentTuning_.getVocalLevelDb();
        output.projectManifest.selectedVariationId = "variation_a";

        isReady_ = true;
        return output;
    }

    /**
     * @brief Live dynamic tuning update (applied seamlessly in real-time).
     */
    void updateTuning(const contracts::TuningParameters& params) {
        if (!isReady_) return;

        currentTuning_ = params;
        transport_.setVocalGainDb(currentTuning_.getVocalLevelDb());
        dubProcessor_.setDubAmount(currentTuning_.getDubEffectsAmount());

        // Re-generate arrangement if intensity changed
        arrangement_ = ReggaeArranger::arrange(transport_.getTotalLengthSamples(), analysisReport_, currentTuning_.getReggaeIntensity());
        transport_.loadVariationA(arrangement_.variationA);
        transport_.loadVariationB(arrangement_.variationB);
    }

    void setActiveVariation(ActiveVariation variation) {
        transport_.setActiveVariation(variation);
    }

    [[nodiscard]] DualTransportSource& getTransport() noexcept { return transport_; }
    [[nodiscard]] DubEffectsProcessor& getDubProcessor() noexcept { return dubProcessor_; }
    [[nodiscard]] bool isReady() const noexcept { return isReady_; }
    [[nodiscard]] bool isCleanupEnabled() const noexcept { return enableCleanup_; }
    void setCleanupEnabled(bool enabled) noexcept { enableCleanup_ = enabled; }

    /**
     * @brief Process real-time block for output audio device callback.
     */
    void processBlock(float* const* outputChannels, int numChannels, int numSamples) {
        if (!isReady_) return;
        transport_.renderNextBlock(outputChannels, numChannels, numSamples);
        dubProcessor_.process(outputChannels, numChannels, numSamples);
    }

private:
    bool isReady_ = false;
    bool enableCleanup_ = true;
    contracts::TuningParameters currentTuning_;
    AnalysisReport analysisReport_;
    ArrangementResult arrangement_;
    std::vector<std::vector<float>> leadVocal_;

    DualTransportSource transport_;
    DubEffectsProcessor dubProcessor_;
};

} // namespace reggaewave::audio

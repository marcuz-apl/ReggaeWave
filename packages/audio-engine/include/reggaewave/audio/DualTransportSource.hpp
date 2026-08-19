#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>
#include <atomic>

namespace reggaewave::audio {

enum class ActiveVariation {
    VariationA,
    VariationB
};

/**
 * @brief Dual-track synchronized playback engine with glitch-free A/B crossfading.
 * 
 * Invariants:
 * - Playhead position is shared and perfectly synchronized between Variation A and B.
 * - Switching between variations preserves the timestamp and applies an equal-power crossfade.
 * - Vocal level gain (-6 dB to +6 dB) is applied dynamically.
 */
class DualTransportSource {
public:
    DualTransportSource() = default;

    void prepare(double sampleRate, int numChannels = 2) {
        sampleRate_ = sampleRate > 0.0 ? sampleRate : 44100.0;
        numChannels_ = std::max(1, numChannels);
        crossfadeSamples_ = static_cast<int>(sampleRate_ * 0.03); // 30ms smooth crossfade
    }

    void loadVariationA(std::vector<std::vector<float>> channels) {
        variationA_ = std::move(channels);
        updateTotalLength();
    }

    void loadVariationB(std::vector<std::vector<float>> channels) {
        variationB_ = std::move(channels);
        updateTotalLength();
    }

    void loadLeadVocal(std::vector<std::vector<float>> channels) {
        leadVocal_ = std::move(channels);
    }

    void setActiveVariation(ActiveVariation target) {
        if (targetVariation_ != target) {
            targetVariation_ = target;
            crossfadeProgress_ = 0;
            isCrossfading_ = true;
        }
    }

    [[nodiscard]] ActiveVariation getActiveVariation() const noexcept {
        return targetVariation_;
    }

    void setVocalGainDb(double vocalGainDb) {
        vocalGainDb = std::clamp(vocalGainDb, -6.0, 6.0);
        vocalLinearGain_ = static_cast<float>(std::pow(10.0, vocalGainDb / 20.0));
    }

    void setPlayheadSample(size_t sampleIndex) {
        playheadSample_ = std::min(sampleIndex, totalLengthSamples_);
    }

    [[nodiscard]] size_t getPlayheadSample() const noexcept {
        return playheadSample_;
    }

    [[nodiscard]] double getPlayheadSeconds() const noexcept {
        return sampleRate_ > 0.0 ? static_cast<double>(playheadSample_) / sampleRate_ : 0.0;
    }

    [[nodiscard]] size_t getTotalLengthSamples() const noexcept {
        return totalLengthSamples_;
    }

    [[nodiscard]] double getTotalLengthSeconds() const noexcept {
        return sampleRate_ > 0.0 ? static_cast<double>(totalLengthSamples_) / sampleRate_ : 0.0;
    }

    void renderNextBlock(float* const* outputChannels, int numOutputChannels, int numSamples) {
        const int chLimit = std::min(numOutputChannels, numChannels_);

        for (int ch = 0; ch < chLimit; ++ch) {
            std::fill(outputChannels[ch], outputChannels[ch] + numSamples, 0.0f);
        }

        if (totalLengthSamples_ == 0 || playheadSample_ >= totalLengthSamples_) {
            return;
        }

        const size_t startPos = playheadSample_;
        const size_t samplesToRead = std::min(static_cast<size_t>(numSamples), totalLengthSamples_ - startPos);

        for (size_t i = 0; i < samplesToRead; ++i) {
            const size_t readPos = startPos + i;

            float weightA = (activeVariation_ == ActiveVariation::VariationA) ? 1.0f : 0.0f;
            float weightB = (activeVariation_ == ActiveVariation::VariationB) ? 1.0f : 0.0f;

            if (isCrossfading_) {
                float progress = static_cast<float>(crossfadeProgress_) / static_cast<float>(crossfadeSamples_);
                if (targetVariation_ == ActiveVariation::VariationB) {
                    weightA = std::cos(progress * 1.5707963f); // Equal power
                    weightB = std::sin(progress * 1.5707963f);
                } else {
                    weightA = std::sin(progress * 1.5707963f);
                    weightB = std::cos(progress * 1.5707963f);
                }

                crossfadeProgress_++;
                if (crossfadeProgress_ >= crossfadeSamples_) {
                    isCrossfading_ = false;
                    activeVariation_ = targetVariation_;
                }
            }

            for (int ch = 0; ch < chLimit; ++ch) {
                float sampleA = (ch < static_cast<int>(variationA_.size()) && readPos < variationA_[ch].size()) 
                                ? variationA_[ch][readPos] : 0.0f;
                float sampleB = (ch < static_cast<int>(variationB_.size()) && readPos < variationB_[ch].size()) 
                                ? variationB_[ch][readPos] : 0.0f;

                float accompaniment = sampleA * weightA + sampleB * weightB;

                float vocal = (ch < static_cast<int>(leadVocal_.size()) && readPos < leadVocal_[ch].size())
                              ? leadVocal_[ch][readPos] * vocalLinearGain_ : 0.0f;

                outputChannels[ch][i] = accompaniment + vocal;
            }
        }

        playheadSample_ += samplesToRead;
    }

private:
    void updateTotalLength() {
        size_t lenA = variationA_.empty() ? 0 : variationA_[0].size();
        size_t lenB = variationB_.empty() ? 0 : variationB_[0].size();
        totalLengthSamples_ = std::max(lenA, lenB);
    }

    double sampleRate_ = 44100.0;
    int numChannels_ = 2;
    size_t playheadSample_ = 0;
    size_t totalLengthSamples_ = 0;

    ActiveVariation activeVariation_ = ActiveVariation::VariationA;
    ActiveVariation targetVariation_ = ActiveVariation::VariationA;
    bool isCrossfading_ = false;
    int crossfadeProgress_ = 0;
    int crossfadeSamples_ = 1323; // ~30ms at 44.1kHz

    float vocalLinearGain_ = 1.0f;

    std::vector<std::vector<float>> variationA_;
    std::vector<std::vector<float>> variationB_;
    std::vector<std::vector<float>> leadVocal_;
};

} // namespace reggaewave::audio

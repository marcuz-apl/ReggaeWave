#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <format>

namespace reggaewave::contracts {

/**
 * @brief Strictly validated tuning parameters for ReggaeWave conversion.
 * 
 * Invariants from PRD Section 8.3:
 * - Reggae intensity: [0, 100], default 70
 * - Dub-effects amount: [0, 100], default 20
 * - Vocal level: [-6.0, +6.0] dB, default 0.0 dB
 */
class TuningParameters {
public:
    static constexpr int DEFAULT_REGGAE_INTENSITY = 70;
    static constexpr int MIN_REGGAE_INTENSITY = 0;
    static constexpr int MAX_REGGAE_INTENSITY = 100;

    static constexpr int DEFAULT_DUB_EFFECTS_AMOUNT = 20;
    static constexpr int MIN_DUB_EFFECTS_AMOUNT = 0;
    static constexpr int MAX_DUB_EFFECTS_AMOUNT = 100;

    static constexpr double DEFAULT_VOCAL_LEVEL_DB = 0.0;
    static constexpr double MIN_VOCAL_LEVEL_DB = -6.0;
    static constexpr double MAX_VOCAL_LEVEL_DB = +6.0;

    TuningParameters() = default;

    TuningParameters(int reggaeIntensity, int dubEffectsAmount, double vocalLevelDb) {
        setReggaeIntensity(reggaeIntensity);
        setDubEffectsAmount(dubEffectsAmount);
        setVocalLevelDb(vocalLevelDb);
    }

    [[nodiscard]] int getReggaeIntensity() const noexcept { return reggaeIntensity_; }
    void setReggaeIntensity(int value) {
        if (value < MIN_REGGAE_INTENSITY || value > MAX_REGGAE_INTENSITY) {
            throw std::out_of_range("Reggae intensity must be between 0 and 100");
        }
        reggaeIntensity_ = value;
    }

    [[nodiscard]] int getDubEffectsAmount() const noexcept { return dubEffectsAmount_; }
    void setDubEffectsAmount(int value) {
        if (value < MIN_DUB_EFFECTS_AMOUNT || value > MAX_DUB_EFFECTS_AMOUNT) {
            throw std::out_of_range("Dub-effects amount must be between 0 and 100");
        }
        dubEffectsAmount_ = value;
    }

    [[nodiscard]] double getVocalLevelDb() const noexcept { return vocalLevelDb_; }
    void setVocalLevelDb(double value) {
        if (value < MIN_VOCAL_LEVEL_DB || value > MAX_VOCAL_LEVEL_DB) {
            throw std::out_of_range("Vocal level must be between -6.0 dB and +6.0 dB");
        }
        vocalLevelDb_ = value;
    }

    [[nodiscard]] bool operator==(const TuningParameters& other) const noexcept {
        return reggaeIntensity_ == other.reggaeIntensity_ &&
               dubEffectsAmount_ == other.dubEffectsAmount_ &&
               std::abs(vocalLevelDb_ - other.vocalLevelDb_) < 1e-6;
    }

private:
    int reggaeIntensity_ = DEFAULT_REGGAE_INTENSITY;
    int dubEffectsAmount_ = DEFAULT_DUB_EFFECTS_AMOUNT;
    double vocalLevelDb_ = DEFAULT_VOCAL_LEVEL_DB;
};

} // namespace reggaewave::contracts

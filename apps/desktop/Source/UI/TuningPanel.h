#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/contracts/TuningParameters.hpp>
#include <functional>

namespace reggaewave::ui {

/**
 * @brief Compact tuning panel exposing the 3 PRD creative controls.
 */
class TuningPanel : public juce::Component {
public:
    using OnParametersChangedCallback = std::function<void(const contracts::TuningParameters&)>;

    explicit TuningPanel(OnParametersChangedCallback onParamsChanged = nullptr);
    ~TuningPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    [[nodiscard]] contracts::TuningParameters getParameters() const;
    void setParameters(const contracts::TuningParameters& params);

private:
    void notifyChange();

    OnParametersChangedCallback onParamsChanged_;

    juce::Slider intensitySlider_;
    juce::Label intensityLabel_;

    juce::Slider dubSlider_;
    juce::Label dubLabel_;

    juce::Slider vocalSlider_;
    juce::Label vocalLabel_;
};

} // namespace reggaewave::ui

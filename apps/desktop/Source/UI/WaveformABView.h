#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/audio/DualTransportSource.hpp>
#include <functional>
#include <vector>

namespace reggaewave::ui {

/**
 * @brief Synchronized waveform overview and A/B variation switcher component.
 */
class WaveformABView : public juce::Component {
public:
    using OnVariationChanged = std::function<void(audio::ActiveVariation)>;
    using OnPlayheadSeek = std::function<void(double normalizedPosition)>;

    WaveformABView(OnVariationChanged onVarChanged, OnPlayheadSeek onSeek);
    ~WaveformABView() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    void setPlaybackProgress(double progress0To1);
    void setWaveformData(std::vector<float> peaks);
    void setActiveVariation(audio::ActiveVariation variation);

private:
    OnVariationChanged onVarChanged_;
    OnPlayheadSeek onSeek_;

    juce::TextButton varAButton_{"Variation A (Roots)"};
    juce::TextButton varBButton_{"Variation B (Steppers)"};

    double playheadProgress_ = 0.0;
    std::vector<float> waveformPeaks_;
    audio::ActiveVariation activeVariation_ = audio::ActiveVariation::VariationA;
};

} // namespace reggaewave::ui

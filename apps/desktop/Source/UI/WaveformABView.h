#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/audio/DualTransportSource.hpp>
#include <functional>
#include <vector>

namespace reggaewave::ui {

/**
 * @brief Dynamic rhythmic waveform visualizer & synchronized A/B variation switcher.
 */
class WaveformABView : public juce::Component, public juce::Timer {
public:
    using OnVariationChanged = std::function<void(audio::ActiveVariation)>;
    using OnPlayheadSeek = std::function<void(double normalizedPosition)>;

    WaveformABView(OnVariationChanged onVarChanged, OnPlayheadSeek onSeek);
    ~WaveformABView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void timerCallback() override;

    void setPlaybackProgress(double progress0To1);
    void setIsPlaying(bool playing);
    void setAudioEnergyLevel(float energy);
    void setWaveformData(std::vector<float> peaks);
    void setActiveVariation(audio::ActiveVariation variation);

private:
    OnVariationChanged onVarChanged_;
    OnPlayheadSeek onSeek_;

    juce::TextButton varAButton_{"Variation A (Classic Roots / One-Drop)"};
    juce::TextButton varBButton_{"Variation B (Modern Steppers)"};

    double playheadProgress_ = 0.0;
    bool isPlaying_ = false;
    float animPhase_ = 0.0f;
    float audioEnergy_ = 0.7f;
    std::vector<float> waveformPeaks_;
    audio::ActiveVariation activeVariation_ = audio::ActiveVariation::VariationA;
};

} // namespace reggaewave::ui

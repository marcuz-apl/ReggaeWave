#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/audio/DualTransportSource.hpp>
#include <functional>
#include <vector>
#include <string>

namespace reggaewave::ui {

/**
 * @brief Ultra-fancy rhythmic dancing waveform & spectrum visualizer with interactive scrub slider.
 */
class WaveformABView : public juce::Component, public juce::Timer {
public:
    using OnPlayheadSeek = std::function<void(double normalizedPosition)>;

    explicit WaveformABView(OnPlayheadSeek onSeek);
    ~WaveformABView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    void setPlaybackProgress(double progress0To1);
    void setDurationSeconds(double totalSeconds);
    void setIsPlaying(bool playing);
    void setAudioEnergyLevel(float energy);
    void setWaveformData(std::vector<float> peaks);
    void setActiveVariation(audio::ActiveVariation variation);

private:
    OnPlayheadSeek onSeek_;

    juce::Slider scrubSlider_;
    juce::Label timecodeLabel_;

    double playheadProgress_ = 0.0;
    double totalDurationSeconds_ = 0.0;
    bool isPlaying_ = false;
    bool isUserScrubbing_ = false;
    float animPhase_ = 0.0f;
    float audioEnergy_ = 0.85f;
    std::vector<float> waveformPeaks_;
    audio::ActiveVariation activeVariation_ = audio::ActiveVariation::VariationA;
};

} // namespace reggaewave::ui

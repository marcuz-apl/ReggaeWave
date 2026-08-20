#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/contracts/TuningParameters.hpp>
#include <reggaewave/audio/DualTransportSource.hpp>
#include "WaveformABView.h"
#include "TuningPanel.h"
#include <functional>
#include <vector>

namespace reggaewave::ui {

/**
 * @brief Middle Card 2: Riddim & Dub Studio with left-side controls and right-side ultra-fancy rhythmic visualizer.
 */
class StudioPlaybackCard : public juce::Component {
public:
    using OnPlayToggled = std::function<void()>;
    using OnRewindClicked = std::function<void()>;
    using OnVariationChanged = std::function<void(audio::ActiveVariation)>;
    using OnTuningChanged = std::function<void(const contracts::TuningParameters&)>;
    using OnPlayheadSeek = std::function<void(double normPos)>;

    StudioPlaybackCard(OnPlayToggled onPlay,
                       OnRewindClicked onRewind,
                       OnVariationChanged onVarChanged,
                       OnTuningChanged onTuningChanged,
                       OnPlayheadSeek onSeek);
    ~StudioPlaybackCard() override = default;

    void setIsPlaying(bool isPlaying);
    void setPlaybackProgress(double progress0To1);
    void setDurationSeconds(double totalSeconds) { waveformView_.setDurationSeconds(totalSeconds); }
    void setWaveformData(std::vector<float> peaks);
    void setActiveVariation(audio::ActiveVariation variation);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    OnPlayToggled onPlay_;
    OnRewindClicked onRewind_;

    juce::Label cardTitleLabel_;

    // Transport
    juce::TextButton playButton_{"Play"};
    juce::TextButton rewindButton_{"Rewind"};

    // Variation toggles
    juce::TextButton varAButton_{"Var A: One-Drop"};
    juce::TextButton varBButton_{"Var B: Steppers"};

    // Tuning Sliders
    TuningPanel tuningPanel_;

    // Right-side Fancy Visualizer & Scrub Slider
    WaveformABView waveformView_;
};

} // namespace reggaewave::ui

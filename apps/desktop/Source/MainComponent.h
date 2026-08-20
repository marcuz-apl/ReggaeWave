#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <reggaewave/contracts/JobState.hpp>
#include <reggaewave/contracts/TuningParameters.hpp>
#include <reggaewave/contracts/RightsAttestation.hpp>
#include <reggaewave/audio/ConversionPipeline.hpp>
#include <reggaewave/audio/AudioDecoder.hpp>
#include <reggaewave/audio/AudioExporter.hpp>
#include <reggaewave/audio/AudioMasterer.hpp>

#include "UI/ReggaeWaveTheme.h"
#include "UI/TuningPanel.h"
#include "UI/WaveformABView.h"
#include "UI/RightsAttestationModal.h"
#include "UI/LyricEditorView.h"

#include <memory>

namespace reggaewave::desktop {

class MainComponent : public juce::AudioAppComponent, public juce::Timer {
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    void handleImportRequested();
    void handleRightsConfirmed(contracts::RightsBasis basis);
    void processImportedFile(const juce::File& file);
    void handleExportRequested();
    void togglePlayback();

    // Custom Theme
    ui::ReggaeWaveTheme theme_;

    // Transformation Pipeline & DSP
    audio::ConversionPipeline pipeline_;
    audio::DualTransportSource dualTransport_;
    audio::DubEffectsProcessor dubProcessor_;
    contracts::TuningParameters currentTuning_{50, 0, 0.0};
    double currentSampleRate_ = 44100.0;

    // UI Header
    juce::Label appTitleLabel_;
    juce::Label statusBadgeLabel_;

    // Primary Actions
    juce::TextButton importButton_{"Import Track"};
    juce::TextButton playButton_{"Play"};
    juce::TextButton exportButton_{"Export Master (MP3/WAV)"};

    // Panels
    ui::WaveformABView waveformView_;
    ui::TuningPanel tuningPanel_;
    ui::LyricEditorView lyricEditor_;
    std::unique_ptr<ui::RightsAttestationModal> rightsModal_;
    std::unique_ptr<juce::FileChooser> fileChooser_;

    bool isPlaying_ = false;
    contracts::ConversionJobState currentState_ = contracts::ConversionJobState::Created;
    std::optional<contracts::RightsBasis> attestedBasis_;
};

} // namespace reggaewave::desktop

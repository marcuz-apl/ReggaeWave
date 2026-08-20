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
#include "UI/ExportProgressModal.h"
#include "UI/LyricEditorView.h"

#include <memory>
#include <string>

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
    void openAudioFileChooser();
    void handleRightsConfirmed(contracts::RightsBasis basis);
    void processImportedFile(const juce::File& file);
    void handleExportRequested(audio::AudioExportFormat format);
    void performExportToFile(const juce::File& destinationFile, audio::AudioExportFormat format);
    void togglePlayback();
    void renderPreviewWavToDisk();

    // Custom Theme
    ui::ReggaeWaveTheme theme_;

    // Transformation Pipeline & DSP
    audio::ConversionPipeline pipeline_;
    audio::DualTransportSource dualTransport_;
    audio::DubEffectsProcessor dubProcessor_;
    contracts::TuningParameters currentTuning_{70, 20, 0.0};
    double currentSampleRate_ = 44100.0;
    std::string currentTrackTitle_ = "Track";
    audio::ActiveVariation currentVariation_ = audio::ActiveVariation::VariationA;

    // UI Header
    juce::Label appTitleLabel_;
    juce::Label statusBadgeLabel_;
    juce::TextButton rightsStatusButton_{"Rights: Confirmed"};

    // Primary Actions
    juce::TextButton importButton_{"Import Track"};
    juce::TextButton playButton_{"Play"};
    juce::TextButton exportMp3Button_{"Export MP3 (320k)"};
    juce::TextButton exportWavButton_{"Export WAV (24-bit)"};

    // Panels
    ui::WaveformABView waveformView_;
    ui::TuningPanel tuningPanel_;
    ui::LyricEditorView lyricEditor_;
    std::unique_ptr<ui::RightsAttestationModal> rightsModal_;
    std::unique_ptr<ui::ExportProgressModal> exportProgressModal_;
    std::unique_ptr<juce::FileChooser> fileChooser_;

    bool isPlaying_ = false;
    bool rightsConfirmedOnce_ = false;
    contracts::ConversionJobState currentState_ = contracts::ConversionJobState::Created;
    contracts::RightsBasis attestedBasis_ = contracts::RightsBasis::Owned;
};

} // namespace reggaewave::desktop

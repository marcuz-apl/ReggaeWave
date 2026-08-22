#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include <reggaewave/contracts/JobState.hpp>
#include <reggaewave/contracts/TuningParameters.hpp>
#include <reggaewave/contracts/RightsAttestation.hpp>
#include <reggaewave/audio/ConversionPipeline.hpp>
#include <reggaewave/audio/AudioDecoder.hpp>
#include <reggaewave/audio/AudioExporter.hpp>
#include <reggaewave/audio/AudioMasterer.hpp>

#include "UI/ReggaeWaveTheme.h"
#include "UI/ReggaeWaveIcon.h"
#include "UI/ImportCard.h"
#include "UI/StudioPlaybackCard.h"
#include "UI/ExportDeckCard.h"
#include "UI/RightsAttestationModal.h"
#include "UI/ExportDialogModal.h"
#include "UI/AboutDialogModal.h"
#include "UI/HelpDialogModal.h"
#include "UI/ImportFileModal.h"

#include <memory>
#include <string>
#include <atomic>

namespace reggaewave::desktop {

class MainComponent : public juce::Component,
                      public juce::Timer,
                      public juce::AudioIODeviceCallback {
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // juce::AudioIODeviceCallback native OS hardware callbacks
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

private:
    void handleImportRequested();
    void openAudioFileChooser();
    void handleRightsConfirmed(contracts::RightsBasis basis);
    void processImportedFile(const juce::File& file);
    void handleExportRequested(audio::AudioExportFormat format, bool includeSubtitles);
    void togglePlayback();
    void rewindPlayback();
    void showAboutModal();
    void showHelpModal();
    void handleCleanupToggled(bool enabled);

    // Theme & Audio Thread Synchronization
    ui::ReggaeWaveTheme theme_;
    juce::CriticalSection audioLock_;

    // Native JUCE Audio Hardware Device Manager (WASAPI/DirectSound, CoreAudio, ALSA)
    juce::AudioDeviceManager deviceManager_;

    // DSP & Pipeline
    audio::ConversionPipeline pipeline_;
    audio::DualTransportSource dualTransport_;
    audio::DubEffectsProcessor dubProcessor_;
    contracts::TuningParameters currentTuning_{70, 20, 0.0};
    double currentSampleRate_ = 44100.0;
    double currentDurationSecs_ = 0.0;
    std::string currentTrackTitle_ = "Track";
    juce::File currentLoadedFile_;
    audio::ActiveVariation currentVariation_ = audio::ActiveVariation::VariationA;

    // Header Bar
    ui::ReggaeWaveIcon appIcon_;
    juce::Label appTitleLabel_;
    juce::Label versionBadgeLabel_;
    juce::TextButton rightsStatusButton_{"Rights: Owned"};
    juce::Label engineStatusBadge_;
    juce::TextButton helpButton_{"Help"};
    juce::TextButton aboutButton_{"About"};

    // 3 Stacked Functional Cards
    ui::ImportCard importCard_;
    ui::StudioPlaybackCard studioCard_;
    ui::ExportDeckCard exportCard_;

    // Modal Overlays (Always 100% Centered)
    std::unique_ptr<ui::RightsAttestationModal> rightsModal_;
    std::unique_ptr<ui::ExportDialogModal> exportDialogModal_;
    std::unique_ptr<ui::AboutDialogModal> aboutModal_;
    std::unique_ptr<ui::HelpDialogModal> helpModal_;
    std::unique_ptr<ui::ImportFileModal> importFileModal_;

    std::atomic<bool> isPlaying_{false};
    bool rightsConfirmedOnce_ = false;
    contracts::ConversionJobState currentState_ = contracts::ConversionJobState::Created;
    contracts::RightsBasis attestedBasis_ = contracts::RightsBasis::Owned;
};

} // namespace reggaewave::desktop


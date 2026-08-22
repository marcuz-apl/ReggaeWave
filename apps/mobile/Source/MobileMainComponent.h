#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <reggaewave/contracts/RightsAttestation.hpp>
#include <reggaewave/contracts/JobState.hpp>
#include <reggaewave/contracts/TuningParameters.hpp>
#include <reggaewave/audio/ConversionPipeline.hpp>
#include <reggaewave/audio/DualTransportSource.hpp>
#include <reggaewave/audio/DubEffectsProcessor.hpp>

#include "UI/ReggaeWaveTheme.h"
#include "UI/ReggaeWaveIcon.h"
#include "UI/RightsAttestationModal.h"
#include "UI/AboutDialogModal.h"
#include "UI/HelpDialogModal.h"
#include "UI/ImportFileModal.h"
#include "UI/ExportDialogModal.h"
#include "UI/TuningPanel.h"
#include "UI/WaveformABView.h"

namespace reggaewave::mobile {

/**
 * @brief Main Mobile UI Component engineered for portrait touchscreen smartphones and tablets.
 * Implements a responsive touch deck with seamless 3-way A/B/Original reference switching,
 * 1-click source audio denoising, and native audio pipeline orchestration.
 */
class MobileMainComponent : public juce::Component,
                            public juce::AudioIODeviceCallback,
                            public juce::Timer,
                            public juce::FileDragAndDropTarget {
public:
    MobileMainComponent();
    ~MobileMainComponent() override;

    // juce::AudioIODeviceCallback
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext& context) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

    // juce::Timer
    void timerCallback() override;

    // juce::FileDragAndDropTarget
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    // Component Lifecycle
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void openFilePicker();
    void processImportedFile(const juce::File& file);
    void handlePlayToggled();
    void handleRewind();
    void handleVariationChanged(audio::ActiveVariation variation);
    void handleTuningChanged(const contracts::TuningParameters& params);
    void handleExportRequested(audio::AudioExportFormat format, bool includeSubtitles);
    void showRightsModal();
    void showHelpModal();
    void showAboutModal();

    // Audio Engine & Device Manager
    juce::AudioDeviceManager deviceManager_;
    juce::CriticalSection audioLock_;
    audio::ConversionPipeline pipeline_;
    audio::DualTransportSource dualTransport_;
    audio::DubEffectsProcessor dubProcessor_;
    contracts::TuningParameters currentTuning_{70, 20, 0.0};
    contracts::RightsBasis attestedBasis_ = contracts::RightsBasis::Owned;
    contracts::ConversionJobState currentState_ = contracts::ConversionJobState::Created;

    double currentSampleRate_ = 44100.0;
    double currentDurationSecs_ = 0.0;
    juce::String currentTrackTitle_ = "Track";
    juce::File currentLoadedFile_;
    audio::ActiveVariation currentVariation_ = audio::ActiveVariation::VariationA;
    bool isPlaying_ = false;
    bool isCleanupEnabled_ = true;

    // Top Mobile Navigation Bar
    ui::ReggaeWaveIcon appIcon_;
    juce::Label appTitleLabel_;
    juce::TextButton rightsBadgeButton_{"Rights: Owned"};
    juce::TextButton helpButton_{"Help"};
    juce::TextButton aboutButton_{"About"};

    // Scrollable Touch Content Viewport
    juce::Viewport viewport_;
    juce::Component contentContainer_;

    // Mobile Card 1: Import & Intake
    juce::Label intakeHeaderLabel_{"1. Input Track", "1. Input Track & AI Stems"};
    juce::TextButton importFileButton_{"🎵 Tap to Select Audio File"};
    juce::TextButton denoiseToggle_{"⚡ Denoise: ON"};
    juce::Label trackInfoBadge_;

    // Mobile Card 2: Riddim & Dub Studio Deck
    juce::Label studioHeaderLabel_{"2. Riddim Studio", "2. Riddim & Dub Studio (3-Way Audition)"};
    ui::WaveformABView waveformView_;
    
    juce::TextButton playButton_{"▶ Play"};
    juce::TextButton rewindButton_{"↺ Rewind"};
    
    juce::TextButton origButton_{"Original"};
    juce::TextButton varAButton_{"Var A: One-Drop"};
    juce::TextButton varBButton_{"Var B: Steppers"};
    
    ui::TuningPanel tuningPanel_;

    // Mobile Card 3: Export Deck
    juce::Label exportHeaderLabel_{"3. Export", "3. Master & Export"};
    juce::TextButton exportMp3Button_{"Export MP3 (320k)"};
    juce::TextButton exportWavButton_{"Export WAV (24-bit)"};
    juce::ToggleButton subtitleToggle_{"Embed Subtitles (.srt/.vtt)"};

    // Modal Overlays
    std::unique_ptr<ui::RightsAttestationModal> rightsModal_;
    std::unique_ptr<ui::AboutDialogModal> aboutModal_;
    std::unique_ptr<ui::HelpDialogModal> helpModal_;
    std::unique_ptr<ui::ImportFileModal> importFileModal_;
    std::unique_ptr<ui::ExportDialogModal> exportDialogModal_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MobileMainComponent)
};

} // namespace reggaewave::mobile

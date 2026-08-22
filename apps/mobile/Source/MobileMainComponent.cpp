#include "MobileMainComponent.h"
#include <reggaewave/audio/AudioDecoder.hpp>
#include <reggaewave/audio/AudioExporter.hpp>
#include <reggaewave/audio/SubtitleManager.hpp>
#include <reggaewave/audio/AudioMasterer.hpp>

namespace reggaewave::mobile {

MobileMainComponent::MobileMainComponent()
    : waveformView_([this](double normPos) {
        const juce::ScopedLock sl(audioLock_);
        if (dualTransport_.getTotalLengthSamples() > 0) {
            dualTransport_.setPlayheadSample(static_cast<size_t>(normPos * dualTransport_.getTotalLengthSamples()));
        }
    })
    , tuningPanel_([this](const contracts::TuningParameters& params) {
        handleTuningChanged(params);
    })
{
    // 1. Initialize Native Audio Hardware
    deviceManager_.initialiseWithDefaultDevices(0, 2);
    deviceManager_.addAudioCallback(this);
    dualTransport_.prepare(44100.0, 2);
    dubProcessor_.prepare(44100.0, 512, 2);

    // 2. Mobile Top Navigation Bar
    appTitleLabel_.setText("ReggaeWave", juce::dontSendNotification);
    appTitleLabel_.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    appTitleLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    addAndMakeVisible(appTitleLabel_);
    addAndMakeVisible(appIcon_);

    rightsBadgeButton_.setButtonText("Rights: Owned");
    rightsBadgeButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    rightsBadgeButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::accentGreen);
    rightsBadgeButton_.onClick = [this]() { showRightsModal(); };
    addAndMakeVisible(rightsBadgeButton_);

    helpButton_.setButtonText("Help");
    helpButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    helpButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::textPrimary);
    helpButton_.onClick = [this]() { showHelpModal(); };
    addAndMakeVisible(helpButton_);

    aboutButton_.setButtonText("About");
    aboutButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    aboutButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::textSecondary);
    aboutButton_.onClick = [this]() { showAboutModal(); };
    addAndMakeVisible(aboutButton_);

    // 3. Scrollable Viewport & Container
    viewport_.setViewedComponent(&contentContainer_, false);
    viewport_.setScrollBarsShown(true, false, true, false);
    addAndMakeVisible(viewport_);

    // Card 1: Intake & Denoise
    intakeHeaderLabel_.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    intakeHeaderLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    contentContainer_.addAndMakeVisible(intakeHeaderLabel_);

    importFileButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::accentGold);
    importFileButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::bgDark);
    importFileButton_.onClick = [this]() { openFilePicker(); };
    contentContainer_.addAndMakeVisible(importFileButton_);

    denoiseToggle_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    denoiseToggle_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::accentGreen);
    denoiseToggle_.setClickingTogglesState(true);
    denoiseToggle_.setToggleState(true, juce::dontSendNotification);
    denoiseToggle_.onClick = [this]() {
        isCleanupEnabled_ = denoiseToggle_.getToggleState();
        denoiseToggle_.setButtonText(isCleanupEnabled_ ? "⚡ Denoise: ON" : "Denoise: OFF");
        denoiseToggle_.setColour(juce::TextButton::textColourOffId, isCleanupEnabled_ ? ui::ReggaeWaveTheme::accentGreen : ui::ReggaeWaveTheme::textSecondary);
        if (currentLoadedFile_.existsAsFile()) {
            processImportedFile(currentLoadedFile_);
        }
    };
    contentContainer_.addAndMakeVisible(denoiseToggle_);

    trackInfoBadge_.setText("Drop or tap to select audio file (MP3, WAV, M4A, FLAC)", juce::dontSendNotification);
    trackInfoBadge_.setFont(juce::FontOptions(11.5f));
    trackInfoBadge_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::textSecondary);
    trackInfoBadge_.setJustificationType(juce::Justification::centred);
    contentContainer_.addAndMakeVisible(trackInfoBadge_);

    // Card 2: Studio Visualizer & Deck
    studioHeaderLabel_.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    studioHeaderLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    contentContainer_.addAndMakeVisible(studioHeaderLabel_);

    contentContainer_.addAndMakeVisible(waveformView_);

    playButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::accentGold);
    playButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::bgDark);
    playButton_.onClick = [this]() { handlePlayToggled(); };
    contentContainer_.addAndMakeVisible(playButton_);

    rewindButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    rewindButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::textPrimary);
    rewindButton_.onClick = [this]() { handleRewind(); };
    contentContainer_.addAndMakeVisible(rewindButton_);

    origButton_.setClickingTogglesState(true);
    origButton_.setRadioGroupId(4001);
    origButton_.setToggleState(false, juce::dontSendNotification);
    origButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    origButton_.setColour(juce::TextButton::buttonOnColourId, ui::ReggaeWaveTheme::textPrimary);
    origButton_.setColour(juce::TextButton::textColourOnId, ui::ReggaeWaveTheme::bgDark);
    origButton_.onClick = [this]() { handleVariationChanged(audio::ActiveVariation::Original); };
    contentContainer_.addAndMakeVisible(origButton_);

    varAButton_.setClickingTogglesState(true);
    varAButton_.setRadioGroupId(4001);
    varAButton_.setToggleState(true, juce::dontSendNotification);
    varAButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    varAButton_.setColour(juce::TextButton::buttonOnColourId, ui::ReggaeWaveTheme::accentGold);
    varAButton_.setColour(juce::TextButton::textColourOnId, ui::ReggaeWaveTheme::bgDark);
    varAButton_.onClick = [this]() { handleVariationChanged(audio::ActiveVariation::VariationA); };
    contentContainer_.addAndMakeVisible(varAButton_);

    varBButton_.setClickingTogglesState(true);
    varBButton_.setRadioGroupId(4001);
    varBButton_.setToggleState(false, juce::dontSendNotification);
    varBButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    varBButton_.setColour(juce::TextButton::buttonOnColourId, ui::ReggaeWaveTheme::accentGreen);
    varBButton_.setColour(juce::TextButton::textColourOnId, ui::ReggaeWaveTheme::bgDark);
    varBButton_.onClick = [this]() { handleVariationChanged(audio::ActiveVariation::VariationB); };
    contentContainer_.addAndMakeVisible(varBButton_);

    contentContainer_.addAndMakeVisible(tuningPanel_);

    // Card 3: Export Deck
    exportHeaderLabel_.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    exportHeaderLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    contentContainer_.addAndMakeVisible(exportHeaderLabel_);

    exportMp3Button_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::accentGold);
    exportMp3Button_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::bgDark);
    exportMp3Button_.onClick = [this]() { handleExportRequested(audio::AudioExportFormat::Mp3_320Kbps, subtitleToggle_.getToggleState()); };
    contentContainer_.addAndMakeVisible(exportMp3Button_);

    exportWavButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    exportWavButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::accentGreen);
    exportWavButton_.onClick = [this]() { handleExportRequested(audio::AudioExportFormat::Wav24Bit, subtitleToggle_.getToggleState()); };
    contentContainer_.addAndMakeVisible(exportWavButton_);

    subtitleToggle_.setColour(juce::ToggleButton::textColourId, ui::ReggaeWaveTheme::textSecondary);
    contentContainer_.addAndMakeVisible(subtitleToggle_);

    startTimerHz(30);
}

MobileMainComponent::~MobileMainComponent() {
    stopTimer();
    deviceManager_.removeAudioCallback(this);
}

void MobileMainComponent::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    if (device != nullptr) {
        currentSampleRate_ = device->getCurrentSampleRate();
        const juce::ScopedLock sl(audioLock_);
        dualTransport_.prepare(currentSampleRate_, 2);
        dubProcessor_.prepare(currentSampleRate_, 512, 2);
    }
}

void MobileMainComponent::audioDeviceStopped() {}

void MobileMainComponent::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                           int numInputChannels,
                                                           float* const* outputChannelData,
                                                           int numOutputChannels,
                                                           int numSamples,
                                                           const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(inputChannelData, numInputChannels, context);
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        std::fill(outputChannelData[ch], outputChannelData[ch] + numSamples, 0.0f);
    }

    if (!isPlaying_) return;

    const juce::ScopedLock sl(audioLock_);
    dualTransport_.renderNextBlock(outputChannelData, numOutputChannels, numSamples);
    dubProcessor_.process(outputChannelData, numOutputChannels, numSamples);
}

void MobileMainComponent::timerCallback() {
    if (isPlaying_) {
        const juce::ScopedLock sl(audioLock_);
        double totalSecs = dualTransport_.getTotalLengthSeconds();
        if (totalSecs > 0.0) {
            double currentSecs = dualTransport_.getPlayheadSeconds();
            waveformView_.setPlaybackProgress(currentSecs / totalSecs);
        }
    }
}

bool MobileMainComponent::isInterestedInFileDrag(const juce::StringArray& files) {
    return files.size() > 0;
}

void MobileMainComponent::filesDropped(const juce::StringArray& files, int x, int y) {
    juce::ignoreUnused(x, y);
    if (files.size() > 0) {
        processImportedFile(juce::File(files[0]));
    }
}

void MobileMainComponent::openFilePicker() {
    importFileModal_ = std::make_unique<ui::ImportFileModal>(
        [this](const juce::File& file) {
            processImportedFile(file);
            importFileModal_.reset();
        },
        [this]() {
            importFileModal_.reset();
        }
    );
    importFileModal_->setBounds(getLocalBounds());
    addAndMakeVisible(importFileModal_.get());
}

void MobileMainComponent::processImportedFile(const juce::File& file) {
    isPlaying_ = false;
    playButton_.setButtonText("▶ Play");
    waveformView_.setIsPlaying(false);
    currentLoadedFile_ = file;
    currentTrackTitle_ = file.getFileNameWithoutExtension();

    trackInfoBadge_.setText("Transforming: " + file.getFileName() + "...", juce::dontSendNotification);
    repaint();

    try {
        auto decoded = audio::AudioDecoder::decodeAnyAudioFile(file.getFullPathName().toRawUTF8());
        currentDurationSecs_ = static_cast<double>(decoded.channels[0].size()) / decoded.sampleRate;
        auto wavBytes = audio::AudioExporter::encodeWav24Bit(decoded.channels, decoded.sampleRate);
        contracts::RightsAttestation attestation(attestedBasis_, true, "project-mobile");

        auto output = pipeline_.execute(wavBytes, attestation, currentTuning_, "project-mobile", currentTrackTitle_.toRawUTF8(), isCleanupEnabled_);

        waveformView_.setDurationSeconds(currentDurationSecs_);
        waveformView_.setWaveformData(output.waveformOverviewPeaks);

        {
            const juce::ScopedLock sl(audioLock_);
            dualTransport_ = std::move(pipeline_.getTransport());
            dubProcessor_ = std::move(pipeline_.getDubProcessor());
            dualTransport_.prepare(currentSampleRate_, 2);
            dualTransport_.setPlayheadSample(0);
        }

        juce::String bpmText = juce::String(output.analysisManifest.key.c_str()) + " • " +
                               juce::String(output.analysisManifest.bpm, 1) + " BPM • " +
                               juce::String(static_cast<int>(currentDurationSecs_)) + "s";
        trackInfoBadge_.setText(file.getFileName() + " [" + bpmText + "]", juce::dontSendNotification);
        trackInfoBadge_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGreen);

        currentState_ = contracts::ConversionJobState::Completed;
    } catch (const std::exception& ex) {
        trackInfoBadge_.setText("Error: " + juce::String(ex.what()), juce::dontSendNotification);
        trackInfoBadge_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentRed);
    }
}

void MobileMainComponent::handlePlayToggled() {
    if (dualTransport_.getTotalLengthSamples() == 0) return;
    isPlaying_ = !isPlaying_;
    playButton_.setButtonText(isPlaying_ ? "⏸ Pause" : "▶ Play");
    waveformView_.setIsPlaying(isPlaying_);
}

void MobileMainComponent::handleRewind() {
    const juce::ScopedLock sl(audioLock_);
    dualTransport_.setPlayheadSample(0);
    waveformView_.setPlaybackProgress(0.0);
}

void MobileMainComponent::handleVariationChanged(audio::ActiveVariation variation) {
    currentVariation_ = variation;
    const juce::ScopedLock sl(audioLock_);
    dualTransport_.setActiveVariation(variation);
    origButton_.setToggleState(variation == audio::ActiveVariation::Original, juce::dontSendNotification);
    varAButton_.setToggleState(variation == audio::ActiveVariation::VariationA, juce::dontSendNotification);
    varBButton_.setToggleState(variation == audio::ActiveVariation::VariationB, juce::dontSendNotification);
    waveformView_.setActiveVariation(variation);
}

void MobileMainComponent::handleTuningChanged(const contracts::TuningParameters& params) {
    currentTuning_ = params;
    const juce::ScopedLock sl(audioLock_);
    dualTransport_.setVocalGainDb(params.getVocalLevelDb());
    dubProcessor_.setDubAmount(params.getDubEffectsAmount());
}

void MobileMainComponent::handleExportRequested(audio::AudioExportFormat format, bool includeSubtitles) {
    if (dualTransport_.getTotalLengthSamples() == 0) {
        trackInfoBadge_.setText("Please select an audio file first.", juce::dontSendNotification);
        return;
    }

    exportDialogModal_ = std::make_unique<ui::ExportDialogModal>(
        currentTrackTitle_,
        format,
        [this, includeSubtitles](const juce::File& destFile, audio::AudioExportFormat fmt, ui::ExportDialogModal* modal) {
            juce::Thread::launch([this, destFile, fmt, modal, includeSubtitles]() {
                try {
                    modal->updateProgress(0.20f, "Rendering Audio Stems (20%)...");

                    audio::DualTransportSource tempTransport;
                    audio::DubEffectsProcessor tempDub;
                    size_t totalSamples = 0;
                    {
                        const juce::ScopedLock sl(audioLock_);
                        totalSamples = dualTransport_.getTotalLengthSamples();
                        tempTransport = dualTransport_;
                        tempDub = dubProcessor_;
                    }

                    std::vector<float> leftCh(totalSamples, 0.0f);
                    std::vector<float> rightCh(totalSamples, 0.0f);
                    tempTransport.setPlayheadSample(0);

                    const int blockSize = 1024;
                    for (size_t pos = 0; pos < totalSamples; pos += blockSize) {
                        int samplesToProcess = static_cast<int>(std::min(size_t{blockSize}, totalSamples - pos));
                        std::vector<float*> blockPtrs = {leftCh.data() + pos, rightCh.data() + pos};
                        tempTransport.renderNextBlock(blockPtrs.data(), 2, samplesToProcess);
                        tempDub.process(blockPtrs.data(), 2, samplesToProcess);
                    }

                    modal->updateProgress(0.55f, "Mastering to -14.0 LUFS & -1.0 dBTP (55%)...");
                    auto mastered = audio::AudioMasterer::master({leftCh, rightCh}, 44100.0);

                    modal->updateProgress(0.85f, "Encoding format container (85%)...");
                    std::vector<uint8_t> outputBytes;
                    if (fmt == audio::AudioExportFormat::Mp3_320Kbps) {
                        outputBytes = audio::AudioExporter::encodeMp3(mastered.masteredAudio, 44100.0);
                    } else {
                        outputBytes = audio::AudioExporter::encodeWav24Bit(mastered.masteredAudio, 44100.0);
                    }

                    destFile.getParentDirectory().createDirectory();
                    destFile.deleteFile();
                    if (!destFile.replaceWithData(outputBytes.data(), outputBytes.size())) {
                        throw std::runtime_error("Could not write destination file: " + destFile.getFileName().toStdString());
                    }

                    auto exportsDir = juce::File::getCurrentWorkingDirectory().getChildFile("exports");
                    exportsDir.createDirectory();
                    auto workspaceCopy = exportsDir.getChildFile(destFile.getFileName());
                    if (workspaceCopy.getFullPathName() != destFile.getFullPathName()) {
                        workspaceCopy.deleteFile();
                        workspaceCopy.replaceWithData(outputBytes.data(), outputBytes.size());
                    }

                    if (includeSubtitles) {
                        modal->updateProgress(0.95f, "Generating synchronized subtitles (95%)...");
                        audio::SubtitleManager subMgr;
                        subMgr.setUserRevisions({
                            {0.0, currentDurationSecs_ * 0.5, "[Instrumental Intro & One-Drop Riddim]"},
                            {currentDurationSecs_ * 0.5, currentDurationSecs_, "[Lead Vocal & Dub Section]"}
                        });

                        auto srtFile = exportsDir.getChildFile(currentTrackTitle_ + "_reggae_subtitles.srt");
                        auto vttFile = exportsDir.getChildFile(currentTrackTitle_ + "_reggae_subtitles.vtt");
                        srtFile.replaceWithText(juce::String::fromUTF8(subMgr.formatSrt().c_str()));
                        vttFile.replaceWithText(juce::String::fromUTF8(subMgr.formatVtt().c_str()));
                    }

                    modal->setExportCompleted(destFile.getFileName());
                } catch (const std::exception& ex) {
                    modal->setExportError(ex.what());
                }
            });
        },
        [this]() {
            if (exportDialogModal_) {
                removeChildComponent(exportDialogModal_.get());
                exportDialogModal_.reset();
            }
        }
    );
    exportDialogModal_->setBounds(getLocalBounds());
    addAndMakeVisible(exportDialogModal_.get());
}

void MobileMainComponent::showRightsModal() {
    rightsModal_ = std::make_unique<ui::RightsAttestationModal>(
        [this](contracts::RightsBasis basis) {
            attestedBasis_ = basis;
            rightsBadgeButton_.setButtonText(
                (basis == contracts::RightsBasis::PublicDomain) ? "Rights: Public Domain" :
                (basis == contracts::RightsBasis::Licensed) ? "Rights: Licensed" : "Rights: Owned"
            );
            if (rightsModal_) {
                removeChildComponent(rightsModal_.get());
                rightsModal_.reset();
            }
        }
    );
    rightsModal_->setBounds(getLocalBounds());
    addAndMakeVisible(rightsModal_.get());
}

void MobileMainComponent::showHelpModal() {
    helpModal_ = std::make_unique<ui::HelpDialogModal>([this]() {
        if (helpModal_) {
            removeChildComponent(helpModal_.get());
            helpModal_.reset();
        }
    });
    helpModal_->setBounds(getLocalBounds());
    addAndMakeVisible(helpModal_.get());
}

void MobileMainComponent::showAboutModal() {
    aboutModal_ = std::make_unique<ui::AboutDialogModal>([this]() {
        if (aboutModal_) {
            removeChildComponent(aboutModal_.get());
            aboutModal_.reset();
        }
    });
    aboutModal_->setBounds(getLocalBounds());
    addAndMakeVisible(aboutModal_.get());
}

void MobileMainComponent::paint(juce::Graphics& g) {
    g.fillAll(ui::ReggaeWaveTheme::bgDark);

    // Top mobile header bar background
    auto headerArea = getLocalBounds().removeFromTop(54).toFloat();
    g.setColour(ui::ReggaeWaveTheme::bgSurface);
    g.fillRect(headerArea);

    g.setColour(ui::ReggaeWaveTheme::bgElevated);
    g.drawHorizontalLine(54, 0.0f, static_cast<float>(getWidth()));
}

void MobileMainComponent::resized() {
    auto bounds = getLocalBounds();

    // 1. Top Mobile Navigation Bar (54px)
    auto header = bounds.removeFromTop(54).reduced(10, 8);
    appIcon_.setBounds(header.removeFromLeft(32).withSizeKeepingCentre(28, 28));
    header.removeFromLeft(6);
    appTitleLabel_.setBounds(header.removeFromLeft(105));

    aboutButton_.setBounds(header.removeFromRight(48).reduced(0, 3));
    header.removeFromRight(4);
    helpButton_.setBounds(header.removeFromRight(46).reduced(0, 3));
    header.removeFromRight(6);
    rightsBadgeButton_.setBounds(header.removeFromRight(100).reduced(0, 3));

    // 2. Viewport & Content Container (Single-Column Portrait Layout)
    viewport_.setBounds(bounds);

    const int contentWidth = bounds.getWidth();
    const int contentHeight = 780; // Full scroll height for single-column mobile view
    contentContainer_.setBounds(0, 0, contentWidth, contentHeight);

    auto area = contentContainer_.getLocalBounds().reduced(12);

    // --- Mobile Section 1: Intake & Denoise Card (130px) ---
    auto card1Area = area.removeFromTop(128);
    intakeHeaderLabel_.setBounds(card1Area.removeFromTop(20));
    card1Area.removeFromTop(6);

    auto btnRow = card1Area.removeFromTop(44);
    importFileButton_.setBounds(btnRow.removeFromLeft(btnRow.getWidth() - 110));
    btnRow.removeFromLeft(8);
    denoiseToggle_.setBounds(btnRow);

    card1Area.removeFromTop(6);
    trackInfoBadge_.setBounds(card1Area.removeFromTop(22));

    area.removeFromTop(14);

    // --- Mobile Section 2: Studio Deck Card (420px) ---
    auto card2Area = area.removeFromTop(420);
    studioHeaderLabel_.setBounds(card2Area.removeFromTop(20));
    card2Area.removeFromTop(6);

    // Dancing Waveform (130px height)
    waveformView_.setBounds(card2Area.removeFromTop(130));
    card2Area.removeFromTop(10);

    // Transport buttons (44px touch height)
    auto transportRow = card2Area.removeFromTop(44);
    playButton_.setBounds(transportRow.removeFromLeft(transportRow.getWidth() - 100));
    transportRow.removeFromLeft(8);
    rewindButton_.setBounds(transportRow);

    card2Area.removeFromTop(8);

    // 3-Way Reference Switcher (38px height)
    auto switchRow = card2Area.removeFromTop(38);
    int switchBtnW = (switchRow.getWidth() - 12) / 3;
    origButton_.setBounds(switchRow.removeFromLeft(switchBtnW));
    switchRow.removeFromLeft(6);
    varAButton_.setBounds(switchRow.removeFromLeft(switchBtnW));
    switchRow.removeFromLeft(6);
    varBButton_.setBounds(switchRow);

    card2Area.removeFromTop(10);

    // 3 Big Rotary Touch Dials (110px height)
    tuningPanel_.setBounds(card2Area.removeFromTop(110));

    area.removeFromTop(14);

    // --- Mobile Section 3: Master & Export Card (140px) ---
    auto card3Area = area.removeFromTop(140);
    exportHeaderLabel_.setBounds(card3Area.removeFromTop(20));
    card3Area.removeFromTop(8);

    auto exportBtnRow = card3Area.removeFromTop(44);
    int exportBtnW = (exportBtnRow.getWidth() - 8) / 2;
    exportMp3Button_.setBounds(exportBtnRow.removeFromLeft(exportBtnW));
    exportBtnRow.removeFromLeft(8);
    exportWavButton_.setBounds(exportBtnRow);

    card3Area.removeFromTop(8);
    subtitleToggle_.setBounds(card3Area.removeFromTop(28));

    // Reposition modals if open
    if (rightsModal_) rightsModal_->setBounds(getLocalBounds());
    if (exportDialogModal_) exportDialogModal_->setBounds(getLocalBounds());
    if (aboutModal_) aboutModal_->setBounds(getLocalBounds());
    if (helpModal_) helpModal_->setBounds(getLocalBounds());
    if (importFileModal_) importFileModal_->setBounds(getLocalBounds());
}

} // namespace reggaewave::mobile

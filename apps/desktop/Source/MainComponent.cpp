#include "MainComponent.h"
#include <reggaewave/audio/SubtitleManager.hpp>
#include <fstream>
#include <cstdlib>
#include <chrono>

namespace reggaewave::desktop {

MainComponent::MainComponent()
    : importCard_(
        [this]() { handleImportRequested(); },
        [this](bool enabled) { handleCleanupToggled(enabled); }
    )
    , studioCard_(
        [this]() { togglePlayback(); },
        [this]() { rewindPlayback(); },
        [this](audio::ActiveVariation var) {
            const juce::ScopedLock sl(audioLock_);
            currentVariation_ = var;
            dualTransport_.setActiveVariation(var);
        },
        [this](const contracts::TuningParameters& params) {
            const juce::ScopedLock sl(audioLock_);
            currentTuning_ = params;
            dubProcessor_.setDubAmount(params.getDubEffectsAmount());
            dualTransport_.setVocalGainDb(params.getVocalLevelDb());
        },
        [this](double normPos) {
            const juce::ScopedLock sl(audioLock_);
            size_t targetSample = static_cast<size_t>(normPos * dualTransport_.getTotalLengthSamples());
            dualTransport_.setPlayheadSample(targetSample);
        }
    )
    , exportCard_([this](audio::AudioExportFormat fmt, bool subs) { handleExportRequested(fmt, subs); })
{
    juce::LookAndFeel::setDefaultLookAndFeel(&theme_);

    // 1. Header Bar: Branding & Status (Clean typography)
    addAndMakeVisible(appIcon_);

    appTitleLabel_.setText("ReggaeWave", juce::dontSendNotification);
    appTitleLabel_.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    appTitleLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    addAndMakeVisible(appTitleLabel_);

#ifdef REGGAEWAVE_APP_SEMVER
    versionBadgeLabel_.setText("v" REGGAEWAVE_APP_SEMVER, juce::dontSendNotification);
#elif defined(REGGAEWAVE_APP_VERSION_STRING)
    versionBadgeLabel_.setText("v" REGGAEWAVE_APP_VERSION_STRING, juce::dontSendNotification);
#else
    versionBadgeLabel_.setText("v1.2.8", juce::dontSendNotification);
#endif
    versionBadgeLabel_.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    versionBadgeLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::textSecondary);
    versionBadgeLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(versionBadgeLabel_);

    rightsStatusButton_.setButtonText("Rights: Owned");
    rightsStatusButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgSurface);
    rightsStatusButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::accentGreen);
    rightsStatusButton_.onClick = [this]() {
        rightsConfirmedOnce_ = false;
        handleImportRequested();
    };
    addAndMakeVisible(rightsStatusButton_);

    engineStatusBadge_.setText("Roots Engine | 44.1k/24b", juce::dontSendNotification);
    engineStatusBadge_.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    engineStatusBadge_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    engineStatusBadge_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(engineStatusBadge_);

    helpButton_.setButtonText("Help");
    helpButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    helpButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::textPrimary);
    helpButton_.onClick = [this]() { showHelpModal(); };
    addAndMakeVisible(helpButton_);

    aboutButton_.setButtonText("About");
    aboutButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    aboutButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::textPrimary);
    aboutButton_.onClick = [this]() { showAboutModal(); };
    addAndMakeVisible(aboutButton_);

    // 2. Add 3 Stacked Cards
    addAndMakeVisible(importCard_);
    addAndMakeVisible(studioCard_);
    addAndMakeVisible(exportCard_);

    // Initialize DSP
    dualTransport_.prepare(44100.0, 2);
    dubProcessor_.prepare(44100.0, 512, 2);

    // Initialize Native OS Audio Device Hardware (WASAPI on Windows, CoreAudio on macOS, ALSA/Pulse on Linux & WSL)
    deviceManager_.initialiseWithDefaultDevices(0, 2);

    // Fallback for virtualized/WSL ALSA environments where hardware soundcards are routed via PulseAudio/PipeWire
    if (deviceManager_.getCurrentAudioDevice() == nullptr) {
        const juce::StringArray preferredDevices = { "default", "pulse", "sysdefault", "" };
        for (const auto& dev : preferredDevices) {
            juce::AudioDeviceManager::AudioDeviceSetup setup;
            setup.outputDeviceName = dev;
            setup.inputDeviceName = "";
            setup.sampleRate = 44100.0;
            setup.bufferSize = 512;
            setup.useDefaultInputChannels = false;
            setup.useDefaultOutputChannels = true;
            setup.outputChannels.setBit(0);
            setup.outputChannels.setBit(1);
            if (deviceManager_.setAudioDeviceSetup(setup, true).isEmpty() &&
                deviceManager_.getCurrentAudioDevice() != nullptr) {
                break;
            }
        }
    }

    if (deviceManager_.getCurrentAudioDevice() == nullptr) {
        for (auto* dt : deviceManager_.getAvailableDeviceTypes()) {
            dt->scanForDevices();
            const auto outDevices = dt->getDeviceNames(false);
            for (const auto& devName : outDevices) {
                juce::AudioDeviceManager::AudioDeviceSetup setup;
                setup.outputDeviceName = devName;
                setup.inputDeviceName = "";
                setup.sampleRate = 44100.0;
                setup.bufferSize = 512;
                setup.useDefaultInputChannels = false;
                setup.useDefaultOutputChannels = true;
                setup.outputChannels.setBit(0);
                setup.outputChannels.setBit(1);
                if (deviceManager_.setAudioDeviceSetup(setup, true).isEmpty() &&
                    deviceManager_.getCurrentAudioDevice() != nullptr) {
                    break;
                }
            }
            if (deviceManager_.getCurrentAudioDevice() != nullptr) {
                break;
            }
        }
    }

    if (deviceManager_.getCurrentAudioDevice() == nullptr) {
        deviceManager_.initialise(0, 2, nullptr, true);
    }
    deviceManager_.addAudioCallback(this);

    startTimerHz(30);
    setSize(1020, 720);
}

MainComponent::~MainComponent() {
    stopTimer();
    isPlaying_ = false;
    deviceManager_.removeAudioCallback(this);
    deviceManager_.closeAudioDevice();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void MainComponent::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    if (device != nullptr) {
        const juce::ScopedLock sl(audioLock_);
        currentSampleRate_ = device->getCurrentSampleRate();
        dualTransport_.prepare(currentSampleRate_, 2);
        dubProcessor_.prepare(currentSampleRate_, 512, 2);
    }
}

void MainComponent::audioDeviceStopped() {
}

void MainComponent::audioDeviceIOCallbackWithContext(const float* const* /*inputChannelData*/,
                                                      int /*numInputChannels*/,
                                                      float* const* outputChannelData,
                                                      int numOutputChannels,
                                                      int numSamples,
                                                      const juce::AudioIODeviceCallbackContext& /*context*/) {
    // 1. Always zero out output channels first to avoid hardware noise
    for (int ch = 0; ch < numOutputChannels; ++ch) {
        if (outputChannelData[ch] != nullptr) {
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
        }
    }

    if (!isPlaying_) {
        return;
    }

    const juce::ScopedLock sl(audioLock_);
    if (dualTransport_.getTotalLengthSamples() == 0 ||
        dualTransport_.getPlayheadSample() >= dualTransport_.getTotalLengthSamples()) {
        juce::MessageManager::callAsync([this]() {
            isPlaying_ = false;
            studioCard_.setIsPlaying(false);
        });
        return;
    }

    // 2. Render dualTransport_ directly into hardware output channels
    if (numOutputChannels >= 2 && outputChannelData[0] != nullptr && outputChannelData[1] != nullptr) {
        float* channels[2] = { outputChannelData[0], outputChannelData[1] };
        dualTransport_.renderNextBlock(channels, 2, numSamples);
        dubProcessor_.process(channels, 2, numSamples);
    } else if (numOutputChannels == 1 && outputChannelData[0] != nullptr) {
        std::vector<float> rightScratch(numSamples, 0.0f);
        float* channels[2] = { outputChannelData[0], rightScratch.data() };
        dualTransport_.renderNextBlock(channels, 2, numSamples);
        dubProcessor_.process(channels, 2, numSamples);
    }
}

void MainComponent::timerCallback() {
    const juce::ScopedLock sl(audioLock_);
    if (isPlaying_ && dualTransport_.getTotalLengthSamples() > 0) {
        // If no hardware audio device is active (e.g. running in WSL headless / virtual environment without audio driver pumping callbacks), simulate playhead advancing
        if (deviceManager_.getCurrentAudioDevice() == nullptr) {
            size_t current = dualTransport_.getPlayheadSample();
            size_t total = dualTransport_.getTotalLengthSamples();
            size_t step = static_cast<size_t>(currentSampleRate_ / 30.0);
            if (current + step >= total) {
                dualTransport_.setPlayheadSample(total);
                juce::MessageManager::callAsync([this]() {
                    isPlaying_ = false;
                    studioCard_.setIsPlaying(false);
                });
            } else {
                dualTransport_.setPlayheadSample(current + step);
            }
        }
        double progress = static_cast<double>(dualTransport_.getPlayheadSample()) / 
                          static_cast<double>(dualTransport_.getTotalLengthSamples());
        studioCard_.setPlaybackProgress(progress);
    }
}

void MainComponent::togglePlayback() {
    bool nowPlaying = false;
    {
        const juce::ScopedLock sl(audioLock_);
        if (dualTransport_.getTotalLengthSamples() == 0) {
            return;
        }
        if (dualTransport_.getPlayheadSample() >= dualTransport_.getTotalLengthSamples()) {
            dualTransport_.setPlayheadSample(0);
        }
        isPlaying_ = !isPlaying_;
        nowPlaying = isPlaying_;
    }

    studioCard_.setIsPlaying(nowPlaying);
}

void MainComponent::rewindPlayback() {
    {
        const juce::ScopedLock sl(audioLock_);
        dualTransport_.setPlayheadSample(0);
    }
    studioCard_.setPlaybackProgress(0.0);
}

void MainComponent::handleImportRequested() {
    if (!rightsConfirmedOnce_) {
        rightsModal_ = std::make_unique<ui::RightsAttestationModal>(
            [this](contracts::RightsBasis basis) {
                handleRightsConfirmed(basis);
            }
        );
        rightsModal_->setBounds(getLocalBounds());
        addAndMakeVisible(rightsModal_.get());
    } else {
        openAudioFileChooser();
    }
}

void MainComponent::handleRightsConfirmed(contracts::RightsBasis basis) {
    attestedBasis_ = basis;
    rightsConfirmedOnce_ = true;
    if (rightsModal_) {
        removeChildComponent(rightsModal_.get());
        rightsModal_.reset();
    }
    rightsStatusButton_.setButtonText("Rights: " + juce::String(std::string(contracts::toString(basis))));
    openAudioFileChooser();
}

void MainComponent::openAudioFileChooser() {
    importFileModal_ = std::make_unique<ui::ImportFileModal>(
        [this](const juce::File& file) {
            if (importFileModal_) {
                removeChildComponent(importFileModal_.get());
                importFileModal_.reset();
            }
            processImportedFile(file);
        },
        [this]() {
            if (importFileModal_) {
                removeChildComponent(importFileModal_.get());
                importFileModal_.reset();
            }
            importCard_.setImportStatus("Import cancelled");
        }
    );
    importFileModal_->setBounds(getLocalBounds());
    addAndMakeVisible(importFileModal_.get());
}

void MainComponent::processImportedFile(const juce::File& file) {
    isPlaying_ = false;
    studioCard_.setIsPlaying(false);
    currentLoadedFile_ = file;
    currentTrackTitle_ = file.getFileNameWithoutExtension();
    importCard_.setImportStatus("Transforming: " + file.getFileName().toStdString() + "...");
    repaint();

    try {
        // 1. Multi-format decode (WAV, M4A, MP3, FLAC)
        auto decoded = audio::AudioDecoder::decodeAnyAudioFile(file.getFullPathName().toRawUTF8());
        currentDurationSecs_ = static_cast<double>(decoded.channels[0].size()) / decoded.sampleRate;
        
        // 2. Encode to PCM WAV for pipeline validator
        auto wavBytes = audio::AudioExporter::encodeWav24Bit(decoded.channels, decoded.sampleRate);
        
        // 3. Construct verified rights attestation
        contracts::RightsAttestation attestation(attestedBasis_, true, "project-desktop");
        
        // 4. Run pipeline with 1-click source cleanup & denoise option
        bool enableCleanup = importCard_.isCleanupEnabled();
        auto output = pipeline_.execute(wavBytes, attestation, currentTuning_, "project-desktop", currentTrackTitle_.toRawUTF8(), enableCleanup);
        
        // 5. Update Waveform UI & Duration
        studioCard_.setDurationSeconds(currentDurationSecs_);
        studioCard_.setWaveformData(output.waveformOverviewPeaks);
        
        // 6. Connect audio transport and DSP under lock
        {
            const juce::ScopedLock sl(audioLock_);
            dualTransport_ = std::move(pipeline_.getTransport());
            dubProcessor_ = std::move(pipeline_.getDubProcessor());
            dualTransport_.prepare(currentSampleRate_, 2);
            dualTransport_.setPlayheadSample(0);
        }

        // Update Card 1 with rich musical analysis badges
        importCard_.setTrackInfo(file.getFileName().toStdString(), output.analysisManifest, currentDurationSecs_);
        
        currentState_ = contracts::ConversionJobState::Completed;
    } catch (const std::exception& ex) {
        importCard_.setImportStatus("Import Error: " + std::string(ex.what()), true);
    }
}

void MainComponent::handleCleanupToggled(bool enabled) {
    juce::ignoreUnused(enabled);
    if (currentLoadedFile_.existsAsFile()) {
        processImportedFile(currentLoadedFile_);
    }
}

void MainComponent::handleExportRequested(audio::AudioExportFormat format, bool includeSubtitles) {
    if (dualTransport_.getTotalLengthSamples() == 0) {
        importCard_.setImportStatus("No track loaded to export — please import a track first.", true);
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

                    // Master to -14.0 LUFS and -1.0 dBTP ceiling
                    auto mastered = audio::AudioMasterer::master({leftCh, rightCh}, 44100.0);

                    modal->updateProgress(0.85f, "Encoding format container (85%)...");

                    // Encode based on format
                    std::vector<uint8_t> outputBytes;
                    if (fmt == audio::AudioExportFormat::Mp3_320Kbps) {
                        outputBytes = audio::AudioExporter::encodeMp3(mastered.masteredAudio, 44100.0);
                    } else {
                        outputBytes = audio::AudioExporter::encodeWav24Bit(mastered.masteredAudio, 44100.0);
                    }

                    // Ensure parent directory exists
                    destFile.getParentDirectory().createDirectory();

                    // Safely write binary data using JUCE native wide-character file I/O (100% Unicode & Chinese safe)
                    destFile.deleteFile();
                    if (!destFile.replaceWithData(outputBytes.data(), outputBytes.size())) {
                        throw std::runtime_error("Could not write destination file: " + destFile.getFileName().toStdString());
                    }

                    // Also save copy into ./exports/ folder
                    auto exportsDir = juce::File::getCurrentWorkingDirectory().getChildFile("exports");
                    exportsDir.createDirectory();
                    auto workspaceCopy = exportsDir.getChildFile(destFile.getFileName());
                    if (workspaceCopy.getFullPathName() != destFile.getFullPathName()) {
                        workspaceCopy.deleteFile();
                        workspaceCopy.replaceWithData(outputBytes.data(), outputBytes.size());
                    }

                    // Export subtitles if requested
                    if (includeSubtitles) {
                        modal->updateProgress(0.95f, "Generating synchronized subtitles (95%)...");
                        audio::SubtitleManager subMgr;
                        subMgr.setUserRevisions({
                            {0.0, currentDurationSecs_ * 0.5, "[Instrumental Intro & One-Drop Riddim]"},
                            {currentDurationSecs_ * 0.5, currentDurationSecs_, "[Lead Vocal & Dub Section]"}
                        });

                        auto srtText = subMgr.formatSrt();
                        auto vttText = subMgr.formatVtt();

                        auto srtFile = exportsDir.getChildFile(currentTrackTitle_ + "_reggae_subtitles.srt");
                        auto vttFile = exportsDir.getChildFile(currentTrackTitle_ + "_reggae_subtitles.vtt");

                        srtFile.replaceWithText(juce::String::fromUTF8(srtText.c_str()));
                        vttFile.replaceWithText(juce::String::fromUTF8(vttText.c_str()));
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

void MainComponent::showAboutModal() {
    aboutModal_ = std::make_unique<ui::AboutDialogModal>([this]() {
        if (aboutModal_) {
            removeChildComponent(aboutModal_.get());
            aboutModal_.reset();
        }
    });
    aboutModal_->setBounds(getLocalBounds());
    addAndMakeVisible(aboutModal_.get());
}

void MainComponent::showHelpModal() {
    helpModal_ = std::make_unique<ui::HelpDialogModal>([this]() {
        if (helpModal_) {
            removeChildComponent(helpModal_.get());
            helpModal_.reset();
        }
    });
    helpModal_->setBounds(getLocalBounds());
    addAndMakeVisible(helpModal_.get());
}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(ui::ReggaeWaveTheme::bgDark);

    // Header background bar
    auto headerBounds = getLocalBounds().removeFromTop(54).toFloat();
    g.setColour(ui::ReggaeWaveTheme::bgSurface);
    g.fillRect(headerBounds);

    g.setColour(ui::ReggaeWaveTheme::bgElevated);
    g.drawHorizontalLine(54, 0.0f, static_cast<float>(getWidth()));

    // Version badge pill background
    g.setColour(ui::ReggaeWaveTheme::bgDark);
    g.fillRoundedRectangle(versionBadgeLabel_.getBounds().toFloat().expanded(4, 2), 10.0f);
    g.setColour(ui::ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(versionBadgeLabel_.getBounds().toFloat().expanded(4, 2), 10.0f, 1.0f);

    // Engine status badge pill background
    g.setColour(ui::ReggaeWaveTheme::bgDark);
    g.fillRoundedRectangle(engineStatusBadge_.getBounds().toFloat().expanded(4, 2), 10.0f);
    g.setColour(ui::ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(engineStatusBadge_.getBounds().toFloat().expanded(4, 2), 10.0f, 1.0f);
}

void MainComponent::resized() {
    auto area = getLocalBounds();

    // 1. Header Bar (54px)
    auto headerArea = area.removeFromTop(54).reduced(16, 8);
    
    // Left: Icon + Title + Version
    appIcon_.setBounds(headerArea.removeFromLeft(36).withSizeKeepingCentre(32, 32));
    headerArea.removeFromLeft(8);
    appTitleLabel_.setBounds(headerArea.removeFromLeft(140));
    versionBadgeLabel_.setBounds(headerArea.removeFromLeft(55).withSizeKeepingCentre(50, 22));

    // Right: About + Help + Engine Status + Rights Button
    aboutButton_.setBounds(headerArea.removeFromRight(60).reduced(0, 3));
    headerArea.removeFromRight(6);
    helpButton_.setBounds(headerArea.removeFromRight(55).reduced(0, 3));
    headerArea.removeFromRight(10);
    engineStatusBadge_.setBounds(headerArea.removeFromRight(175).withSizeKeepingCentre(170, 24));
    headerArea.removeFromRight(10);
    rightsStatusButton_.setBounds(headerArea.removeFromRight(120).reduced(0, 3));

    // 2. Stacked 3 Cards Layout
    auto cardsArea = area.reduced(16);

    // Card 1: Top Intake & Analysis Card (86px)
    importCard_.setBounds(cardsArea.removeFromTop(86));
    cardsArea.removeFromTop(12);

    // Card 3: Bottom Export Card (86px)
    exportCard_.setBounds(cardsArea.removeFromBottom(86));
    cardsArea.removeFromBottom(12);

    // Card 2: Middle Riddim Studio & Visualizer (remaining height)
    studioCard_.setBounds(cardsArea);

    // Reposition any open modals (guaranteed 100% centered)
    if (rightsModal_) rightsModal_->setBounds(getLocalBounds());
    if (exportDialogModal_) exportDialogModal_->setBounds(getLocalBounds());
    if (aboutModal_) aboutModal_->setBounds(getLocalBounds());
    if (helpModal_) helpModal_->setBounds(getLocalBounds());
    if (importFileModal_) importFileModal_->setBounds(getLocalBounds());
}

} // namespace reggaewave::desktop

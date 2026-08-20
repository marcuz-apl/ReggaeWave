#include "MainComponent.h"
#include <reggaewave/audio/SubtitleManager.hpp>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <csignal>

#if defined(_WIN32)
#define rw_popen _popen
#define rw_pclose _pclose
#else
#define rw_popen popen
#define rw_pclose pclose
#endif

namespace reggaewave::desktop {

MainComponent::MainComponent()
    : importCard_([this]() { handleImportRequested(); })
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
#if defined(SIGPIPE)
    // Ignore SIGPIPE to prevent exit code 141
    std::signal(SIGPIPE, SIG_IGN);
#endif

    juce::LookAndFeel::setDefaultLookAndFeel(&theme_);

    // 1. Header Bar: Branding & Status (Clean typography)
    addAndMakeVisible(appIcon_);

    appTitleLabel_.setText("ReggaeWave", juce::dontSendNotification);
    appTitleLabel_.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    appTitleLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    addAndMakeVisible(appTitleLabel_);

    versionBadgeLabel_.setText("v1.2.1", juce::dontSendNotification);
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

    startTimerHz(30);
    setSize(1020, 720);
}

MainComponent::~MainComponent() {
    stopTimer();
    stopLiveAudioStreaming();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void MainComponent::timerCallback() {
    const juce::ScopedLock sl(audioLock_);
    if (isPlaying_ && dualTransport_.getTotalLengthSamples() > 0) {
        double progress = static_cast<double>(dualTransport_.getPlayheadSample()) / 
                          static_cast<double>(dualTransport_.getTotalLengthSamples());
        studioCard_.setPlaybackProgress(progress);
    }
}

void MainComponent::startLiveAudioStreaming() {
    stopLiveAudioStreaming();
    isStreaming_ = true;

    liveAudioThread_ = std::thread([this]() {
#if defined(_WIN32)
        const char* streamCmd = "ffplay -nodisp -autoexit -f f32le -ar 44100 -ch_layout stereo -i - >nul 2>&1";
#else
        const char* streamCmd = "ffplay -nodisp -autoexit -f f32le -ar 44100 -ch_layout stereo -i - 2>/dev/null";
#endif
        FILE* pipe = rw_popen(streamCmd, "w");
        if (!pipe) return;

        const int blockSize = 2048;
        std::vector<float> leftCh(blockSize, 0.0f);
        std::vector<float> rightCh(blockSize, 0.0f);
        std::vector<float> interleaved(blockSize * 2, 0.0f);

        while (isStreaming_) {
            bool hasMoreAudio = true;
            {
                const juce::ScopedLock sl(audioLock_);
                if (dualTransport_.getTotalLengthSamples() == 0 ||
                    dualTransport_.getPlayheadSample() >= dualTransport_.getTotalLengthSamples()) {
                    hasMoreAudio = false;
                } else {
                    std::vector<float*> ptrs = {leftCh.data(), rightCh.data()};
                    dualTransport_.renderNextBlock(ptrs.data(), 2, blockSize);
                    dubProcessor_.process(ptrs.data(), 2, blockSize);

                    for (int i = 0; i < blockSize; ++i) {
                        interleaved[i * 2] = leftCh[i];
                        interleaved[i * 2 + 1] = rightCh[i];
                    }
                }
            }

            if (!hasMoreAudio) {
                juce::MessageManager::callAsync([this]() {
                    isPlaying_ = false;
                    studioCard_.setIsPlaying(false);
                });
                break;
            }

            size_t written = fwrite(interleaved.data(), sizeof(float), interleaved.size(), pipe);
            fflush(pipe);
            if (written < interleaved.size() || ferror(pipe)) {
                break;
            }
        }

        rw_pclose(pipe);
    });
}

void MainComponent::stopLiveAudioStreaming() {
    isStreaming_ = false;
#if defined(_WIN32)
    std::system("taskkill /F /IM ffplay.exe >nul 2>&1");
#else
    std::system("killall -9 ffplay 2>/dev/null");
#endif
    if (liveAudioThread_.joinable()) {
        liveAudioThread_.join();
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

    if (nowPlaying) {
        startLiveAudioStreaming();
    } else {
        stopLiveAudioStreaming();
    }
}

void MainComponent::rewindPlayback() {
    {
        const juce::ScopedLock sl(audioLock_);
        dualTransport_.setPlayheadSample(0);
    }
    studioCard_.setPlaybackProgress(0.0);

    if (isPlaying_) {
        startLiveAudioStreaming();
    }
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
    stopLiveAudioStreaming();
    currentTrackTitle_ = file.getFileNameWithoutExtension().toStdString();
    importCard_.setImportStatus("Transforming: " + file.getFileName().toStdString() + "...");
    repaint();

    try {
        // 1. Multi-format decode (WAV, M4A, MP3, FLAC)
        auto decoded = audio::AudioDecoder::decodeAnyAudioFile(file.getFullPathName().toStdString());
        currentDurationSecs_ = static_cast<double>(decoded.channels[0].size()) / decoded.sampleRate;
        
        // 2. Encode to PCM WAV for pipeline validator
        auto wavBytes = audio::AudioExporter::encodeWav24Bit(decoded.channels, decoded.sampleRate);
        
        // 3. Construct verified rights attestation
        contracts::RightsAttestation attestation(attestedBasis_, true, "project-desktop");
        
        // 4. Run pipeline
        auto output = pipeline_.execute(wavBytes, attestation, currentTuning_, "project-desktop", currentTrackTitle_);
        
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

                    std::ofstream out(destFile.getFullPathName().toStdString(), std::ios::binary);
                    if (!out.is_open()) {
                        throw std::runtime_error("Could not open destination file for writing");
                    }
                    out.write(reinterpret_cast<const char*>(outputBytes.data()), outputBytes.size());
                    out.close();

                    // Also save copy into ./exports/ folder
                    auto exportsDir = juce::File::getCurrentWorkingDirectory().getChildFile("exports");
                    exportsDir.createDirectory();
                    auto workspaceCopy = exportsDir.getChildFile(destFile.getFileName());
                    if (workspaceCopy.getFullPathName() != destFile.getFullPathName()) {
                        std::ofstream copyOut(workspaceCopy.getFullPathName().toStdString(), std::ios::binary);
                        if (copyOut.is_open()) {
                            copyOut.write(reinterpret_cast<const char*>(outputBytes.data()), outputBytes.size());
                        }
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

                        auto srtFile = exportsDir.getChildFile(juce::String(currentTrackTitle_) + "_reggae_subtitles.srt");
                        auto vttFile = exportsDir.getChildFile(juce::String(currentTrackTitle_) + "_reggae_subtitles.vtt");

                        std::ofstream srtOut(srtFile.getFullPathName().toStdString());
                        if (srtOut.is_open()) srtOut << srtText;

                        std::ofstream vttOut(vttFile.getFullPathName().toStdString());
                        if (vttOut.is_open()) vttOut << vttText;
                    }

                    modal->setExportCompleted(destFile.getFileName().toStdString());
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
    aboutModal_ = std::make_unique<ui::InfoDialogModal>([this]() {
        if (aboutModal_) {
            removeChildComponent(aboutModal_.get());
            aboutModal_.reset();
        }
    });
    aboutModal_->setBounds(getLocalBounds());
    addAndMakeVisible(aboutModal_.get());
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

    // Right: About + Engine Status + Rights Button
    aboutButton_.setBounds(headerArea.removeFromRight(65).reduced(0, 3));
    headerArea.removeFromRight(10);
    engineStatusBadge_.setBounds(headerArea.removeFromRight(180).withSizeKeepingCentre(175, 24));
    headerArea.removeFromRight(10);
    rightsStatusButton_.setBounds(headerArea.removeFromRight(130).reduced(0, 3));

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
    if (importFileModal_) importFileModal_->setBounds(getLocalBounds());
}

} // namespace reggaewave::desktop

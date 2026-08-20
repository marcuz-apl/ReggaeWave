#include "MainComponent.h"
#include <fstream>
#include <cstdlib>

namespace reggaewave::desktop {

MainComponent::MainComponent()
    : waveformView_(
        [this](audio::ActiveVariation var) {
            currentVariation_ = var;
            dualTransport_.setActiveVariation(var);
            renderPreviewWavToDisk();
        },
        [this](double normPos) {
            size_t targetSample = static_cast<size_t>(normPos * dualTransport_.getTotalLengthSamples());
            dualTransport_.setPlayheadSample(targetSample);
        }
    )
    , tuningPanel_([this](const contracts::TuningParameters& params) {
        currentTuning_ = params;
        dubProcessor_.setDubAmount(params.getDubEffectsAmount());
        dualTransport_.setVocalGainDb(params.getVocalLevelDb());
        if (pipeline_.isReady()) {
            pipeline_.updateTuning(params);
            renderPreviewWavToDisk();
        }
    })
{
    juce::LookAndFeel::setDefaultLookAndFeel(&theme_);

    // 1. Branding Header
    appTitleLabel_.setText("ReggaeWave", juce::dontSendNotification);
    appTitleLabel_.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    appTitleLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    addAndMakeVisible(appTitleLabel_);

    statusBadgeLabel_.setText("Ready", juce::dontSendNotification);
    statusBadgeLabel_.setFont(juce::FontOptions(13.0f));
    statusBadgeLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGreen);
    addAndMakeVisible(statusBadgeLabel_);

    rightsStatusButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgSurface);
    rightsStatusButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::textSecondary);
    rightsStatusButton_.onClick = [this]() {
        rightsConfirmedOnce_ = false;
        handleImportRequested();
    };
    addAndMakeVisible(rightsStatusButton_);

    // 2. Buttons
    importButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    importButton_.onClick = [this]() { handleImportRequested(); };
    addAndMakeVisible(importButton_);

    playButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::accentGold);
    playButton_.setColour(juce::TextButton::textColourOffId, ui::ReggaeWaveTheme::bgDark);
    playButton_.onClick = [this]() { togglePlayback(); };
    addAndMakeVisible(playButton_);

    exportButton_.setColour(juce::TextButton::buttonColourId, ui::ReggaeWaveTheme::bgElevated);
    exportButton_.onClick = [this]() { handleExportRequested(); };
    addAndMakeVisible(exportButton_);

    // 3. Main Views
    addAndMakeVisible(waveformView_);
    addAndMakeVisible(tuningPanel_);
    addAndMakeVisible(lyricEditor_);

    // Audio setup
    setAudioChannels(0, 2);
    startTimerHz(30);

    setSize(920, 640);
}

MainComponent::~MainComponent() {
    stopTimer();
    shutdownAudio();
    std::system("pkill -9 mpv 2>/dev/null");
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    currentSampleRate_ = sampleRate > 0.0 ? sampleRate : 44100.0;
    dualTransport_.prepare(currentSampleRate_, 2);
    dubProcessor_.prepare(currentSampleRate_, samplesPerBlockExpected, 2);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    if (!isPlaying_ || dualTransport_.getTotalLengthSamples() == 0) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    const int numChannels = bufferToFill.buffer->getNumChannels();
    const int numSamples = bufferToFill.numSamples;
    const int startSample = bufferToFill.startSample;

    std::vector<float*> channels(numChannels);
    for (int ch = 0; ch < numChannels; ++ch) {
        channels[ch] = bufferToFill.buffer->getWritePointer(ch, startSample);
    }

    dualTransport_.renderNextBlock(channels.data(), numChannels, numSamples);
    dubProcessor_.process(channels.data(), numChannels, numSamples);

    if (dualTransport_.getPlayheadSample() >= dualTransport_.getTotalLengthSamples()) {
        isPlaying_ = false;
        playButton_.setButtonText("Play");
    }
}

void MainComponent::releaseResources() {
    dubProcessor_.reset();
}

void MainComponent::timerCallback() {
    if (isPlaying_ && dualTransport_.getTotalLengthSamples() > 0) {
        double progress = static_cast<double>(dualTransport_.getPlayheadSample()) / 
                          static_cast<double>(dualTransport_.getTotalLengthSamples());
        waveformView_.setPlaybackProgress(progress);
    }
}

void MainComponent::togglePlayback() {
    if (dualTransport_.getTotalLengthSamples() == 0) {
        return;
    }
    if (dualTransport_.getPlayheadSample() >= dualTransport_.getTotalLengthSamples()) {
        dualTransport_.setPlayheadSample(0);
    }
    isPlaying_ = !isPlaying_;
    playButton_.setButtonText(isPlaying_ ? "Pause" : "Play");

    if (isPlaying_) {
        // In WSL2, seamlessly route playback via mpv if ALSA hardware is virtualized
        std::system("pkill -9 mpv 2>/dev/null");
        std::system("mpv --no-terminal --really-quiet /tmp/reggaewave_preview.wav &");
    } else {
        std::system("pkill -9 mpv 2>/dev/null");
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
    statusBadgeLabel_.setText("Opening file chooser...", juce::dontSendNotification);

    auto initialDir = juce::File::getCurrentWorkingDirectory().getChildFile("test-tracks");
    if (!initialDir.exists()) {
        initialDir = juce::File::getCurrentWorkingDirectory();
    }

    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Select Audio Track to Transform to Reggae",
        initialDir,
        "*.wav;*.mp3;*.m4a;*.flac;*.ogg;*.aac;*.aiff"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file.existsAsFile()) {
            processImportedFile(file);
        } else {
            statusBadgeLabel_.setText("Ready", juce::dontSendNotification);
        }
    });
}

void MainComponent::processImportedFile(const juce::File& file) {
    currentTrackTitle_ = file.getFileNameWithoutExtension().toStdString();
    statusBadgeLabel_.setText("Transforming: " + file.getFileName() + "...", juce::dontSendNotification);
    statusBadgeLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    repaint();

    try {
        // 1. Multi-format decode (WAV, M4A, MP3, FLAC)
        auto decoded = audio::AudioDecoder::decodeAnyAudioFile(file.getFullPathName().toStdString());
        
        // 2. Encode to PCM WAV for pipeline validator
        auto wavBytes = audio::AudioExporter::encodeWav24Bit(decoded.channels, decoded.sampleRate);
        
        // 3. Construct verified rights attestation
        contracts::RightsAttestation attestation(attestedBasis_, true, "project-desktop");
        
        // 4. Run pipeline
        auto output = pipeline_.execute(wavBytes, attestation, currentTuning_, "project-desktop", currentTrackTitle_);
        
        // 5. Update Waveform UI
        waveformView_.setWaveformData(output.waveformOverviewPeaks);
        
        // 6. Connect audio transport and DSP
        dualTransport_ = std::move(pipeline_.getTransport());
        dubProcessor_ = std::move(pipeline_.getDubProcessor());
        dualTransport_.prepare(currentSampleRate_, 2);
        dualTransport_.setPlayheadSample(0);

        // Render preview cache to disk for instant WSL audio playback
        renderPreviewWavToDisk();

        statusBadgeLabel_.setText("Ready: " + file.getFileNameWithoutExtension() + 
                                  " (" + juce::String(static_cast<int>(output.analysisManifest.bpm)) + " BPM, " +
                                  juce::String(output.analysisManifest.key) + ")", juce::dontSendNotification);
        statusBadgeLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGreen);
        
        currentState_ = contracts::ConversionJobState::Completed;
    } catch (const std::exception& ex) {
        statusBadgeLabel_.setText("Import Error: " + juce::String(ex.what()), juce::dontSendNotification);
        statusBadgeLabel_.setColour(juce::Label::textColourId, juce::Colours::red);
    }
}

void MainComponent::renderPreviewWavToDisk() {
    if (dualTransport_.getTotalLengthSamples() == 0) return;

    const size_t totalSamples = dualTransport_.getTotalLengthSamples();
    const size_t renderSamples = std::min(totalSamples, size_t{44100 * 180}); // Up to 3 mins preview
    
    // Render full mix to buffer
    std::vector<float> leftCh(renderSamples, 0.0f);
    std::vector<float> rightCh(renderSamples, 0.0f);
    std::vector<float*> ptrs = {leftCh.data(), rightCh.data()};

    // Snapshot transport
    audio::DualTransportSource tempTransport = dualTransport_;
    audio::DubEffectsProcessor tempDub = dubProcessor_;
    tempTransport.setPlayheadSample(0);

    const int blockSize = 1024;
    for (size_t pos = 0; pos < renderSamples; pos += blockSize) {
        int samplesToProcess = static_cast<int>(std::min(size_t{blockSize}, renderSamples - pos));
        std::vector<float*> blockPtrs = {leftCh.data() + pos, rightCh.data() + pos};
        tempTransport.renderNextBlock(blockPtrs.data(), 2, samplesToProcess);
        tempDub.process(blockPtrs.data(), 2, samplesToProcess);
    }

    auto mastered = audio::AudioMasterer::master({leftCh, rightCh}, 44100.0);
    auto wavBytes = audio::AudioExporter::encodeWav24Bit(mastered.masteredAudio, 44100.0);

    std::ofstream out("/tmp/reggaewave_preview.wav", std::ios::binary);
    if (out.is_open()) {
        out.write(reinterpret_cast<const char*>(wavBytes.data()), wavBytes.size());
    }
}

void MainComponent::handleExportRequested() {
    if (dualTransport_.getTotalLengthSamples() == 0) {
        statusBadgeLabel_.setText("No track loaded to export", juce::dontSendNotification);
        return;
    }

    auto defaultExportFile = juce::File::getCurrentWorkingDirectory()
                                .getChildFile("exports")
                                .getChildFile(juce::String(currentTrackTitle_) + "_reggae_master.wav");

    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Export Reggae Master (-14 LUFS / -1 dBTP)",
        defaultExportFile,
        "*.wav;*.mp3"
    );

    auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting;
    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto dest = fc.getResult();
        if (dest.getFullPathName().isNotEmpty()) {
            performExportToFile(dest);
        }
    });
}

void MainComponent::performExportToFile(const juce::File& destinationFile) {
    statusBadgeLabel_.setText("Mastering & Exporting: " + destinationFile.getFileName() + "...", juce::dontSendNotification);
    statusBadgeLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    repaint();

    try {
        const size_t totalSamples = dualTransport_.getTotalLengthSamples();
        std::vector<float> leftCh(totalSamples, 0.0f);
        std::vector<float> rightCh(totalSamples, 0.0f);

        audio::DualTransportSource tempTransport = dualTransport_;
        audio::DubEffectsProcessor tempDub = dubProcessor_;
        tempTransport.setPlayheadSample(0);

        const int blockSize = 1024;
        for (size_t pos = 0; pos < totalSamples; pos += blockSize) {
            int samplesToProcess = static_cast<int>(std::min(size_t{blockSize}, totalSamples - pos));
            std::vector<float*> blockPtrs = {leftCh.data() + pos, rightCh.data() + pos};
            tempTransport.renderNextBlock(blockPtrs.data(), 2, samplesToProcess);
            tempDub.process(blockPtrs.data(), 2, samplesToProcess);
        }

        // Master to -14.0 LUFS and -1.0 dBTP ceiling
        auto mastered = audio::AudioMasterer::master({leftCh, rightCh}, 44100.0);
        auto wavBytes = audio::AudioExporter::encodeWav24Bit(mastered.masteredAudio, 44100.0);

        // Ensure parent directory exists
        destinationFile.getParentDirectory().createDirectory();

        std::ofstream out(destinationFile.getFullPathName().toStdString(), std::ios::binary);
        if (!out.is_open()) {
            throw std::runtime_error("Could not open destination file for writing");
        }
        out.write(reinterpret_cast<const char*>(wavBytes.data()), wavBytes.size());
        out.close();

        // Also save a copy to workspace ./exports/ folder
        auto exportsDir = juce::File::getCurrentWorkingDirectory().getChildFile("exports");
        exportsDir.createDirectory();
        auto workspaceCopy = exportsDir.getChildFile(destinationFile.getFileName());
        if (workspaceCopy.getFullPathName() != destinationFile.getFullPathName()) {
            std::ofstream copyOut(workspaceCopy.getFullPathName().toStdString(), std::ios::binary);
            if (copyOut.is_open()) {
                copyOut.write(reinterpret_cast<const char*>(wavBytes.data()), wavBytes.size());
            }
        }

        statusBadgeLabel_.setText("Master Exported: " + destinationFile.getFileName() + " (" + 
                                  juce::String(mastered.integratedLufs, 1) + " LUFS, " +
                                  juce::String(mastered.truePeakDb, 1) + " dBTP)", juce::dontSendNotification);
        statusBadgeLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGreen);
    } catch (const std::exception& ex) {
        statusBadgeLabel_.setText("Export Error: " + juce::String(ex.what()), juce::dontSendNotification);
        statusBadgeLabel_.setColour(juce::Label::textColourId, juce::Colours::red);
    }
}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(ui::ReggaeWaveTheme::bgDark);
}

void MainComponent::resized() {
    auto area = getLocalBounds().reduced(24);

    // Top Header
    auto headerArea = area.removeFromTop(44);
    appTitleLabel_.setBounds(headerArea.removeFromLeft(170));
    rightsStatusButton_.setBounds(headerArea.removeFromLeft(150).reduced(0, 6));
    headerArea.removeFromLeft(10);
    statusBadgeLabel_.setBounds(headerArea.removeFromLeft(280));
    exportButton_.setBounds(headerArea.removeFromRight(190));
    headerArea.removeFromRight(10);
    importButton_.setBounds(headerArea.removeFromRight(130));

    area.removeFromTop(16);

    // Waveform & A/B Section
    waveformView_.setBounds(area.removeFromTop(180));
    area.removeFromTop(16);

    // Bottom Split: Left Tuning Panel (3 controls), Right Lyrics / Options
    auto bottomArea = area.removeFromTop(180);
    playButton_.setBounds(bottomArea.removeFromTop(40).removeFromLeft(120));
    bottomArea.removeFromTop(10);

    auto splitArea = bottomArea;
    tuningPanel_.setBounds(splitArea.removeFromLeft(splitArea.getWidth() / 2 - 8));
    splitArea.removeFromLeft(16);
    lyricEditor_.setBounds(splitArea);

    if (rightsModal_) {
        rightsModal_->setBounds(getLocalBounds());
    }
}

} // namespace reggaewave::desktop

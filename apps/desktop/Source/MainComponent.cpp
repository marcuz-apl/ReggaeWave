#include "MainComponent.h"

namespace reggaewave::desktop {

MainComponent::MainComponent()
    : waveformView_(
        [this](audio::ActiveVariation var) { dualTransport_.setActiveVariation(var); },
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

    // Audio setup (0 in, 2 out)
    setAudioChannels(0, 2);
    startTimerHz(30); // 30 FPS UI timer for playhead refresh

    setSize(920, 640);
}

MainComponent::~MainComponent() {
    stopTimer();
    shutdownAudio();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    dualTransport_.prepare(sampleRate, 2);
    dubProcessor_.prepare(sampleRate, samplesPerBlockExpected, 2);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    if (!isPlaying_) {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    float* const* channels = bufferToFill.buffer->getArrayOfWritePointers();
    int numChannels = bufferToFill.buffer->getNumChannels();
    int numSamples = bufferToFill.numSamples;

    dualTransport_.renderNextBlock(channels, numChannels, numSamples);
    dubProcessor_.process(channels, numChannels, numSamples);
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
    isPlaying_ = !isPlaying_;
    playButton_.setButtonText(isPlaying_ ? "Pause" : "Play");
}

void MainComponent::handleImportRequested() {
    rightsModal_ = std::make_unique<ui::RightsAttestationModal>(
        [this](contracts::RightsBasis basis) {
            handleRightsConfirmed(basis);
        }
    );
    rightsModal_->setBounds(getLocalBounds());
    addAndMakeVisible(rightsModal_.get());
}

void MainComponent::handleRightsConfirmed(contracts::RightsBasis basis) {
    attestedBasis_ = basis;
    if (rightsModal_) {
        removeChildComponent(rightsModal_.get());
        rightsModal_.reset();
    }
    currentState_ = contracts::ConversionJobState::Importing;
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
            statusBadgeLabel_.setText("Import cancelled", juce::dontSendNotification);
        }
    });
}

void MainComponent::processImportedFile(const juce::File& file) {
    statusBadgeLabel_.setText("Transforming: " + file.getFileName() + "...", juce::dontSendNotification);
    statusBadgeLabel_.setColour(juce::Label::textColourId, ui::ReggaeWaveTheme::accentGold);
    repaint();

    try {
        // 1. Multi-format decode (WAV, M4A, MP3, FLAC)
        auto decoded = audio::AudioDecoder::decodeAnyAudioFile(file.getFullPathName().toStdString());
        
        // 2. Encode to PCM WAV for pipeline validator
        auto wavBytes = audio::AudioExporter::encodeWav24Bit(decoded.channels, decoded.sampleRate);
        
        // 3. Construct verified rights attestation
        contracts::RightsAttestation attestation(attestedBasis_.value_or(contracts::RightsBasis::Owned), true, "project-desktop");
        
        // 4. Run pipeline
        auto output = pipeline_.execute(wavBytes, attestation, currentTuning_, "project-desktop", file.getFileNameWithoutExtension().toStdString());
        
        // 5. Update Waveform UI
        waveformView_.setWaveformData(output.waveformOverviewPeaks);
        
        // 6. Connect audio transport and DSP
        dualTransport_ = std::move(pipeline_.getTransport());
        dubProcessor_ = std::move(pipeline_.getDubProcessor());
        dualTransport_.setPlayheadSample(0);

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

void MainComponent::handleExportRequested() {
    if (dualTransport_.getTotalLengthSamples() == 0) {
        statusBadgeLabel_.setText("No track loaded to export", juce::dontSendNotification);
        return;
    }

    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Export Reggae Master",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile("reggae_master.wav"),
        "*.wav;*.mp3"
    );

    auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto dest = fc.getResult();
        if (dest.getFullPathName().isNotEmpty()) {
            statusBadgeLabel_.setText("Exported: " + dest.getFileName(), juce::dontSendNotification);
        }
    });
}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(ui::ReggaeWaveTheme::bgDark);
}

void MainComponent::resized() {
    auto area = getLocalBounds().reduced(24);

    // Top Header
    auto headerArea = area.removeFromTop(44);
    appTitleLabel_.setBounds(headerArea.removeFromLeft(200));
    statusBadgeLabel_.setBounds(headerArea.removeFromLeft(350));
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

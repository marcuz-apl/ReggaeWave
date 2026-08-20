#include "ExportDialogModal.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

ExportDialogModal::ExportDialogModal(const std::string& trackTitle,
                                     audio::AudioExportFormat format,
                                     OnPerformExport onPerformExport,
                                     OnClose onClose)
    : trackTitle_(trackTitle)
    , format_(format)
    , onPerformExport_(std::move(onPerformExport))
    , onClose_(std::move(onClose))
{
    setAlwaysOnTop(true);

    // 1. Header & Format Specs
    titleLabel_.setText("Mastering & Export", juce::dontSendNotification);
    titleLabel_.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(titleLabel_);

    std::string ext = (format_ == audio::AudioExportFormat::Mp3_320Kbps) ? ".mp3" : ".wav";
    std::string specText = (format_ == audio::AudioExportFormat::Mp3_320Kbps)
        ? "Format: MP3 • 320 kbps CBR • 44.1 kHz • -14.0 LUFS Integrated • -1.0 dBTP Ceiling"
        : "Format: WAV • 24-bit PCM • 44.1 kHz Stereo • -14.0 LUFS Integrated • -1.0 dBTP Ceiling";

    descriptionLabel_.setText(specText, juce::dontSendNotification);
    descriptionLabel_.setFont(juce::FontOptions(12.0f));
    descriptionLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    addAndMakeVisible(descriptionLabel_);

    pathHeaderLabel_.setText("Destination File Path:", juce::dontSendNotification);
    pathHeaderLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    pathHeaderLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textPrimary);
    addAndMakeVisible(pathHeaderLabel_);

    // 2. Default Path in exports/
    auto defaultExportFile = juce::File::getCurrentWorkingDirectory()
                                .getChildFile("exports")
                                .getChildFile(juce::String(trackTitle_) + "_reggae_master" + ext);

    pathEditor_.setText(defaultExportFile.getFullPathName());
    pathEditor_.setFont(juce::FontOptions(13.0f));
    pathEditor_.setColour(juce::TextEditor::backgroundColourId, ReggaeWaveTheme::bgDark);
    pathEditor_.setColour(juce::TextEditor::textColourId, ReggaeWaveTheme::textPrimary);
    pathEditor_.setColour(juce::TextEditor::outlineColourId, ReggaeWaveTheme::bgElevated);
    addAndMakeVisible(pathEditor_);

    browseButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    browseButton_.onClick = [this]() { handleBrowseClicked(); };
    addAndMakeVisible(browseButton_);

    // 3. Progress Bar (hidden child initially, NOT visible!)
    addChildComponent(progressBar_);
    progressBar_.setVisible(false);

    // 4. Action Buttons
    cancelButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    cancelButton_.onClick = [this]() {
        if (!isExporting_ && onClose_) {
            onClose_();
        }
    };
    addAndMakeVisible(cancelButton_);

    actionButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGold);
    actionButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    actionButton_.onClick = [this]() {
        if (isDone_) {
            if (onClose_) onClose_();
        } else if (!isExporting_) {
            handleSaveClicked();
        }
    };
    addAndMakeVisible(actionButton_);
}

void ExportDialogModal::handleBrowseClicked() {
    std::string ext = (format_ == audio::AudioExportFormat::Mp3_320Kbps) ? "*.mp3" : "*.wav";
    auto currentFile = juce::File(pathEditor_.getText());

    fileChooser_ = std::make_unique<juce::FileChooser>(
        "Choose Destination File",
        currentFile.exists() ? currentFile : juce::File::getCurrentWorkingDirectory().getChildFile("exports"),
        ext
    );

    auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting;
    fileChooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result.getFullPathName().isNotEmpty()) {
            pathEditor_.setText(result.getFullPathName());
        }
    });
}

void ExportDialogModal::handleSaveClicked() {
    juce::File destFile(pathEditor_.getText());
    if (destFile.getFullPathName().isEmpty()) {
        return;
    }

    isExporting_ = true;
    pathHeaderLabel_.setText("Exporting Progress:", juce::dontSendNotification);

    // Hide path editor & browse button, immediately show progress bar at 0%
    pathEditor_.setVisible(false);
    browseButton_.setVisible(false);
    progressBar_.setVisible(true);
    progressBar_.reset();
    progressBar_.setProgress(0.0f, "Initializing Mastering Engine (0%)...");

    cancelButton_.setEnabled(false);
    actionButton_.setEnabled(false);
    actionButton_.setButtonText("Exporting...");

    repaint();

    // Trigger export in background
    if (onPerformExport_) {
        onPerformExport_(destFile, format_, this);
    }
}

void ExportDialogModal::updateProgress(float progress0To1, const std::string& stageText) {
    juce::MessageManager::callAsync([this, progress0To1, stageText]() {
        progressBar_.setProgress(progress0To1, stageText);
        repaint();
    });
}

void ExportDialogModal::setExportCompleted(const std::string& savedFilename) {
    juce::MessageManager::callAsync([this, savedFilename]() {
        isDone_ = true;
        isExporting_ = false;

        progressBar_.setProgress(1.0f, "Saved: " + savedFilename);

        pathHeaderLabel_.setText("Status:", juce::dontSendNotification);
        cancelButton_.setVisible(false);

        actionButton_.setEnabled(true);
        actionButton_.setButtonText("Done");
        actionButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGreen);
        actionButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
        repaint();
    });
}

void ExportDialogModal::setExportError(const std::string& errorMessage) {
    juce::MessageManager::callAsync([this, errorMessage]() {
        isExporting_ = false;
        progressBar_.setProgress(1.0f, "Error: " + errorMessage);

        cancelButton_.setEnabled(true);
        actionButton_.setEnabled(true);
        actionButton_.setButtonText("Retry");
        repaint();
    });
}

void ExportDialogModal::paint(juce::Graphics& g) {
    // Dimmed background
    g.fillAll(juce::Colours::black.withAlpha(0.75f));

    // Centered modal card
    auto cardArea = getLocalBounds().withSizeKeepingCentre(620, 270).toFloat();
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(cardArea, 12.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(cardArea, 12.0f, 1.5f);
}

void ExportDialogModal::resized() {
    auto cardArea = getLocalBounds().withSizeKeepingCentre(620, 270).reduced(24);

    // 1. Top Title & Specs
    titleLabel_.setBounds(cardArea.removeFromTop(28));
    descriptionLabel_.setBounds(cardArea.removeFromTop(20));
    cardArea.removeFromTop(12);

    // 2. Action Buttons at bottom
    auto btnRow = cardArea.removeFromBottom(38);
    actionButton_.setBounds(btnRow.removeFromRight(120));
    btnRow.removeFromRight(10);
    cancelButton_.setBounds(btnRow.removeFromRight(100));

    cardArea.removeFromBottom(14);

    // 3. Middle Path / Progress Bar Area
    pathHeaderLabel_.setBounds(cardArea.removeFromTop(20));
    cardArea.removeFromTop(6);

    auto pathRow = cardArea.removeFromTop(38);

    // Path editor + browse button initially take pathRow
    pathEditor_.setBounds(pathRow.removeFromLeft(pathRow.getWidth() - 90));
    pathRow.removeFromLeft(8);
    browseButton_.setBounds(pathRow);

    // Progress bar overlaps the exact path editor bounds when visible!
    progressBar_.setBounds(pathEditor_.getBounds().withRight(browseButton_.getRight()));
}

} // namespace reggaewave::ui

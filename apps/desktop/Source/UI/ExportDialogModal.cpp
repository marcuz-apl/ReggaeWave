#include "ExportDialogModal.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

ExportDialogModal::ExportDialogModal(const juce::String& trackTitle,
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

    juce::String ext = (format_ == audio::AudioExportFormat::Mp3_320Kbps) ? ".mp3" : ".wav";
    juce::String specText = (format_ == audio::AudioExportFormat::Mp3_320Kbps)
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
                                .getChildFile(trackTitle_ + "_reggae_master" + ext);

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
    juce::String ext = (format_ == audio::AudioExportFormat::Mp3_320Kbps) ? "*.mp3" : "*.wav";
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

void ExportDialogModal::updateProgress(float progress0To1, const juce::String& stageText) {
    juce::MessageManager::callAsync([this, progress0To1, stageText]() {
        progressBar_.setProgress(progress0To1, stageText);
        repaint();
    });
}

void ExportDialogModal::setExportCompleted(const juce::String& savedFilename) {
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

void ExportDialogModal::setExportError(const juce::String& errorMessage) {
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
    // 1. Semi-transparent backdrop overlay
    g.fillAll(juce::Colours::black.withAlpha(0.65f));

    // 2. Centered dialog box
    auto bounds = getLocalBounds().toFloat();
    auto dialogArea = bounds.withSizeKeepingCentre(540.0f, 240.0f);

    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(dialogArea, 12.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(dialogArea, 12.0f, 1.5f);
}

void ExportDialogModal::resized() {
    auto bounds = getLocalBounds();
    auto dialogArea = bounds.withSizeKeepingCentre(540, 240).reduced(24);

    // Header
    titleLabel_.setBounds(dialogArea.removeFromTop(26));
    dialogArea.removeFromTop(4);
    descriptionLabel_.setBounds(dialogArea.removeFromTop(18));
    dialogArea.removeFromTop(18);

    // Path Label
    pathHeaderLabel_.setBounds(dialogArea.removeFromTop(18));
    dialogArea.removeFromTop(6);

    // Path Editor & Browse Button OR Progress Bar
    auto pathRow = dialogArea.removeFromTop(36);
    browseButton_.setBounds(pathRow.removeFromRight(90));
    pathRow.removeFromRight(8);
    pathEditor_.setBounds(pathRow);

    // Progress bar perfectly overlaps the pathRow
    progressBar_.setBounds(pathRow.getX(), pathRow.getY(), pathRow.getWidth() + 98, pathRow.getHeight());

    dialogArea.removeFromTop(20);

    // Action buttons at bottom right
    auto btnRow = dialogArea.removeFromBottom(34);
    actionButton_.setBounds(btnRow.removeFromRight(100));
    btnRow.removeFromRight(10);
    cancelButton_.setBounds(btnRow.removeFromRight(90));
}

} // namespace reggaewave::ui

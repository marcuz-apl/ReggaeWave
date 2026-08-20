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

    std::string ext = (format_ == audio::AudioExportFormat::Mp3_320Kbps) ? ".mp3" : ".wav";
    std::string formatLabel = (format_ == audio::AudioExportFormat::Mp3_320Kbps) ? "320 kbps MP3" : "24-bit WAV";

    // 1. Headers
    titleLabel_.setText("Export Reggae Master (" + juce::String(formatLabel) + ")", juce::dontSendNotification);
    titleLabel_.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(titleLabel_);

    descriptionLabel_.setText("Processed with authentic One-Drop / Steppers riddims, sub-bass, and -14.0 LUFS mastering.", juce::dontSendNotification);
    descriptionLabel_.setFont(juce::FontOptions(13.0f));
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

    // Hide path editor & browse button, show progress bar
    pathEditor_.setVisible(false);
    browseButton_.setVisible(false);
    progressBar_.setVisible(true);
    progressBar_.reset();
    progressBar_.setProgress(0.15f, "Rendering Audio Stems...");

    cancelButton_.setEnabled(false);
    actionButton_.setEnabled(false);
    actionButton_.setButtonText("Exporting...");

    // Execute export asynchronously so UI refreshes immediately on single click
    juce::MessageManager::callAsync([this, destFile]() {
        if (onPerformExport_) {
            onPerformExport_(destFile, format_, this);
        }
    });
}

void ExportDialogModal::updateProgress(float progress0To1, const std::string& stageText) {
    progressBar_.setProgress(progress0To1, stageText);
}

void ExportDialogModal::setExportCompleted(const std::string& savedFilename) {
    isDone_ = true;
    isExporting_ = false;

    progressBar_.setProgress(1.0f, "Saved: " + savedFilename);

    pathHeaderLabel_.setText("Status:", juce::dontSendNotification);
    cancelButton_.setVisible(false);

    actionButton_.setEnabled(true);
    actionButton_.setButtonText("Done");
    actionButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGreen);
    actionButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
}

void ExportDialogModal::setExportError(const std::string& errorMessage) {
    isExporting_ = false;
    progressBar_.setProgress(1.0f, "Error: " + errorMessage);

    cancelButton_.setEnabled(true);
    actionButton_.setEnabled(true);
    actionButton_.setButtonText("Retry");
}

void ExportDialogModal::paint(juce::Graphics& g) {
    // Backdrop shadow overlay
    g.fillAll(juce::Colours::black.withAlpha(0.72f));

    auto cardArea = getLocalBounds().withSizeKeepingCentre(580, 270).toFloat();

    // Modal Card
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(cardArea, 14.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(cardArea, 14.0f, 1.5f);
}

void ExportDialogModal::resized() {
    auto cardArea = getLocalBounds().withSizeKeepingCentre(580, 270).reduced(24);

    titleLabel_.setBounds(cardArea.removeFromTop(32));
    descriptionLabel_.setBounds(cardArea.removeFromTop(24));
    cardArea.removeFromTop(16);

    pathHeaderLabel_.setBounds(cardArea.removeFromTop(20));
    cardArea.removeFromTop(6);

    // Path row / Overlapping Progress Bar bounds
    auto pathRow = cardArea.removeFromTop(36);
    
    // Position path editor and browse button
    auto browseBounds = pathRow.removeFromRight(95);
    pathRow.removeFromRight(8);
    pathEditor_.setBounds(pathRow);
    browseButton_.setBounds(browseBounds);

    // Progress bar takes the entire combined path row width
    auto progressBounds = pathRow;
    progressBounds.setRight(browseBounds.getRight());
    progressBar_.setBounds(progressBounds);

    cardArea.removeFromTop(22);

    // Bottom Action Buttons
    auto buttonRow = cardArea.removeFromTop(38);
    actionButton_.setBounds(buttonRow.removeFromRight(120));
    buttonRow.removeFromRight(12);
    cancelButton_.setBounds(buttonRow.removeFromRight(100));
}

} // namespace reggaewave::ui

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/audio/AudioExporter.hpp>
#include "InlineExportProgressBar.h"
#include <functional>

namespace reggaewave::ui {

/**
 * @brief Dedicated Export Window with in-place progress bar overlapping the filename path area,
 * transitioning the "Save" button to "Done" upon completion.
 * Fully supports Unicode / Chinese song titles without mojibake.
 */
class ExportDialogModal : public juce::Component {
public:
    using OnPerformExport = std::function<void(const juce::File& destination, audio::AudioExportFormat format, class ExportDialogModal*)>;
    using OnClose = std::function<void()>;

    ExportDialogModal(const juce::String& trackTitle,
                      audio::AudioExportFormat format,
                      OnPerformExport onPerformExport,
                      OnClose onClose);
    ~ExportDialogModal() override = default;

    void updateProgress(float progress0To1, const juce::String& stageText);
    void setExportCompleted(const juce::String& savedFilename);
    void setExportError(const juce::String& errorMessage);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void handleSaveClicked();
    void handleBrowseClicked();

    juce::String trackTitle_;
    audio::AudioExportFormat format_;
    OnPerformExport onPerformExport_;
    OnClose onClose_;

    juce::Label titleLabel_;
    juce::Label descriptionLabel_;
    juce::Label pathHeaderLabel_;
    
    juce::TextEditor pathEditor_;
    juce::TextButton browseButton_{"Browse..."};
    
    InlineExportProgressBar progressBar_;

    juce::TextButton cancelButton_{"Cancel"};
    juce::TextButton actionButton_{"Save"};

    std::unique_ptr<juce::FileChooser> fileChooser_;
    bool isExporting_ = false;
    bool isDone_ = false;
};

} // namespace reggaewave::ui

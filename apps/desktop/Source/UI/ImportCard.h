#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/contracts/Manifests.hpp>
#include <functional>
#include <string>

namespace reggaewave::ui {

/**
 * @brief Top Card 1: Track Intake, Decoding & Musical Analysis Overview.
 */
class ImportCard : public juce::Component {
public:
    using OnImportClicked = std::function<void()>;
    using OnCleanupToggled = std::function<void(bool enabled)>;

    explicit ImportCard(OnImportClicked onImportClicked, OnCleanupToggled onCleanupToggled = nullptr);
    ~ImportCard() override = default;

    void setTrackInfo(const std::string& filename, const contracts::MusicalAnalysisManifest& manifest, double durationSeconds);
    void setImportStatus(const std::string& statusText, bool isError = false);
    void reset();

    [[nodiscard]] bool isCleanupEnabled() const noexcept { return isCleanupEnabled_; }
    void setCleanupEnabled(bool enabled);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void updateDenoiseButtonAppearance();

    OnImportClicked onImportClicked_;
    OnCleanupToggled onCleanupToggled_;

    juce::Label cardTitleLabel_;
    juce::TextButton importButton_{"+ Import Track"};
    juce::TextButton denoiseButton_{"⚡ Denoise: ON"};

    juce::Label filenameLabel_;
    juce::Label bpmBadgeLabel_;
    juce::Label keyBadgeLabel_;
    juce::Label durationBadgeLabel_;
    juce::Label vocalBadgeLabel_;

    bool isCleanupEnabled_ = true;
    bool hasTrack_ = false;
    std::string currentFilename_{"No track imported — click '+ Import Track' to begin"};
};

} // namespace reggaewave::ui

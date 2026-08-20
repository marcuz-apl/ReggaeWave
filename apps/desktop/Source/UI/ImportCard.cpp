#include "ImportCard.h"
#include "ReggaeWaveTheme.h"
#include <iomanip>
#include <sstream>

namespace reggaewave::ui {

ImportCard::ImportCard(OnImportClicked onImportClicked)
    : onImportClicked_(std::move(onImportClicked))
{
    cardTitleLabel_.setText("1. Audio Intake & Analysis", juce::dontSendNotification);
    cardTitleLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    cardTitleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(cardTitleLabel_);

    importButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGold);
    importButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    importButton_.onClick = [this]() {
        if (onImportClicked_) onImportClicked_();
    };
    addAndMakeVisible(importButton_);

    filenameLabel_.setText(currentFilename_, juce::dontSendNotification);
    filenameLabel_.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    filenameLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    addAndMakeVisible(filenameLabel_);

    // Badges
    auto setupBadge = [this](juce::Label& label, const juce::String& text, juce::Colour col) {
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::FontOptions(11.5f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, col);
        label.setJustificationType(juce::Justification::centred);
        label.setVisible(false);
        addAndMakeVisible(label);
    };

    setupBadge(bpmBadgeLabel_, "-- BPM", ReggaeWaveTheme::accentGreen);
    setupBadge(keyBadgeLabel_, "-- Key", ReggaeWaveTheme::accentGold);
    setupBadge(durationBadgeLabel_, "--:--", ReggaeWaveTheme::textPrimary);
    setupBadge(vocalBadgeLabel_, "Lead Vocal Extracted", ReggaeWaveTheme::accentGreen);
}

void ImportCard::setTrackInfo(const std::string& filename, const contracts::MusicalAnalysisManifest& manifest, double durationSeconds) {
    hasTrack_ = true;
    currentFilename_ = filename;
    filenameLabel_.setText(filename, juce::dontSendNotification);
    filenameLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textPrimary);

    // BPM Badge
    int bpm = static_cast<int>(std::round(manifest.bpm));
    bpmBadgeLabel_.setText(juce::String(bpm) + " BPM", juce::dontSendNotification);
    bpmBadgeLabel_.setVisible(true);

    // Key Badge
    keyBadgeLabel_.setText(juce::String(manifest.key.empty() ? "C Major" : manifest.key), juce::dontSendNotification);
    keyBadgeLabel_.setVisible(true);

    // Duration Badge
    int mins = static_cast<int>(durationSeconds) / 60;
    int secs = static_cast<int>(durationSeconds) % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << mins << ":" << std::setw(2) << secs;
    durationBadgeLabel_.setText(juce::String(oss.str()), juce::dontSendNotification);
    durationBadgeLabel_.setVisible(true);

    // Vocal Badge
    vocalBadgeLabel_.setVisible(true);

    resized();
    repaint();
}

void ImportCard::setImportStatus(const std::string& statusText, bool isError) {
    filenameLabel_.setText(statusText, juce::dontSendNotification);
    filenameLabel_.setColour(juce::Label::textColourId, isError ? ReggaeWaveTheme::accentRed : ReggaeWaveTheme::accentGold);
}

void ImportCard::reset() {
    hasTrack_ = false;
    currentFilename_ = "No track imported — click '+ Import Track' to begin";
    filenameLabel_.setText(currentFilename_, juce::dontSendNotification);
    filenameLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    bpmBadgeLabel_.setVisible(false);
    keyBadgeLabel_.setVisible(false);
    durationBadgeLabel_.setVisible(false);
    vocalBadgeLabel_.setVisible(false);
    repaint();
}

void ImportCard::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Card background
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(bounds, 10.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(bounds, 10.0f, 1.2f);

    // Badges pill backgrounds
    if (hasTrack_) {
        auto drawPill = [&g](const juce::Component& c, juce::Colour bg) {
            if (c.isVisible()) {
                g.setColour(bg);
                g.fillRoundedRectangle(c.getBounds().toFloat(), c.getHeight() * 0.5f);
                g.setColour(ReggaeWaveTheme::bgElevated);
                g.drawRoundedRectangle(c.getBounds().toFloat(), c.getHeight() * 0.5f, 1.0f);
            }
        };

        drawPill(bpmBadgeLabel_, ReggaeWaveTheme::bgDark);
        drawPill(keyBadgeLabel_, ReggaeWaveTheme::bgDark);
        drawPill(durationBadgeLabel_, ReggaeWaveTheme::bgDark);
        drawPill(vocalBadgeLabel_, ReggaeWaveTheme::bgDark);
    }
}

void ImportCard::resized() {
    auto area = getLocalBounds().reduced(14);

    // Top Header line
    auto topRow = area.removeFromTop(20);
    cardTitleLabel_.setBounds(topRow);

    area.removeFromTop(6);

    // Main interactive line: Button on left, filename + badges on right
    auto contentRow = area.removeFromTop(38);
    importButton_.setBounds(contentRow.removeFromLeft(140));
    contentRow.removeFromLeft(14);

    if (hasTrack_) {
        // Badges on right side
        vocalBadgeLabel_.setBounds(contentRow.removeFromRight(150).reduced(0, 5));
        contentRow.removeFromRight(8);
        durationBadgeLabel_.setBounds(contentRow.removeFromRight(65).reduced(0, 5));
        contentRow.removeFromRight(8);
        keyBadgeLabel_.setBounds(contentRow.removeFromRight(85).reduced(0, 5));
        contentRow.removeFromRight(8);
        bpmBadgeLabel_.setBounds(contentRow.removeFromRight(75).reduced(0, 5));
        contentRow.removeFromRight(12);
    }

    filenameLabel_.setBounds(contentRow);
}

} // namespace reggaewave::ui

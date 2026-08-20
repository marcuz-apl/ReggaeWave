#include "ExportDeckCard.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

ExportDeckCard::ExportDeckCard(OnExportTriggered onExportTriggered)
    : onExportTriggered_(std::move(onExportTriggered))
{
    cardTitleLabel_.setText("3. Mastering & Export Engine", juce::dontSendNotification);
    cardTitleLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    cardTitleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(cardTitleLabel_);

    specLabel_.setText("Specs: -14.0 LUFS Integrated | -1.0 dBTP True Peak Ceiling | 44.1 kHz / 24-bit PCM | ID3v2 Tags", juce::dontSendNotification);
    specLabel_.setFont(juce::FontOptions(12.0f));
    specLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    addAndMakeVisible(specLabel_);

    subtitleToggle_.setToggleState(false, juce::dontSendNotification);
    subtitleToggle_.setColour(juce::ToggleButton::textColourId, ReggaeWaveTheme::textPrimary);
    subtitleToggle_.setColour(juce::ToggleButton::tickColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(subtitleToggle_);

    exportMp3Button_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGreen);
    exportMp3Button_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    exportMp3Button_.onClick = [this]() {
        if (onExportTriggered_) onExportTriggered_(audio::AudioExportFormat::Mp3_320Kbps, subtitleToggle_.getToggleState());
    };
    addAndMakeVisible(exportMp3Button_);

    exportWavButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    exportWavButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::textPrimary);
    exportWavButton_.onClick = [this]() {
        if (onExportTriggered_) onExportTriggered_(audio::AudioExportFormat::Wav24Bit, subtitleToggle_.getToggleState());
    };
    addAndMakeVisible(exportWavButton_);
}

void ExportDeckCard::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Card background
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(bounds, 10.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(bounds, 10.0f, 1.2f);
}

void ExportDeckCard::resized() {
    auto area = getLocalBounds().reduced(14);

    // Header line
    cardTitleLabel_.setBounds(area.removeFromTop(20));
    area.removeFromTop(4);

    auto contentRow = area.removeFromTop(42);

    // Buttons on right
    exportMp3Button_.setBounds(contentRow.removeFromRight(150));
    contentRow.removeFromRight(10);
    exportWavButton_.setBounds(contentRow.removeFromRight(150));
    contentRow.removeFromRight(16);

    // Specs + Subtitle toggle on left
    specLabel_.setBounds(contentRow.removeFromTop(18));
    subtitleToggle_.setBounds(contentRow.removeFromTop(22));
}

} // namespace reggaewave::ui

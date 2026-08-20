#include "InfoDialogModal.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

InfoDialogModal::InfoDialogModal(OnClose onClose)
    : onClose_(std::move(onClose))
{
    setAlwaysOnTop(true);

    titleLabel_.setText("About ReggaeWave Studio", juce::dontSendNotification);
    titleLabel_.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(titleLabel_);

#ifdef REGGAEWAVE_APP_VERSION_STRING
    versionLabel_.setText("Version " REGGAEWAVE_APP_VERSION_STRING " | Jamaican Living Heritage Engine", juce::dontSendNotification);
#else
    versionLabel_.setText("Version 1.2.5 | Jamaican Living Heritage Engine", juce::dontSendNotification);
#endif
    versionLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    versionLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGreen);
    addAndMakeVisible(versionLabel_);

    juce::String content = 
        "- Transformation Engine: Authentic Jamaican One-Drop and Steppers riddim synthesis.\n"
        "- Cultural Safeguards: Preserves separated lead vocals without voice cloning or caricature.\n"
        "- Audio Fidelity: High-resolution 44.1 kHz / 24-bit PCM internal processing.\n"
        "- Mastering Chain: Broadcast-compliant -14.0 LUFS integrated loudness and -1.0 dBTP ceiling.\n"
        "- Export Profiles: 320 kbps CBR MP3 with ID3 headers and uncompressed 24-bit WAV.";

    infoContentLabel_.setText(content, juce::dontSendNotification);
    infoContentLabel_.setFont(juce::FontOptions(13.0f));
    infoContentLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    infoContentLabel_.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(infoContentLabel_);

    closeButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGold);
    closeButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    closeButton_.onClick = [this]() {
        if (onClose_) onClose_();
    };
    addAndMakeVisible(closeButton_);
}

void InfoDialogModal::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black.withAlpha(0.75f));

    auto cardArea = getLocalBounds().withSizeKeepingCentre(580, 320).toFloat();
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(cardArea, 14.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(cardArea, 14.0f, 1.5f);
}

void InfoDialogModal::resized() {
    auto cardArea = getLocalBounds().withSizeKeepingCentre(580, 320).reduced(24);

    titleLabel_.setBounds(cardArea.removeFromTop(32));
    versionLabel_.setBounds(cardArea.removeFromTop(22));
    cardArea.removeFromTop(14);

    infoContentLabel_.setBounds(cardArea.removeFromTop(140));

    closeButton_.setBounds(cardArea.removeFromBottom(38).withSizeKeepingCentre(130, 38));
}

} // namespace reggaewave::ui

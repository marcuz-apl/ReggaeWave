#include "AboutDialogModal.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

AboutDialogModal::AboutDialogModal(OnClose onClose)
    : onClose_(std::move(onClose))
{
    setAlwaysOnTop(true);

    titleLabel_.setText("About ReggaeWave", juce::dontSendNotification);
    titleLabel_.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(titleLabel_);

#if defined(REGGAEWAVE_APP_VERSION_STRING)
    versionLabel_.setText("v" REGGAEWAVE_APP_VERSION_STRING " | Jamaican Living Heritage Engine", juce::dontSendNotification);
#else
    versionLabel_.setText("v1.3.0-2608213 | Jamaican Living Heritage Engine", juce::dontSendNotification);
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

    copyrightLabel_.setText(juce::String::fromUTF8("© 2026 Alfazen Inc. All rights reserved."), juce::dontSendNotification);
    copyrightLabel_.setFont(juce::FontOptions(11.5f));
    copyrightLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary.withAlpha(0.7f));
    copyrightLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(copyrightLabel_);

    closeButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGold);
    closeButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    closeButton_.onClick = [this]() {
        if (onClose_) onClose_();
    };
    addAndMakeVisible(closeButton_);
}

void AboutDialogModal::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black.withAlpha(0.75f));

    auto cardArea = getLocalBounds().withSizeKeepingCentre(580, 345).toFloat();
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(cardArea, 14.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(cardArea, 14.0f, 1.5f);
}

void AboutDialogModal::resized() {
    auto cardArea = getLocalBounds().withSizeKeepingCentre(580, 345).reduced(24);

    titleLabel_.setBounds(cardArea.removeFromTop(32));
    versionLabel_.setBounds(cardArea.removeFromTop(22));
    cardArea.removeFromTop(12);

    infoContentLabel_.setBounds(cardArea.removeFromTop(140));

    copyrightLabel_.setBounds(cardArea.removeFromBottom(20));
    cardArea.removeFromBottom(8);

    closeButton_.setBounds(cardArea.removeFromBottom(36).withSizeKeepingCentre(130, 36));
}

} // namespace reggaewave::ui

#include "RightsAttestationModal.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

RightsAttestationModal::RightsAttestationModal(OnConfirmedCallback onConfirmed)
    : onConfirmed_(std::move(onConfirmed))
{
    setAlwaysOnTop(true);

    titleLabel_.setText("Music Rights Attestation", juce::dontSendNotification);
    titleLabel_.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(titleLabel_);

    subtitleLabel_.setText("ReggaeWave requires an authorized basis before any audio transformation begins.", juce::dontSendNotification);
    subtitleLabel_.setFont(juce::FontOptions(13.5f));
    subtitleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    addAndMakeVisible(subtitleLabel_);

    const int radioGroupId = 1001;
    ownedOption_.setRadioGroupId(radioGroupId);
    licensedOption_.setRadioGroupId(radioGroupId);
    publicDomainOption_.setRadioGroupId(radioGroupId);

    ownedOption_.onClick = [this]() { selectedBasis_ = contracts::RightsBasis::Owned; updateButtonState(); };
    licensedOption_.onClick = [this]() { selectedBasis_ = contracts::RightsBasis::Licensed; updateButtonState(); };
    publicDomainOption_.onClick = [this]() { selectedBasis_ = contracts::RightsBasis::PublicDomain; updateButtonState(); };
    confirmCheckbox_.onClick = [this]() { updateButtonState(); };

    addAndMakeVisible(ownedOption_);
    addAndMakeVisible(licensedOption_);
    addAndMakeVisible(publicDomainOption_);
    addAndMakeVisible(confirmCheckbox_);

    confirmButton_.setEnabled(false);
    confirmButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGold);
    confirmButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    confirmButton_.onClick = [this]() {
        if (selectedBasis_.has_value() && confirmCheckbox_.getToggleState()) {
            if (onConfirmed_) {
                onConfirmed_(*selectedBasis_);
            }
        }
    };
    addAndMakeVisible(confirmButton_);
}

void RightsAttestationModal::updateButtonState() {
    bool canConfirm = selectedBasis_.has_value() && confirmCheckbox_.getToggleState();
    confirmButton_.setEnabled(canConfirm);
}

void RightsAttestationModal::paint(juce::Graphics& g) {
    // Dim background overlay
    g.fillAll(juce::Colours::black.withAlpha(0.75f));

    // Centered Dialog Card
    auto cardArea = getLocalBounds().withSizeKeepingCentre(620, 360).toFloat();
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(cardArea, 14.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(cardArea, 14.0f, 1.5f);
}

void RightsAttestationModal::resized() {
    auto cardArea = getLocalBounds().withSizeKeepingCentre(620, 360).reduced(24);

    titleLabel_.setBounds(cardArea.removeFromTop(32));
    subtitleLabel_.setBounds(cardArea.removeFromTop(24));
    cardArea.removeFromTop(16);

    ownedOption_.setBounds(cardArea.removeFromTop(28));
    cardArea.removeFromTop(4);
    licensedOption_.setBounds(cardArea.removeFromTop(28));
    cardArea.removeFromTop(4);
    publicDomainOption_.setBounds(cardArea.removeFromTop(28));
    cardArea.removeFromTop(16);

    confirmCheckbox_.setBounds(cardArea.removeFromTop(28));
    cardArea.removeFromTop(20);

    confirmButton_.setBounds(cardArea.removeFromTop(40).withSizeKeepingCentre(220, 40));
}

} // namespace reggaewave::ui

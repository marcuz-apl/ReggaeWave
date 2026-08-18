#include "RightsAttestationModal.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

RightsAttestationModal::RightsAttestationModal(OnConfirmedCallback onConfirmed)
    : onConfirmed_(std::move(onConfirmed))
{
    titleLabel_.setText("Music Rights Attestation", juce::dontSendNotification);
    titleLabel_.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textPrimary);
    addAndMakeVisible(titleLabel_);

    subtitleLabel_.setText("ReggaeWave requires an authorized basis before any audio transformation begins.", juce::dontSendNotification);
    subtitleLabel_.setFont(juce::FontOptions(14.0f));
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
    g.fillAll(ReggaeWaveTheme::bgDark.withAlpha(0.9f));

    auto bounds = getLocalBounds().toFloat().reduced(20.0f);
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(bounds, 12.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(bounds, 12.0f, 1.5f);
}

void RightsAttestationModal::resized() {
    auto area = getLocalBounds().reduced(40);

    titleLabel_.setBounds(area.removeFromTop(32));
    subtitleLabel_.setBounds(area.removeFromTop(24));
    area.removeFromTop(20);

    ownedOption_.setBounds(area.removeFromTop(30));
    area.removeFromTop(6);
    licensedOption_.setBounds(area.removeFromTop(30));
    area.removeFromTop(6);
    publicDomainOption_.setBounds(area.removeFromTop(30));
    area.removeFromTop(20);

    confirmCheckbox_.setBounds(area.removeFromTop(30));
    area.removeFromTop(24);

    confirmButton_.setBounds(area.removeFromTop(44).reduced(60, 0));
}

} // namespace reggaewave::ui

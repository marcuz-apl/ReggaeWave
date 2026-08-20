#include "StudioPlaybackCard.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

StudioPlaybackCard::StudioPlaybackCard(OnPlayToggled onPlay,
                                       OnRewindClicked onRewind,
                                       OnVariationChanged onVarChanged,
                                       OnTuningChanged onTuningChanged,
                                       OnPlayheadSeek onSeek)
    : onPlay_(std::move(onPlay))
    , onRewind_(std::move(onRewind))
    , tuningPanel_(std::move(onTuningChanged))
    , waveformView_(std::move(onSeek))
{
    cardTitleLabel_.setText("2. Riddim & Dub Studio (A/B Variations & DSP)", juce::dontSendNotification);
    cardTitleLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    cardTitleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(cardTitleLabel_);

    // Transport buttons with clean text (no messy Unicode)
    playButton_.setButtonText("Play");
    playButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGold);
    playButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    playButton_.onClick = [this]() { if (onPlay_) onPlay_(); };
    addAndMakeVisible(playButton_);

    rewindButton_.setButtonText("Rewind");
    rewindButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    rewindButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::textPrimary);
    rewindButton_.onClick = [this]() { if (onRewind_) onRewind_(); };
    addAndMakeVisible(rewindButton_);

    // Single Authoritative Variation Switchers
    varAButton_.setButtonText("Var A: One-Drop");
    varAButton_.setClickingTogglesState(true);
    varAButton_.setRadioGroupId(3001);
    varAButton_.setToggleState(true, juce::dontSendNotification);
    varAButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    varAButton_.setColour(juce::TextButton::buttonOnColourId, ReggaeWaveTheme::accentGold);
    varAButton_.onClick = [this, onVarChanged]() {
        if (onVarChanged) onVarChanged(audio::ActiveVariation::VariationA);
        waveformView_.setActiveVariation(audio::ActiveVariation::VariationA);
    };
    addAndMakeVisible(varAButton_);

    varBButton_.setButtonText("Var B: Steppers");
    varBButton_.setClickingTogglesState(true);
    varBButton_.setRadioGroupId(3001);
    varBButton_.setToggleState(false, juce::dontSendNotification);
    varBButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    varBButton_.setColour(juce::TextButton::buttonOnColourId, ReggaeWaveTheme::accentGreen);
    varBButton_.onClick = [this, onVarChanged]() {
        if (onVarChanged) onVarChanged(audio::ActiveVariation::VariationB);
        waveformView_.setActiveVariation(audio::ActiveVariation::VariationB);
    };
    addAndMakeVisible(varBButton_);

    addAndMakeVisible(tuningPanel_);
    addAndMakeVisible(waveformView_);
}

void StudioPlaybackCard::setIsPlaying(bool isPlaying) {
    playButton_.setButtonText(isPlaying ? "Pause" : "Play");
    waveformView_.setIsPlaying(isPlaying);
}

void StudioPlaybackCard::setPlaybackProgress(double progress0To1) {
    waveformView_.setPlaybackProgress(progress0To1);
}

void StudioPlaybackCard::setWaveformData(std::vector<float> peaks) {
    waveformView_.setWaveformData(std::move(peaks));
}

void StudioPlaybackCard::setActiveVariation(audio::ActiveVariation variation) {
    varAButton_.setToggleState(variation == audio::ActiveVariation::VariationA, juce::dontSendNotification);
    varBButton_.setToggleState(variation == audio::ActiveVariation::VariationB, juce::dontSendNotification);
    waveformView_.setActiveVariation(variation);
}

void StudioPlaybackCard::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Card background
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(bounds, 10.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(bounds, 10.0f, 1.2f);
}

void StudioPlaybackCard::resized() {
    auto area = getLocalBounds().reduced(14);

    // Title line
    cardTitleLabel_.setBounds(area.removeFromTop(20));
    area.removeFromTop(8);

    // Left controls column (width 320) vs Right Visualizer (remaining)
    auto leftCol = area.removeFromLeft(330);
    area.removeFromLeft(16);
    auto rightCol = area;

    // Left Column Layout:
    // Row 1: Transport buttons
    auto transportRow = leftCol.removeFromTop(36);
    playButton_.setBounds(transportRow.removeFromLeft(120));
    transportRow.removeFromLeft(8);
    rewindButton_.setBounds(transportRow.removeFromLeft(90));

    leftCol.removeFromTop(8);

    // Row 2: Variation Switchers
    auto varRow = leftCol.removeFromTop(30);
    varAButton_.setBounds(varRow.removeFromLeft(155));
    varRow.removeFromLeft(8);
    varBButton_.setBounds(varRow);

    leftCol.removeFromTop(10);

    // Row 3: Rotary Dials
    tuningPanel_.setBounds(leftCol);

    // Right Column Layout: Visualizer
    waveformView_.setBounds(rightCol);
}

} // namespace reggaewave::ui

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
    cardTitleLabel_.setText("2. Riddim & Dub Studio (3-Way A/B/Original Audition & DSP)", juce::dontSendNotification);
    cardTitleLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    cardTitleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(cardTitleLabel_);

    // Transport buttons
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

    // 3-Way Authoritative Switchers (Original vs Variation A vs Variation B)
    origButton_.setButtonText("Original");
    origButton_.setClickingTogglesState(true);
    origButton_.setRadioGroupId(3001);
    origButton_.setToggleState(false, juce::dontSendNotification);
    origButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    origButton_.setColour(juce::TextButton::buttonOnColourId, ReggaeWaveTheme::textPrimary);
    origButton_.setColour(juce::TextButton::textColourOnId, ReggaeWaveTheme::bgDark);
    origButton_.onClick = [this, onVarChanged]() {
        if (onVarChanged) onVarChanged(audio::ActiveVariation::Original);
        waveformView_.setActiveVariation(audio::ActiveVariation::Original);
    };
    addAndMakeVisible(origButton_);

    varAButton_.setButtonText("Var A: One-Drop");
    varAButton_.setClickingTogglesState(true);
    varAButton_.setRadioGroupId(3001);
    varAButton_.setToggleState(true, juce::dontSendNotification);
    varAButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    varAButton_.setColour(juce::TextButton::buttonOnColourId, ReggaeWaveTheme::accentGold);
    varAButton_.setColour(juce::TextButton::textColourOnId, ReggaeWaveTheme::bgDark);
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
    varBButton_.setColour(juce::TextButton::textColourOnId, ReggaeWaveTheme::bgDark);
    varBButton_.onClick = [this, onVarChanged]() {
        if (onVarChanged) onVarChanged(audio::ActiveVariation::VariationB);
        waveformView_.setActiveVariation(audio::ActiveVariation::VariationB);
    };
    addAndMakeVisible(varBButton_);

    addAndMakeVisible(waveformView_);
    addAndMakeVisible(tuningPanel_);
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
    origButton_.setToggleState(variation == audio::ActiveVariation::Original, juce::dontSendNotification);
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

    // 1. Title line
    cardTitleLabel_.setBounds(area.removeFromTop(20));
    area.removeFromTop(8);

    // 2. Bottom Control Deck (height 94px)
    auto bottomDeck = area.removeFromBottom(94);
    area.removeFromBottom(10);

    // 3. Top Full-Width Panoramic Visualizer (takes all remaining height and width!)
    waveformView_.setBounds(area);

    // 4. Layout Bottom Deck:
    // Left Section: Transport + 3-Way Switchers (width ~390px)
    int leftWidth = std::min(400, bottomDeck.getWidth() / 2);
    auto leftSection = bottomDeck.removeFromLeft(leftWidth);
    bottomDeck.removeFromLeft(14);

    // Left Row 1: Transport buttons (36px height)
    auto transportRow = leftSection.removeFromTop(38);
    playButton_.setBounds(transportRow.removeFromLeft(130));
    transportRow.removeFromLeft(10);
    rewindButton_.setBounds(transportRow.removeFromLeft(100));

    leftSection.removeFromTop(8);

    // Left Row 2: 3-way Switchers (34px height)
    auto varRow = leftSection.removeFromTop(34);
    int btnWidth = (varRow.getWidth() - 12) / 3;
    origButton_.setBounds(varRow.removeFromLeft(btnWidth));
    varRow.removeFromLeft(6);
    varAButton_.setBounds(varRow.removeFromLeft(btnWidth));
    varRow.removeFromLeft(6);
    varBButton_.setBounds(varRow);

    // Right Section: 3 Rotary Dials Panel (spread horizontally across remaining width)
    tuningPanel_.setBounds(bottomDeck);
}

} // namespace reggaewave::ui

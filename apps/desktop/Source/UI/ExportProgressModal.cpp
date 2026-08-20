#include "ExportProgressModal.h"
#include "ReggaeWaveTheme.h"
#include <cmath>
#include <algorithm>

namespace reggaewave::ui {

ExportProgressModal::ExportProgressModal(const std::string& title, const std::string& formatLabel, OnCompleteCallback onComplete)
    : title_(title)
    , formatLabel_(formatLabel)
    , onComplete_(std::move(onComplete))
{
    setAlwaysOnTop(true);

    titleLabel_.setText("Exporting " + juce::String(formatLabel_), juce::dontSendNotification);
    titleLabel_.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    titleLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel_);

    stageLabel_.setText("Step 1/3: Rendering Audio Stems...", juce::dontSendNotification);
    stageLabel_.setFont(juce::FontOptions(13.0f));
    stageLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    stageLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stageLabel_);

    percentLabel_.setText("0%", juce::dontSendNotification);
    percentLabel_.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    percentLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textPrimary);
    percentLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(percentLabel_);

    closeButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGreen);
    closeButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    closeButton_.setVisible(false);
    closeButton_.onClick = [this]() {
        if (onComplete_) onComplete_();
    };
    addAndMakeVisible(closeButton_);

    startTimerHz(60); // 60 FPS animation
}

ExportProgressModal::~ExportProgressModal() {
    stopTimer();
}

void ExportProgressModal::setProgress(float progress0To1, const std::string& stageText) {
    targetProgress_ = std::clamp(progress0To1, 0.0f, 1.0f);
    stageText_ = stageText;
    stageLabel_.setText(juce::String(stageText_), juce::dontSendNotification);
    
    if (targetProgress_ >= 1.0f && !isDone_) {
        isDone_ = true;
        closeButton_.setVisible(true);
        titleLabel_.setText("Export Complete! 🎉", juce::dontSendNotification);
        titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGreen);
    }
}

void ExportProgressModal::timerCallback() {
    // Smooth progress spring animation
    currentProgress_ += (targetProgress_ - currentProgress_) * 0.18f;
    animPhase_ += 0.05f;
    if (animPhase_ > 6.2831853f) animPhase_ -= 6.2831853f;

    int percentInt = static_cast<int>(std::round(currentProgress_ * 100.0f));
    percentLabel_.setText(juce::String(percentInt) + "%", juce::dontSendNotification);

    repaint();
}

void ExportProgressModal::paint(juce::Graphics& g) {
    // Dim background overlay
    g.fillAll(juce::Colours::black.withAlpha(0.75f));

    auto cardArea = getLocalBounds().withSizeKeepingCentre(480, 240).toFloat();

    // Card background with subtle glow
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(cardArea, 14.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(cardArea, 14.0f, 1.5f);

    // Progress Track Area
    auto progressTrack = cardArea.withSizeKeepingCentre(cardArea.getWidth() - 64.0f, 22.0f);
    progressTrack.setY(cardArea.getY() + 115.0f);

    g.setColour(ReggaeWaveTheme::bgDark);
    g.fillRoundedRectangle(progressTrack, 11.0f);

    float fillWidth = progressTrack.getWidth() * std::clamp(currentProgress_, 0.01f, 1.0f);
    auto fillArea = progressTrack.withWidth(fillWidth);

    // Dynamic Color Transition: Green (0%) -> Gold (50%) -> Coral Red (100%)
    juce::Colour startColour;
    juce::Colour endColour;

    if (currentProgress_ < 0.5f) {
        float t = currentProgress_ / 0.5f;
        startColour = ReggaeWaveTheme::accentGreen;
        endColour = ReggaeWaveTheme::accentGreen.interpolatedWith(ReggaeWaveTheme::accentGold, t);
    } else {
        float t = (currentProgress_ - 0.5f) / 0.5f;
        startColour = ReggaeWaveTheme::accentGreen.interpolatedWith(ReggaeWaveTheme::accentGold, t);
        endColour = ReggaeWaveTheme::accentGold.interpolatedWith(ReggaeWaveTheme::accentRed, t);
    }

    juce::ColourGradient grad(startColour, fillArea.getX(), fillArea.getY(),
                              endColour, fillArea.getRight(), fillArea.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(fillArea, 11.0f);

    // Animated shimmering pulse overlay
    float shimmerX = fillArea.getX() + std::fmod(animPhase_ * 80.0f, fillWidth + 40.0f) - 40.0f;
    g.setColour(juce::Colours::white.withAlpha(0.25f));
    g.fillRoundedRectangle(fillArea.reduced(2.0f), 9.0f);

    // Outer glow for progress bar
    g.setColour(endColour.withAlpha(0.4f));
    g.drawRoundedRectangle(fillArea, 11.0f, 2.0f);
}

void ExportProgressModal::resized() {
    auto cardArea = getLocalBounds().withSizeKeepingCentre(480, 240);

    titleLabel_.setBounds(cardArea.removeFromTop(50).reduced(10, 0));
    stageLabel_.setBounds(cardArea.removeFromTop(30));
    cardArea.removeFromTop(35); // Progress bar gap

    percentLabel_.setBounds(cardArea.removeFromTop(30));

    closeButton_.setBounds(cardArea.removeFromBottom(40).withSizeKeepingCentre(140, 36));
}

} // namespace reggaewave::ui

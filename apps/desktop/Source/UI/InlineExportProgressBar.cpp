#include "InlineExportProgressBar.h"
#include "ReggaeWaveTheme.h"
#include <cmath>
#include <algorithm>

namespace reggaewave::ui {

InlineExportProgressBar::InlineExportProgressBar() {
    startTimerHz(60);
}

InlineExportProgressBar::~InlineExportProgressBar() {
    stopTimer();
}

void InlineExportProgressBar::setProgress(float progress0To1, const std::string& stageText) {
    targetProgress_ = std::clamp(progress0To1, 0.0f, 1.0f);
    stageText_ = stageText;
    if (targetProgress_ >= 1.0f) {
        isComplete_ = true;
    }
}

void InlineExportProgressBar::reset() {
    currentProgress_ = 0.0f;
    targetProgress_ = 0.0f;
    isComplete_ = false;
    stageText_ = "Ready";
    repaint();
}

void InlineExportProgressBar::timerCallback() {
    if (isVisible()) {
        currentProgress_ += (targetProgress_ - currentProgress_) * 0.2f;
        animPhase_ += 0.06f;
        if (animPhase_ > 6.2831853f) animPhase_ -= 6.2831853f;
        repaint();
    }
}

void InlineExportProgressBar::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Track background
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(bounds, bounds.getHeight() * 0.5f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(bounds, bounds.getHeight() * 0.5f, 1.2f);

    float fillWidth = bounds.getWidth() * std::clamp(currentProgress_, 0.02f, 1.0f);
    auto fillArea = bounds.withWidth(fillWidth);

    // Color transition: Green (0%) -> Gold (50%) -> Coral Red (100%)
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
    g.fillRoundedRectangle(fillArea, bounds.getHeight() * 0.5f);

    // Shimmer pulse overlay
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.fillRoundedRectangle(fillArea.reduced(2.0f), bounds.getHeight() * 0.5f - 2.0f);

    // Centered status text & percentage
    int percentInt = static_cast<int>(std::round(currentProgress_ * 100.0f));
    juce::String displayString = isComplete_
        ? "Export Complete! (100%)"
        : juce::String(stageText_) + " (" + juce::String(percentInt) + "%)";

    g.setColour(ReggaeWaveTheme::textPrimary);
    g.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    g.drawText(displayString, bounds, juce::Justification::centred, true);
}

} // namespace reggaewave::ui

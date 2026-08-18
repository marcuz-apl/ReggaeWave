#include "WaveformABView.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

WaveformABView::WaveformABView(OnVariationChanged onVarChanged, OnPlayheadSeek onSeek)
    : onVarChanged_(std::move(onVarChanged))
    , onSeek_(std::move(onSeek))
{
    varAButton_.setClickingTogglesState(true);
    varAButton_.setRadioGroupId(2002);
    varAButton_.setToggleState(true, juce::dontSendNotification);
    varAButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    varAButton_.setColour(juce::TextButton::buttonOnColourId, ReggaeWaveTheme::accentGold);
    varAButton_.onClick = [this]() {
        activeVariation_ = audio::ActiveVariation::VariationA;
        if (onVarChanged_) onVarChanged_(activeVariation_);
        repaint();
    };
    addAndMakeVisible(varAButton_);

    varBButton_.setClickingTogglesState(true);
    varBButton_.setRadioGroupId(2002);
    varBButton_.setToggleState(false, juce::dontSendNotification);
    varBButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    varBButton_.setColour(juce::TextButton::buttonOnColourId, ReggaeWaveTheme::accentGreen);
    varBButton_.onClick = [this]() {
        activeVariation_ = audio::ActiveVariation::VariationB;
        if (onVarChanged_) onVarChanged_(activeVariation_);
        repaint();
    };
    addAndMakeVisible(varBButton_);

    // Generate dummy preview peaks for visualization
    waveformPeaks_.resize(120);
    for (size_t i = 0; i < waveformPeaks_.size(); ++i) {
        waveformPeaks_[i] = 0.2f + 0.7f * std::sin(i * 0.15f) * std::sin(i * 0.15f);
    }
}

void WaveformABView::setPlaybackProgress(double progress0To1) {
    playheadProgress_ = std::clamp(progress0To1, 0.0, 1.0);
    repaint();
}

void WaveformABView::setWaveformData(std::vector<float> peaks) {
    waveformPeaks_ = std::move(peaks);
    repaint();
}

void WaveformABView::setActiveVariation(audio::ActiveVariation variation) {
    activeVariation_ = variation;
    varAButton_.setToggleState(variation == audio::ActiveVariation::VariationA, juce::dontSendNotification);
    varBButton_.setToggleState(variation == audio::ActiveVariation::VariationB, juce::dontSendNotification);
    repaint();
}

void WaveformABView::mouseDown(const juce::MouseEvent& event) {
    auto waveformBounds = getLocalBounds().removeFromTop(getHeight() - 48).toFloat();
    if (waveformBounds.contains(event.position)) {
        double normalized = std::clamp(static_cast<double>(event.position.x - waveformBounds.getX()) / waveformBounds.getWidth(), 0.0, 1.0);
        setPlaybackProgress(normalized);
        if (onSeek_) {
            onSeek_(normalized);
        }
    }
}

void WaveformABView::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto waveformArea = bounds.removeFromTop(getHeight() - 48.0f).reduced(2.0f);

    // Waveform Background
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(waveformArea, 8.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(waveformArea, 8.0f, 1.0f);

    // Center baseline
    float midY = waveformArea.getCentreY();
    g.setColour(ReggaeWaveTheme::bgElevated.brighter(0.1f));
    g.drawHorizontalLine(static_cast<int>(midY), waveformArea.getX(), waveformArea.getRight());

    // Draw peaks
    if (!waveformPeaks_.empty()) {
        const float barWidth = waveformArea.getWidth() / static_cast<float>(waveformPeaks_.size());
        const auto activeColour = (activeVariation_ == audio::ActiveVariation::VariationA) 
            ? ReggaeWaveTheme::accentGold 
            : ReggaeWaveTheme::accentGreen;

        for (size_t i = 0; i < waveformPeaks_.size(); ++i) {
            float x = waveformArea.getX() + i * barWidth;
            float height = waveformPeaks_[i] * (waveformArea.getHeight() * 0.42f);

            bool isPlayed = (x <= waveformArea.getX() + playheadProgress_ * waveformArea.getWidth());
            g.setColour(isPlayed ? activeColour : ReggaeWaveTheme::textSecondary.withAlpha(0.4f));

            g.fillRoundedRectangle(x + 1.0f, midY - height, barWidth - 2.0f, height * 2.0f, 2.0f);
        }
    }

    // Playhead line
    float playheadX = waveformArea.getX() + static_cast<float>(playheadProgress_) * waveformArea.getWidth();
    g.setColour(ReggaeWaveTheme::textPrimary);
    g.drawLine(playheadX, waveformArea.getY(), playheadX, waveformArea.getBottom(), 2.0f);
}

void WaveformABView::resized() {
    auto area = getLocalBounds();
    area.removeFromTop(getHeight() - 44);

    int buttonWidth = 180;
    varAButton_.setBounds(area.removeFromLeft(buttonWidth));
    area.removeFromLeft(12);
    varBButton_.setBounds(area.removeFromLeft(buttonWidth));
}

} // namespace reggaewave::ui

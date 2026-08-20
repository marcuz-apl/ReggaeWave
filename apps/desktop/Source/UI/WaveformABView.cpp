#include "WaveformABView.h"
#include "ReggaeWaveTheme.h"
#include <cmath>
#include <algorithm>

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

    // Generate preview peaks for visualization
    waveformPeaks_.resize(140);
    for (size_t i = 0; i < waveformPeaks_.size(); ++i) {
        waveformPeaks_[i] = 0.25f + 0.65f * std::sin(i * 0.12f) * std::sin(i * 0.12f);
    }

    startTimerHz(60); // 60 FPS animation for rhythmic waving
}

WaveformABView::~WaveformABView() {
    stopTimer();
}

void WaveformABView::setIsPlaying(bool playing) {
    isPlaying_ = playing;
}

void WaveformABView::setAudioEnergyLevel(float energy) {
    audioEnergy_ = std::clamp(energy, 0.2f, 1.0f);
}

void WaveformABView::setPlaybackProgress(double progress0To1) {
    playheadProgress_ = std::clamp(progress0To1, 0.0, 1.0);
}

void WaveformABView::setWaveformData(std::vector<float> peaks) {
    waveformPeaks_ = std::move(peaks);
}

void WaveformABView::setActiveVariation(audio::ActiveVariation variation) {
    activeVariation_ = variation;
    varAButton_.setToggleState(variation == audio::ActiveVariation::VariationA, juce::dontSendNotification);
    varBButton_.setToggleState(variation == audio::ActiveVariation::VariationB, juce::dontSendNotification);
}

void WaveformABView::timerCallback() {
    if (isPlaying_) {
        animPhase_ += 0.08f;
        if (animPhase_ > 6.2831853f) animPhase_ -= 6.2831853f;
        repaint();
    }
}

void WaveformABView::mouseDown(const juce::MouseEvent& event) {
    auto waveformBounds = getLocalBounds().removeFromTop(getHeight() - 48).toFloat();
    if (waveformBounds.contains(event.position)) {
        double normalized = std::clamp(static_cast<double>(event.position.x - waveformBounds.getX()) / waveformBounds.getWidth(), 0.0, 1.0);
        setPlaybackProgress(normalized);
        if (onSeek_) {
            onSeek_(normalized);
        }
        repaint();
    }
}

void WaveformABView::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto waveformArea = bounds.removeFromTop(getHeight() - 48.0f).reduced(2.0f);

    // Waveform Background Canvas
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(waveformArea, 10.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(waveformArea, 10.0f, 1.2f);

    // Center Baseline with subtle glow
    float midY = waveformArea.getCentreY();
    g.setColour(ReggaeWaveTheme::bgElevated.brighter(0.15f));
    g.drawHorizontalLine(static_cast<int>(midY), waveformArea.getX(), waveformArea.getRight());

    // Active theme colors
    const auto activePrimary = (activeVariation_ == audio::ActiveVariation::VariationA) 
                               ? ReggaeWaveTheme::accentGold 
                               : ReggaeWaveTheme::accentGreen;
    const auto activeSecondary = (activeVariation_ == audio::ActiveVariation::VariationA) 
                                 ? ReggaeWaveTheme::accentGreen 
                                 : ReggaeWaveTheme::accentGold;

    float playheadX = waveformArea.getX() + static_cast<float>(playheadProgress_) * waveformArea.getWidth();

    // Draw Dynamic Waving Bars
    if (!waveformPeaks_.empty()) {
        const float barWidth = waveformArea.getWidth() / static_cast<float>(waveformPeaks_.size());

        for (size_t i = 0; i < waveformPeaks_.size(); ++i) {
            float x = waveformArea.getX() + i * barWidth;
            float basePeak = waveformPeaks_[i];

            // Rhythmic dynamic wave harmonic modulation
            float waveRipple = 0.0f;
            if (isPlaying_) {
                // Distance to playhead influences ripple intensity
                float distToPlayhead = std::abs(x - playheadX) / waveformArea.getWidth();
                float proximityWeight = std::exp(-distToPlayhead * 4.0f);
                float rippleSin = std::sin(animPhase_ * 3.0f + i * 0.22f);
                waveRipple = rippleSin * 0.22f * audioEnergy_ * (0.4f + 0.6f * proximityWeight);
            }

            float modHeight = std::clamp(basePeak + waveRipple, 0.08f, 1.0f) * (waveformArea.getHeight() * 0.44f);
            bool isPlayed = (x <= playheadX);

            // Bar Gradient
            juce::Colour barTop = isPlayed ? activePrimary : ReggaeWaveTheme::textSecondary.withAlpha(0.35f);
            juce::Colour barBottom = isPlayed ? activeSecondary.withAlpha(0.6f) : ReggaeWaveTheme::bgDark.brighter(0.1f);

            juce::ColourGradient barGrad(barTop, x, midY - modHeight,
                                        barBottom, x, midY + modHeight, false);
            g.setGradientFill(barGrad);
            g.fillRoundedRectangle(x + 1.0f, midY - modHeight, barWidth - 1.5f, modHeight * 2.0f, 2.0f);

            // Dancing Glowing Particles / Beads on peak envelope when playing
            if (isPlaying_ && isPlayed && (i % 3 == 0)) {
                float beadRadius = 2.0f + 1.5f * std::sin(animPhase_ * 4.0f + i);
                g.setColour(juce::Colours::white.withAlpha(0.85f));
                g.fillEllipse(x + barWidth * 0.5f - beadRadius, midY - modHeight - beadRadius, beadRadius * 2.0f, beadRadius * 2.0f);
            }
        }
    }

    // Glowing Playhead Beam
    if (waveformArea.getWidth() > 0.0f) {
        // Soft outer playhead glow
        g.setColour(activePrimary.withAlpha(0.3f));
        g.fillRect(playheadX - 3.0f, waveformArea.getY(), 7.0f, waveformArea.getHeight());

        // Solid playhead center beam
        g.setColour(juce::Colours::white);
        g.drawLine(playheadX, waveformArea.getY(), playheadX, waveformArea.getBottom(), 2.0f);

        // Top and bottom glowing indicators
        g.setColour(activePrimary);
        g.fillEllipse(playheadX - 4.0f, waveformArea.getY() + 1.0f, 8.0f, 8.0f);
        g.fillEllipse(playheadX - 4.0f, waveformArea.getBottom() - 9.0f, 8.0f, 8.0f);
    }
}

void WaveformABView::resized() {
    auto area = getLocalBounds();
    area.removeFromTop(getHeight() - 44);

    int buttonWidth = 240;
    varAButton_.setBounds(area.removeFromLeft(buttonWidth));
    area.removeFromLeft(12);
    varBButton_.setBounds(area.removeFromLeft(buttonWidth));
}

} // namespace reggaewave::ui

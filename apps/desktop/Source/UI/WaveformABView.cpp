#include "WaveformABView.h"
#include "ReggaeWaveTheme.h"
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace reggaewave::ui {

namespace {

/**
 * @brief Interpolates RGB color from Green -> Yellow -> Red based on peak height (0.0 to 1.0).
 */
juce::Colour getRgbHeightColour(float normHeight, bool isPlayed) {
    normHeight = std::clamp(normHeight, 0.0f, 1.0f);
    uint8_t r, g, b;

    if (normHeight < 0.50f) {
        // Green (46, 204, 113) -> Yellow/Gold (241, 196, 15)
        float t = normHeight / 0.50f;
        r = static_cast<uint8_t>(46.0f + t * (241.0f - 46.0f));
        g = static_cast<uint8_t>(204.0f + t * (196.0f - 204.0f));
        b = static_cast<uint8_t>(113.0f + t * (15.0f - 113.0f));
    } else {
        // Yellow/Gold (241, 196, 15) -> Fiery Red (231, 76, 60)
        float t = (normHeight - 0.50f) / 0.50f;
        r = static_cast<uint8_t>(241.0f + t * (231.0f - 241.0f));
        g = static_cast<uint8_t>(196.0f + t * (60.0f - 196.0f));
        b = static_cast<uint8_t>(15.0f + t * (45.0f - 15.0f));
    }

    auto col = juce::Colour(r, g, b);
    return isPlayed ? col : col.withAlpha(0.35f);
}

} // anonymous namespace

WaveformABView::WaveformABView(OnPlayheadSeek onSeek)
    : onSeek_(std::move(onSeek))
{
    // Initial preview peaks with natural dynamic contour
    waveformPeaks_.resize(120);
    for (size_t i = 0; i < waveformPeaks_.size(); ++i) {
        float s = std::sin(i * 0.14f);
        waveformPeaks_[i] = 0.08f + 0.85f * (s * s);
    }

    // Interactive Scrubber Slider
    scrubSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    scrubSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    scrubSlider_.setRange(0.0, 1.0, 0.001);
    scrubSlider_.setValue(0.0, juce::dontSendNotification);
    scrubSlider_.setColour(juce::Slider::trackColourId, ReggaeWaveTheme::accentGold);
    scrubSlider_.setColour(juce::Slider::backgroundColourId, ReggaeWaveTheme::bgDark);
    scrubSlider_.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    
    scrubSlider_.onValueChange = [this]() {
        if (isUserScrubbing_ && onSeek_) {
            onSeek_(scrubSlider_.getValue());
        }
    };
    scrubSlider_.onDragStart = [this]() { isUserScrubbing_ = true; };
    scrubSlider_.onDragEnd = [this]() {
        isUserScrubbing_ = false;
        if (onSeek_) onSeek_(scrubSlider_.getValue());
    };
    addAndMakeVisible(scrubSlider_);

    timecodeLabel_.setText("00:00 / 00:00", juce::dontSendNotification);
    timecodeLabel_.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    timecodeLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    timecodeLabel_.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(timecodeLabel_);

    startTimerHz(30); // 30 FPS smooth rendering
}

WaveformABView::~WaveformABView() {
    stopTimer();
}

void WaveformABView::setIsPlaying(bool playing) {
    isPlaying_ = playing;
    repaint();
}

void WaveformABView::setAudioEnergyLevel(float energy) {
    audioEnergy_ = std::clamp(energy, 0.3f, 1.0f);
}

void WaveformABView::setDurationSeconds(double totalSeconds) {
    totalDurationSeconds_ = totalSeconds;
    setPlaybackProgress(playheadProgress_);
}

void WaveformABView::setPlaybackProgress(double progress0To1) {
    playheadProgress_ = std::clamp(progress0To1, 0.0, 1.0);
    if (!isUserScrubbing_) {
        scrubSlider_.setValue(playheadProgress_, juce::dontSendNotification);
    }

    int currentSecs = static_cast<int>(playheadProgress_ * totalDurationSeconds_);
    int totalSecs = static_cast<int>(totalDurationSeconds_);
    
    int cMin = currentSecs / 60;
    int cSec = currentSecs % 60;
    int tMin = totalSecs / 60;
    int tSec = totalSecs % 60;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << cMin << ":" << std::setw(2) << cSec
        << " / " << std::setw(2) << tMin << ":" << std::setw(2) << tSec;
    timecodeLabel_.setText(juce::String(oss.str()), juce::dontSendNotification);
    repaint();
}

void WaveformABView::setWaveformData(std::vector<float> peaks) {
    waveformPeaks_ = std::move(peaks);
    repaint();
}

void WaveformABView::setActiveVariation(audio::ActiveVariation variation) {
    activeVariation_ = variation;
    juce::Colour trackCol = ReggaeWaveTheme::accentGold;
    if (variation == audio::ActiveVariation::Original) {
        trackCol = ReggaeWaveTheme::textPrimary;
    } else if (variation == audio::ActiveVariation::VariationB) {
        trackCol = ReggaeWaveTheme::accentGreen;
    }
    scrubSlider_.setColour(juce::Slider::trackColourId, trackCol);
    repaint();
}

void WaveformABView::timerCallback() {
    if (isPlaying_) {
        animPhase_ += 0.12f;
        if (animPhase_ > 6.2831853f) animPhase_ -= 6.2831853f;
        repaint();
    }
}

void WaveformABView::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto waveformArea = bounds.removeFromTop(getHeight() - 30.0f).reduced(2.0f);

    // 1. Dark background canvas
    g.setColour(ReggaeWaveTheme::bgDark);
    g.fillRoundedRectangle(waveformArea, 8.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(waveformArea, 8.0f, 1.0f);

    // 2. Baseline at the bottom with headroom (peaks scaled smoothly)
    float bottomY = waveformArea.getBottom() - 4.0f;
    float maxAvailableHeight = (waveformArea.getHeight() - 14.0f) * 0.85f;

    g.setColour(ReggaeWaveTheme::bgElevated.brighter(0.20f));
    g.drawHorizontalLine(static_cast<int>(bottomY), waveformArea.getX(), waveformArea.getRight());

    float playheadX = waveformArea.getX() + static_cast<float>(playheadProgress_) * waveformArea.getWidth();

    // 3. Upper-Half Spectrum Bars without artificial trimming/flat-topping
    if (!waveformPeaks_.empty()) {
        const float barWidth = waveformArea.getWidth() / static_cast<float>(waveformPeaks_.size());

        for (size_t i = 0; i < waveformPeaks_.size(); ++i) {
            float x = waveformArea.getX() + i * barWidth;
            float basePeak = waveformPeaks_[i];

            // Dynamic live wave ripple when playing
            float waveRipple = 0.0f;
            if (isPlaying_) {
                float bassHarmonic = std::sin(animPhase_ * 3.2f + i * 0.18f) * 0.18f;
                float skankHarmonic = std::cos(animPhase_ * 6.4f + i * 0.36f) * 0.10f;
                waveRipple = (bassHarmonic + skankHarmonic) * audioEnergy_;
            }

            // Natural dynamic scaling without flat-top ceiling clipping
            float dynamicPeak = basePeak * (1.0f + waveRipple * 0.35f);
            float normHeight = std::clamp(dynamicPeak, 0.03f, 1.0f);
            float barHeight = normHeight * maxAvailableHeight;
            float barY = bottomY - barHeight;

            bool isPlayed = (x <= playheadX);

            // Bottom color is always vibrant Roots Green
            juce::Colour greenBase = isPlayed ? juce::Colour(46, 204, 113) : juce::Colour(46, 204, 113).withAlpha(0.35f);
            
            // Top color transitions to Red as peak reaches higher
            juce::Colour topColor = getRgbHeightColour(normHeight, isPlayed);

            // Vertical linear gradient from bottom (Green) to top (Redder)
            juce::ColourGradient pillarGrad(greenBase, x, bottomY, topColor, x, barY, false);
            g.setGradientFill(pillarGrad);
            g.fillRoundedRectangle(x + 0.8f, barY, barWidth - 1.2f, barHeight, 2.0f);

            // Glowing crest bead on peaks when playing
            if (isPlaying_ && isPlayed && (i % 2 == 0) && (normHeight > 0.40f)) {
                float beadRadius = 2.0f + 1.0f * std::sin(animPhase_ * 4.5f + i * 0.3f);
                g.setColour(juce::Colours::white);
                g.fillEllipse(x + barWidth * 0.5f - beadRadius, barY - beadRadius - 1.0f, beadRadius * 2.0f, beadRadius * 2.0f);
            }
        }
    }

    // 4. Glowing Playhead Beam
    if (waveformArea.getWidth() > 0.0f) {
        g.setColour(ReggaeWaveTheme::accentGold.withAlpha(0.25f));
        g.fillRect(playheadX - 3.0f, waveformArea.getY(), 6.0f, waveformArea.getHeight());

        g.setColour(juce::Colours::white);
        g.drawLine(playheadX, waveformArea.getY(), playheadX, bottomY, 2.0f);

        g.setColour(ReggaeWaveTheme::accentGold);
        g.fillEllipse(playheadX - 4.0f, waveformArea.getY() + 2.0f, 8.0f, 8.0f);
        g.fillEllipse(playheadX - 4.0f, bottomY - 6.0f, 8.0f, 8.0f);
    }
}

void WaveformABView::resized() {
    auto area = getLocalBounds();
    auto bottomRow = area.removeFromBottom(28);

    timecodeLabel_.setBounds(bottomRow.removeFromRight(110));
    bottomRow.removeFromRight(8);
    scrubSlider_.setBounds(bottomRow);
}

} // namespace reggaewave::ui

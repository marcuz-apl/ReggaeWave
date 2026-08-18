#include "ReggaeWaveTheme.h"
#include <cmath>

namespace reggaewave::ui {

const juce::Colour ReggaeWaveTheme::bgDark       = juce::Colour::fromRGB(18, 20, 24);
const juce::Colour ReggaeWaveTheme::bgSurface    = juce::Colour::fromRGB(28, 32, 38);
const juce::Colour ReggaeWaveTheme::bgElevated   = juce::Colour::fromRGB(38, 44, 53);
const juce::Colour ReggaeWaveTheme::accentGold   = juce::Colour::fromRGB(245, 166, 35);
const juce::Colour ReggaeWaveTheme::accentGreen  = juce::Colour::fromRGB(46, 204, 113);
const juce::Colour ReggaeWaveTheme::accentRed    = juce::Colour::fromRGB(231, 76, 60);
const juce::Colour ReggaeWaveTheme::textPrimary  = juce::Colour::fromRGB(240, 243, 246);
const juce::Colour ReggaeWaveTheme::textSecondary= juce::Colour::fromRGB(155, 164, 178);

ReggaeWaveTheme::ReggaeWaveTheme() {
    setColour(juce::ResizableWindow::backgroundColourId, bgDark);
    setColour(juce::Label::textColourId, textPrimary);
    setColour(juce::TextButton::textColourOffId, textPrimary);
    setColour(juce::TextButton::textColourOnId, textPrimary);
}

void ReggaeWaveTheme::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float rotaryStartAngle,
                                       float rotaryEndAngle, juce::Slider& /*slider*/) {
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto centre = bounds.getCentre();
    auto lineW = 6.0f;
    auto arcRadius = radius - lineW * 0.5f;

    // Background track arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(bgElevated);
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active value arc with Gold-to-Green gradient
    if (sliderPos > 0.0f) {
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, toAngle, true);

        juce::ColourGradient grad(accentGold, centre.x - radius, centre.y,
                                  accentGreen, centre.x + radius, centre.y, false);
        g.setGradientFill(grad);
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Inner dial body
    auto innerRadius = radius - lineW - 4.0f;
    g.setColour(bgSurface);
    g.fillEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);
    g.setColour(bgElevated);
    g.drawEllipse(centre.x - innerRadius, centre.y - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f, 1.5f);

    // Indicator thumb dot
    juce::Point<float> thumbPoint(centre.x + (innerRadius - 6.0f) * std::sin(toAngle),
                                  centre.y - (innerRadius - 6.0f) * std::cos(toAngle));
    g.setColour(accentGold);
    g.fillEllipse(thumbPoint.x - 3.5f, thumbPoint.y - 3.5f, 7.0f, 7.0f);
}

void ReggaeWaveTheme::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto baseCol = backgroundColour.isTransparent() ? bgSurface : backgroundColour;

    if (shouldDrawButtonAsDown) {
        baseCol = baseCol.darker(0.2f);
    } else if (shouldDrawButtonAsHighlighted) {
        baseCol = baseCol.brighter(0.15f);
    }

    g.setColour(baseCol);
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(shouldDrawButtonAsHighlighted ? accentGold : bgElevated);
    g.drawRoundedRectangle(bounds, 8.0f, 1.2f);
}

} // namespace reggaewave::ui

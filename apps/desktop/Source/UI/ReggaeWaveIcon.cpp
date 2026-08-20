#include "ReggaeWaveIcon.h"
#include "ReggaeWaveTheme.h"
#include <cmath>

namespace reggaewave::ui {

void ReggaeWaveIcon::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    float size = std::min(bounds.getWidth(), bounds.getHeight());
    auto circleArea = bounds.withSizeKeepingCentre(size, size);
    float centreX = circleArea.getCentreX();
    float centreY = circleArea.getCentreY();
    float radius = size * 0.48f;

    // Outer subtle gold glow
    g.setColour(ReggaeWaveTheme::accentGold.withAlpha(0.2f));
    g.fillEllipse(circleArea);

    // Inner dark vinyl disc
    g.setColour(ReggaeWaveTheme::bgDark);
    g.fillEllipse(circleArea.reduced(2.0f));

    // Concentric soundwave vinyl grooves
    g.setColour(ReggaeWaveTheme::bgElevated.brighter(0.2f));
    g.drawEllipse(circleArea.reduced(size * 0.12f), 1.0f);
    g.setColour(ReggaeWaveTheme::accentGold.withAlpha(0.35f));
    g.drawEllipse(circleArea.reduced(size * 0.22f), 1.2f);

    // Gold gradient rim
    juce::ColourGradient rimGrad(ReggaeWaveTheme::accentGold, circleArea.getX(), circleArea.getY(),
                                ReggaeWaveTheme::accentGold.darker(0.3f), circleArea.getRight(), circleArea.getBottom(), false);
    g.setGradientFill(rimGrad);
    g.drawEllipse(circleArea.reduced(2.0f), 2.0f);

    // Roots green & coral micro-dots on top & bottom of rim
    g.setColour(ReggaeWaveTheme::accentGreen);
    g.fillEllipse(centreX - 2.5f, circleArea.getY() + 4.0f, 5.0f, 5.0f);
    g.setColour(ReggaeWaveTheme::accentRed);
    g.fillEllipse(centreX - 2.5f, circleArea.getBottom() - 9.0f, 5.0f, 5.0f);

    // "RW" Monogram
    g.setColour(ReggaeWaveTheme::accentGold);
    g.setFont(juce::FontOptions(size * 0.38f, juce::Font::bold));
    g.drawText("RW", circleArea, juce::Justification::centred, true);
}

juce::Image ReggaeWaveIcon::createIconImage(int size) {
    juce::Image img(juce::Image::ARGB, size, size, true);
    juce::Graphics g(img);
    ReggaeWaveIcon icon;
    icon.setBounds(0, 0, size, size);
    icon.paint(g);
    return img;
}

} // namespace reggaewave::ui

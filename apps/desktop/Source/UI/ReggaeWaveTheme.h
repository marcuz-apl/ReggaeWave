#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace reggaewave::ui {

/**
 * @brief Custom LookAndFeel for ReggaeWave Desktop.
 * 
 * Design aesthetic:
 * - Ultra-clean dark theme (deep charcoal background #121417)
 * - Accent colors: Reggae Gold (#F5A623), Roots Green (#2ECC71), Coral/Red (#E74C3C)
 * - Custom rotary sliders with glow arcs and numerical readouts
 * - Modern rounded buttons and status badges
 */
class ReggaeWaveTheme : public juce::LookAndFeel_V4 {
public:
    ReggaeWaveTheme();
    ~ReggaeWaveTheme() override = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    // Palette tokens
    static const juce::Colour bgDark;
    static const juce::Colour bgSurface;
    static const juce::Colour bgElevated;
    static const juce::Colour accentGold;
    static const juce::Colour accentGreen;
    static const juce::Colour accentRed;
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;
};

} // namespace reggaewave::ui

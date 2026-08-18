#include "TuningPanel.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

TuningPanel::TuningPanel(OnParametersChangedCallback onParamsChanged)
    : onParamsChanged_(std::move(onParamsChanged))
{
    // 1. Reggae Intensity (0 - 100, default 70)
    intensitySlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    intensitySlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    intensitySlider_.setRange(0.0, 100.0, 1.0);
    intensitySlider_.setValue(contracts::TuningParameters::DEFAULT_REGGAE_INTENSITY);
    intensitySlider_.onValueChange = [this]() { notifyChange(); };
    addAndMakeVisible(intensitySlider_);

    intensityLabel_.setText("Intensity", juce::dontSendNotification);
    intensityLabel_.setJustificationType(juce::Justification::centred);
    intensityLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    intensityLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    addAndMakeVisible(intensityLabel_);

    // 2. Dub-Effects (0 - 100, default 20)
    dubSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dubSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    dubSlider_.setRange(0.0, 100.0, 1.0);
    dubSlider_.setValue(contracts::TuningParameters::DEFAULT_DUB_EFFECTS_AMOUNT);
    dubSlider_.onValueChange = [this]() { notifyChange(); };
    addAndMakeVisible(dubSlider_);

    dubLabel_.setText("Dub FX", juce::dontSendNotification);
    dubLabel_.setJustificationType(juce::Justification::centred);
    dubLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    dubLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    addAndMakeVisible(dubLabel_);

    // 3. Vocal Gain (-6.0 dB to +6.0 dB, default 0.0 dB)
    vocalSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    vocalSlider_.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 20);
    vocalSlider_.setRange(-6.0, 6.0, 0.1);
    vocalSlider_.setValue(contracts::TuningParameters::DEFAULT_VOCAL_LEVEL_DB);
    vocalSlider_.setTextValueSuffix(" dB");
    vocalSlider_.onValueChange = [this]() { notifyChange(); };
    addAndMakeVisible(vocalSlider_);

    vocalLabel_.setText("Vocal Gain", juce::dontSendNotification);
    vocalLabel_.setJustificationType(juce::Justification::centred);
    vocalLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    vocalLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    addAndMakeVisible(vocalLabel_);
}

void TuningPanel::notifyChange() {
    if (onParamsChanged_) {
        onParamsChanged_(getParameters());
    }
}

contracts::TuningParameters TuningPanel::getParameters() const {
    return contracts::TuningParameters(
        static_cast<int>(intensitySlider_.getValue()),
        static_cast<int>(dubSlider_.getValue()),
        vocalSlider_.getValue()
    );
}

void TuningPanel::setParameters(const contracts::TuningParameters& params) {
    intensitySlider_.setValue(params.getReggaeIntensity(), juce::dontSendNotification);
    dubSlider_.setValue(params.getDubEffectsAmount(), juce::dontSendNotification);
    vocalSlider_.setValue(params.getVocalLevelDb(), juce::dontSendNotification);
}

void TuningPanel::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);
}

void TuningPanel::resized() {
    auto area = getLocalBounds().reduced(8);
    int knobWidth = area.getWidth() / 3;

    auto col1 = area.removeFromLeft(knobWidth);
    intensityLabel_.setBounds(col1.removeFromTop(18));
    intensitySlider_.setBounds(col1);

    auto col2 = area.removeFromLeft(knobWidth);
    dubLabel_.setBounds(col2.removeFromTop(18));
    dubSlider_.setBounds(col2);

    vocalLabel_.setBounds(area.removeFromTop(18));
    vocalSlider_.setBounds(area);
}

} // namespace reggaewave::ui

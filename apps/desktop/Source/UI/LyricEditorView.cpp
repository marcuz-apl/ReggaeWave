#include "LyricEditorView.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

LyricEditorView::LyricEditorView() {
    titleLabel_.setText("Lyrics & Subtitles (Optional)", juce::dontSendNotification);
    titleLabel_.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::textSecondary);
    addAndMakeVisible(titleLabel_);

    textEditor_.setMultiLine(true);
    textEditor_.setReturnKeyStartsNewLine(true);
    textEditor_.setScrollbarsShown(true);
    textEditor_.setColour(juce::TextEditor::backgroundColourId, ReggaeWaveTheme::bgSurface);
    textEditor_.setColour(juce::TextEditor::textColourId, ReggaeWaveTheme::textPrimary);
    textEditor_.setColour(juce::TextEditor::outlineColourId, ReggaeWaveTheme::bgElevated);
    addAndMakeVisible(textEditor_);
}

void LyricEditorView::setLyrics(const juce::String& text) {
    textEditor_.setText(text);
}

juce::String LyricEditorView::getLyrics() const {
    return textEditor_.getText();
}

void LyricEditorView::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(bounds, 8.0f);
}

void LyricEditorView::resized() {
    auto area = getLocalBounds().reduced(8);
    titleLabel_.setBounds(area.removeFromTop(20));
    area.removeFromTop(4);
    textEditor_.setBounds(area);
}

} // namespace reggaewave::ui

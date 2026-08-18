#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace reggaewave::ui {

/**
 * @brief Optional subtitle and lyric editor component (SRT/VTT).
 */
class LyricEditorView : public juce::Component {
public:
    LyricEditorView();
    ~LyricEditorView() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setLyrics(const juce::String& text);
    [[nodiscard]] juce::String getLyrics() const;

private:
    juce::Label titleLabel_;
    juce::TextEditor textEditor_;
};

} // namespace reggaewave::ui

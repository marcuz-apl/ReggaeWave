#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace reggaewave::ui {

/**
 * @brief Sleek inline export progress bar with transitioning color gradients (Green -> Gold -> Coral Red).
 * Embedded directly in the header/file path area without blocking the UI with a separate window.
 * Uses native juce::String to preserve 100% full Unicode and Chinese/CJK character fidelity.
 */
class InlineExportProgressBar : public juce::Component, public juce::Timer {
public:
    InlineExportProgressBar();
    ~InlineExportProgressBar() override;

    void setProgress(float progress0To1, const juce::String& stageText);
    void reset();
    [[nodiscard]] bool isComplete() const noexcept { return isComplete_; }

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    float currentProgress_ = 0.0f;
    float targetProgress_ = 0.0f;
    float animPhase_ = 0.0f;
    bool isComplete_ = false;
    juce::String stageText_{"Exporting..."};
};

} // namespace reggaewave::ui

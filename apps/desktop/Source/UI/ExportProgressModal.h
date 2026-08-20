#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <string>

namespace reggaewave::ui {

/**
 * @brief Modal progress dialog with smooth transitioning color gradients (Green -> Gold -> Coral Red).
 */
class ExportProgressModal : public juce::Component, public juce::Timer {
public:
    using OnCompleteCallback = std::function<void()>;

    ExportProgressModal(const std::string& title, const std::string& formatLabel, OnCompleteCallback onComplete);
    ~ExportProgressModal() override;

    void setProgress(float progress0To1, const std::string& stageText);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    std::string title_;
    std::string formatLabel_;
    std::string stageText_{"Initializing export pipeline..."};
    float currentProgress_ = 0.0f;
    float targetProgress_ = 0.0f;
    float animPhase_ = 0.0f;
    bool isDone_ = false;
    OnCompleteCallback onComplete_;

    juce::Label titleLabel_;
    juce::Label stageLabel_;
    juce::Label percentLabel_;
    juce::TextButton closeButton_{"Done"};
};

} // namespace reggaewave::ui

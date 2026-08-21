#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace reggaewave::ui {

/**
 * @brief Modal dialog offering in-depth documentation on Reggae musical heritage,
 *        application workflow / creative controls, and audio denoising/cleaning theory.
 */
class HelpDialogModal : public juce::Component {
public:
    using OnClose = std::function<void()>;

    explicit HelpDialogModal(OnClose onClose);
    ~HelpDialogModal() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void selectSection(int sectionIndex);
    void updateContentForSection(int sectionIndex);

    OnClose onClose_;

    juce::Label headerTitleLabel_;
    juce::Label headerSubtitleLabel_;

    // Navigation section tabs
    juce::TextButton heritageTabButton_{"1. Heritage & Roots"};
    juce::TextButton userGuideTabButton_{"2. Workflow & 3 Dials"};
    juce::TextButton denoiseTabButton_{"3. Denoising & Cleanup"};

    // Content Display Area
    juce::TextEditor contentEditor_;

    juce::TextButton closeButton_{"Close"};

    int currentSectionIndex_ = 0;
};

} // namespace reggaewave::ui

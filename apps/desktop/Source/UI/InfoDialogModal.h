#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace reggaewave::ui {

/**
 * @brief Modal dialog displaying product information, cultural heritage safeguards, and mastering specs.
 */
class InfoDialogModal : public juce::Component {
public:
    using OnClose = std::function<void()>;

    explicit InfoDialogModal(OnClose onClose);
    ~InfoDialogModal() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    OnClose onClose_;
    juce::Label titleLabel_;
    juce::Label versionLabel_;
    juce::Label infoContentLabel_;
    juce::TextButton closeButton_{"Close"};
};

} // namespace reggaewave::ui

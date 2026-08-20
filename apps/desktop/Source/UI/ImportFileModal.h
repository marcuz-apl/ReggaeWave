#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace reggaewave::ui {

/**
 * @brief Sleek, centered in-app file browser modal dialog matching ReggaeWave dark theme.
 */
class ImportFileModal : public juce::Component {
public:
    using OnFileSelected = std::function<void(const juce::File&)>;
    using OnCancel = std::function<void()>;

    ImportFileModal(OnFileSelected onSelected, OnCancel onCancel);
    ~ImportFileModal() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    OnFileSelected onSelected_;
    OnCancel onCancel_;

    juce::Label titleLabel_{"title", "Select Audio Track to Transform to Reggae"};
    juce::WildcardFileFilter wildcardFilter_{"*.wav;*.mp3;*.m4a;*.flac;*.ogg;*.aac;*.aiff", "*", "Audio Files"};
    juce::FileBrowserComponent fileBrowser_;
    juce::TextButton openButton_{"Import Selected"};
    juce::TextButton cancelButton_{"Cancel"};
};

} // namespace reggaewave::ui

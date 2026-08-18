#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"

namespace reggaewave::desktop {

class MainWindow : public juce::DocumentWindow {
public:
    MainWindow(const juce::String& name);
    void closeButtonPressed() override;

private:
    std::unique_ptr<MainComponent> mainComponent_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace reggaewave::desktop

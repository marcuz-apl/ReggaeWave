#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MobileMainComponent.h"

namespace reggaewave::mobile {

class MobileMainWindow : public juce::DocumentWindow {
public:
    MobileMainWindow(const juce::String& name);
    void closeButtonPressed() override;

private:
    std::unique_ptr<MobileMainComponent> mainComponent_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MobileMainWindow)
};

} // namespace reggaewave::mobile

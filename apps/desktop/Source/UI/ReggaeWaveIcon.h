#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace reggaewave::ui {

/**
 * @brief Golden "RW" circular soundwave badge component and icon generator.
 */
class ReggaeWaveIcon : public juce::Component {
public:
    ReggaeWaveIcon() = default;
    ~ReggaeWaveIcon() override = default;

    void paint(juce::Graphics& g) override;

    /**
     * @brief Generates an Image representation of the icon for window icons/favicons.
     */
    static juce::Image createIconImage(int size = 64);
};

} // namespace reggaewave::ui

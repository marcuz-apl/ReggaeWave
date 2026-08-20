#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/audio/AudioExporter.hpp>
#include <functional>

namespace reggaewave::ui {

/**
 * @brief Bottom Card 3: Broadcast Mastering Specifications & High-Fidelity Export Triggers.
 */
class ExportDeckCard : public juce::Component {
public:
    using OnExportTriggered = std::function<void(audio::AudioExportFormat, bool includeSubtitles)>;

    explicit ExportDeckCard(OnExportTriggered onExportTriggered);
    ~ExportDeckCard() override = default;

    [[nodiscard]] bool isSubtitlesEnabled() const noexcept { return subtitleToggle_.getToggleState(); }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    OnExportTriggered onExportTriggered_;

    juce::Label cardTitleLabel_;
    juce::Label specLabel_;
    juce::ToggleButton subtitleToggle_{"Embed Subtitles / Lyrics (.srt and .vtt)"};

    juce::TextButton exportMp3Button_{"Export MP3 (320k)"};
    juce::TextButton exportWavButton_{"Export WAV (24-bit)"};
};

} // namespace reggaewave::ui

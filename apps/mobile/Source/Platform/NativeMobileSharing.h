#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace reggaewave::mobile {

/**
 * @brief Native Mobile Sharing & Storage Adapter.
 * Integrates directly with:
 * - iOS: UIActivityViewController (AirDrop, Apple Files, Messages, WhatsApp, Mail)
 * - Android: Intent.ACTION_SEND with FileProvider URI
 * - Desktop/Simulator: Native File Chooser & revealToUser()
 */
class NativeMobileSharing {
public:
    /**
     * @brief Presents the native mobile Share Sheet for an exported audio or lyric file.
     */
    static void shareExportedFile(const juce::File& file,
                                  const juce::String& mimeType = "audio/mpeg",
                                  const juce::String& shareTitle = "ReggaeWave Master Export");

    /**
     * @brief Launches the native OS audio file picker (UIDocumentPicker on iOS, SAF on Android).
     */
    static void openDocumentPicker(std::function<void(const juce::File&)> onFileSelected,
                                   std::function<void()> onCancelled = nullptr);
};

} // namespace reggaewave::mobile

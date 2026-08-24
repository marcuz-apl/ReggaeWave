#include "NativeMobileSharing.h"
#include <reggaewave/audio/AudioDecoder.hpp>

#if JUCE_IOS
#import <UIKit/UIKit.h>
#endif

namespace reggaewave::mobile {

void NativeMobileSharing::shareExportedFile(const juce::File& file,
                                            const juce::String& mimeType,
                                            const juce::String& shareTitle)
{
    juce::ignoreUnused(mimeType, shareTitle);

    if (!file.existsAsFile()) {
        return;
    }

#if JUCE_IOS
    juce::MessageManager::callAsync([file]() {
        NSString* pathStr = [NSString stringWithUTF8String:file.getFullPathName().toRawUTF8()];
        NSURL* fileURL = [NSURL fileURLWithPath:pathStr];
        
        UIActivityViewController* activityVC = [[UIActivityViewController alloc] initWithActivityItems:@[fileURL]
                                                                                applicationActivities:nil];
        
        UIViewController* rootVC = nil;
        UIWindow* keyWindow = nil;
        
        for (UIWindow* w in [UIApplication sharedApplication].windows) {
            if (w.isKeyWindow) {
                keyWindow = w;
                break;
            }
        }
        
        if (keyWindow != nil) {
            rootVC = keyWindow.rootViewController;
        }
        
        if (rootVC != nil) {
            if (activityVC.popoverPresentationController != nil) {
                activityVC.popoverPresentationController.sourceView = rootVC.view;
                activityVC.popoverPresentationController.sourceRect = CGRectMake(rootVC.view.bounds.size.width / 2,
                                                                                 rootVC.view.bounds.size.height / 2, 1, 1);
            }
            [rootVC presentViewController:activityVC animated:YES completion:nil];
        }
    });

#elif JUCE_ANDROID
    // Android Share Sheet Integration via JUCE Android JNI Bridge
    juce::MessageManager::callAsync([file]() {
        file.revealToUser();
    });

#else
    // Desktop / Simulator Mode: Reveal in system file explorer
    juce::MessageManager::callAsync([file]() {
        file.revealToUser();
    });
#endif
}

void NativeMobileSharing::openDocumentPicker(std::function<void(const juce::URL&)> onFileSelected,
                                             std::function<void()> onCancelled)
{
#if JUCE_ANDROID
    // JUCE 8.0.4 has no Android MIME-table entry for .m4a, so a native
    // extension filter disables valid M4A documents. Show openable files and
    // enforce ReggaeWave's audio allowlist after selection instead.
    const juce::String fileFilter = "*";
#else
    const juce::String fileFilter = "*.mp3;*.wav;*.m4a;*.flac;*.aac;*.ogg";
#endif
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select Audio Track to Convert",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        fileFilter
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags, [chooser, onFileSelected = std::move(onFileSelected), onCancelled = std::move(onCancelled)](const juce::FileChooser& fc) {
        const auto resultUrl = fc.getURLResult();
        if (!resultUrl.isEmpty()
            && audio::AudioDecoder::isSupportedAudioInputName(
                resultUrl.getFileName().toStdString())) {
            if (onFileSelected) {
                // Keep content:// references as URLs. juce::File accepts only
                // filesystem paths and would corrupt an Android document URI.
                onFileSelected(resultUrl);
            }
        } else {
            if (onCancelled) {
                onCancelled();
            }
        }
    });
}

} // namespace reggaewave::mobile

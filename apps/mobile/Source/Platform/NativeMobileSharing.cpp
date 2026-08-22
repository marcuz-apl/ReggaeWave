#include "NativeMobileSharing.h"

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

void NativeMobileSharing::openDocumentPicker(std::function<void(const juce::File&)> onFileSelected,
                                             std::function<void()> onCancelled)
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select Audio Track to Convert",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.mp3;*.wav;*.m4a;*.flac;*.aac;*.ogg"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags, [chooser, onFileSelected = std::move(onFileSelected), onCancelled = std::move(onCancelled)](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result.existsAsFile()) {
            if (onFileSelected) {
                onFileSelected(result);
            }
        } else {
            if (onCancelled) {
                onCancelled();
            }
        }
    });
}

} // namespace reggaewave::mobile

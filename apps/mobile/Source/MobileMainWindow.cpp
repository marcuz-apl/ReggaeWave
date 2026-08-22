#include "MobileMainWindow.h"
#include "UI/ReggaeWaveTheme.h"
#include "UI/ReggaeWaveIcon.h"

namespace reggaewave::mobile {

MobileMainWindow::MobileMainWindow(const juce::String& name)
    : DocumentWindow(name,
                     ui::ReggaeWaveTheme::bgDark,
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setIcon(ui::ReggaeWaveIcon::createIconImage(256));

    mainComponent_ = std::make_unique<MobileMainComponent>();
    setContentOwned(mainComponent_.release(), true);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
#else
    // Desktop Mobile Simulator Mode (iPhone 15 Resolution: 393 x 852 px)
    setResizable(true, true);
    setResizeLimits(320, 568, 480, 1024);
    centreWithSize(393, 852);
#endif

    setVisible(true);
}

void MobileMainWindow::closeButtonPressed() {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace reggaewave::mobile

#include "MainWindow.h"
#include "UI/ReggaeWaveTheme.h"
#include "UI/ReggaeWaveIcon.h"

namespace reggaewave::desktop {

MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name,
                     ui::ReggaeWaveTheme::bgDark,
                     DocumentWindow::allButtons)
{
    // Use custom dark titlebar with embedded golden icon for reliable centering & branding
    setUsingNativeTitleBar(false);
    setIcon(ui::ReggaeWaveIcon::createIconImage(64));
    setTitleBarHeight(32);

    mainComponent_ = std::make_unique<MainComponent>();
    setContentOwned(mainComponent_.release(), true);

    setResizable(true, true);
    setResizeLimits(860, 600, 1920, 1080);

    const int targetWidth = 1040;
    const int targetHeight = 740;

    auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    if (display != nullptr) {
        auto area = display->userArea;
        const int x = area.getX() + std::max(0, (area.getWidth() - targetWidth) / 2);
        const int y = area.getY() + std::max(0, (area.getHeight() - targetHeight) / 2);
        setBounds(x, y, targetWidth, targetHeight);
    } else {
        centreWithSize(targetWidth, targetHeight);
    }

    setVisible(true);
}

void MainWindow::closeButtonPressed() {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace reggaewave::desktop

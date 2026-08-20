#include "MainWindow.h"
#include "UI/ReggaeWaveTheme.h"

namespace reggaewave::desktop {

MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name,
                     ui::ReggaeWaveTheme::bgDark,
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    mainComponent_ = std::make_unique<MainComponent>();
    setContentOwned(mainComponent_.release(), true);

    setResizable(true, true);
    setResizeLimits(800, 560, 1920, 1080);

    const int targetWidth = 980;
    const int targetHeight = 660;

    centreWithSize(targetWidth, targetHeight);
    setVisible(true);

    // Re-center explicitly after native peer creation for X11 / WSLg
    centreWithSize(targetWidth, targetHeight);
    auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
    if (display != nullptr) {
        auto area = display->userArea;
        const int x = area.getX() + (area.getWidth() - targetWidth) / 2;
        const int y = area.getY() + (area.getHeight() - targetHeight) / 2;
        setBounds(x, y, targetWidth, targetHeight);
        if (auto* peer = getPeer()) {
            peer->setBounds(juce::Rectangle<int>(x, y, targetWidth, targetHeight), true);
        }
    }
}

void MainWindow::closeButtonPressed() {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace reggaewave::desktop

#include "MainWindow.h"

namespace reggaewave::desktop {

MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name,
                     juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                     DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    mainComponent_ = std::make_unique<MainComponent>();
    setContentOwned(mainComponent_.release(), true);

    setResizable(true, true);
    setResizeLimits(760, 520, 1920, 1080);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
}

void MainWindow::closeButtonPressed() {
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace reggaewave::desktop

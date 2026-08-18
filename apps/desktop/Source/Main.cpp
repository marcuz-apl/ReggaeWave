#include <juce_gui_basics/juce_gui_basics.h>
#include "MainWindow.h"

namespace reggaewave::desktop {

class ReggaeWaveApplication : public juce::JUCEApplication {
public:
    ReggaeWaveApplication() = default;

    const juce::String getApplicationName() override { return "ReggaeWave"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& /*commandLine*/) override {
        mainWindow_ = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override {
        mainWindow_.reset();
    }

    void systemRequestedQuit() override {
        quit();
    }

    void anotherInstanceStarted(const juce::String& /*commandLine*/) override {}

private:
    std::unique_ptr<MainWindow> mainWindow_;
};

} // namespace reggaewave::desktop

START_JUCE_APPLICATION(reggaewave::desktop::ReggaeWaveApplication)

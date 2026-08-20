#include <juce_gui_basics/juce_gui_basics.h>
#include "MainWindow.h"
#include <csignal>

namespace reggaewave::desktop {

class ReggaeWaveApplication : public juce::JUCEApplication {
public:
    ReggaeWaveApplication() = default;

    const juce::String getApplicationName() override { return "ReggaeWave"; }
#ifdef REGGAEWAVE_APP_VERSION_STRING
    const juce::String getApplicationVersion() override { return REGGAEWAVE_APP_VERSION_STRING; }
#else
    const juce::String getApplicationVersion() override { return "1.2.5"; }
#endif
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& /*commandLine*/) override {
#if defined(SIGPIPE)
        // Prevent SIGPIPE (signal 13 / exit code 141) on audio streaming pipes
        std::signal(SIGPIPE, SIG_IGN);
#endif

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

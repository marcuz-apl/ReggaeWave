#include <juce_gui_basics/juce_gui_basics.h>
#include "MobileMainWindow.h"

namespace reggaewave::mobile {

class ReggaeWaveMobileApplication : public juce::JUCEApplication {
public:
    ReggaeWaveMobileApplication() = default;

    const juce::String getApplicationName() override {
        return "ReggaeWave Mobile";
    }

    const juce::String getApplicationVersion() override {
        return REGGAEWAVE_APP_VERSION_STRING;
    }

    bool moreThanOneInstanceAllowed() override {
        return true;
    }

    void initialise(const juce::String& commandLine) override {
        juce::ignoreUnused(commandLine);
        mainWindow_ = std::make_unique<MobileMainWindow>(getApplicationName());
    }

    void shutdown() override {
        mainWindow_.reset();
    }

    void systemRequestedQuit() override {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override {
        juce::ignoreUnused(commandLine);
    }

private:
    std::unique_ptr<MobileMainWindow> mainWindow_;
};

} // namespace reggaewave::mobile

START_JUCE_APPLICATION(reggaewave::mobile::ReggaeWaveMobileApplication)

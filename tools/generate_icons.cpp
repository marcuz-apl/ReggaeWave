#include <juce_gui_basics/juce_gui_basics.h>
#include "../apps/desktop/Source/UI/ReggaeWaveIcon.h"
#include <iostream>

int main() {
    juce::MessageManager::getInstance();

    auto icon512 = reggaewave::ui::ReggaeWaveIcon::createIconImage(512);
    auto icon256 = reggaewave::ui::ReggaeWaveIcon::createIconImage(256);
    auto icon128 = reggaewave::ui::ReggaeWaveIcon::createIconImage(128);
    auto icon64  = reggaewave::ui::ReggaeWaveIcon::createIconImage(64);
    auto icon32  = reggaewave::ui::ReggaeWaveIcon::createIconImage(32);

    juce::File dir("apps/desktop/Resources");
    dir.createDirectory();

    juce::PNGImageFormat png;

    auto savePng = [&](const juce::Image& img, const juce::String& filename) {
        juce::File f = dir.getChildFile(filename);
        juce::FileOutputStream fos(f);
        if (fos.openedOk()) {
            png.writeImageToStream(img, fos);
            std::cout << "Generated: " << f.getFullPathName() << std::endl;
        }
    };

    savePng(icon512, "reggaewave_512.png");
    savePng(icon256, "reggaewave_256.png");
    savePng(icon128, "reggaewave_128.png");
    savePng(icon64, "reggaewave_64.png");
    savePng(icon32, "reggaewave_32.png");

    juce::File assetsDir("infra/packaging/assets");
    assetsDir.createDirectory();
    juce::File iconAsset = assetsDir.getChildFile("reggaewave.png");
    juce::FileOutputStream assetFos(iconAsset);
    if (assetFos.openedOk()) {
        png.writeImageToStream(icon512, assetFos);
        std::cout << "Generated asset: " << iconAsset.getFullPathName() << std::endl;
    }

    juce::MessageManager::deleteInstance();
    return 0;
}

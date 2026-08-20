#include "ImportFileModal.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

ImportFileModal::ImportFileModal(OnFileSelected onSelected, OnCancel onCancel)
    : onSelected_(std::move(onSelected))
    , onCancel_(std::move(onCancel))
    , fileBrowser_(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        juce::File::getCurrentWorkingDirectory().getChildFile("test-tracks").exists()
            ? juce::File::getCurrentWorkingDirectory().getChildFile("test-tracks")
            : juce::File::getCurrentWorkingDirectory(),
        &wildcardFilter_,
        nullptr
    )
{
    setAlwaysOnTop(true);

    titleLabel_.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    titleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(titleLabel_);

    addAndMakeVisible(fileBrowser_);

    openButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGold);
    openButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    openButton_.onClick = [this]() {
        auto file = fileBrowser_.getSelectedFile(0);
        if (file.existsAsFile() && onSelected_) {
            onSelected_(file);
        }
    };
    addAndMakeVisible(openButton_);

    cancelButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::bgElevated);
    cancelButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::textPrimary);
    cancelButton_.onClick = [this]() {
        if (onCancel_) onCancel_();
    };
    addAndMakeVisible(cancelButton_);
}

void ImportFileModal::paint(juce::Graphics& g) {
    g.fillAll(juce::Colours::black.withAlpha(0.75f));

    auto cardArea = getLocalBounds().withSizeKeepingCentre(640, 480).toFloat();
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(cardArea, 12.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(cardArea, 12.0f, 1.5f);
}

void ImportFileModal::resized() {
    auto cardArea = getLocalBounds().withSizeKeepingCentre(640, 480).reduced(20);

    titleLabel_.setBounds(cardArea.removeFromTop(28));
    cardArea.removeFromTop(10);

    auto btnRow = cardArea.removeFromBottom(36);
    openButton_.setBounds(btnRow.removeFromRight(140));
    btnRow.removeFromRight(10);
    cancelButton_.setBounds(btnRow.removeFromRight(100));

    cardArea.removeFromBottom(10);
    fileBrowser_.setBounds(cardArea);
}

} // namespace reggaewave::ui

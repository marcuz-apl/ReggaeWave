#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <reggaewave/contracts/RightsAttestation.hpp>
#include <functional>
#include <optional>

namespace reggaewave::ui {

class RightsAttestationModal : public juce::Component {
public:
    using OnConfirmedCallback = std::function<void(contracts::RightsBasis)>;

    explicit RightsAttestationModal(OnConfirmedCallback onConfirmed);
    ~RightsAttestationModal() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void updateButtonState();

    OnConfirmedCallback onConfirmed_;

    juce::Label titleLabel_;
    juce::Label subtitleLabel_;

    juce::ToggleButton ownedOption_{"I own the relevant composition and recording rights."};
    juce::ToggleButton licensedOption_{"I have permission or a license to create this version."};
    juce::ToggleButton publicDomainOption_{"The material is in the public domain, and this recording is authorized."};

    juce::ToggleButton confirmCheckbox_{"I understand that I am legally responsible for the rights attestation."};
    juce::TextButton confirmButton_{"Confirm & Import Music"};

    std::optional<contracts::RightsBasis> selectedBasis_;
};

} // namespace reggaewave::ui

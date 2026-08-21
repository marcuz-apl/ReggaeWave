#include "HelpDialogModal.h"
#include "ReggaeWaveTheme.h"

namespace reggaewave::ui {

HelpDialogModal::HelpDialogModal(OnClose onClose)
    : onClose_(std::move(onClose))
{
    setAlwaysOnTop(true);

    // Header Labels
    headerTitleLabel_.setText("ReggaeWave Guide & Heritage", juce::dontSendNotification);
    headerTitleLabel_.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    headerTitleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGold);
    addAndMakeVisible(headerTitleLabel_);

    headerSubtitleLabel_.setText("Jamaican Living Cultural Heritage • Workflow • Audio Engineering Theory", juce::dontSendNotification);
    headerSubtitleLabel_.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    headerSubtitleLabel_.setColour(juce::Label::textColourId, ReggaeWaveTheme::accentGreen);
    addAndMakeVisible(headerSubtitleLabel_);

    // Section Tab Buttons
    auto setupTabButton = [this](juce::TextButton& btn, int index) {
        btn.onClick = [this, index]() { selectSection(index); };
        addAndMakeVisible(btn);
    };

    setupTabButton(heritageTabButton_, 0);
    setupTabButton(userGuideTabButton_, 1);
    setupTabButton(denoiseTabButton_, 2);

    // Multi-line Read-Only Content Viewer
    contentEditor_.setMultiLine(true);
    contentEditor_.setReadOnly(true);
    contentEditor_.setCaretVisible(false);
    contentEditor_.setScrollbarsShown(true);
    contentEditor_.setColour(juce::TextEditor::backgroundColourId, ReggaeWaveTheme::bgDark);
    contentEditor_.setColour(juce::TextEditor::textColourId, ReggaeWaveTheme::textPrimary);
    contentEditor_.setColour(juce::TextEditor::outlineColourId, ReggaeWaveTheme::bgElevated);
    contentEditor_.setFont(juce::FontOptions(13.5f));
    addAndMakeVisible(contentEditor_);

    // Close Button
    closeButton_.setColour(juce::TextButton::buttonColourId, ReggaeWaveTheme::accentGold);
    closeButton_.setColour(juce::TextButton::textColourOffId, ReggaeWaveTheme::bgDark);
    closeButton_.onClick = [this]() {
        if (onClose_) onClose_();
    };
    addAndMakeVisible(closeButton_);

    // Initial Section
    selectSection(0);
}

void HelpDialogModal::selectSection(int sectionIndex)
{
    currentSectionIndex_ = sectionIndex;

    auto updateButtonState = [](juce::TextButton& btn, bool active) {
        btn.setColour(juce::TextButton::buttonColourId, active ? ReggaeWaveTheme::accentGreen : ReggaeWaveTheme::bgElevated);
        btn.setColour(juce::TextButton::textColourOffId, active ? ReggaeWaveTheme::bgDark : ReggaeWaveTheme::textPrimary);
    };

    updateButtonState(heritageTabButton_, currentSectionIndex_ == 0);
    updateButtonState(userGuideTabButton_, currentSectionIndex_ == 1);
    updateButtonState(denoiseTabButton_, currentSectionIndex_ == 2);

    updateContentForSection(currentSectionIndex_);
}

void HelpDialogModal::updateContentForSection(int sectionIndex)
{
    juce::String text;

    if (sectionIndex == 0)
    {
        text = 
            "====================================================================\n"
            " SECTION 1: REGGAE MUSICAL HERITAGE & ORIGINS\n"
            "====================================================================\n\n"
            "1. Cultural Roots & Evolution\n"
            "   Reggae originated in Kingston, Jamaica during the late 1960s, evolving\n"
            "   from earlier Jamaican genres including Mento, Ska, and Rocksteady.\n"
            "   Fostered in neighborhood Sound Systems (outdoor mobile discotheques),\n"
            "   Reggae became a powerful global voice for social consciousness, spiritual\n"
            "   resilience, and community unity.\n\n"
            "2. Sonic Architecture & Groove Foundations\n"
            "   - The 'One Drop' Beat:\n"
            "     Unlike Western 4/4 pop where beat 1 is heavily accented, classic Reggae\n"
            "     drops the kick on beat 1 entirely and places a simultaneous bass drum\n"
            "     and snare/rimshot accent strictly on BEAT 3.\n"
            "   - The 'Steppers' Beat:\n"
            "     A driving variant featuring a steady four-on-the-floor bass drum pulse\n"
            "     complemented by heavy offbeat syncopation.\n"
            "   - The 'Skank' (or Chop):\n"
            "     Staccato, percussive chords played on guitar and Hammond organ / piano\n"
            "     accenting the offbeats ('ands' or beats 2 & 4).\n"
            "   - The Riddim & Bassline:\n"
            "     Deep, low-end melodic basslines that provide both the harmonic foundation\n"
            "     and rhythmic counterpoint to the drums.\n"
            "   - Dub Echo & Space Processing:\n"
            "     Pioneered in Kingston recording studios by mixing engineers who turned the\n"
            "     mixing console into an instrument using tape delays and spring reverbs.\n\n"
            "3. Legendary Pioneers & Innovators\n"
            "   - King Tubby (Osbourne Ruddock): Father of Dub & spatial manipulation.\n"
            "   - Lee 'Scratch' Perry: Visionary producer behind the Black Ark sound.\n"
            "   - Bob Marley & The Wailers, Peter Tosh, Bunny Wailer: Global ambassadors.\n"
            "   - Dennis Brown, Gregory Isaacs, Burning Spear, Culture: Roots masters.\n"
            "   - Sly & Robbie (Sly Dunbar & Robbie Shakespeare): The ultimate rhythm architects.\n\n"
            "4. Cultural Safeguards in ReggaeWave\n"
            "   Reggae is a living Jamaican cultural heritage. ReggaeWave is designed with\n"
            "   strict cultural integrity: it preserves original lead vocals without voice\n"
            "   cloning or caricature, applying authentic acoustic riddim orchestration.";
    }
    else if (sectionIndex == 1)
    {
        text = 
            "====================================================================\n"
            " SECTION 2: WORKFLOW & THE 3 CREATIVE CONTROLS\n"
            "====================================================================\n\n"
            "1. The 4-Step Reggae Transformation Workflow\n"
            "   Step 1: Import Track\n"
            "     Drop any audio file (WAV, MP3, FLAC, AIFF) or stem. Confirm the\n"
            "     rights attestation to ensure authorized source material.\n"
            "   Step 2: Creative Tuning\n"
            "     Keep the default settings for instant transformation, or open the\n"
            "     Tuning Panel to shape the mood using the 3 macro controls.\n"
            "   Step 3: A/B Auditioning & Variation Comparison\n"
            "     ReggaeWave automatically renders 2 distinct variations per conversion:\n"
            "     - Variation A: Authentic One-Drop Roots arrangement.\n"
            "     - Variation B: Steppers / Dubwise alternative groove.\n"
            "     Switch between A and B seamlessly with continuous timestamp playback.\n"
            "   Step 4: Mastering & Pro Export\n"
            "     Export broadcast-compliant 24-bit WAV or 320 kbps MP3 files mastered to\n"
            "     -14.0 LUFS integrated loudness with a -1.0 dBTP ceiling.\n\n"
            "2. Why Exactly 3 Controls Make the Magic Happen\n"
            "   Instead of overwhelming you with 50 complex DAW knobs, ReggaeWave uses\n"
            "   three intelligent macro-conductors over multi-stage DSP engines:\n\n"
            "   [ Dial 1: Reggae Intensity (0% - 100%) ]\n"
            "   - Controls the harmonic density and dynamic balance between the dry song\n"
            "     and the synthesized Jamaican riddim section (drum skank, bubble organ,\n"
            "     one-drop kick/snare, and walking bassline weight).\n\n"
            "   [ Dial 2: Dub Effects Amount (0% - 100%) ]\n"
            "   - Governs tape delay feedback, spring reverb resonance, high-frequency\n"
            "     rolloff, and spontaneous King Tubby-style snare/vocal dub throws.\n\n"
            "   [ Dial 3: Vocal Level (-6.0 dB to +6.0 dB) ]\n"
            "   - Adjusts the precise mix level of the isolated lead vocal against\n"
            "     the newly arranged backing riddim.\n\n"
            "3. Two Variations per Conversion\n"
            "   No need to re-render repeatedly: you always receive two synchronized,\n"
            "   ready-to-export artistic interpretations with zero extra effort.";
    }
    else
    {
        text = 
            "====================================================================\n"
            " SECTION 3: AUDIO PRE-CONDITIONING & DENOISING (THEORY & PRACTICE)\n"
            "====================================================================\n\n"
            "1. The Challenge of Real-World Source Audio\n"
            "   Musical recordings often suffer from low-frequency stage rumble, electrical\n"
            "   hum (50/60 Hz), microphone hiss, room acoustic reverberation, or backing\n"
            "   instrument bleed. These artifacts contaminate vocal isolation and confuse\n"
            "   harmonic key/chord detection.\n\n"
            "2. The 3-Stage Audio Cleanup Pipeline\n"
            "   When the 'Enhance & Clean Source Audio' option is activated, the engine\n"
            "   executes a comprehensive multi-tier restoration process:\n\n"
            "   [ Stage 1: Input Pre-Conditioning & Spectral Cleanup ]\n"
            "   - Sub-35 Hz High-Pass Filter: Cuts sub-audible DC offset and mechanical rumble.\n"
            "   - Mains Hum Rejection: Dual-notch filters targeting 50 Hz and 60 Hz harmonics.\n"
            "   - Adaptive Spectral Gating: Attenuates static room noise floor and tape hiss.\n\n"
            "   [ Stage 2: Enhanced AI Stem Separation & Harmonic Analysis ]\n"
            "   - Receiving a pre-conditioned audio stream allows the neural separator and\n"
            "     chroma pitch detector to identify chord progressions and downbeats with\n"
            "     significantly higher confidence and precision.\n\n"
            "   [ Stage 3: Lead Vocal Polish & De-Bleed ]\n"
            "   - Dedicated spectral gating is applied to the extracted lead vocal channel,\n"
            "     silencing leftover drum bleed, guitar spill, and ambient chaos during vocal\n"
            "     pauses without altering the singer's natural timbre.\n\n"
            "3. Practical Usage Guidelines\n"
            "   - ENABLE Cleanup for:\n"
            "     * Live concert recordings, rehearsal tapes, and voice memo drafts.\n"
            "     * Vinyl rips, cassette transfers, and vintage recordings.\n"
            "     * Home studio vocal tracks recorded without acoustic isolation.\n"
            "   - DISABLE Cleanup for:\n"
            "     * Pristine, commercially mastered studio audio where zero preprocessing\n"
            "       is desired.";
    }

    contentEditor_.setText(text);
    contentEditor_.setCaretPosition(0);
}

void HelpDialogModal::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.80f));

    auto cardArea = getLocalBounds().withSizeKeepingCentre(720, 520).toFloat();
    g.setColour(ReggaeWaveTheme::bgSurface);
    g.fillRoundedRectangle(cardArea, 14.0f);

    g.setColour(ReggaeWaveTheme::bgElevated);
    g.drawRoundedRectangle(cardArea, 14.0f, 1.5f);
}

void HelpDialogModal::resized()
{
    auto cardArea = getLocalBounds().withSizeKeepingCentre(720, 520).reduced(24);

    // Title and subtitle
    headerTitleLabel_.setBounds(cardArea.removeFromTop(30));
    headerSubtitleLabel_.setBounds(cardArea.removeFromTop(20));
    cardArea.removeFromTop(12);

    // Tab buttons row (32px height)
    auto tabRow = cardArea.removeFromTop(32);
    int tabWidth = (tabRow.getWidth() - 16) / 3;
    heritageTabButton_.setBounds(tabRow.removeFromLeft(tabWidth));
    tabRow.removeFromLeft(8);
    userGuideTabButton_.setBounds(tabRow.removeFromLeft(tabWidth));
    tabRow.removeFromLeft(8);
    denoiseTabButton_.setBounds(tabRow);

    cardArea.removeFromTop(12);

    // Close button at bottom
    closeButton_.setBounds(cardArea.removeFromBottom(36).withSizeKeepingCentre(140, 36));
    cardArea.removeFromBottom(10);

    // Multi-line content takes remaining space
    contentEditor_.setBounds(cardArea);
}

} // namespace reggaewave::ui

# ReggaeWave Tech Note: Desktop C++20 & JUCE 8 Implementation Plan

| Field | Value |
| --- | --- |
| Date | 2026-08-19 |
| Status | Approved & In Progress |
| Architecture Reference | [ADR 0001: Desktop Application Architecture using C++20 and JUCE 8](../adr/0001-desktop-cpp-juce-architecture.md) |
| Target Platforms | macOS 13+ (CoreAudio / Metal) and Windows 11 x64 (WASAPI / Direct2D) |

---

## 1. Overview & Objectives

ReggaeWave Desktop is an installed, pro-audio desktop application engineered in **C++20** and **JUCE 8**. It enables creators and producers to transform rights-cleared musical input from any genre into an authentic Reggae arrangement with precision real-time tuning, dual-variation comparison, and local offline media rendering.

---

## 2. Component Boundaries & Repository Architecture

```text
ReggaeWave/
├── CMakeLists.txt                      # Root CMake configuration (C++20, JUCE 8, Catch2)
├── apps/
│   └── desktop/                        # Main Desktop App executable target
│       ├── CMakeLists.txt
│       ├── Source/
│       │   ├── Main.cpp                # JUCE Application lifecycle entry point
│       │   ├── MainWindow.h/.cpp       # Window management & display scaling
│       │   ├── MainComponent.h/.cpp    # Root view orchestrator
│       │   └── UI/
│       │       ├── ReggaeWaveTheme.h/.cpp     # Custom Dark LookAndFeel (gold/green accents)
│       │       ├── RightsAttestationModal.h/.cpp # Mandatory 3-basis attestation
│       │       ├── TuningPanel.h/.cpp         # 3 Controls: Intensity, Dub FX, Vocal Gain
│       │       ├── WaveformABView.h/.cpp      # Dual variation waveform & A/B switcher
│       │       └── LyricEditorView.h/.cpp     # Optional subtitle editor (SRT/VTT)
├── packages/
│   ├── contracts/                      # Domain types, state machines & validation
│   │   ├── CMakeLists.txt
│   │   └── include/reggaewave/contracts/
│   │       ├── TuningParameters.hpp    # Bounds: Intensity [0-100], Dub [0-100], Vocal [-6 to +6 dB]
│   │       ├── RightsAttestation.hpp   # Owned, Licensed, Public Domain verification
│   │       ├── JobState.hpp            # Transactional state machine
│   │       └── Manifests.hpp           # Musical analysis & variation metadata
│   ├── audio-engine/                   # Real-time DSP, A/B transport & Loudness
│   │   ├── CMakeLists.txt
│   │   └── include/reggaewave/audio/
│   │       ├── DualTransportSource.hpp # Sample-accurate, glitch-free A/B crossfader
│   │       ├── DubEffectsProcessor.hpp # Tape delay feedback, spring reverb, resonant lowpass
│   │       └── LoudnessMeter.hpp       # -14 LUFS integrated & -1 dBTP true peak monitoring
│   └── storage/                        # SQLite3 local persistence layer
│       ├── CMakeLists.txt
│       └── include/reggaewave/storage/
│           └── LocalDatabase.hpp       # Durable job state and project storage
└── tests/                              # Unit & integration test suites
    ├── CMakeLists.txt
    ├── contracts/
    │   ├── TuningParametersTests.cpp
    │   ├── RightsAttestationTests.cpp
    │   └── JobStateTests.cpp
    └── audio/
        ├── DubEffectsTests.cpp
        └── DualTransportTests.cpp
```

---

## 3. Detailed Implementation Phases

### Phase 1: Build System & Domain Contracts (`packages/contracts`)
- **Tuning Parameters**: Strictly validate the 3 MVP controls (Reggae Intensity: 0–100 [default 70], Dub-effects: 0–100 [default 20], Vocal level: -6.0 to +6.0 dB [default 0.0 dB]).
- **Rights Attestation**: Mandatory 3-basis verification (`Owned`, `Licensed`, `PublicDomain`), requiring explicit operator confirmation and audit timestamps.
- **State Machine**: Full transition rules for conversion (`Created → Importing → Validating → Queued → Normalizing → Separating → Analyzing → Arranging → Mixing → Transcribing → PreviewReady → Completed`) and export pipelines.

### Phase 2: Audio & DSP Engine (`packages/audio-engine`)
- **DubEffectsProcessor**: Real-time tape delay with saturation modeling (`tanh` clipping), tempo sync, resonant low-pass filter sweeps, and spring reverb diffusion.
- **DualTransportSource**: Synchronized dual-variation playback with equal-power crossfading, playhead alignment, and lead-vocal gain control.
- **LoudnessMeter**: ITU-R BS.1770-4 / EBU R128 loudness measurement targeting -14.0 LUFS integrated and -1.0 dBTP true peak ceiling.

### Phase 3: Desktop UI & Visual Components (`apps/desktop`)
- **ReggaeWaveTheme**: Custom hardware-accelerated dark theme extending `juce::LookAndFeel_V4`.
- **RightsAttestationModal**: Modal dialog forcing explicit selection of ownership/license basis before import.
- **WaveformABView**: High-DPI interactive waveform display with synchronized seek and seamless A/B variation switching.
- **TuningPanel**: Rotary knobs for the 3 controls with visual numerical readouts.

---

## 4. Verification & Validation Strategy

1. **Automated Unit Tests**:
   - `reggaewave_tests` (Catch2) verifying contract validation, state machine boundaries, and DSP buffer calculations.
2. **Audio Fixture Synthesizers**:
   - Programmatically generated audio signals (impulse, sine sweep) ensuring zero unlicensed audio in the repository.
3. **GUI Verification**:
   - Interactive verification of window rendering, rotary knobs, and playback controls.

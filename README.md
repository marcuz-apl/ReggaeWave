# ReggaeWave

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![JUCE 8](https://img.shields.io/badge/JUCE-8.0-orange.svg)](https://juce.com/)
[![License](https://img.shields.io/badge/License-Proprietary-red.svg)](#rights-and-licensing)

**ReggaeWave** transforms rights-cleared musical input from any source genre into an authentic, culturally reviewed **Reggae** arrangement. 

The output is always **Reggae**—no target-genre selector is needed. ReggaeWave separates stems, preserves the original lead vocal without voice cloning, generates authentic drum, bass, and skank rhythm sections, provides real-time Dub effects, and renders two synchronized variations for comparison and export.

---

## Key Features

- **Any Genre In, Reggae Out**: Feed in Rock, Pop, Classical, Hip-Hop, or Jazz; receive an authentic Reggae arrangement.
- **Vocal Preservation**: The separated lead vocal melody, phrasing, and timbre are preserved intact—no voice cloning, celebrity voices, or synthetic replacements.
- **Three Precision Tuning Controls**:
  1. **Reggae Intensity** (`0–100`, default `70`): Controls bass presence, offbeat skank aggression, and rhythmic substitution.
  2. **Dub-Effects Amount** (`0–100`, default `20`): Real-time tape delay with saturation, spring reverb diffusion, and resonant low-pass filter sweeps.
  3. **Vocal Level** (`-6.0 dB to +6.0 dB`, default `0.0 dB`): Lead vocal gain staging in the final mix.
- **Dual-Variation A/B Comparison**: Generates two synchronized variations (e.g., *Variation A: Classic Roots / One-Drop* vs. *Variation B: Modern Steppers*) with click-free, sample-accurate timestamp crossfading.
- **Master-Ready Exports**: 320 kbps MP3 and 44.1 kHz 24-bit stereo WAV mastered to `-14 LUFS` integrated with a `-1 dBTP` true peak ceiling.
- **Optional Lyric Visualizer**: Optional transcription with user revision support and export to SRT, VTT, and MP4 lyric video.

---

## Rights & Cultural Safeguards

- **Mandatory Rights Attestation**: ReggaeWave processes only user-owned, explicitly licensed, or authorized public-domain material. Attestations can never be bypassed or pre-selected.
- **Jamaican Living Cultural Heritage**: Reggae is recognized by UNESCO as Intangible Cultural Heritage. ReggaeWave avoids caricature and superficial presets, subjecting musical pipelines to review by qualified Reggae practitioners.

---

## Repository Structure

```text
ReggaeWave/
├── apps/
│   └── desktop/                  # JUCE 8 desktop application (GUI & audio host)
│       └── Source/
│           ├── UI/               # Custom dark theme, rotary dials, A/B waveform
│           ├── MainComponent.cpp # Application coordinator & DSP wiring
│           └── MainWindow.cpp    # Window lifecycle & display management
├── packages/
│   ├── contracts/                # Domain types, state machines & tuning bounds
│   ├── audio-engine/             # Real-time Dub DSP, A/B transport & LUFS metering
│   └── storage/                  # Transactional local SQLite state storage
├── docs/
│   ├── PRD.md                    # Product Requirements Document (Source of Truth)
│   ├── adr/                      # Architecture Decision Records (ADR 0001)
│   └── tech-notes/               # Technical implementation plans & walkthroughs
└── tests/                        # Catch2 unit & DSP integration test suite
```

---

## Getting Started

### Prerequisites

- **C++ Compiler**: GCC 13+, Clang 16+, or MSVC 2022+ supporting **C++20**.
- **CMake**: Version 3.22 or higher.
- **Audio/Graphics Dependencies** (Linux only):
  ```bash
  sudo apt install -y libasound2-dev libx11-dev libxinerama-dev libxext-dev \
                      libfreetype6-dev libcurl4-openssl-dev libgl1-mesa-dev
  ```

### Building & Running Unit Tests

```bash
# 1. Configure CMake with tests enabled
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DREGGAEWAVE_BUILD_DESKTOP=OFF

# 2. Build the test suite
cmake --build build --target reggaewave_tests -j$(nproc)

# 3. Run all test cases
./build/tests/reggaewave_tests
```

### Building the Desktop Application

```bash
# Configure with desktop application enabled (fetches JUCE 8 via CMake FetchContent)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DREGGAEWAVE_BUILD_DESKTOP=ON

# Build the desktop executable
cmake --build build --target ReggaeWave -j$(nproc)
```

---

## Documentation

- **[Product Requirements Document (PRD)](docs/PRD.md)** — Source of truth for product goals, constraints, and tuning ranges.
- **[ADR 0001: C++20 & JUCE 8 Architecture](docs/adr/0001-desktop-cpp-juce-architecture.md)** — Architecture decision record for the desktop platform.
- **[Tech Notes Index](docs/tech-notes/README.md)** — Implementation plans and walkthrough technical notes.

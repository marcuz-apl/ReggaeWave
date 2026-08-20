# Tech Note 0004: Desktop Studio Architecture, Real-Time DSP Engine & Cross-Platform Testing Guide

**Date:** 2026-08-20  
**Version:** 1.2.3  
**Status:** Implemented, Documented & Verified  

---

## 1. Executive Summary

This document details the architecture, component hierarchy, real-time DSP pipeline, and step-by-step cross-platform build/testing procedures for the **ReggaeWave Desktop Studio** application across **Linux, macOS, and Windows**.

---

## 2. Component Hierarchy & 3-Tier Card Layout

The user interface follows a 3-tier stacked card architecture with a persistent branding and metadata header bar:

```text
+---------------------------------------------------------------------------------------+
| [RW Icon] ReggaeWave v1.2.3          [Rights: Owned]  [Roots Engine | 44.1k]  [About] |
+---------------------------------------------------------------------------------------+
| 1. INTAKE & ANALYSIS CARD (Top)                                                       |
|   [+ Import Track]  |  Filename.wav  [110 BPM] [C Major] [03:24] [Lead Vocal Isolated]|
+---------------------------------------------------------------------------------------+
| 2. RIDDIM & DUB STUDIO (Middle)                                                       |
|   Controls (Left 35%):               | Dynamic Visualizer (Right 65%):                |
|   - [Play / Pause] [Rewind]          | - Single-Sided Upper-Half Spectrum (68% height)|
|   - [Var A: One-Drop] [Var B: Step]  | - Green -> Yellow -> Red RGB Height Gradient   |
|   - Dials: Intensity, Dub FX, Gain   | - Scrubber Slider & Real-time Timecode Readout |
+---------------------------------------------------------------------------------------+
| 3. MASTERING & EXPORT DECK (Bottom)                                                   |
|   - -14.0 LUFS / -1.0 dBTP Ceiling   | [ Export WAV (24-bit) ]  [ Export MP3 (320k) ] |
|   - [x] Embed Subtitles (.srt/.vtt)  | -> Opens centered ExportDialogModal (0% -> Done)|
+---------------------------------------------------------------------------------------+
```

---

## 3. Real-Time Threading & DSP Architecture

```mermaid
graph TD
    A[GUI Thread - Sliders & Buttons] -->|O(1) Mutex Protected Updates| B[DSP State: DualTransport + DubProcessor]
    C[Audio Streaming Thread] -->|Read 2048 Samples| B
    C -->|Self-Clocked PCM Float32LE| D[Audio Output Device / Pipe]
    E[Export Worker Thread] -->|Render Full Master| B
    E -->|Progress Updates| F[Export Dialog UI]
```

### Key Technical Safeguards:
1. **Zero UI Thread Blocking**: Parameter changes (`setDubAmount`, `setVocalGainDb`) are atomic $\mathcal{O}(1)$ updates taking $< 0.001$ ms.
2. **Audio Pipe Self-Clocking**: Removes artificial sleep timers, relying on OS pipe buffer backpressure to synchronize with hardware clock rates.
3. **Signal Guarding**: Ignores `SIGPIPE` to prevent process termination on pipe reset (Exit Code 141 fix).
4. **Soft Saturation**: Uses `std::tanh` in the stem mixer to eliminate waveform flat-top clipping.

---

## 4. Cross-Platform Compilation & Testing Guide

ReggaeWave is architected using **standard C++20 + JUCE 8 + CMake**, making the entire codebase natively cross-platform without platform-specific forks.

```
                  +--------------------------------+
                  |    ReggaeWave C++20 Core       |
                  |  (Contracts, DSP, UI Layout)   |
                  +---------------+----------------+
                                  |
            +---------------------+---------------------+
            |                     |                     |
     +------v------+       +------v------+       +------v------+
     |    Linux    |       |    macOS    |       |   Windows   |
     | ALSA/Pulse  |       |  CoreAudio  |       |   WASAPI    |
     | X11/Wayland |       | Cocoa/Metal |       | Direct2D/DX |
     +-------------+       +-------------+       +-------------+
```

---

### 🍏 A. Building & Testing on macOS

#### Prerequisites:
- **Xcode** or **Apple Command Line Tools** (`xcode-select --install`)
- **CMake** (`brew install cmake`)

#### Terminal Commands:
```bash
# 1. Clone or pull the repository
git clone https://github.com/marcuz-apl/ReggaeWave.git
cd ReggaeWave

# 2. Configure with CMake in Release mode
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Compile the Desktop App & Test Suite
cmake --build build --target ReggaeWave -j$(sysctl -n hw.ncpu)
ctest --test-dir build --output-on-failure

# 4. Launch the Native macOS Bundle:
open ./build/apps/desktop/ReggaeWave_artefacts/Release/ReggaeWave.app
```

> **macOS Audio Advantage:** JUCE uses Apple's native **CoreAudio** subsystem. Multi-channel audio, live DSP crossfading, and low-latency buffer management work automatically with zero configuration.

---

### 🪟 B. Building & Testing on Windows

#### Prerequisites:
- **Visual Studio 2022** (Community or Professional with *"Desktop development with C++"*)
- **CMake** (Bundled with Visual Studio or via `winget install Kitware.CMake`)

#### Terminal Commands (PowerShell / Command Prompt / VS Developer Prompt):
```powershell
# 1. Clone repository
git clone https://github.com/marcuz-apl/ReggaeWave.git
cd ReggaeWave

# 2. Generate Visual Studio 2022 x64 Solution
cmake -B build -G "Visual Studio 17 2022" -A x64

# 3. Compile in Release Mode
cmake --build build --config Release --target ReggaeWave -j %NUMBER_OF_PROCESSORS%
ctest --test-dir build -C Release --output-on-failure

# 4. Run the Windows Executable:
.\build\apps\desktop\ReggaeWave_artefacts\Release\ReggaeWave.exe
```

> **Windows Audio Advantage:** JUCE uses the native **WASAPI** (Windows Audio Session API) subsystem, providing 44.1 kHz 24-bit exclusive and shared output with ultra-low latency directly to your default Windows audio device.

---

### 🐧 C. Building & Testing on Linux / WSL2

#### Prerequisites:
- **GCC 13+** or **Clang 16+**
- `libasound2-dev`, `libgtk-3-dev`, `libwebkit2gtk-4.1-dev`, `libcurl4-openssl-dev`, `ffmpeg`

#### Terminal Commands:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target ReggaeWave -j$(nproc)
ctest --test-dir build --output-on-failure

./build/apps/desktop/ReggaeWave_artefacts/ReggaeWave
```

---

## 5. Quality Checklist & Verification Criteria

| Test Area | Criteria | Expected Result |
| :--- | :--- | :--- |
| **Rights Attestation** | Enforces rights check before file analysis | Modal blocks bypass; records basis in manifest |
| **Audio Playback** | A/B Variation crossfading & transport | Shared playhead; equal-power crossfade |
| **Real-Time DSP** | Live knob adjustment (Dub FX, Gain, Intensity) | Instantaneous audible feedback |
| **Visualizer** | Upper-half single-sided spectrum with RGB gradient | Green at base, rising to Red at high transients |
| **Mastering Quality** | Mastered output loudness & peak | $-14.0 \pm 0.5$ LUFS integrated; $\le -1.0$ dBTP |
| **Subtitles** | Synchronized SRT and VTT generation | Generated in `exports/` when enabled |

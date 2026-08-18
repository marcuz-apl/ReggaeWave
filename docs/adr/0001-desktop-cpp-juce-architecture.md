# ADR 0001: Desktop Application Architecture using C++20 and JUCE 8

| Field | Value |
| --- | --- |
| Status | Approved |
| Date | 2026-08-19 |
| Authors | ReggaeWave Core Team |
| Deciders | Product Engineering |

## Context

ReggaeWave is a dedicated audio transformation system that converts rights-cleared musical input from any genre into an authentic Reggae arrangement. The MVP flow requires:
1. One-click conversion into genuine Reggae output (no other destination genres).
2. Exactly three creative tuning controls: Reggae intensity (0–100), Dub-effects amount (0–100), and Vocal level (-6 dB to +6 dB).
3. Seamless, synchronized A/B dual-variation preview playback without playback glitches or phase misalignments.
4. Preserving the original separated lead vocal without voice cloning or impersonation.
5. Real-time Dub audio effects (dynamic tape delay feedback, spring reverb modeling, and resonant filtering).
6. Cross-platform offline desktop operation across macOS (CoreAudio / Metal) and Windows (WASAPI / Direct2D).

Previous design iterations evaluated web/cloud stacks and multi-language hybrids (Tauri/Rust, Electron/Node, Python/Qt). A technology stack decision was required to determine the programming language and core framework for the desktop edition.

## Decision

We decide to build the ReggaeWave Desktop Edition using **C++20** and **JUCE 8** as the primary application and audio framework, structured as follows:

1. **User Interface & Application Host**: JUCE 8 component hierarchy with a custom hardware-accelerated dark theme (`juce::LookAndFeel_V4`), responsive vector rendering, interactive waveform visualizer (`juce::AudioThumbnail`), and keyboard-accessible controls adhering to WCAG 2.2 AA.
2. **Audio & DSP Engine**: Native `juce_audio_devices` and `juce_dsp` pipeline providing real-time sample-accurate playback, synchronized A/B dual-variation cross-fading, dynamic Dub delay/reverb sends, and loudness normalization meeting the -14 LUFS integrated / -1 dBTP true peak master target.
3. **ML & Inference Layer**: Native C++ ONNX Runtime (`onnxruntime-cxx`) for on-device stem separation (Demucs) and optional lyric transcription (Whisper), supporting CoreML Execution Provider on macOS and DirectML/TensorRT Execution Providers on Windows with CPU fallback.
4. **Media I/O & Decoding**: FFmpeg C API (`libavcodec`, `libavformat`, `libswresample`) and JUCE audio formats for multi-format decoding (MP3, WAV, M4A, FLAC) and canonical 44.1 kHz 24-bit PCM processing.
5. **Durable Local State**: SQLite3 (`sqlite_orm` or C API) managing transactional job states (`created → importing → validating → queued → normalizing → separating → analyzing → arranging → mixing → transcribing → preview_ready → completed`), variation selections, and export tasks with zero data races.

## Architecture Boundaries

```text
┌─────────────────────────────────────────────────────────────────────────┐
│                    ReggaeWave Desktop (C++20 / JUCE 8)                  │
├─────────────────────────────────────────────────────────────────────────┤
│  UI Layer (JUCE 8):                                                     │
│  - Modern Custom Dark Theme LookAndFeel                                 │
│  - Rights Attestation Dialog (Mandatory 3-basis selection)              │
│  - Interactive Waveform & Synchronized A/B Variation Switcher          │
│  - Three Dedicated Tuning Knobs (Intensity, Dub FX, Vocal Gain)         │
│  - Optional Lyric Editor (SRT / VTT / Visualizer Preview)               │
├─────────────────────────────────────────────────────────────────────────┤
│  Audio Engine (`juce_audio_basics`, `juce_audio_devices`, `juce_dsp`):   │
│  - Multi-track Lock-Free Ring Buffer & Transport Coordinator           │
│  - Real-time Dub Delay Line, Spring Reverb, & Resonant Filter Graph    │
│  - LUFS Integrated & True Peak Metering / Normalizer                   │
├─────────────────────────────────────────────────────────────────────────┤
│  Inference & Media Engine (C++):                                        │
│  - ONNX Runtime C++ (`onnxruntime-cxx`) for Stem Separation             │
│  - Reggae Arrangement & Pattern Generator (One-Drop, Bass, Skank)      │
│  - FFmpeg C API for Multi-Format Audio Decoding & Mastering Export      │
├─────────────────────────────────────────────────────────────────────────┤
│  Persistence:                                                           │
│  - Transactional SQLite3 State Machine & Artifact Manifests             │
└─────────────────────────────────────────────────────────────────────────┘
```

## Consequences

### Positive
- **Pro-Audio Performance**: Zero-latency real-time DSP, sample-accurate transport, and glitch-free A/B playback without WebAudio or IPC overhead.
- **Native Efficiency**: Compact binary footprint and predictable low memory footprint compared to webview/Electron runtimes.
- **Unified C++ Codebase**: Single language across UI, audio engine, DSP algorithms, and ONNX Runtime ML inference.
- **Plugin Path**: The core audio graph and DSP components can easily be compiled into VST3, AU, or CLAP plugin targets in future phases.

### Negative / Trade-offs
- **Custom UI Implementation**: JUCE UI requires explicit component drawing and custom LookAndFeel styling rather than declarative HTML/CSS.
- **Cross-Platform Toolchain Management**: Requires configuring CMake build pipelines for macOS (Xcode/Clang) and Windows (MSVC) with bundled native libraries.

## Rollback Path

If C++ / JUCE 8 development presents insurmountable UI development velocity constraints, the core C++ DSP and ONNX Runtime engine can be wrapped as a C-ABI shared library or sidecar process, and integrated into a Tauri v2 / Rust host with a web-based frontend.

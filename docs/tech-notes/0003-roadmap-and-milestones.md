# ReggaeWave Tech Note: Roadmap & Milestones (Phases 0–7)

| Field | Value |
| --- | --- |
| Date | 2026-08-19 |
| Status | Approved Baseline Roadmap |
| Target Stack | C++20, JUCE 8, ONNX Runtime C++, FFmpeg, SQLite3 |
| Platforms | macOS 13+ and Windows 11 x64 |

---

## 1. Roadmap Overview

```text
┌─────────────────────────────────────────────────────────────────────────┐
│                 ReggaeWave Desktop Development Lifecycle                 │
├─────────────────────────────────────────────────────────────────────────┤
│  Phase 0: Foundation, Architecture & Domain Contracts         [DONE]    │
│  Phase 1: Audio Intake, Decoding & Rights Attestation Gate    [DONE]    │
│  Phase 2: On-Device Stem Separation & Musical Analysis        [DONE]    │
│  Phase 3: Reggae Arrangement & Composition Engine             [NEXT]    │
│  Phase 4: Real-Time Tuning Controls, Dub FX & A/B Playback              │
│  Phase 5: Optional Subtitles, Lyric Editor & MP4 Visualizer             │
│  Phase 6: Mastering, Multi-Format Export & Project Storage              │
│  Phase 7: Cross-Platform Packaging & Cultural Quality Audit             │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Phase-by-Phase Breakdown

### **Phase 0: Foundation, Architecture & Domain Contracts** `[DONE]`
- [x] Modern CMake build system with `FetchContent` (JUCE 8, Catch2).
- [x] Domain contracts in `packages/contracts`:
  - `TuningParameters`: Bounds checking for the 3 controls (`[0-100]`, `[0-100]`, `[-6 to +6 dB]`).
  - `RightsAttestation`: Mandatory 3-basis verification (`Owned`, `Licensed`, `PublicDomain`).
  - `JobState`: Transactional state machine for conversion and export pipelines.
- [x] Core DSP prototypes in `packages/audio-engine`:
  - `DubEffectsProcessor`: Dotted-eighth tape delay with saturation & filter sweeps.
  - `DualTransportSource`: Equal-power A/B variation crossfading.
  - `LoudnessMeter`: BS.1770 -14 LUFS / -1 dBTP true peak analyzer.
- [x] Local storage baseline in `packages/storage`: `LocalDatabase`.
- [x] Automated unit test suite: 20 test cases, 76 assertions passing.

---

### **Phase 1: Audio Intake, Decoding & Rights Gate** `[DONE]`
*Goal: Accept, inspect, validate, and normalize input audio from any genre while strictly enforcing rights attestation.*

- [x] **1.1 Rights Attestation Gate**:
  - Modal UI workflow preventing file ingestion until user chooses a valid rights basis and confirms liability.
  - Audit logging with UTC timestamp, project ID, and policy version `2026.1`.
- [x] **1.2 Audio Stream Inspection & Validation**:
  - Decode PCM WAV formats directly with bit depth inspection.
  - Enforce constraints: duration $\le 10$ minutes, file size $\le 200$ MB, at least 1 decodable stereo/mono stream.
  - Sanitized validation error codes (`ValidationErrorCode`).
- [x] **1.3 Canonical Normalization**:
  - Resample and convert input stream into canonical 44.1 kHz, 32-bit floating-point PCM buffer via Cubic Hermite spline interpolation without mutating original file.
  - Mono-to-stereo channel duplication and 5.1 multichannel downmixing.
- [x] **1.4 Audio Test Fixtures**:
  - In-memory programmatic PCM WAV synthesizer in `packages/audio-fixtures`.
  - Comprehensive unit test suite: 32 test cases, 117 assertions passing.

---

### **Phase 2: On-Device Stem Separation & Musical Analysis** `[DONE]`
*Goal: Separate lead vocal from accompaniment locally and extract musical structure.*

- [x] **2.1 Native Stem Separation Pipeline**:
  - `StemSeparator`: Separates lead vocal from accompaniment while preserving original vocal identity, phrasing, and sample length.
  - Native execution provider abstraction (`CPU`, `CoreML`, `DirectML`, `CUDA`).
- [x] **2.2 Harmony & Musical Analysis Engine**:
  - `MusicAnalyzer`:
    - Tempo & Beat-Grid Tracking: Detects BPM (60–180 BPM range), downbeat bar positions, and beat intervals.
    - Key & Harmony Detection: 12-bin Pitch Class Profile / Chromagram with Krumhansl-Schmuckler correlation across 24 keys.
    - Chord Timeline Extraction: Generates diatonic chord progressions aligned to downbeats.
    - Structural Segmentation: Segments audio into `Intro`, `Verse`, `Chorus`, `Outro`.
    - Manifest Reporting: Populates `MusicalAnalysisManifest` with confidence scoring.
- [x] **2.3 Unit & Integration Tests**:
  - 36 test cases, 139 assertions passing.

---

### **Phase 3: Reggae Arrangement & Composition Engine**
*Goal: Synthesize genuine Reggae rhythm, bass, and accompaniment synchronized to the original song.*

- [ ] **3.1 Authentic Drum Synthesis**:
  - Generation of characteristic One-Drop (kick & rim on beat 3), Steppers (four-on-the-floor syncopated), and Rockers drum patterns.
  - Dynamic hi-hat ghost notes and percussion (Nyabinghi, shaker, cowbell).
- [ ] **3.2 Melodic Reggae Bassline Generation**:
  - Heavy sub-bass melodic composition locked to drum kick/snare and chord root/5th movements.
- [ ] **3.3 Offbeat Skank & Bubble Chords**:
  - Piano and guitar chop on upbeats (the "and" of 2 & 4).
  - Hammond organ bubble rolling patterns following chord changes.
- [ ] **3.4 Dual Variation Output Generation**:
  - **Variation A**: Classic Roots / One-Drop rhythm.
  - **Variation B**: Modern Steppers / Rub-a-Dub rhythm.

---

### **Phase 4: Real-Time Tuning Controls, Dub FX & A/B Playback**
*Goal: Provide interactive UI for real-time fine-tuning and comparison.*

- [ ] **4.1 Interactive Waveform & Transport**:
  - Multi-threaded waveform rendering with smooth playhead scrubbing.
  - Instantaneous, glitch-free A/B switching preserving exact timestamp alignment.
- [ ] **4.2 Real-Time Tuning Dials**:
  - **Reggae Intensity (0–100)**: Adjusts arrangement density, chord substitutions, and skank prominence.
  - **Dub-Effects Amount (0–100)**: Live tape delay sends, spring reverb splashes, and filter resonance sweeps.
  - **Vocal Gain (-6 dB to +6 dB)**: Real-time volume balancing of the preserved lead vocal.

---

### **Phase 5: Optional Subtitles, Lyric Editor & MP4 Visualizer**
*Goal: Generate and edit optional subtitles without blocking audio exports.*

- [ ] **5.1 Optional Transcription Pipeline**:
  - Whisper ONNX model execution (skipped by default unless user enables subtitles).
- [ ] **5.2 In-App Lyric Editor**:
  - Interactive transcript editor with timestamp adjustment.
  - Subtitle export to standard `.srt` and `.vtt` formats.
- [ ] **5.3 MP4 Lyric Visualizer**:
  - Offline video renderer producing 1080p H.264 video with synchronized lyrics and AAC audio.

---

### **Phase 6: Mastering, Multi-Format Export & Local Retention**
*Goal: Master output to pro-audio specifications and handle local file retention.*

- [ ] **6.1 Mastering Chain**:
  - Integrated loudness normalization to **-14.0 LUFS** ($\pm 1.0$ LUFS tolerance).
  - True peak ceiling limiter enforcing **-1.0 dBTP**.
- [ ] **6.2 Audio Exporters**:
  - 320 kbps MP3 stereo encoder.
  - 44.1 kHz 24-bit stereo WAV PCM encoder.
- [ ] **6.3 Local Retention & Privacy**:
  - Automatic deletion of intermediate separation stems after export.
  - Immediate user project deletion support.

---

### **Phase 7: Cross-Platform Packaging & Quality Audit**
*Goal: Package signed desktop bundles and conduct musical quality audits.*

- [ ] **7.1 macOS Distribution**:
  - Universal binary (Apple Silicon & Intel) notarized `.dmg` / `.app` bundle.
- [ ] **7.2 Windows Distribution**:
  - Signed Windows 11 x64 installer (`.msi` / Inno Setup) with DirectML runtime.
- [ ] **7.3 Reggae Practitioner Evaluation**:
  - Musical quality evaluation against the PRD rubric with qualified Reggae producers and Jamaican cultural heritage safeguards.

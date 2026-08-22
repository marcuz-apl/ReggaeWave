# Note 0005: Mobile Edition Architecture, Testing Strategy, and Developer Guide

**Document Status:** Approved & Active  
**Author:** Antigravity Team & Product Engineering  
**Applies To:** ReggaeWave Mobile (iOS & Android)  
**Related Specs:** [PRD.md](PRD.md), [spec-offline-desktop-architecture-design.md](spec-offline-desktop-architecture-design.md)

---

## 1. Executive Summary

This document captures the strategic design, developer onboarding notes, testing methodologies, and architectural decisions for extending **ReggaeWave** to mobile platforms (**Apple iOS** and **Google Android**).

By leveraging our modular C++20 audio architecture and JUCE 8 cross-platform framework, **over 85% of our codebase** (`packages/audio-engine/`, `packages/contracts/`, DSP filters, and stem generators) is directly shared between desktop and mobile targets with zero cloud dependencies and complete offline privacy.

---

## 2. First-Time Mobile Developer Guide: Testing Without Store Accounts

A primary concern when starting mobile development is:  
> *"How do we test mobile features without having an Apple App Store or Google Play Store developer account or paying fees?"*

You **do not need any store account, paid subscription, or approval** to build and test ReggaeWave on mobile. We employ a 4-tier testing strategy:

```text
┌─────────────────────────────────────────────────────────────────────────┐
│                     4-Tier Mobile Testing Matrix                        │
├───────────────────┬───────────────────┬─────────────────────────────────┤
│ Tier              │ Device Needed     │ Cost & Store Account Required?  │
├───────────────────┼───────────────────┼─────────────────────────────────┤
│ 1. PC Simulator   │ Any PC / Mac / Lin│ $0 (Zero accounts needed)       │
│ 2. Automated CI   │ GitHub Actions    │ $0 (Free via GitHub CI)         │
│ 3. Android Phone  │ Real Android Phone│ $0 (100% Free sideloading)      │
│ 4. iPhone / iPad  │ Real iPhone + Mac │ $0 (Free personal Apple ID)     │
└───────────────────┴───────────────────┴─────────────────────────────────┘
```

### Tier 1: Desktop Mobile Simulator Mode (Instant Local Testing)
* **How it works:** When running `ReggaeWaveMobile` on your development computer, the app opens in **Mobile Simulation Mode** locked to standard smartphone dimensions (iPhone 15: `393 x 852 px`).
* **What you test:**
  * Portrait single-column viewport touch scrolling.
  * 3-Way A/B/Original audition crossfade with real-time sound output.
  * Big rotary dial ergonomics (`Intensity`, `Dub FX`, `Vocal Gain`).
  * 1-Click `⚡ Denoise` source audio restoration.
* **Benefit:** Instant visual and audible feedback in 2 seconds without connecting a physical phone.

### Tier 2: Automated Headless C++ Test Suite (Continuous Integration)
* All core transformation math (audio decoding, stem separation, riddim generation, mastering, and MP3/WAV encoding) is tested headlessly by `reggaewave_tests` (58 test suites, 132,848 assertions).
* Every commit automatically proves mathematical correctness before packaging.

### Tier 3: Direct Android Device Sideloading (1-Tap Installation)
* Android allows direct app installation without the Google Play Store:
  1. Download the compiled `.apk` from GitHub Actions.
  2. Transfer the file to your phone via USB, WeChat, WhatsApp, Telegram, or Google Drive.
  3. Tap the `.apk` file on your phone → Tap **Install** → ReggaeWave runs as a native Android app!

### Tier 4: Direct iOS Device Installation (Free Personal Apple ID)
* Apple permits developers to run apps on their personal iPhone/iPad:
  * Connect your iPhone to a Mac via USB cable.
  * Open the generated Xcode project, sign in with your free Apple ID, and click **Run**.
  * The app installs and runs natively on your physical iPhone.

---

## 3. Mobile Touch Deck Architecture (`apps/mobile/`)

Mobile ergonomics require rethinking the 3-card layout from horizontal desktop monitors into a **vertical portrait touch flow**:

```text
┌───────────────────────────────────────────────────────────┐
│ [Icon]  ReggaeWave Mobile          [Rights] [Help] [About]│
├───────────────────────────────────────────────────────────┤
│                                                           │
│ 1. Intake & AI Stems                                      │
│ ┌───────────────────────────────────────────────────────┐ │
│ │ 🎵  Tap to Select Audio File   │  [ ⚡ Denoise: ON ]  │ │
│ └───────────────────────────────────────────────────────┘ │
│  Track: Soul_Rebel.mp3 [C Major • 76.0 BPM • 185s]        │
│                                                           │
│ 2. Riddim & Dub Studio (3-Way Audition)                   │
│ ┌───────────────────────────────────────────────────────┐ │
│ │ ~~~~~~~~~~~~ [ DANCING WAVEFORM SPECTRUM ] ~~~~~~~~~~ │ │
│ └───────────────────────────────────────────────────────┘ │
│ ┌─────────────────────────┬─────────────────────────────┐ │
│ │       ▶  Play           │         ↺  Rewind           │ │
│ ├─────────────┬───────────┴─────────────┬───────────────┤ │
│ │  Original   │     Var A: One-Drop     │Var B: Steppers│ │
│ └─────────────┴─────────────────────────┴───────────────┘ │
│                                                           │
│   ( 70% ) Intensity   ( 20% ) Dub FX   ( 0.0dB ) Vocal    │
│                                                           │
│ 3. Master & Export                                        │
│ ┌───────────────────────────┬───────────────────────────┐ │
│ │     Export MP3 (320k)     │     Export WAV (24-bit)   │ │
│ └───────────────────────────┴───────────────────────────┘ │
│  [✓] Embed Subtitles (.srt / .vtt)                        │
│                                                           │
└───────────────────────────────────────────────────────────┘
```

---

## 4. Automated Cross-Platform CI/CD Pipeline

The GitHub Actions workflow (`.github/workflows/build-and-release.yml`) builds and packages 5 platform artifacts in parallel:

1. **Windows (x64):** `ReggaeWave-<ver>-Windows-Setup.exe` (NSIS dual-track installer)
2. **macOS (Universal):** `ReggaeWave-<ver>-macOS-Universal.dmg` (Apple Silicon & Intel DMG)
3. **Linux (x86_64):** `ReggaeWave-<ver>-Linux-x86_64.deb` and `.rpm`
4. **Android (ARM64):** `ReggaeWave-<ver>-Android-ARM64` (Native NDK binary & APK)
5. **iOS (Simulator):** `ReggaeWave-<ver>-iOS-Simulator.zip` (Xcode iOS 15.0+ bundle)

---

## 5. Phase-by-Phase Mobile Action Plan

* **Phase M1 (Completed):** Established `apps/mobile/` target structure and `MobileMainComponent` touch deck.
* **Phase M2 (Completed):** Automated cross-compilation CI pipeline for Android NDK and iOS Simulator.
* **Phase M3 (Upcoming):** Native Mobile OS Storage & File Sharing (`UIDocumentPicker`, iOS Share Sheet `UIActivityViewController`, and Android `Intent.ACTION_SEND`).
* **Phase M4 (Upcoming):** Low-latency mobile audio backends (`AVAudioSession` on iOS and `AAudio/Oboe` on Android).
* **Phase M5 (Future):** Production app store signing and submission guidelines.

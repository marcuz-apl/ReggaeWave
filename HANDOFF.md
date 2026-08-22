# Project Handoff

Updated: 2026-08-22 18:14 UTC
Branch: master
Commit: fc373d3 (1.5.1-260822j)
Status: blocked

## Summary

ReggaeWave multi-platform release CI is configured on GitHub Actions across Desktop (Linux, macOS Universal, Windows x64) and Mobile (iOS Simulator, Android ARM64). Desktop (3 targets) and iOS (1 target) builds pass completely (100% green). The Android ARM64 NDK CMake cross-compilation job remains blocked during CI execution in the `Configure and Compile Android ARM64` step.

## Completed

- **Core Audio Engine & Contracts**: Full suite of 58 automated unit and integration tests (132,848 assertions) passing 100% locally (`cmake -B build -DREGGAEWAVE_BUILD_TESTS=ON && ctest`).
- **Desktop Multi-Platform**: Linux x86_64, macOS Universal (arm64 + x86_64), and Windows x64 binaries build and package cleanly on CI.
- **iOS Simulator Bundle**: iOS mobile bundle compiles and passes on CI runner.
- **CPack Packaging Guard**: Guarded `CPackConfig.cmake` behind `if(REGGAEWAVE_BUILD_DESKTOP)` in root `CMakeLists.txt` so mobile builds without desktop targets do not fail packaging evaluation.
- **String Literal Defines Escaping**: Properly escaped quotation marks for `REGGAEWAVE_APP_VERSION_STRING` in `CMakeLists.txt` to prevent Ninja from stripping quotes.
- **Native OpenSL ES Configuration**: Added `JUCE_USE_ANDROID_OPENSLES=1` and `JUCE_USE_ANDROID_OBOE=0` definitions in `apps/mobile/CMakeLists.txt`.

## In progress

- Android NDK ARM64 binary build stabilization in `.github/workflows/build-and-release.yml` and `apps/mobile/CMakeLists.txt`.

## Working tree

- Clean (`master` up to date with `origin/master`).

## Checks

- `ctest --test-dir build --output-on-failure` — PASS (58/58 tests passed, 0 failures, 132,848 assertions verified).
- `Build Desktop on Linux (x86_64)` (CI) — PASS.
- `Build Desktop on macOS (Universal)` (CI) — PASS.
- `Build Desktop on Windows (x64)` (CI) — PASS.
- `Build iOS Mobile (Simulator Bundle)` (CI) — PASS.
- `Build Android NDK Binary` (CI) — FAIL (Step 7: `Process completed with exit code 1`).

## Decisions and context

- Fixed destination genre is strictly Reggae (no target genre selector).
- Exactly 3 user tuning controls (Reggae intensity, Dub-effects amount, Vocal level).
- Separated lead vocal must be preserved (no voice cloning or replacement).
- All desktop and iOS builds are gated behind `refs/tags/v*` on CI to speed up iteration on `master`.

## Blockers

- **Android NDK Compilation Error on GitHub Actions Runner**:
  ```text
  Build Android NDK Binary
  Process completed with exit code 1.

  Build Android NDK Binary
  Node.js 20 is deprecated. The following actions target Node.js 20 but are being forced to run on Node.js 24: actions/checkout@v4, actions/upload-artifact@v4. For more information see: https://github.blog/changelog/2025-09-19-deprecation-of-node-20-on-github-actions-runners/

  Build Android NDK Binary
  No files were found with the provided path: dist-android/*. No artifacts will be uploaded.
  ```

## Next action

1. Retrieve the detailed raw compiler/linker log output from GitHub Actions for step 7 of the Android build job to pinpoint the exact undefined symbol, header mismatch, or CMake toolchain error in `build-android`.
2. Inspect whether `juce_gui_basics` requires custom Gradle integration or specific Android NDK sysroot headers for `ComponentPeerView` JNI byte-code linking on Android.

## Resume notes

- Local test execution:
  ```bash
  cmake -B build -DREGGAEWAVE_BUILD_TESTS=ON
  cmake --build build --target reggaewave_tests
  ctest --test-dir build --output-on-failure
  ```
- Workflow file: `.github/workflows/build-and-release.yml`
- Mobile CMake definition: `apps/mobile/CMakeLists.txt`

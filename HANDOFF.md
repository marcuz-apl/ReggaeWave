# Project Handoff

Updated: 2026-08-24 05:46 UTC
Branch: codex/task-0-android-packaging
Commit: be07e7d
Status: in progress

## Summary

ReggaeWave mobile packaging is being maintained for Android and iOS. Android native library page alignment is fixed, but Android audio import remains unverified and was previously reported by the user as unable to decode both MP3 and M4A. Mobile version metadata has now been aligned with `VERSION` (`v1.6.6-260823q`) locally.

## Completed

- Added Android document-picker URI handling in `apps/mobile/Source/Platform/NativeMobileSharing.cpp` using `FileChooser::getURLResult()` so `content://` references are preserved.
- Added Android `MediaExtractor`/`MediaCodec` decoding and PCM conversion in `packages/audio-engine/include/reggaewave/audio/AudioDecoder.hpp`.
- Added 16 KB ELF page alignment linker flags in `apps/mobile/Builds/Android/app/CMakeLists.txt`; the user reported the launch alignment warning is gone.
- Updated mobile version metadata and About-version normalization in:
  - `apps/mobile/ReggaeWaveMobile.jucer`
  - `apps/mobile/Builds/Android/app/CMakeLists.txt`
  - `apps/mobile/Builds/Android/app/src/main/AndroidManifest.xml`
  - `apps/mobile/JuceLibraryCode/JuceHeader.h`
  - `apps/desktop/Source/UI/AboutDialogModal.cpp`
- Added decoder regression coverage in `tests/audio/AudioDecoderTests.cpp`.

## In progress

- Confirm the latest Android APK on a real emulator/device. The connected emulator was unavailable during the last local installation attempt.
- Determine why the user still sees the generic decode error if it persists with the latest APK; do not resume blind decoder changes without exact APK identity and Android logcat evidence.
- iOS testing is currently simulator-only through the macOS GitHub Actions job; the workflow builds iOS only when the ref is a `v*` tag.

## Working tree

- Uncommitted changes exist in the Android picker/decoder, mobile version metadata, About dialog, and decoder tests.
- `docs/assets/reggaewave-ui.png` has an unrelated user modification and must remain untouched.
- Nothing from the current work has been committed or pushed.

## Checks

- `cmake --build build --config Debug --target reggaewave_tests -j 1` — PASS
- `build/tests/Debug/reggaewave_tests.exe '[decoder]'` — PASS, 25 assertions in 5 test cases
- `gradlew.bat --no-daemon :app:assembleDebug_Debug` — PASS
- Android APK metadata inspection with `aapt dump badging` — PASS; versionCode `10607`, versionName `1.6.7-2608241`
- Mobile stale-version check for `1.5.0`, `0x10500`, and `10500` — PASS
- `git diff --check` — PASS
- Physical Android MP3/M4A import verification — NOT RUN; emulator was unavailable
- iOS build/test — NOT RUN on Windows; requires macOS/Xcode or the macOS GitHub Actions runner

## Decisions and context

- The canonical version source is the root `VERSION` file. Mobile generated metadata must be updated whenever the version changes; Android currently uses numeric versionCode `10607` and versionName `1.6.7-2608241`.
- Android/iOS About text normalizes the `v` prefix so it works whether the build definition includes it or not.
- JUCE mobile file pickers return URLs rather than guaranteed local files; Android document URLs must be preserved and read through the Android document stream bridge.
- Do not push or commit the handoff automatically.

## Blockers

- Android audio decoding has not been conclusively validated on-device. The user previously reported the same generic error for MP3 and M4A.
- iOS cannot be run or built natively from Windows. The existing GitHub Actions iOS job is gated to version tags and produces an unsigned simulator bundle.

## Next action

1. Decide whether to commit/push the current local changes. Then produce a new GitHub Actions build and verify the exact artifact hash before testing Android.

## Resume notes

- Android local build directory: `apps/mobile/Builds/Android`
- Recommended Windows Android build environment: Java `C:\Program Files\Java\jdk-21.0.12`, Android SDK `C:\Users\MZou\AppData\Local\Android\Sdk`, and a task-specific `GRADLE_USER_HOME`.
- Local APK output: `apps/mobile/Builds/Android/app/build/outputs/apk/debug_/debug/app-debug_-debug.apk`
- iOS workflow: `.github/workflows/build-and-release.yml`, job `build-ios`; it runs on `macos-latest` only for refs beginning with `refs/tags/v`.

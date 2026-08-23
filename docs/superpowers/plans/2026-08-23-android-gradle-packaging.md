# Android Gradle Packaging Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Package ReggaeWave as an installable Android APK/AAB for local testing and CI without publishing, signing, or pushing.

**Architecture:** `apps/mobile/ReggaeWaveMobile.jucer` owns JUCE Android exporter metadata. Its tracked Android Studio export at `apps/mobile/Builds/Android/` owns Gradle, manifest, Java/JUCE glue, and native build integration. The existing top-level CMake build remains for desktop and C++ tests.

**Tech Stack:** JUCE 8.0.4 Projucer Android Studio exporter, Gradle, Android SDK API 29+, NDK r26b, CMake, Bash checks, GitHub Actions.

**Spec:** `docs/superpowers/specs/2026-08-23-android-gradle-packaging-design.md`

## Global Constraints

- Application id: `com.alfazen.reggaewave`; product name: ReggaeWave.
- API 29 minimum. Debug APK supports `x86_64` and `arm64-v8a`; CI release APK/AAB supports `arm64-v8a`.
- Never commit credentials, keystores, `local.properties`, SDK paths, Gradle caches, build output, or user media.
- Do not add Play Store publication, signing secrets, tags, pushes, or releases. Local commits on the isolated implementation branch are permitted for task review and remain unpushed.

---

### Task 0: Record the approved Android packaging scope

**Files:** Modify `docs/PRD.md`.

**Produces:** a source-of-truth record of the approved exception to the initial native-client MVP non-goal.

- [x] **Step 1: Amend the non-goal statement**

Replace the blanket Android native-client non-goal with the approved scope: Android local testing and CI packaging is in scope; Play Store distribution remains out of scope.

- [x] **Step 2: Verify the product boundary**

Run `rg -n 'Native iOS, Android|Android.*Play Store|Android.*CI' docs/PRD.md` and confirm the PRD retains the responsive web/PWA as the initial product while accurately recording this approved Android packaging work.

---

### Task 1: Android project contract test

**Files:** Create `tests/android/test-gradle-project.sh`; create `tests/android/README.md`.

**Produces:** a portable Bash exit-code contract for the generated Android project.

- [x] **Step 1: Write the failing test**

Create `tests/android/test-gradle-project.sh` to require these files:

```sh
#!/usr/bin/env bash
set -euo pipefail
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
android="$root/apps/mobile/Builds/Android"
test -x "$android/gradlew"
test -f "$android/settings.gradle"
test -f "$android/app/build.gradle"
test -f "$android/app/src/main/AndroidManifest.xml"
grep -q 'com.alfazen.reggaewave' "$android/app/build.gradle"
grep -Eq 'minSdk(V|ersion)?[[:space:]]+29' "$android/app/build.gradle"
grep -q 'arm64-v8a' "$android/app/build.gradle"
grep -q 'x86_64' "$android/app/build.gradle"
```

- [x] **Step 2: Verify RED**

Run `bash tests/android/test-gradle-project.sh`. Expect failure because the Gradle project does not exist.

- [x] **Step 3: Document the test**

Describe the contract and Git Bash invocation in `tests/android/README.md`.

### Task 2: Generate the JUCE Android Studio project

**Files:** Create `apps/mobile/ReggaeWaveMobile.jucer`; create tracked `apps/mobile/Builds/Android/`; modify `.gitignore`.

**Produces:** a Gradle `app` module that packages the existing mobile C++ sources through JUCE’s Android exporter.

- [x] **Step 1: Create the Projucer definition**

Create a GUI application project named `ReggaeWaveMobile` with bundle identifier `com.alfazen.reggaewave`, company `Alfazen-Inc`, Android minimum SDK 29, and the existing `apps/mobile/Source/` plus shared source membership from `apps/mobile/CMakeLists.txt`. Set `JUCE_USE_ANDROID_OPENSLES=1`, `JUCE_USE_ANDROID_OBOE=0`, and `REGGAEWAVE_MOBILE=1`.

- [x] **Step 2: Export Android Studio files**

Use a Projucer built from JUCE 8.0.4 to generate `apps/mobile/Builds/Android/`, including `gradlew`, `settings.gradle`, `app/build.gradle`, and `app/src/main/AndroidManifest.xml`.

- [x] **Step 3: Set ABI and signing behavior**

Configure debug packaging for `x86_64` and `arm64-v8a`; configure release packaging for `arm64-v8a`. Keep Android tooling’s debug keystore and generate unsigned release APK/AAB unless separately authorized secrets are configured.

- [x] **Step 4: Bootstrap the pinned JUCE source locally**

Use an ignored `apps/mobile/third_party/JUCE/` checkout at tag `8.0.4` so the Projucer project and generated Gradle paths stay project-relative. CI and developer setup must recreate this checkout before building; do not commit JUCE sources or absolute module paths.

- [x] **Step 5: Ignore local-only Android files**

Add these exact ignore entries while retaining wrapper and source files:

```gitignore
apps/mobile/Builds/Android/.gradle/
apps/mobile/Builds/Android/local.properties
apps/mobile/Builds/Android/**/build/
apps/mobile/third_party/JUCE/
*.keystore
*.jks
```

- [x] **Step 6: Verify GREEN**

Run `bash tests/android/test-gradle-project.sh`; expect exit 0.

### Task 3: Build and inspect packages on Windows

**Files:** Modify `tests/android/test-gradle-project.sh`.

**Produces:** validated debug APK and release AAB outputs.

- [x] **Step 1: Make Gradle execution opt-in**

Append this test block:

```sh
if [ "${REGGAEWAVE_RUN_ANDROID_BUILD:-0}" = "1" ]; then
  (cd "$android" && ./gradlew --no-daemon :app:assembleDebug :app:bundleRelease)
  find "$android/app/build/outputs/apk" -name '*.apk' -type f | grep -q .
  find "$android/app/build/outputs/bundle" -name '*.aab' -type f | grep -q .
fi
```

- [x] **Step 2: Verify environment failure before Android Studio is ready**

Run `$env:REGGAEWAVE_RUN_ANDROID_BUILD='1'; bash tests/android/test-gradle-project.sh`. Expect a specific Gradle/SDK/NDK setup error if required Android Studio components are unavailable.

- [x] **Step 3: Configure Android Studio locally**

Open `apps/mobile/Builds/Android/`, allow Android Studio to create ignored `local.properties`, install SDK API 29+, NDK r26b, CMake, and an `x86_64` emulator image, then sync Gradle.

- [x] **Step 4: Verify package output**

Repeat the opt-in test. Expect a debug APK containing `x86_64` and `arm64-v8a` libraries plus a release AAB.

### Task 4: Replace CI library artifacts with APK/AAB artifacts

**Files:** Modify `.github/workflows/build-and-release.yml`.

**Produces:** Android CI artifacts that contain only installable APK/AAB packages.

- [x] **Step 1: Establish the current CI gap**

Confirm the existing Android workflow discovers `*.so` and has no required APK/AAB output check.

- [x] **Step 2: Use Gradle packaging in CI**

Replace the Android release-path build invocation with `./gradlew --no-daemon :app:assembleRelease :app:bundleRelease` from `apps/mobile/Builds/Android/`. Copy only `*.apk` and `*.aab` from `app/build/outputs/` to `dist-android/`; fail if either package type is absent.

- [x] **Step 3: Run packaging contract before upload**

Add `bash tests/android/test-gradle-project.sh` before artifact upload. Remove `.so` discovery from the release path.

- [x] **Step 4: Verify YAML and contract**

Run `bash tests/android/test-gradle-project.sh` and `git diff --check`; both must exit 0.

### Task 5: Correct developer documentation

**Files:** Modify `README.md`; modify `docs/note-0005-mobile-edition-architecture-testing-and-roadmap.md`.

**Produces:** accurate Android Studio, emulator, device, and artifact guidance.

- [x] **Step 1: Document exact Windows setup and commands**

Add Android Studio components, the `apps/mobile/Builds/Android/` path, `assembleDebug`, `bundleRelease`, emulator ABI, physical-device USB-debugging flow, and debug APK sideloading without a store account.

- [x] **Step 2: Remove stale loose-library claims**

Replace statements that describe a native NDK binary as a releasable Android artifact with the verified APK/AAB names; keep iOS Simulator instructions macOS-only.

- [x] **Step 3: Verify documentation consistency**

Run `rg -n 'libReggaeWaveMobile\.so|Native NDK binary & APK|Builds/Android|assembleDebug|bundleRelease' README.md docs` and `git diff --check`. No stale `.so` release claim may remain; any `.so` mention must identify it as an internal payload.

### Task 6: Final local verification

**Files:** Verify `tests/android/test-gradle-project.sh`, generated Android packages, and C++ tests.

**Produces:** local evidence only; no commit, tag, push, or release action.

- [x] **Step 1: Run Android checks**

Run `bash tests/android/test-gradle-project.sh` and `$env:REGGAEWAVE_RUN_ANDROID_BUILD='1'; bash tests/android/test-gradle-project.sh`.

- [x] **Step 2: Run the C++ suite**

Run `ctest --test-dir build-local-verify -C Debug --output-on-failure`. If absent, configure a fresh ignored verification directory with `/FS` before CTest.

- [x] **Step 3: Inspect packages**

Use `aapt dump badging` and `unzip -l` to verify `com.alfazen.reggaewave`, both debug ABI library directories, and the ARM64 AAB native library directory.

- [x] **Step 4: Confirm local-only state**

Run `git status --short --branch` and `git diff --check`. Intended changes remain local and uncommitted.

# Testing the iOS Edition from Windows

This guide explains how to coordinate ReggaeWave iOS testing from a Windows development computer. Windows can start and inspect the GitHub Actions build, download the iOS Simulator artifact, and manage test evidence. Running the app still requires macOS because Apple distributes Xcode and the iOS Simulator for supported macOS versions.

The repository's iOS artifact is an unsigned `ReggaeWave.app` bundle for the iOS Simulator. It is not an App Store package, an `.ipa`, or an application that can run directly on Windows or a physical iPhone.

## Testing paths

Use these paths for different kinds of evidence:

1. **Windows plus GitHub Actions** verifies that the shared C++ and JUCE mobile sources compile for the iOS Simulator and that the simulator bundle can be packaged.
2. **Windows plus a remote or local Mac** provides interactive Simulator testing of import, conversion, playback, tuning, and export.
3. **A Mac plus a physical iPhone** is required for signed on-device testing. The unsigned Simulator artifact cannot be installed on an iPhone.

Apple lists the supported macOS versions for each Xcode and Simulator release at <https://developer.apple.com/xcode/system-requirements>. GitHub documents its available macOS-hosted runners at <https://docs.github.com/en/actions/reference/runners/github-hosted-runners>.

## Prerequisites on Windows

- Git and the GitHub CLI (`gh`)
- Access to the ReggaeWave GitHub repository and Actions artifacts
- An authenticated GitHub CLI session, confirmed with `gh auth status`
- For interactive testing, access to a Mac with a supported Xcode installation

Run commands from the ReggaeWave repository root in PowerShell.

## Verify the iOS build in GitHub Actions

The `build-ios` job is defined in `.github/workflows/build-and-release.yml` and runs on `macos-latest`. It configures an unsigned iOS Simulator build with a deployment target of iOS 15.0, packages `ReggaeWave.app`, and uploads a ZIP artifact.

The job currently runs only for Git refs beginning with `refs/tags/v`. A normal branch push or manual dispatch does not execute the iOS job. Use the project's approved version-and-release process when a fresh tagged build is required; do not create a disposable test tag because a `v*` tag also activates the full release workflow.

List recent workflow runs:

```powershell
gh run list --workflow build-and-release.yml --limit 10
```

Inspect the jobs and conclusion for a selected run:

```powershell
$runId = 32696223128
gh run view $runId
```

The known successful `v1.6.8-2608242` run is `32696223128`. Download its iOS Simulator artifact:

```powershell
$runId = 32696223128
$tag = 'v1.6.8-2608242'
$destination = 'C:\tmp\ReggaeWave-iOS'

gh run download $runId `
  --name "ReggaeWave-$tag-iOS-Simulator" `
  --dir $destination
```

Confirm that the download contains a ZIP whose payload is `ReggaeWave.app`. The successful CI job proves compilation and packaging only; it does not prove interactive runtime behavior.

## Run the artifact on a Mac

Transfer the downloaded ZIP to a local, hosted, or remote Mac. On that Mac, open Terminal in a temporary test directory and extract it:

```bash
unzip ReggaeWave-v1.6.8-2608242-iOS-Simulator.zip
```

Open Simulator and choose an available iPhone running iOS 15 or later:

```bash
open -a Simulator
xcrun simctl list devices available
```

After the chosen Simulator has booted, wait for it to become ready, install the app, and launch the ReggaeWave bundle identifier:

```bash
xcrun simctl bootstatus booted -b
xcrun simctl install booted ReggaeWave.app
xcrun simctl launch booted com.alfazen.reggaewave
```

If no Simulator is booted, create or select one in Xcode under **Window → Devices and Simulators**, then rerun the commands.

To remove the test installation before retesting a new artifact:

```bash
xcrun simctl uninstall booted com.alfazen.reggaewave
```

## Prepare authorized test audio

Use only programmatically synthesized, owned, explicitly licensed, or public-domain audio with an authorized recording. Do not use commercial recordings as test inputs.

For a quick synthesized WAV fixture on the Mac, use an installed FFmpeg build:

```bash
ffmpeg -hide_banner -loglevel error \
  -f lavfi -i 'sine=frequency=440:duration=2' \
  -ar 44100 -ac 2 -c:a pcm_s16le \
  reggaewave-synthetic.wav
```

Drag the synthesized file into the Simulator to make it available to the iOS document picker.

## Interactive smoke test

Verify each item and retain the Simulator, Xcode, and app logs for failures:

- ReggaeWave launches without a crash at the selected Simulator width.
- The rights attestation is visible, required, and not preselected or bypassable.
- A synthesized or otherwise authorized audio file can be selected and decoded.
- A low-confidence musical analysis warns and continues rather than rejecting the input genre.
- The only creative controls are Reggae intensity, Dub-effects amount, and vocal level.
- The conversion creates two duration-aligned variations.
- Switching variations preserves the playback timestamp.
- The separated lead vocal is preserved without singer replacement or cloning.
- Subtitles are disabled for a new project until explicitly enabled.
- MP3 and WAV audio exports work independently of subtitle state.
- SRT, VTT, and MP4 lyric-visualizer exports appear only after subtitles are enabled.
- Cancellation, relaunch, VoiceOver focus, and touch targets work without trapping the user.

## Capture diagnostics on the Mac

Stream logs for the booted Simulator while reproducing a failure:

```bash
xcrun simctl spawn booted log stream \
  --level debug \
  --predicate 'process == "ReggaeWave" OR subsystem CONTAINS "com.alfazen.reggaewave"'
```

Capture the installed application container when investigating local files or exports:

```bash
xcrun simctl get_app_container booted com.alfazen.reggaewave
```

Do not copy user audio, filenames, lyrics, signed URLs, or other private media into issue reports or repository logs. Record the app version, Simulator device and iOS version, exact reproduction steps, sanitized error text, and relevant stack trace.

## Physical iPhone limitation

Testing on a physical iPhone requires Xcode on a Mac, an Apple ID or development team configuration, code signing, and a device provisioning workflow. Build a signed device target from source on the Mac. Do not attempt to install the unsigned Simulator ZIP on a physical device.

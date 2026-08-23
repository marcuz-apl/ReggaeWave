# Android Edition Testing on Windows 11

This guide explains how to test the downloaded ReggaeWave Android build with Android Studio on Windows 11.

Use the **debug APK** for installation and device testing. The release AAB is unsigned and is intended for later signing and distribution tooling; it is not installed directly with `adb`.

## Test with an Android Studio emulator

1. Open Android Studio and select **Device Manager**.
2. Choose **Create Virtual Device**.
3. Select a Pixel phone and an `x86_64` system image.
4. API 34 or newer is suitable for the debug APK.
5. Start the emulator.

Android's official emulator guide is available at <https://developer.android.com/studio/run/emulator>.

## Install the debug APK

The simplest method is to drag the downloaded `.apk` file onto the running emulator.

Alternatively, install it from PowerShell:

```powershell
$adb = "$env:LOCALAPPDATA\Android\Sdk\platform-tools\adb.exe"

& $adb devices
& $adb install -r "C:\Path\To\ReggaeWave-debug.apk"
& $adb shell monkey -p com.alfazen.reggaewave 1
```

If more than one device is connected, use its identifier with `adb -s <device-id>`.

## Test on a physical Android phone

1. Enable **Developer options** on the phone.
2. Enable **USB debugging**.
3. Connect the phone by USB.
4. Accept the USB-debugging authorization prompt.
5. Run the same `adb devices` and `adb install` commands above.

The debug APK contains both `arm64-v8a` and `x86_64` native libraries. A physical phone normally uses ARM64; an emulator should use an x86_64 image.

Android's hardware-device guide is available at <https://developer.android.com/studio/run/device.html>.

## Smoke-test checklist

Verify that:

- the app launches without crashing;
- the portrait mobile interface is displayed correctly;
- audio-file selection opens;
- the rights attestation is visible and cannot be bypassed;
- Reggae Intensity, Dub Effects, and Vocal Level respond;
- playback and variation switching work;
- back navigation and relaunch work; and
- no crash occurs after rotating or backgrounding the app.

## Inspect the package

In Android Studio, select **Build → Analyze APK** and open the downloaded APK. Confirm:

```text
Package: com.alfazen.reggaewave
lib/arm64-v8a/libjuce_jni.so
lib/x86_64/libjuce_jni.so
```

Android Studio's APK Analyzer can inspect APK and Android App Bundle contents: <https://developer.android.com/studio/debug/apk-analyzer.html>.

## Capture crash logs

```powershell
& $adb logcat -c
& $adb shell monkey -p com.alfazen.reggaewave 1
& $adb logcat -d -v time | Select-String "AndroidRuntime|com.alfazen.reggaewave"
```

If installation reports a signature conflict, remove the previous debug installation and retry:

```powershell
& $adb uninstall com.alfazen.reggaewave
& $adb install "C:\Path\To\ReggaeWave-debug.apk"
```

#!/usr/bin/env bash
set -euo pipefail
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
android="$root/apps/mobile/Builds/Android"
test -x "$android/gradlew"
test -f "$android/settings.gradle"
test -f "$android/app/build.gradle"
test -f "$android/app/src/main/AndroidManifest.xml"
grep -q 'com.alfazen.reggaewave' "$android/app/build.gradle"
grep -Eq 'minSdk(Version)?[[:space:]]+(=[[:space:]]*)?29([[:space:]]|$)' "$android/app/build.gradle"
grep -q 'arm64-v8a' "$android/app/build.gradle"
grep -q 'x86_64' "$android/app/build.gradle"
bash "$root/tests/android/test-mobile-ui-text.sh"

if [ "${REGGAEWAVE_RUN_ANDROID_BUILD:-0}" = "1" ]; then
  if command -v cmd.exe >/dev/null 2>&1; then
    (cd "$android" && cmd.exe /d /s /c "gradlew.bat --no-daemon :app:assembleDebug_Debug :app:bundleRelease_Release")
  else
    (cd "$android" && ./gradlew --no-daemon :app:assembleDebug_Debug :app:bundleRelease_Release)
  fi

  find "$android/app/build/outputs/apk" -name '*.apk' -type f | grep -q .
  find "$android/app/build/outputs/bundle" -name '*.aab' -type f | grep -q .
fi

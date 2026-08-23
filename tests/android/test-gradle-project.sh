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

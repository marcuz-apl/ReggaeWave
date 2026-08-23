#!/usr/bin/env bash
set -euo pipefail
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
mobile="$root/apps/mobile/Source/MobileMainComponent.h"
source="$root/apps/mobile/Source/MobileMainComponent.cpp"

# Android can render emoji and symbol glyphs inconsistently across device fonts.
! grep -nE '🎵|⚡|▶|⏸|↺' "$mobile" "$source"

#!/bin/sh

set -eu

[ "$#" -eq 2 ] || {
  printf 'alfazen-versioning: usage: stamp-message.sh MESSAGE_FILE VERSION_FILE\n' >&2
  exit 1
}

message_file=$1
version_file=$2
script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
version=$(
  "$script_directory/version.sh" validate "$version_file" "$(date -u +%y%m%d)"
)
temporary_message="$message_file.alfazen.$$"
trap 'rm -f -- "$temporary_message"' EXIT HUP INT TERM

awk -v prefix="$version" '
  BEGIN { stamped = 0 }
  {
    line = $0
    if (!stamped && line !~ /^[ \t]*#/ && line !~ /^[ \t]*$/) {
      sub(/^[0-9][0-9]*\.[0-9]\.[0-9]-[0-9][0-9][0-9][0-9][0-9][0-9][1-9a-z][ \t]+/, "", line)
      print prefix " " line
      stamped = 1
    } else {
      print line
    }
  }
  END {
    if (!stamped) {
      exit 2
    }
  }
' "$message_file" > "$temporary_message" || {
  printf 'alfazen-versioning: commit message has no subject line\n' >&2
  exit 1
}

mv -- "$temporary_message" "$message_file"
trap - EXIT HUP INT TERM

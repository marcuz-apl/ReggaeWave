#!/bin/sh

set -eu
set -f
LC_ALL=C
export LC_ALL

die() {
  printf 'alfazen-versioning: %s\n' "$1" >&2
  exit 1
}

decimal_value() {
  decimal_input=$1
  while [ "${decimal_input#0}" != "$decimal_input" ]; do
    decimal_input=${decimal_input#0}
  done
  [ -n "$decimal_input" ] || decimal_input=0
  printf '%s\n' "$decimal_input"
}

date_number() {
  date_value=$1
  [ "${#date_value}" -eq 6 ] || return 1
  case "$date_value" in
    *[!0-9]*) return 1 ;;
  esac

  date_yy=${date_value%????}
  date_tail=${date_value#??}
  date_mm=${date_tail%??}
  date_dd=${date_tail#??}
  date_year_short=$(decimal_value "$date_yy")
  date_month=$(decimal_value "$date_mm")
  date_day=$(decimal_value "$date_dd")
  date_year=$((2000 + date_year_short))

  case "$date_month" in
    1|3|5|7|8|10|12) date_max_day=31 ;;
    4|6|9|11) date_max_day=30 ;;
    2)
      if [ $((date_year % 400)) -eq 0 ] || { [ $((date_year % 4)) -eq 0 ] && [ $((date_year % 100)) -ne 0 ]; }; then
        date_max_day=29
      else
        date_max_day=28
      fi
      ;;
    *) return 1 ;;
  esac

  [ "$date_day" -ge 1 ] && [ "$date_day" -le "$date_max_day" ] || return 1
  printf '%s\n' $((date_year_short * 10000 + date_month * 100 + date_day))
}

validate_version() {
  version_file=$1
  today=$2

  [ -f "$version_file" ] || die "missing VERSION file: $version_file"
  [ "$(awk 'END { print NR }' "$version_file")" -eq 1 ] || die 'VERSION must contain exactly one line'
  current=$(sed -n '1p' "$version_file")

  old_ifs=$IFS
  IFS=-
  set -- $current
  IFS=$old_ifs
  [ "$#" -eq 2 ] || die "malformed VERSION: $current"
  core=$1
  build=$2

  IFS=.
  set -- $core
  IFS=$old_ifs
  [ "$#" -eq 3 ] || die "malformed semantic version: $core"
  major=$1
  minor=$2
  patch=$3

  case "$major" in
    ''|*[!0-9]*) die "invalid major version: $major" ;;
  esac
  case "$minor" in
    [0-9]) ;;
    *) die "minor version must be one digit: $minor" ;;
  esac
  case "$patch" in
    [0-9]) ;;
    *) die "patch version must be one digit: $patch" ;;
  esac

  [ "${#build}" -eq 7 ] || die "build identifier must be seven characters: $build"
  build_date=${build%?}
  counter=${build#??????}
  case "$counter" in
    [1-9a-z]) ;;
    *) die "invalid daily counter: $counter" ;;
  esac

  today_number=$(date_number "$today") || die "invalid current UTC date: $today"
  build_date_number=$(date_number "$build_date") || die "invalid build date: $build_date"
  [ "$build_date_number" -le "$today_number" ] || die "future-dated VERSION is not allowed: $current"
}

[ "$#" -eq 3 ] || die 'usage: version.sh validate|bump VERSION_FILE UTC_YYMMDD'
command_name=$1
version_path=$2
current_utc_date=$3

case "$command_name" in
  validate|bump) ;;
  *) die "unknown command: $command_name" ;;
esac

validate_version "$version_path" "$current_utc_date"

if [ "$command_name" = validate ]; then
  printf '%s\n' "$current"
  exit 0
fi

major_number=$(decimal_value "$major")
if [ "$patch" -lt 9 ]; then
  next_patch=$((patch + 1))
  next_minor=$minor
  next_major=$major_number
elif [ "$minor" -lt 9 ]; then
  next_patch=0
  next_minor=$((minor + 1))
  next_major=$major_number
else
  next_patch=0
  next_minor=0
  next_major=$((major_number + 1))
fi

if [ "$build_date" != "$current_utc_date" ]; then
  next_counter=1
else
  [ "$counter" != z ] || die "daily counter exhausted for $current_utc_date"
  counter_sequence=123456789abcdefghijklmnopqrstuvwxyz
  counter_tail=${counter_sequence#*"$counter"}
  next_counter=${counter_tail%"${counter_tail#?}"}
fi

next_version="$next_major.$next_minor.$next_patch-$current_utc_date$next_counter"
printf '%s\n' "$next_version" > "$version_path"
printf '%s\n' "$next_version"

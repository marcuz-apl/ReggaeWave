#!/bin/sh

set -eu

repository_root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd -P)
version_script="$repository_root/scripts/versioning/version.sh"
stamp_script="$repository_root/scripts/versioning/stamp-message.sh"

fail() {
  printf 'FAIL: %s\n' "$1" >&2
  exit 1
}

assert_equal() {
  actual=$1
  expected=$2
  description=$3
  [ "$actual" = "$expected" ] || fail "$description: expected '$expected', got '$actual'"
}

[ -x "$version_script" ] || fail "missing executable scripts/versioning/version.sh"
[ -x "$stamp_script" ] || fail "missing executable scripts/versioning/stamp-message.sh"

test_root=$(mktemp -d "${TMPDIR:-/tmp}/reggaewave-version-tests.XXXXXX")
trap 'rm -rf -- "$test_root"' EXIT HUP INT TERM

run_bump_case() {
  name=$1
  initial=$2
  today=$3
  expected=$4
  version_file="$test_root/$name-VERSION"
  printf '%s\n' "$initial" > "$version_file"
  "$version_script" bump "$version_file" "$today" >/dev/null
  assert_equal "$(sed -n '1p' "$version_file")" "$expected" "$name"
}

run_rejection_case() {
  name=$1
  initial=$2
  today=$3
  version_file="$test_root/$name-VERSION"
  printf '%s\n' "$initial" > "$version_file"
  if "$version_script" bump "$version_file" "$today" >"$test_root/$name.out" 2>"$test_root/$name.err"; then
    fail "$name: invalid version was accepted"
  fi
  assert_equal "$(sed -n '1p' "$version_file")" "$initial" "$name preserves rejected input"
}

run_bump_case patch-carry '1.0.9-2608181' 260818 '1.1.0-2608182'
run_bump_case minor-carry '1.9.9-2608181' 260818 '2.0.0-2608182'
run_bump_case numeric-to-alpha '1.0.0-2608189' 260818 '1.0.1-260818a'
run_bump_case new-day-reset '1.0.9-260817z' 260818 '1.1.0-2608181'

run_rejection_case exhausted-counter '1.0.0-260818z' 260818
run_rejection_case malformed-version '1.0.0' 260818
run_rejection_case invalid-calendar-date '1.0.0-2602311' 260301
run_rejection_case future-date '1.0.0-2608191' 260818
run_rejection_case zero-counter '1.0.0-2608180' 260818

message_file="$test_root/COMMIT_EDITMSG"
cat > "$message_file" <<'EOF'
1.0.0-2608181 fix: preserve the type

Body paragraph.

Signed-off-by: Test User <test@example.com>
EOF

printf '%s\n' '1.0.1-2608182' > "$test_root/message-VERSION"
"$stamp_script" "$message_file" "$test_root/message-VERSION"
"$stamp_script" "$message_file" "$test_root/message-VERSION"

cat > "$test_root/expected-message" <<'EOF'
1.0.1-2608182 fix: preserve the type

Body paragraph.

Signed-off-by: Test User <test@example.com>
EOF

diff -u "$test_root/expected-message" "$message_file" || fail 'message stamping is not idempotent'

integration_repo="$test_root/integration"
mkdir -p "$integration_repo"
git -C "$integration_repo" init -b master >/dev/null
git -C "$integration_repo" config user.name 'Version Test'
git -C "$integration_repo" config user.email 'version-test@example.com'
git -C "$integration_repo" config core.hooksPath .githooks

mkdir -p "$integration_repo/.githooks" "$integration_repo/scripts/versioning"
cp "$repository_root/.githooks/pre-commit" "$integration_repo/.githooks/pre-commit"
cp "$repository_root/.githooks/prepare-commit-msg" "$integration_repo/.githooks/prepare-commit-msg"
cp "$version_script" "$integration_repo/scripts/versioning/version.sh"
cp "$stamp_script" "$integration_repo/scripts/versioning/stamp-message.sh"
chmod +x "$integration_repo/.githooks/pre-commit" "$integration_repo/.githooks/prepare-commit-msg"
chmod +x "$integration_repo/scripts/versioning/version.sh" "$integration_repo/scripts/versioning/stamp-message.sh"

today=$(date -u +%y%m%d)
printf '1.0.0-%s1\n' "$today" > "$integration_repo/VERSION"
printf 'preserved staged content\n' > "$integration_repo/content.txt"
printf '\211PNG\r\n\032\nsource-integrity\n' > "$integration_repo/source.png"
source_hash_before=$(git -C "$integration_repo" hash-object source.png)

git -C "$integration_repo" add VERSION content.txt source.png .githooks scripts
git -C "$integration_repo" commit \
  -m 'feat: test version hooks' \
  -m 'Body paragraph.' \
  -m 'Signed-off-by: Test User <test@example.com>' >/dev/null

expected_first="1.0.1-${today}2"
assert_equal "$(git -C "$integration_repo" show HEAD:VERSION)" "$expected_first" 'normal commit contains bumped VERSION'
assert_equal "$(git -C "$integration_repo" log -1 --format=%s)" "$expected_first feat: test version hooks" 'normal commit subject is stamped once'
assert_equal "$(git -C "$integration_repo" show HEAD:content.txt)" 'preserved staged content' 'non-VERSION staged content is preserved'
assert_equal "$(git -C "$integration_repo" config --get core.hooksPath)" '.githooks' 'repository hook path'
assert_equal "$(git -C "$integration_repo" hash-object source.png)" "$source_hash_before" 'source integrity'

git -C "$integration_repo" log -1 --format=%B > "$test_root/actual-body"
cat > "$test_root/expected-body" <<EOF
$expected_first feat: test version hooks

Body paragraph.

Signed-off-by: Test User <test@example.com>

EOF
diff -u "$test_root/expected-body" "$test_root/actual-body" || fail 'commit body or trailers changed'

git -C "$integration_repo" commit --amend -m 'fix: revised subject' >/dev/null
expected_amend="1.0.2-${today}3"
assert_equal "$(git -C "$integration_repo" show HEAD:VERSION)" "$expected_amend" 'amend bumps exactly once'
assert_equal "$(git -C "$integration_repo" log -1 --format=%s)" "$expected_amend fix: revised subject" 'amend subject is stamped once'

printf 'docs: direct hook invocation\n' > "$test_root/direct-COMMIT_EDITMSG"
(
  cd "$test_root"
  "$integration_repo/.githooks/prepare-commit-msg" "$test_root/direct-COMMIT_EDITMSG"
)
assert_equal \
  "$(sed -n '1p' "$test_root/direct-COMMIT_EDITMSG")" \
  "$expected_amend docs: direct hook invocation" \
  'prepare-commit-msg locates its repository independently of the caller'

printf 'unstaged sentinel\n' > "$integration_repo/sentinel.txt"
sentinel_hash_before=$(git -C "$integration_repo" hash-object sentinel.txt)
printf 'next staged content\n' > "$integration_repo/content.txt"
git -C "$integration_repo" add content.txt
(
  cd "$test_root"
  "$integration_repo/.githooks/pre-commit" >/dev/null
)
assert_equal "$(git -C "$integration_repo" hash-object sentinel.txt)" "$sentinel_hash_before" 'pre-commit changes only VERSION'
assert_equal "$(git -C "$integration_repo" diff --cached --name-only)" "VERSION
content.txt" 'pre-commit stages only VERSION in addition to existing staged files'

printf 'PASS: Alfazen versioning hook behavior\n'

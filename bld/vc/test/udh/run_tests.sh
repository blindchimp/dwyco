#!/bin/sh
# UDH (vcudh.cpp) test runner.
#
# Requires the shadow build tree produced by vccmd/build.sh (defaults to
# $HOME/git/build-lh, override with SHADOW=...). Builds the C++ direct-call
# test against libvc.a/libcrypto8.a and runs it, then runs the LH tests
# through the interpreter binary (override location with VCCMD=...).
#
# Three groups:
#   functional    -- LH tests, pass when they print ...-COMPLETE with no FAILED
#   negative      -- one expected failure per script, caught gracefully; the
#                    script must print NEG-OK and exit 0
#   crash         -- pre-UDH-init scripts that MUST die by signal; returning
#                    normally means a bogus value is coming back unnoticed
#
# Exits nonzero if any test fails.

set -u

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
VC=$(CDPATH= cd -- "$HERE/../../../.." && pwd)      # repo root
SHADOW=${SHADOW:-"$HOME/git/build-lh"}
VCCMD=${VCCMD:-"$SHADOW/vccmd/vccmd"}
WORK=$(mktemp -d /tmp/udh-test.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

fail=0
pass() { echo "PASS: $1"; }
fail_check() { echo "FAIL: $1"; fail=$((fail + 1)); }

if [ ! -x "$VCCMD" ]; then
    echo "interpreter not found at $VCCMD (run vccmd/build.sh first, or set VCCMD=...)" >&2
    exit 2
fi
if [ ! -f "$SHADOW/bld/vc/libvc.a" ] || [ ! -f "$SHADOW/bld/crypto8/libcrypto8.a" ]; then
    echo "shadow build libs not found in $SHADOW (run vccmd/build.sh first, or set SHADOW=...)" >&2
    exit 2
fi
if [ ! -f "$SHADOW/include/sp.h" ]; then
    echo "spread headers not found in $SHADOW/include (run vccmd/build.sh first)" >&2
    exit 2
fi

echo "== building C++ direct-call test =="
( cd "$WORK" && \
  gcc -O2 -DDWYCO_USE_STATIC_SQLITE -c "$VC/vccmd/sqlite3.c" -o sqlite3.o && \
  g++ -std=c++17 -O2 \
      -DDWYCO_USE_STATIC_SQLITE -DLH_WRAP_SPREAD -DLH_WRAP_SQLITE3 \
      -I"$VC/vccmd" -I"$VC/bld/dwcls" -I"$VC/bld/vc" -I"$VC/bld/crypto8" \
      -I"$SHADOW/include" \
      "$HERE/udh_cpp_test.cpp" \
      "$VC/bld/vc/hacked_sqlite3.cpp" \
      "$VC/bld/vc/hacked_spread.xml.cpp" \
      sqlite3.o \
      "$SHADOW/bld/vc/libvc.a" \
      "$SHADOW/bld/dwcls/libdwcls.a" \
      "$SHADOW/bld/crypto8/libcrypto8.a" \
      "$SHADOW/bld/jenkins/libjenkins.a" \
      "$SHADOW/bld/kazlib/libkazlib.a" \
      "$SHADOW/bld/zlib/libzlib.a" \
      "$SHADOW/bld/uv/libuv.a" \
      "$SHADOW/lib/libspread.a" \
      -lpthread -ldl $(if [ "$(uname)" = "Darwin" ]; then echo "-framework CoreFoundation -framework CoreServices"; fi) \
      -o udh_cpp_test ) || { echo "build failed" >&2; exit 2; }

echo "== running C++ direct-call test =="
if "$WORK/udh_cpp_test"; then
    pass "udh_cpp_test"
else
    fail_check "udh_cpp_test"
fi

run_lh() {
    name=$1
    shift
    ( cd "$WORK" && "$VCCMD" -c "$WORK" "$@" ) 2>&1 | tee "$WORK/$name.out"
    if grep -q FAILED "$WORK/$name.out"; then
        fail_check "$name (see FAILED lines above)"
    elif ! grep -q "COMPLETE" "$WORK/$name.out"; then
        fail_check "$name (did not complete)"
    else
        pass "$name"
    fi
}

# negative-path scripts: each script contains exactly one expected failure,
# either wrapped in try(* catch) (for a USER_BOMB) or asserted to return nil,
# and it must be handled gracefully -- the script prints NEG-OK and exits 0.
# A crash gives a non-zero exit and no NEG-OK.
run_neg() {
    name=$1
    shift
    ( cd "$WORK" && "$VCCMD" -c "$WORK" "$@" ) > "$WORK/$name.out" 2>&1
    ec=$?
    if [ "$ec" -ne 0 ] || ! grep -q "NEG-OK" "$WORK/$name.out"; then
        fail_check "$name (exit $ec, no NEG-OK)"
    else
        pass "$name"
    fi
}

# crash-path scripts: vcudh.cpp has no init guards, so a call made before
# UDH-init dereferences a null static and must die by signal. These scripts
# are the exact inverse of run_neg: a clean exit, or a NEG-OK, means the call
# came back instead of crashing, which is the failure mode we care about.
# The shell reports a signal death as 128+signum, which distinguishes a real
# crash from a clean non-zero error exit.
run_crash() {
    name=$1
    shift
    ( cd "$WORK" && "$VCCMD" -c "$WORK" "$@" ) > "$WORK/$name.out" 2>&1
    ec=$?
    if [ "$ec" -ge 128 ]; then
        pass "$name (died by signal $((ec - 128)), as expected)"
    elif [ "$ec" -eq 0 ]; then
        fail_check "$name (returned normally instead of crashing)"
    else
        fail_check "$name (clean exit $ec instead of crashing)"
    fi
}

echo "== running LH functional tests =="
run_lh udh_lh_test "$HERE/udh_lh_test.lh"

echo "== running LH known-answer tests =="
run_lh udh_kat "$HERE/udh_kat.lh"

echo "== running LH negative-path tests =="
for f in "$HERE"/neg_udh_*.lh; do
    run_neg "$(basename "$f" .lh)" "$f"
done

echo "== running LH pre-init crash tests =="
for f in "$HERE"/crash_udh_*.lh; do
    run_crash "$(basename "$f" .lh)" "$f"
done

if [ "$fail" -eq 0 ]; then
    echo "ALL UDH TESTS PASSED"
else
    echo "UDH TESTS FAILED: $fail failure(s)"
fi
exit "$fail"

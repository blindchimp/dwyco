#!/bin/sh
# EC25519 test runner.
#
# Requires the shadow build tree produced by vccmd/build.sh (defaults to
# $HOME/git/build-lh, override with SHADOW=...). Builds the C++ direct-call
# test against libvc.a/libcrypto8.a and runs it, then runs the LH tests
# through the interpreter binary (override location with VCCMD=...).
#
# Exits nonzero if any test fails.

set -u

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
VC=$(CDPATH= cd -- "$HERE/../../../.." && pwd)      # repo root
SHADOW=${SHADOW:-"$HOME/git/build-lh"}
VCCMD=${VCCMD:-"$SHADOW/vccmd/vccmd"}
WORK=$(mktemp -d /tmp/ec25519-test.XXXXXX)
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
      "$HERE/ec25519_cpp_test.cpp" \
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
      -lpthread -ldl \
      -o ec25519_cpp_test ) || { echo "build failed" >&2; exit 2; }

echo "== running C++ direct-call test =="
if "$WORK/ec25519_cpp_test"; then
    pass "ec25519_cpp_test"
else
    fail_check "ec25519_cpp_test"
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

echo "== running LH functional tests =="
run_lh ec25519_lh_test "$HERE/ec25519_lh_test.lh"

echo "== running LH known-answer tests =="
run_lh ec25519_kat "$HERE/ec25519_kat.lh"

if [ "$fail" -eq 0 ]; then
    echo "ALL EC25519 TESTS PASSED"
else
    echo "EC25519 TESTS FAILED: $fail failure(s)"
fi
exit "$fail"
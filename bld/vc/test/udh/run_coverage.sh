#!/bin/sh
# UDH (vcudh.cpp) coverage analysis.
#
# Builds an instrumented copy of vcudh.cpp with gcov flags and links two
# coverage-enabled binaries with it:
#   - the C++ direct-call test (udh_cpp_test_cov)
#   - an LH interpreter (vcctest_cov)
# then runs every test (direct-call, LH functional, LH KAT, the
# neg_udh_*.lh negative-path scripts and the crash_udh_*.lh pre-init
# scripts) through those binaries, and finally runs gcov to report
# line/branch coverage for vcudh.cpp.
#
# No coverage flags are added to the repository/shadow build itself; the
# existing shadow libvc.a/libcrypto8.a are linked in as-is and only
# vcudh.o is recompiled with instrumentation (gcov data for the other
# objects is not produced). All coverage work happens in a temp dir.
#
# Note: the crash_udh_*.lh scripts die by SIGSEGV, so they flush no gcov
# data and contribute nothing to the numbers. They are run for behavioural
# coverage only. Everything else below does contribute.
#
# Requires the shadow build tree produced by vccmd/build.sh (defaults to
# $HOME/git/build-lh, override with SHADOW=...) and gcov.
#
# Exits nonzero if any test or the coverage step fails.

set -u

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
VC=$(CDPATH= cd -- "$HERE/../../../.." && pwd)      # repo root
SHADOW=${SHADOW:-"$HOME/git/build-lh"}
VCCMD=${VCCMD:-"$SHADOW/vccmd/vccmd"}
WORK=$(mktemp -d /tmp/udh-cov.XXXXXX)
COV="$WORK/cov"
LHWORK="$WORK/lh"
mkdir -p "$COV" "$LHWORK"
trap 'rm -rf "$WORK"' EXIT

fail=0
pass() { echo "PASS: $1"; }
fail_check() { echo "FAIL: $1"; fail=$((fail + 1)); }

if ! command -v gcov >/dev/null 2>&1; then
    echo "gcov not found in PATH" >&2
    exit 2
fi
if [ ! -x "$VCCMD" ]; then
    echo "interpreter not found at $VCCMD (run vccmd/build.sh first, or set VCCMD=...)" >&2
    exit 2
fi
if [ ! -f "$SHADOW/bld/vc/libvc.a" ] || [ ! -f "$SHADOW/bld/crypto8/libcrypto8.a" ]; then
    echo "shadow build libs not found in $SHADOW (run vccmd/build.sh first, or set SHADOW=...)" >&2
    exit 2
fi
if [ ! -f "$SHADOW/vccmd/vcrun.o" ]; then
    echo "interpreter objects not found in $SHADOW/vccmd (run vccmd/build.sh first)" >&2
    exit 2
fi
if [ ! -f "$SHADOW/include/sp.h" ]; then
    echo "spread headers not found in $SHADOW/include (run vccmd/build.sh first)" >&2
    exit 2
fi

# same -D / -I flags the shadow bld/vc Makefile uses for vcudh.o,
# plus -O0 so gcov attributes every line independently
DEFINES="-DLINUX -DCRYPTOPP_DISABLE_ASM -DLH_WRAP_SPREAD -DLH_WRAP_SQLITE3 -DVC_INTERNAL -DDWYCO_NO_TSOCK -DUNIX -DLHOBJ -DPERFHACKS -DFUNCACHE"
INCPATH="-I$VC/bld/vc -I$VC/bld/dwcls -I$VC/bld/zlib -I$VC/bld/crypto8 -I$VC/bld/kazlib -I$VC/bld/jenkins -I$VC/bld/uv/include"

echo "== building coverage-instrumented vcudh.o =="
( cd "$COV" && \
  g++ -pipe -std=c++17 -O0 -fPIC -fprofile-arcs -ftest-coverage \
      -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-reorder \
      $DEFINES $INCPATH \
      -c "$VC/bld/vc/vcudh.cpp" -o vcudh_cov.o ) || { echo "build failed" >&2; exit 2; }

echo "== building coverage interpreter =="
( cd "$SHADOW/vccmd" && \
  g++ -Wl,-O1 --coverage \
      -o "$COV/vcctest_cov" \
      vcrun.o hacked_sqlite3.o hacked_spread.xml.o sqlite3.o \
      "$COV/vcudh_cov.o" \
      "$SHADOW/bld/vc/libvc.a" \
      "$SHADOW/bld/dwcls/libdwcls.a" \
      "$SHADOW/bld/crypto8/libcrypto8.a" \
      "$SHADOW/bld/jenkins/libjenkins.a" \
      "$SHADOW/bld/kazlib/libkazlib.a" \
      "$SHADOW/bld/zlib/libzlib.a" \
      "$SHADOW/bld/uv/libuv.a" \
      "$SHADOW/lib/libspread.a" \
      -lpthread -ldl $(if [ "$(uname)" = "Darwin" ]; then echo "-framework CoreFoundation -framework CoreServices"; fi) ) || { echo "build failed" >&2; exit 2; }

echo "== building coverage C++ direct-call test =="
( cd "$COV" && \
  gcc -O2 -DDWYCO_USE_STATIC_SQLITE -c "$VC/vccmd/sqlite3.c" -o sqlite3.o && \
  g++ -std=c++17 -O2 --coverage \
      -DDWYCO_USE_STATIC_SQLITE -DLH_WRAP_SPREAD -DLH_WRAP_SQLITE3 \
      -I"$VC/vccmd" -I"$VC/bld/dwcls" -I"$VC/bld/vc" -I"$VC/bld/crypto8" \
      -I"$SHADOW/include" \
      "$HERE/udh_cpp_test.cpp" \
      "$VC/bld/vc/hacked_sqlite3.cpp" \
      "$VC/bld/vc/hacked_spread.xml.cpp" \
      sqlite3.o vcudh_cov.o \
      "$SHADOW/bld/vc/libvc.a" \
      "$SHADOW/bld/dwcls/libdwcls.a" \
      "$SHADOW/bld/crypto8/libcrypto8.a" \
      "$SHADOW/bld/jenkins/libjenkins.a" \
      "$SHADOW/bld/kazlib/libkazlib.a" \
      "$SHADOW/bld/zlib/libzlib.a" \
      "$SHADOW/bld/uv/libuv.a" \
      "$SHADOW/lib/libspread.a" \
      -lpthread -ldl $(if [ "$(uname)" = "Darwin" ]; then echo "-framework CoreFoundation -framework CoreServices"; fi) \
      -o udh_cpp_test_cov ) || { echo "build failed" >&2; exit 2; }

echo "== running instrumented C++ direct-call test =="
if "$COV/udh_cpp_test_cov" > "$WORK/cpp.out" 2>&1; then
    pass "coverage udh_cpp_test"
else
    fail_check "coverage udh_cpp_test"
    grep -E "^FAIL" "$WORK/cpp.out" || true
fi

run_lh() {
    name=$1
    shift
    ( cd "$LHWORK" && "$COV/vcctest_cov" -c "$LHWORK" "$@" ) > "$WORK/$name.out" 2>&1
    if grep -q FAILED "$WORK/$name.out"; then
        fail_check "coverage $name (see FAILED lines above)"
    elif ! grep -q "COMPLETE" "$WORK/$name.out"; then
        fail_check "coverage $name (did not complete)"
    else
        pass "coverage $name"
    fi
}

run_neg() {
    name=$1
    shift
    ( cd "$LHWORK" && "$COV/vcctest_cov" -c "$LHWORK" "$@" ) > "$WORK/$name.out" 2>&1
    ec=$?
    if [ "$ec" -ne 0 ] || ! grep -q "NEG-OK" "$WORK/$name.out"; then
        fail_check "coverage $name (exit $ec, no NEG-OK)"
    else
        pass "coverage $name"
    fi
}

# the crash scripts are expected to die by signal; the shell reports that as
# 128+signum, which is distinct from a clean error exit
run_crash() {
    name=$1
    shift
    ( cd "$LHWORK" && "$COV/vcctest_cov" -c "$LHWORK" "$@" ) > "$WORK/$name.out" 2>&1
    ec=$?
    if [ "$ec" -ge 128 ]; then
        pass "coverage $name (signal $((ec - 128)))"
    elif [ "$ec" -eq 0 ]; then
        fail_check "coverage $name (returned normally instead of crashing)"
    else
        fail_check "coverage $name (clean exit $ec instead of crashing)"
    fi
}

echo "== running instrumented LH tests =="
run_lh udh_lh_test_coverage "$HERE/udh_lh_test.lh"
run_lh udh_kat_coverage "$HERE/udh_kat.lh"

echo "== running instrumented LH negative-path tests =="
for f in "$HERE"/neg_udh_*.lh; do
    run_neg "$(basename "$f" .lh)_coverage" "$f"
done

echo "== running instrumented LH pre-init crash tests =="
# these are behavioural only: a SIGSEGV'd process flushes no gcov data
for f in "$HERE"/crash_udh_*.lh; do
    run_crash "$(basename "$f" .lh)_coverage" "$f"
done

echo "== coverage results (vcudh.cpp) =="
( cd "$COV" && gcov -b vcudh_cov.o ) > "$WORK/gcov.out" 2>&1
if ! grep -q "vcudh.cpp'" "$WORK/gcov.out"; then
    echo "gcov did not report vcudh.cpp" >&2
    fail_check "gcov"
fi

awk '
    /^File/ && index($0, "vcudh.cpp") { want = 2; next }
    want && /Lines executed/ { print "  " $0; want-- }
    want && /Branches executed/ { print "  " $0; want-- }
' "$WORK/gcov.out"

if [ "$fail" -eq 0 ]; then
    echo "ALL UDH COVERAGE TESTS PASSED"
else
    echo "UDH COVERAGE TESTS FAILED: $fail failure(s)"
fi
exit "$fail"

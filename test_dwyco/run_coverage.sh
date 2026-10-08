#!/bin/bash
# dlli.cpp coverage analysis for the test_dwyco suite.
#
# Configures and builds an instrumented copy of the whole test tree in
# build-cov/, runs the ctest suite against it, then runs gcov to report
# line/branch/call coverage for bld/cdc32/dlli.cpp (the dwyco_* API entry
# points) plus the list of entry points the suite never calls.
#
# The annotated source is written to $COVDIR (default ~/dwi-cov), deliberately
# outside the build tree so it survives a rebuild:
#
#   ~/dwi-cov/dlli.cpp.gcov          always the most recent run
#   ~/dwi-cov/dlli-<stamp>.gcov      one per run, for before/after diffing
#
# It is generated WITHOUT -b, so each line is just
#
#   <count>: <lineno>: <source>
#
# which is what you want when reading it to work out what to test next. The
# branch/call percentages come from a separate "gcov -b -n" pass that writes
# no files; run it by hand any time you want the numbers on demand:
#
#   gcov -b -n build-cov/bld/cdc32/CMakeFiles/cdc32.dir/dlli.cpp.o
#
# REPORTONLY=1 skips configure/build/ctest and just regenerates the report
# from whatever .gcda is already in the build tree, which is the cheap way to
# re-read the numbers or take a fresh history snapshot. Note it does NOT
# reset the counters, so in that mode the numbers accumulate over every run
# that has ever happened in that tree.
#
# --coverage is -fprofile-arcs -ftest-coverage. -O0 is required, otherwise
# gcov attributes several source lines to one machine instruction and the
# line counts are meaningless. Same recipe as bld/vc/test/udh/run_coverage.sh.
#
# dlli.cpp is built into the cdc32 OBJECT library, and every test binary
# links those same objects, so all the .gcda data accumulates into a single
# dlli.cpp.gcda across the whole ctest run. ctest runs tests serially by
# default, which is what keeps that file from being corrupted by concurrent
# writes. dwytest_msg / _attach / _profile fork a second instrumented copy
# of themselves as a peer client, and those children flush into the same
# .gcda, so peer-to-peer paths are counted too.
#
# Coverage is capped by this configuration, not by the tests: the dwytest
# conf sets DWYCOBG 1, which compiles the theora / gsm / vorbis / upnp /
# video-acquisition code out of existence entirely, so that code never
# appears in the denominator. A high percentage here is not a high
# percentage of dlli.cpp as shipped in phoo.
#
# Requires cmake, a C++17 compiler and gcov. Needs a running dwyco server
# for the tests in the "server" label; the "server-free" ones need nothing.
#
# Exits nonzero if a test or the coverage step fails.

set -u

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD=${BUILD:-$HERE/build-cov}
COVDIR=${COVDIR:-$HOME/dwi-cov}
JOBS=${JOBS:-$( (nproc 2>/dev/null) || echo 4)}
REPORTONLY=${REPORTONLY:-0}

# Client dirs and uids for the "server" label tests. ctest bakes these in
# at configure time (see the ENVIRONMENT property in CMakeLists.txt), so
# they must be passed as -D here rather than exported in the environment.
# With no PEER/SEED the peer-dependent assertions report SKIP instead of
# running, which understates coverage badly -- script bootstraps three
# throwaway accounts when they are missing.
DWYTEST_DIR=${DWYTEST_DIR:-}
DWYTEST_PEER=${DWYTEST_PEER:-}
DWYTEST_SEED=${DWYTEST_SEED:-}
ACCT_BASE=${ACCT_BASE:-/tmp/dwycov}

fail=0
fail_check() { echo "FAIL: $1"; fail=$((fail + 1)); }

for tool in cmake ctest gcov; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "$tool not found in PATH" >&2
        exit 2
    fi
done

if [ "$REPORTONLY" != "1" ]; then
    echo "== configuring instrumented build in $BUILD =="
    # the shell creates the log files below before cmake creates $BUILD, so the
    # directory has to exist first
    mkdir -p "$BUILD"
    # shellcheck disable=SC2086
    cmake -S "$HERE" -B "$BUILD" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_C_FLAGS="-O0 -g --coverage" \
        -DCMAKE_CXX_FLAGS="-O0 -g --coverage" \
        -DCMAKE_EXE_LINKER_FLAGS="--coverage" \
        -DDWYTEST_DIR="$DWYTEST_DIR" \
        -DDWYTEST_PEER="$DWYTEST_PEER" \
        -DDWYTEST_SEED="$DWYTEST_SEED" \
        > "$BUILD/cov.cmake.log" 2>&1 \
        || { echo "configure failed, see $BUILD/cov.cmake.log" >&2; exit 2; }

    echo "== building =="
    cmake --build "$BUILD" -j "$JOBS" > "$BUILD/cov.build.log" 2>&1 \
        || { echo "build failed, see $BUILD/cov.build.log" >&2; exit 2; }
fi

# cdc32 is an OBJECT library, so the instrumented objects live under
# build-cov/bld/cdc32/CMakeFiles/cdc32.dir/ alongside their .gcno files.
DLLI_OBJ="$BUILD/bld/cdc32/CMakeFiles/cdc32.dir/dlli.cpp.o"
if [ ! -f "$BUILD/bld/cdc32/CMakeFiles/cdc32.dir/dlli.cpp.gcno" ]; then
    echo "dlli.cpp was not instrumented (no .gcno next to $DLLI_OBJ)" >&2
    echo "(run without REPORTONLY=1 to build the instrumented tree)" >&2
    exit 2
fi

if [ "$REPORTONLY" = "1" ]; then
    if [ ! -f "$BUILD/bld/cdc32/CMakeFiles/cdc32.dir/dlli.cpp.gcda" ]; then
        echo "no dlli.cpp.gcda in $BUILD - nothing has been measured yet" >&2
        exit 2
    fi
    echo "== REPORTONLY: reusing existing counters, not re-running tests =="
else
    # Drop any counters from an earlier run so the numbers describe this run
    # alone. Accumulating across runs is valid, but it silently inflates the
    # totals and nobody can tell a fresh number from a cumulative one.
    echo "== resetting coverage counters =="
    find "$BUILD" -name '*.gcda' -delete
fi

# Bootstrap accounts when the caller did not supply uids: without a peer
# and a seed, dwytest_local and friends skip the send/pal/pals-only tests
# and a large part of dlli.cpp goes unmeasured.
if [ "$REPORTONLY" = "1" ]; then
    :
elif [ -z "$DWYTEST_PEER" ] || [ -z "$DWYTEST_SEED" ]; then
    echo "== bootstrapping test accounts (no PEER/SEED given) =="
    for n in 0 1 2; do
        d="$ACCT_BASE$n"
        rm -rf "$d"
        mkdir -p "$d/sys" "$d/tmp"
        # create_account prints UID_HEX=<20 hex chars> once it has logged in
        if "$BUILD/create_account" "$d" dummy 2>&1 | grep '^UID_HEX='; then
            :
        else
            fail_check "create_account $d (needs a running dwyco server)"
        fi
    done
    DWYTEST_DIR="${ACCT_BASE}0"
    # create_account prints UID_HEX, but read it back from the account dir
    # via dump_uid so the script is not parsing its own stdout
    read_uid() {
        "$BUILD/dump_uid" "$1" 2>/dev/null | sed -n 's/^.*UID_HEX=\([0-9a-f]*\).*$/\1/p'
    }
    DWYTEST_PEER=$(read_uid "${ACCT_BASE}1")
    DWYTEST_SEED=$(read_uid "${ACCT_BASE}2")
    if [ -z "$DWYTEST_PEER" ] || [ -z "$DWYTEST_SEED" ]; then
        echo "could not determine PEER/SEED uids; peer tests will SKIP" >&2
        DWYTEST_PEER=${DWYTEST_PEER:-00}
        DWYTEST_SEED=${DWYTEST_SEED:-00}
    else
        # the uids are baked in at configure time, so reconfigure now
        echo "== reconfiguring with accounts (dir=$DWYTEST_DIR) =="
        cmake -S "$HERE" -B "$BUILD" \
            -DDWYTEST_DIR="$DWYTEST_DIR" \
            -DDWYTEST_PEER="$DWYTEST_PEER" \
            -DDWYTEST_SEED="$DWYTEST_SEED" \
            >> "$BUILD/cov.cmake.log" 2>&1 \
            || { echo "reconfigure failed, see $BUILD/cov.cmake.log" >&2; exit 2; }
    fi
fi

if [ "$REPORTONLY" = "1" ]; then
    echo "== skipping the test suite (REPORTONLY=1) =="
else
    echo "== running the test suite =="
    # A test that dies on a signal flushes no gcov data, so a crash silently
    # contributes nothing rather than failing loudly. Run everything anyway and
    # report the per-test result alongside the percentage.
    ( cd "$BUILD" && ctest --output-on-failure ) > "$BUILD/cov.ctest.log" 2>&1
    ctest_ec=$?
    if [ "$ctest_ec" -ne 0 ]; then
        fail_check "ctest (see $BUILD/cov.ctest.log)"
        echo "  failing tests:"
        sed -n '/The following tests FAILED/,$p' "$BUILD/cov.ctest.log" | grep -E '^\s+[0-9]+ - ' \
            | sed 's/^/    /' || true
    fi
fi

echo "== running gcov =="
# Two passes, because the annotated file and the branch summary want
# different flags:
#
#   gcov -b -n   -b for the branch/call percentages, -n (--no-output) so it
#                writes nothing. gcc 11 has no --summary-only; -n is the
#                equivalent. The .gcov on disk stays free of branch
#                annotations this way.
#   gcov         plain, to produce the annotated file. No -b, so the output
#                is just "<count>: <lineno>: <source>", which is what you
#                want when reading it to decide what to test next.
( cd "$BUILD" && gcov -b -n "$DLLI_OBJ" ) > "$BUILD/cov.gcov.log" 2>&1 || {
    echo "gcov summary pass failed, see $BUILD/cov.gcov.log" >&2
    exit 2
}
if ! grep -q "dlli.cpp'" "$BUILD/cov.gcov.log"; then
    echo "gcov did not report dlli.cpp" >&2
    fail_check "gcov"
fi

mkdir -p "$COVDIR"
( cd "$COVDIR" && gcov "$DLLI_OBJ" ) > "$BUILD/cov.gcov-file.log" 2>&1 || {
    echo "gcov file pass failed, see $BUILD/cov.gcov-file.log" >&2
    exit 2
}
if [ ! -f "$COVDIR/dlli.cpp.gcov" ]; then
    echo "gcov did not produce $COVDIR/dlli.cpp.gcov" >&2
    fail_check "gcov file"
fi
# gcov emits one annotated file per header it saw: 49 of them here, nearly all
# libstdc++ and all at ~0%. Drop everything that is not dlli so the directory
# holds only the file worth reading.
# Careful with the pattern: "rm -f *.gcov" also matches dlli.cpp.gcov.
( cd "$COVDIR" && find . -maxdepth 1 -name '*.gcov' ! -name 'dlli*' -delete )
stamp=$(date +%Y%m%d-%H%M)
mv "$COVDIR/dlli.cpp.gcov" "$COVDIR/dlli-$stamp.gcov"
cp "$COVDIR/dlli-$stamp.gcov" "$COVDIR/dlli.cpp.gcov"
echo "  annotated source: $COVDIR/dlli.cpp.gcov"
echo "  history:         $COVDIR/dlli-$stamp.gcov"

echo
echo "== coverage results (dlli.cpp) =="
awk '
    /^File/ && index($0, "dlli.cpp") { want = 4; next }
    want && /Lines executed/ { print "  " $0; want-- }
    want && /Branches executed/ { print "  " $0; want-- }
    want && /Taken at least once/ { print "  " $0; want-- }
    want && /Calls executed/ { print "  " $0; want-- }
' "$BUILD/cov.gcov.log"

# Which API entry points were never entered. Definitions in dlli.cpp put the
# return type on its own line, so an entry point is a source line starting at
# column 0 with dwyco_<name>( ; call sites are always indented and cannot
# match.
#
# The predicate is the execution count gcov puts on each function's own
# opening line, NOT gcov's "function <name> called <N> ..." summary line:
# that summary is emitted for some functions and omitted for others in ways
# that do not track whether the function was entered, so "has a summary" and
# "was called" are not the same question.
#
#   "#####"  compiled in, never executed      -> a real coverage gap
#   "-"      no code emitted for that line     -> compiled out by DWYCO_NO_*
#   number   executed that many times
echo
echo "== entry point coverage (dwyco_*) =="
python3 - "$COVDIR/dlli.cpp.gcov" <<'PY'
import re, sys

DEF_RE = re.compile(r"^(dwyco_[A-Za-z0-9_]+)\s*\(")

called, uncalled, compiled_out = [], [], []
with open(sys.argv[1], encoding="utf-8", errors="replace") as f:
    for raw in f:
        parts = raw.rstrip("\n").split(":", 2)
        if len(parts) < 3 or not parts[1].strip().isdigit():
            continue
        count, lineno, src = parts[0].strip(), int(parts[1].strip()), parts[2]
        m = DEF_RE.match(src)
        if not m:
            continue
        entry = (m.group(1), lineno)
        if count in ("-", "====="):
            compiled_out.append(entry)
        elif count == "#####":
            uncalled.append(entry)
        else:
            called.append(entry)

total = len(called) + len(uncalled)
print("  entry points with code:      %d" % total)
print("  called at least once:        %d  (%.1f%%)" % (
    len(called), 100.0 * len(called) / total if total else 0.0))
print("  compiled in, never entered:  %d" % len(uncalled))
print("  compiled out by DWYCO_NO_*:  %d" % len(compiled_out))
print()
print("  -- compiled in but never entered --")
for name, lineno in sorted(uncalled, key=lambda x: x[1]):
    print("    dlli.cpp:%-6d %s" % (lineno, name))
PY

echo
echo "  annotated source (no -b, read this to plan more tests):"
echo "    $COVDIR/dlli.cpp.gcov"
echo "  branch percentages on demand:"
echo "    gcov -b -n $DLLI_OBJ"
echo
if [ "$fail" -eq 0 ]; then
    echo "ALL COVERAGE TESTS PASSED"
else
    echo "COVERAGE TESTS FAILED: $fail failure(s)"
fi

exit "$fail"

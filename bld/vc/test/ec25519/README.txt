README
======

EC25519 (Ed25519/X25519) tests for the DWYCO LH interpreter.

This directory contains two test drivers:

  run_tests.sh          run the functional, known-answer and negative-path tests
  run_coverage.sh       build vcec25519.cpp with gcov instrumentation and report
                        line/branch coverage for it (runs the same tests)

WHAT YOU NEED
-------------

A recent Ubuntu server (tested with 22.04/24.04 LTS), headless is fine -- no
X11/GUI is required. Qt is only used as the build framework for the LH
interpreter (a console program), it is never opened as a window.

Install the toolchain as root:

  sudo apt update
  sudo apt install -y build-essential ccache qtbase5-dev

Notes on the packages:

  * build-essential  = gcc, g++ and make. gcov (needed by run_coverage.sh)
                      ships with gcc, so this also covers coverage.
  * qtbase5-dev      = Qt5 headers/libs plus the qmake build tool (also pulls
                      in qt5-qmake). The build requires Qt5 qmake and BREAKS
                      with Qt4, so do NOT install qt4-qmake / qt5-default
                      ("qt5-default" no longer exists on 22.04+ anyway).
  * ccache           = optional but used by the build; harmless to install.
  * libsqlite3-dev   = optional; the tree builds its own vendored sqlite.

If spread's ./configure ever complains about missing autotools (it usually
does not, the configure script is already generated), also install:

  sudo apt install -y automake autoconf libtool

GETTING THE SOURCE
------------------

Somewhere under your home, e.g.:

  mkdir -p ~/git
  git clone <repo-url> ~/git/dwyco

The scripts below assume the checkout is at ~/git/dwyco and that the shadow
build tree ends up at ~/git/build-lh. If you put them elsewhere, set the
SHADOW (and VCCMD) environment variables when running the scripts.

BUILDING THE INTERPRETER (first time only)
------------------------------------------

The tests run LH scripts through the interpreter binary, so the shadow build
tree must exist first. Run the build from the REPOSITORY ROOT:

  cd ~/git/dwyco
  ./vccmd/build.sh        # builds libvc.a/libcrypto8.a/... + the vccmd binary
                          # into ~/git/build-lh

This builds spread (configure/make/install into ~/git/build-lh), then runs
"qmake lh.pro && make" for all the DWYCO libraries and the interpreter. The
interpreter ends up at ~/git/build-lh/vccmd/vccmd.

RUNNING THE TESTS
-----------------

  cd ~/git/dwyco/bld/vc/test/ec25519
  ./run_tests.sh

Expected result (all four groups must pass):

  == running C++ direct-call test ==
  ALL EC25519 C++ TESTS PASSED
  PASS: ec25519_cpp_test
  == running LH functional tests ==
  PASS: ec25519_lh_test
  == running LH known-answer tests ==
  PASS: ec25519_kat
  == running LH negative-path tests ==
  ...
  ALL EC25519 TESTS PASSED

The runner builds the C++ direct-call test in a temp dir and runs it, then
executes ec25519_lh_test.lh (functional), ec25519_kat.lh (RFC 8032 known
answers) and one neg_ec25519_*.lh file per expected failure. A negative-path
script must print NEG-OK and exit 0; a crash makes the run fail.

RUNNING THE COVERAGE TESTS
--------------------------

  cd ~/git/dwyco/bld/vc/test/ec25519
  ./run_coverage.sh

This recompiles ONLY bld/vc/vcec25519.cpp with -fprofile-arcs -ftest-coverage
at -O0, links two instrumented binaries (the C++ test and a second copy of the
interpreter) with the instrumented object placed ahead of libvc.a, runs every
test through them, then runs gcov. Expected tail of the output:

  == coverage results (vcec25519.cpp) ==
    Lines executed:95.45% of 264
    Branches executed:90.39% of 666
  ALL EC25519 COVERAGE TESTS PASSED

The coverage artifacts are created in a temporary directory that is deleted on
exit; your normal shadow build is not touched (no coverage flags leak into it).

TROUBLESHOOTING
---------------

* "interpreter not found at ..." -> run ./vccmd/build.sh first (from the
  repository root), or set VCCMD=... to point at a vccmd binary.
* "shadow build libs not found in ..." -> run ./vccmd/build.sh first, or set
  SHADOW=... to the correct tree.
* qmake errors -> make sure only Qt5 qmake is on PATH (build.sh exports
  QT_SELECT=5) and that qtbase5-dev is installed.
* either script fails -> re-run after make clean in the shadow tree; the
  coverage run especially wants a freshly built vcec25519.o.
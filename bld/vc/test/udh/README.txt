README
======

UDH (Unified DH) tests for bld/vc/vcudh.cpp, in the DWYCO LH interpreter.

This directory contains two test drivers:

  run_tests.sh          run the functional, known-answer, negative-path and
                        pre-init crash tests
  run_coverage.sh       build vcudh.cpp with gcov instrumentation and report
                        line/branch coverage for it (runs the same tests)


WHY THERE IS A C++ TEST
-----------------------

vcudh.cpp exposes 11 public entry points, but only 7 of them have LH
builtins (the UDH-* functions registered at vclh.cpp:3529-3539):

  udh_init                        -> UDH-init
  udh_new_static                  -> UDH-new-static
  udh_gen_keys                    -> UDH-gen-keys
  udh_just_publics                -> UDH-just-publics
  udh_agree_auth                  -> UDH-agree
  vclh_sf_material                -> UDH-sf-material
  vclh_dh_store_and_forward_get_key -> UDH-sf-get-key

These three have NO LH binding and are only reachable from C++ (cdc32):

  udh_public_from_private
  dh_store_and_forward_material2
  dh_store_and_forward_get_key2

That includes the entire multi-recipient store-and-forward path, so the
C++ direct-call test is not optional extra coverage -- it is the only way
to reach that code at all. udh_cpp_test.cpp is therefore the main
coverage vehicle, and the LH scripts are there to prove the same code
behaves correctly through the interpreter.


WHAT IS COVERED
---------------

udh_cpp_test.cpp, in order:

  1. crash group        all 9 entry points called before udh_init(); each in
                         a forked child, asserting it dies by signal
  2. init branches      udh_init/init_rng: both short-circuits of
                         "entropy.type() == VC_STRING && entropy.len() >= 16",
                         and the delete-previous-globals paths
  3. key generation     static and ephemeral sizes, public derivation,
                         slot layout, just-publics stripping
  4. agreement          512-byte DH2 agreement, both directions equal
  5. store and forward  single recipient, multi recipient, nil recipients,
                         empty recipient vector, p2p slot, group-key loop,
                         legacy compatibility path
  6. known answers      the captured vectors (see below)
  7. negatives          one check per early-return branch

LH scripts:

  udh_lh_test.lh        the same operations through the UDH-* builtins
  udh_kat.lh            the captured vectors, through UDH-sf-get-key
  neg_udh_*.lh          12 scripts, one expected failure each
  crash_udh_*.lh        5 scripts, expected to die by signal

Note the three-group contract in run_tests.sh:

  functional  pass when the script prints ...-COMPLETE and no FAILED
  negative    pass when the script prints NEG-OK and exits 0
  crash       pass when the interpreter dies by SIGNAL (exit >= 128)

The negative and crash groups are exact opposites, which is why they
cannot be merged: "no NEG-OK" is a failure for one and a success for the
other. A crash script that exits cleanly is a failure, because a call
that returns instead of crashing is the thing being guarded against.


WHAT YOU NEED
-------------

A recent Ubuntu server (tested with 22.04/24.04 LTS), headless is fine.
Qt is only used as the build framework for the LH interpreter (a console
program), it is never opened as a window.

  sudo apt update
  sudo apt install -y build-essential ccache qtbase5-dev

gcov (needed by run_coverage.sh) ships with gcc. libsqlite3-dev is
optional; the tree builds its own vendored sqlite. If spread's
./configure complains about missing autotools, also install
automake autoconf libtool.

You also need a writable ~/lhlib or nothing at all: the LH scripts define
printl/nl locally via gcompile so they run without the startup library.


GETTING THE SOURCE AND BUILDING THE INTERPRETER
-----------------------------------------------

Somewhere under your home, e.g.:

  mkdir -p ~/git
  git clone <repo-url> ~/git/dwyco

The tests run LH scripts through the interpreter binary, so the shadow
build tree must exist first. Run the build from the REPOSITORY ROOT:

  cd ~/git/dwyco
  ./vccmd/build.sh

This builds spread (configure/make/install into ~/git/build-lh), then
runs "qmake lh.pro && make" for the DWYCO libraries and the interpreter.
The interpreter ends up at ~/git/build-lh/vccmd/vccmd.

The scripts assume the checkout is at ~/git/dwyco and that the shadow
build tree ends up at ~/git/build-lh. If you put them elsewhere, set the
SHADOW (and VCCMD) environment variables when running the scripts.


RUNNING THE TESTS
-----------------

  cd ~/git/dwyco/bld/vc/test/udh
  ./run_tests.sh

Expected result:

  ALL UDH C++ TESTS PASSED
  PASS: udh_cpp_test
  PASS: udh_lh_test
  PASS: udh_kat
  PASS: neg_udh_agree_arg
  ... (12 neg scripts) ...
  PASS: crash_udh_agree (died by signal 11, as expected)
  ... (5 crash scripts) ...
  ALL UDH TESTS PASSED

The "Segmentation fault (core dumped)" lines printed between the crash
results are expected: those are the crash scripts doing what they exist
to verify.


RUNNING THE COVERAGE TESTS
--------------------------

  cd ~/git/dwyco/bld/vc/test/udh
  ./run_coverage.sh

This recompiles ONLY bld/vc/vcudh.cpp with -fprofile-arcs -ftest-coverage
at -O0, links two instrumented binaries (the C++ test and a second copy of
the interpreter) with the instrumented object placed ahead of libvc.a,
runs every test through them, then runs gcov. Current numbers:

  == coverage results (vcudh.cpp) ==
    Lines executed:95.90% of 244
    Branches executed:97.19% of 569
  ALL UDH COVERAGE TESTS PASSED

The coverage artifacts are created in a temporary directory that is
deleted on exit; your normal shadow build is not touched (no coverage
flags leak into it).

The crash_udh_*.lh scripts are run by the coverage script too, but a
process killed by SIGSEGV never runs its atexit handlers, so those runs
contribute no gcov data. They are behavioural checks only. Everything
else in the suite does contribute.


KNOWN GAPS IN COVERAGE
----------------------

Only two regions of vcudh.cpp are not covered, and both are unreachable
without fault injection rather than untested by oversight:

  vcudh.cpp:70-86   the init_rng fallback that seeds from the caller's
                    entropy or from srand/rand(). It is only reached if
                    reading /dev/urandom fails, which does not happen in
                    practice. Covering it needs open()/read() interposed
                    to fail for that one path.

  vcudh.cpp:108     sha()'s "if (s.type() != VC_STRING) return vcnil;".
                    Dead code: both call sites (vcudh.cpp:257, :319, :400
                    and :460) pass a VC_BSTRING, and VC_BSTRING is a
                    vc_string, so type() is always VC_STRING.

Beyond that, gcov also reports untaken branches for destructors and
exception-cleanup edges (the "taken 0%" entries on delete/new lines and
around the throw paths). Those are not reachable semantic branches.


KNOWN GAPS AND HAZARDS IN vcudh.cpp
-----------------------------------

Found while writing these tests. None of them is a test failure; each is
pinned by a test so it cannot change silently.

1. No initialization guards. Unlike vcec25519.cpp, which bombs with
   "X25519 not inited", vcudh.cpp dereferences the process-global
   EphDH/UDH/Rng with no check. Calling any entry point before udh_init()
   segfaults. The C++ crash group and the crash_udh_*.lh scripts exist to
   make that failure loud rather than silent.

   THE CRASH TESTS PIN CRASH BEHAVIOUR ON PURPOSE. If someone later adds a
   USER_BOMB guard, those tests turn red by design: the child returns
   normally instead of dying by signal. That is a decision to make
   deliberately, not a flake.

   The statics are never reset, so udh_init() cannot be undone; the crash
   group must stay first in main() and must not call udh_init().

2. Short keys are read out of bounds. vcudh.cpp casts a vc to const byte*
   and hands it to Crypto++ with no length check. Integer(priv, 256) at
   pubkey.h then reads 256 bytes regardless of how long the vc actually
   is. A key shorter than 256 bytes is an out-of-bounds read. Every test
   here uses correctly sized keys, and the negative tests use 256 bytes of
   0xFF (numerically invalid, so DH2::Agree cleanly returns false) rather
   than a short key, precisely to avoid tripping this.

3. The legacy single-key pack has no integrity check.
   dh_store_and_forward_get_key has no key check string, so handing it the
   wrong recipient static returns a DIFFERENT 16-byte key rather than nil.
   The AES/GCM MAC one layer up is what actually catches it. The tests pin
   the "different key, not nil" behaviour so nobody later mistakes "it
   returned a key" for "it returned the right key".
   dh_store_and_forward_get_key2 is the one that checks, via the 3-byte
   AES-ECB key check string, which is why it can return nil.

4. A nil recipient in the group-key slot makes that slot unrecoverable.
   dh_store_and_forward_material2 places recipient j at slots 2j/2j+1, and
   a nil recipient yields nil slots. dh_store_and_forward_get_key2 gates
   the group branch on "sfpack[2] and sfpack[3] are strings", so a nil
   there disables the group loop for every later recipient, silently.
   A nil in the tail (recipient index 2) is harmless. Both are pinned.

5. dh_store_and_forward_get_key2 bombs on an empty material vector where
   the older dh_store_and_forward_get_key returns nil. get_key2 has no
   "our_material is a vector" guard, so our_material[0] becomes nil and
   check_and_get_key then indexes that nil, which is an atomic and raises
   a USER_BOMB. Pinned as "known limitation".

6. Indexing an atomic yields nil, not an error value. vc_default's
   USER_BOMB paths return `vcbitbucket`, and that global is default
   constructed, and a default vc is nil (vc.cpp:69). So a non-vector
   argument turns into nil rather than an error, and that nil then gets
   passed on as key material. Concretely: vclh_sf_material with a
   non-vector other_pub hands nil to DH::Agree as a 256-byte public key,
   which is a read from a null data pointer. What happens next is
   undefined and was observed to differ between -O2 and -O0, so the tests
   deliberately do not assert on it; they probe a bare atomic index
   instead, which is deterministic.

7. USER_BOMB is not null-safe. vcmap.h:21 expands to
   "if (Throw_user_panic) throw -1; user_panic(str);
   Vcmap->set_dbg_backout(); return ret". With Throw_user_panic set, it
   throws before touching Vcmap and is safe anywhere. With it clear and no
   Vcmap, it reaches dobacktrace's "if (Vcmap) ... else oopanic(...)"
   branch (vclh.cpp:106-116) and ABORTS the process. Note vcmap.h already
   provides NONLH_CHECK_ANY_BO with an explicit "if (Vcmap)" guard for
   exactly this reason; USER_BOMB and USER_BOMB2 are the macros that lack
   it. udh_cpp_test.cpp sets Throw_user_panic and creates a Vcmap so it
   does not depend on which path is taken. Candidate upstream fix.

8. UDH-sf-material wants a whole static key vector, not a bare public key.
   It indexes [0] of its first argument (vcudh.cpp:251). Passing a public
   key string instead bombs on the atomic index. Easy trap; the LH test
   comments it.


TWO LH IDIOMS WORTH KNOWING
---------------------------

These cost some time to work out, so they are recorded here.

  A macro parameter is referenced as <name>, not bare name, inside a
  gcompile backtick body. `printl(name)` prints the literal string "name";
  `printl(<name>)` substitutes. (The expect macro in test/ec25519 uses the
  bare form, so its failure labels have always printed "name".)

  Use numelems(), not strlen(), to count vector elements. strlen() only
  accepts strings. Both strlen() and numelems() are correct for binary
  data -- strlen() returns the true length, not a NUL scan -- so strlen()
  is fine for checking key and signature sizes.


REGENERATING THE KNOWN-ANSWER VECTORS
-------------------------------------

The captured vectors pin the 2048-bit DH group that is compiled into
udh_init() in vcudh.cpp, so they are stable. Regenerate them only if that
group changes:

  ./run_tests.sh          # builds udh_cpp_test in a temp dir that is deleted
  # so build it by hand, or use run_coverage.sh's build line, then:
  ./udh_cpp_test --gen-kat

That prints KAT_PRIV, KAT_PUB, the legacy pack (KAT_SF_*) and the
multi-recipient pack (KAT_M2_*). Paste the legacy vectors into BOTH
udh_cpp_test.cpp and udh_kat.lh; they must agree, because the LH KAT
asserts the same known answers the C++ test does. The KAT_M2_* vectors are
C++-only, since material2/get_key2 have no LH builtin.

One thing that looks wrong but is not: the captured static PRIVATE key has
a long run of leading zero bytes. Crypto++ caps the exponent at
2^(2*DiscreteLogWorkFactor(bits)) = 2^226 for a 2048-bit prime
(DL_GroupParameters_IntegerBased::GetMaxExponent, gfpcrypt.cpp:322), and
the key is then encoded into a 256-byte buffer sized from the 2047-bit
subgroup order. So ~226 bits of the 256-byte key are always zero. It is
deterministic, not corrupt.

udh_kat.lh also pins that UDH-sf-get-key on the captured legacy pack
returns exactly the captured session key, that a fresh UDH-sf-material
yields a new ephemeral value and a new session key each time, and that the
wrong static does not reproduce the known answer.


TROUBLESHOOTING
---------------

* "interpreter not found at ..." -> run ./vccmd/build.sh first (from the
  repository root), or set VCCMD=... to point at a vccmd binary.
* "shadow build libs not found in ..." -> run ./vccmd/build.sh first, or
  set SHADOW=... to the correct tree.
* qmake errors -> make sure only Qt5 qmake is on PATH (build.sh exports
  QT_SELECT=5) and that qtbase5-dev is installed.
* the coverage run especially wants a freshly built vcudh.o; re-run after
  make clean in the shadow tree.
* "Segmentation fault (core dumped)" during the run is NOT a problem if it
  appears next to a "died by signal 11, as expected" line. It is a problem
  if it appears next to a FAIL.

# dwyco Tests

Integration and unit tests for the dwyco messaging library, built with CMake.

## Building

From the repository root:

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Or configure from this directory directly:

```bash
cd test_dwyco
mkdir build && cd build
cmake ..
make -j$(nproc)
```

All binaries are placed in the build directory.

## Running with ctest

Everything registered in CMakeLists.txt is also runnable through ctest:

```bash
cd build && ctest --output-on-failure
```

Tests are split into two labels:

| Label | Tests | Needs |
|-------|-------|-------|
| `server-free` | the 5 list binaries, `dwytest_list`, `dwytest_utils` | nothing |
| `server` | `dwytest_local`, `dwytest_settings`, `dwytest_media_state` | a running server; a writable client dir |

`dwytest_local` reads `DWYTEST_DIR`, `DWYTEST_PEER` and `DWYTEST_SEED` from
the environment when the corresponding arguments are not on the command line,
so ctest can be pointed at a set of accounts without editing anything:

```bash
cmake -S . -B build -DDWYTEST_DIR=/tmp/dwyctest \
      -DDWYTEST_PEER=<20 hex chars> -DDWYTEST_SEED=<20 hex chars>
cd build && ctest --output-on-failure
```

Command-line arguments take precedence over the environment. With no uids
configured, `dwytest_local` still runs and the peer-dependent tests report
`SKIP` rather than failing.

## Shared Test Harness

`test_common.h` holds the helpers every binary shares. It is header-only and
expects `dlli.h` to have already been included.

| Helper | Purpose |
|--------|---------|
| `uid_from_hex` / `uid_to_hex` | the only place the ascii-hex <-> binary uid conversion is spelled out. Uids are 10 raw bytes; hex is how they appear on the command line, in log output, and in the database. |
| `test_uid_arg` | decode a uid argument, print a diagnostic on failure |
| `g_peer_uid` / `g_has_peer` | the account under test; sends go here |
| `g_seed_uid` / `g_has_seed` | a third long-running account, needed where a test requires both a pal *and* a non-pal (`dwyco_set_pals_only` filtering), or a group member that is not the peer |
| `service_once` / `service_ms` / `service_step` | pump `dwyco_service_channels`. `service_step` owns the "clamp the dll's requested spin to 1..50ms" rule that used to be copy-pasted into five polling loops. |
| `wait_for` / `wait_for_fn` | poll the service loop until a predicate holds or a timeout expires |
| `run_subprocess` | fork/exec/wait and return the child's exit status, for the few APIs that can `exit()` the calling process (`dwyco_update_server_list`, `dwyco_restore_from_backup`) |
| `dwyco_se_name_lookup` | `DWYCO_SE_*` number to name, for debug traces |
| `install_app_files` / `test_bootstrap_profile` | client dir setup |

## Test Executables

### `dwytest_local` — Main Local Test Suite

Tests core messaging functionality against a live dwyco server. Covers:

- **Composition** — create, duplicate, and delete zap compositions (empty, file, special)
- **Send / Outbox** — deferred send, kill message, save-sent, queued-message status
- **Message Body** — get body text and body array from saved messages
- **Message Index** — retrieve message index with and without load-count limits
- **Tagging** — favorites, set/unset/check tags, count, unset-all, disposition, UID-level tags
- **Pals** — add/delete pal, verify pal list
- **Ignore** — ignore/unignore, verify ignore list
- **System Events** — send-cancel and tag-change events

```bash
# Basic run (no peer — send/pal/ignore tests are skipped)
./dwytest_local /tmp/dwyXYZ

# With a peer UID (20 hex chars = 10 bytes) to enable all tests
./dwytest_local /tmp/dwyXYZ aabbccddeeff00112233

# With a peer and a seed account
./dwytest_local /tmp/dwyXYZ aabbccddeeff00112233 00112233445566778899
```

Arguments may also be given as the environment variables `DWYTEST_DIR`,
`DWYTEST_PEER` and `DWYTEST_SEED` (see [Running with ctest](#running-with-ctest)).

Requires a running dwyco server. The first run creates a new account in the specified directory.

### `dwytest_peer` — Peer-to-Peer Round-Trip Tool

Two modes for testing direct message delivery between two online clients:

```bash
# Send mode: wait for peer to come online, then send
./dwytest_peer send <user_dir> <peer_uid_hex> <text> [no_forward] [attachment_path]

# Receive mode: wait for sender to come online, then wait for a message
./dwytest_peer recv <user_dir> <sender_uid_hex>

# Server mode (skip peer-online check, rely on server delivery)
./dwytest_peer --server send <user_dir> <peer_uid_hex> <text>
./dwytest_peer --server recv <user_dir> <sender_uid_hex>
```

Both modes require a running dwyco server. The peer UID must be 20 hex characters (10 bytes).

### List Helper Tests

Standalone test binaries for the dwyco list API (`dwyco_list_*`):

| Binary | What it tests |
|--------|---------------|
| `test_creation_destruction` | Basic list create/release, 1000 create-release cycles |
| `test_append_operations` | Append int/string/NIL, bulk append, mixed types, boundary conditions |
| `test_lists_12` | Combined creation, destruction, and append coverage |
| `list_test_suite_part1` | Minimal list creation smoke test |
| `list_helpers_test` | List helpers with readback verification |
| `dwytest_list` | The list functions none of the above call: `dwyco_list_print`, `dwyco_list_copy`, `dwyco_list_to_string`, `dwyco_list_from_string`, plus column-count and binary-string behavior |

These do not require a server — they test the list data structure in isolation.

```bash
./test_creation_destruction
./test_append_operations
./test_lists_12
./list_test_suite_part1
./list_helpers_test
./dwytest_list
```

### `dwytest_settings` — Settings, Codec, Runtime State, Contacts

Covers `dwyco_set_setting` / `dwyco_get_setting`, `dwyco_set_codec_data` /
`dwyco_get_codec_data`, `dwyco_get_fn_prefixes`, `dwyco_get_authenticator`, the
invisible / suspend / refresh-users / moron-dork state flags, the contact list
and the aux string, plus the debug entry points.

Needs `dwyco_init()` — the settings live in a `set.sql` that init creates —
but does **not** need a server login. Uses its own client dir at
`/tmp/dwytest_settings`.

```bash
./dwytest_settings
```

#### Documentation bugs found

Two things in `dlli.h` do not match the library:

1. **The header's own example for `dwyco_set_setting` does not work.**
   `dlli.h:2261` says:

   ```
   // dwyco_set_setting("user/email", "foo@bar.com");
   // will set the email address in the user data.
   ```

   It returns 0 and changes nothing. `dwyco_set_setting` rejects the whole
   `user/` group ("user settings cant be set this way anymore"); those
   settings are only reachable through the profile API. Reading `user/*` back
   with `dwyco_get_setting` does work.

2. **There is no `display` setting group.** `dlli.h:2256` lists `display`
   among the valid groups, but nothing is ever registered under it. The real
   groups are `net`, `call_acceptance`, `raw_files`, `user`, `video_format`,
   `video_input`, `zap`, `rate`, `auth`, `group`, `sync`, `server`, `app`,
   plus a typo'd `vid_input`.

#### Other contracts this test pins

- **`dwyco_get_setting` / `dwyco_set_setting` abort on an unknown name.**
  They reach `get_settings_value` / `set_settings_value`, which call
  `oopanic("bad setting")` — `[[noreturn]]`, `exit(1)`. Only the two names
  handled *before* the lookup fail safely: a name with no `/` at all, and
  anything under `user`. Verified in a subprocess.
- **`dwyco_get_setting`'s `*value_out` is a borrowed pointer** into the
  setting's internal storage, not an allocation. Freeing it corrupts the
  settings database. This is the opposite of `dwyco_get_authenticator` and
  `dwyco_get_aux_string`, which *do* hand back `new[]` buffers that need
  `dwyco_free_array` — the latter is documented, the former is not.
- An int setting reads back as decimal ASCII with `DWYCO_TYPE_INT`, the same
  convention `dwyco_list_get` uses.
- **Do not assert setting defaults.** `net/primary_port` is overwritten
  during init with whatever port the listener actually bound; it has been
  observed to differ between runs.
- `dwyco_get_fn_prefixes` returns **void**, normalizes every prefix to end in
  `/`, and offers no "how much space do you need" query — pass buffers that
  are already large enough. Too-small buffers are silently skipped with the
  corresponding length left untouched.
- `dwyco_set_codec_data` stores agc/denoise with `!!`, so any nonzero value
  reads back as 1. It always returns 1.
- `dwyco_get_refresh_users` / `dwyco_set_refresh_users` are **both no-ops** —
  the getter returns a literal 0 and the setter's assignment is commented out
  in the library. The test asserts the setter stays inert.
- `dwyco_set_initial_invis` is a **no-op**; its body is entirely commented out
  in the library. `dwyco_set_invisible_state` is the one that works.
- `dwyco_add_contact` stores all three fields unconditionally, so an empty
  email reads back as an **empty string**, not as nil.
- The contact list is **in-memory only** — nothing is persisted, so it starts
  empty after any init/exit cycle.
- `dwyco_app_debug1` / `dwyco_app_debug2` look printf-shaped but their data
  arguments are declared `int`, not varargs. Passing a `const char *` does not
  compile.

### `dwytest_media_state` — Audio / Pause / Video-Capture State

Covers the audio state pairs (`all_mute`, `auto_squelch`, `full_duplex`,
`exclusive_audio`, `audio_hw`, `max_established_originated_calls`), the
`dwyco_pause_channel_media_*` family, the `dwyco_vfw_*` / preview entry
points, and the three external-driver callback setters.

No peer, no call, and no audio or video hardware required — it is all state
and callback plumbing. It does need `dwyco_init()` for the audio subsystem.
Uses its own client dir at `/tmp/dwytest_media`.

```bash
./dwytest_media_state
```

#### The three driver setters are not equivalent

This is the main thing the test establishes, and it is not visible from the
header:

| Setter | Actually installs? | How it was verified |
|--------|--------------------|---------------------|
| `dwyco_set_external_audio_capture_callbacks` | **yes** | the DLL calls `init` on every `dwyco_get_audio_hw` |
| `dwyco_set_external_audio_output_callbacks` | **yes** | the DLL calls `new` and `init` on every `dwyco_get_audio_hw` |
| `dwyco_set_external_video_capture_callbacks` | **no** | nothing ever calls through it |

`dwyco_set_external_video_capture_callbacks` compiles to an **empty function**
in this build. `test_dwyco_cmake/conf.cmake` sets `DWYCOBG=1`, and
`bld/cdc32/CMakeLists.txt` adds `DWYCO_NO_ACQ_VIDEO_MEDIA` when that is set,
which wraps the entire body in `#ifndef`. So installing video capture
callbacks returns success and does nothing, and `dwyco_get_vfw_drivers`
always comes back empty.

The test asserts that, and **will fail loudly if a future build enables video
capture** — that is deliberate, so the behavior change cannot pass unnoticed.

A subtlety worth knowing if you write more tests here: the audio devices are
created by `check_audio_device()` the first time `dwyco_get_audio_hw` runs
and then stay alive. If any audio call happens before the callbacks are
installed, the devices are built holding the default null callbacks and the
installed ones are never reached. `install_audio_drivers()` therefore runs
immediately after init, before any other audio call.

#### Other contracts this test pins

- **`dwyco_is_preview_on()` is hardcoded `return 0`** and
  **`dwyco_vfw_format()` is hardcoded `return 1`** in the library. Both are
  unconditional, so the test asserts they are stubs.
- Every other `dwyco_vfw_*` / preview entry point returns 1 unconditionally,
  guarding the real work behind a null callback check.
- `All_mute` and `Auto_squelch` both **default to 1**, not 0. Probe both
  directions from a known value rather than assuming the initial state.
- `dwyco_set_max_established_originated_calls` returns the value in effect
  *before* the call, so a sequence of calls reads out the previous values in
  reverse. The default is 4.
- The three `dwyco_pause_channel_media_*` entry points take a channel id; an
  unknown id returns 0 and leaves the out parameters untouched.
- `dwyco_set_full_duplex` returns **void**; `dwyco_get_squelched` has no
  setter at all.
- **The video callback typedefs are internally inconsistent.**
  `hw_preview_on` is `DwycoVVCallback` (takes `void *`) but `hw_preview_off`
  is `DwycoVCallback` (takes nothing), even though the prose above them
  documents both as no-argument. A driver has to match the typedefs, not the
  comment.

### `dwytest_utils` — Crypto / String / Password Utilities

Covers the standalone helpers that never touch the network:
`dwyco_random_string2`, `dwyco_eze2`, `dwyco_ezd2`, `dwyco_load_file_e`,
`dwyco_gen_pass`, `dwyco_write_token`, `dwyco_free`, `dwyco_free_array`.

Like the list tests this needs no account and no `dwyco_init()` — the entropy
pool, the eze key and the file-encryption context are all initialized lazily
on first use.

```bash
./dwytest_utils
```

#### Defect found: `dwyco_ezd2` aborts on input shorter than 8 bytes

`dwyco_ezd2` in `bld/cdc32/dlli.cpp` has no minimum-length check:

```cpp
vc iv(VC_BSTRING, str, 8);                 // overreads when len_str < 8
vc es(VC_BSTRING, str + 8, len_str - 8);  // negative length when len_str < 8
```

With `len_str < 8` the second constructor gets a negative length and throws
`std::bad_alloc` from inside the `DwString` constructor. Nothing catches it,
so the process terminates. `dlli.h` documents no such precondition, and
passing a short buffer is not a null-pointer mistake — it is a length this
function never validated.

`dwytest_utils` pins the current behavior in a subprocess
(`ezd2_short_input_aborts`) because an uncaught exception from inside the
library cannot be caught in-process. **If the library is fixed to reject
short input, that test will fail** — that is deliberate, so the fix shows up
as a test change rather than silently.

Other contracts this test pins, none of which are in the header:

- `dwyco_eze2` / `dwyco_ezd2` return **void**. There is no return code.
  `ezd2` signals failure by setting `*str_out` to 0 and leaving `*len_out`
  untouched, so callers must branch on the pointer and must not read
  `*len_out` on the failure path.
- `dwyco_random_string2` has **no length output** and does **not**
  NUL-terminate. It also produces *at least* the requested number of bytes
  rather than exactly that many — it appends whole entropy chunks, so a
  request for 16 has been observed to return 20.
- `dwyco_load_file_e` **decrypts**. An existing but unencrypted file is
  rejected exactly like a missing one; the only files it will load are ones
  the library wrote itself.
- `dwyco_write_token` writes `token.dif` relative to the process working
  directory, **not** against the `dwyco_set_fn_prefixes` user directory.
- `dwyco_gen_pass`'s salt argument is in/out: pass length 0 to have a salt
  generated, or pass a salt back in to reproduce a hash.
- `dwyco_free_image` is *not* exercised — it is
  `ppm_freearray((pixel **)p, rows)` and needs a real ppm pixel array. The
  only API that produces one is `dwyco_zap_create_preview_buf`, which needs
  a zap view, so that pairing is covered alongside attachments instead.

### Things `dwyco_list_*` does that are easy to get wrong

Found while writing `dwytest_list`, recorded here because they are all
surprising and none are in the header comments:

- **A flat list reports `cols == -1`, not 1.** `dwyco_list_numelems` only
  returns a real column count when row 0 is itself a vector. An empty list
  reports `rows == 0, cols == -1` too.
- **Never index past the end.** `dwyco_list_get` calls `oopanic()`, which is
  `[[noreturn]]` and does `exit(1)`. This is an intentional guard, so
  `dwytest_list` verifies it by re-exec'ing itself as a subprocess and
  checking the child exits 1 — it cannot be caught in-process.
- **The serialized form is not printable.** `dwyco_list_to_string` output
  ends in a type byte, so it contains NULs and high bytes. Always use the
  returned length, never `strlen`.
- **`dwyco_list_to_string` allocates with `new[]`**, so the result must be
  released with `dwyco_free_array`, not `dwyco_free`.
- **`dwyco_list_get` on an int hands back decimal ASCII** with
  `type_out == DWYCO_TYPE_INT`, which is why `list_readback.h`'s `lr_int`
  formats the expected value as a string before comparing.
- **`dwyco_list_get` returning a vector** (i.e. a column access that landed
  on a nested vector) silently yields `DWYCO_TYPE_NIL` with the string
  `"nil"` and `len_out` set to the *element count*, not a string length.
- **`dwyco_list_from_string` does not write `*list_out` when it fails**, so
  callers can safely leave the pointer initialized and branch on the return.
- **Multi-column lists cannot be built through the public API** — there is
  no `dwyco_list_append_col`. They only come out of real API calls
  (`dwyco_uid_to_info`, `dwyco_get_server_list`, …), so `list_readback.h`
  provides `lr_cols` / `lr_col_str` / `lr_col_int` / `lr_col_nil` for
  reading them back by column name.

### Utility Binaries

| Binary | Purpose |
|--------|---------|
| `create_account` | Create a new dwyco account. Prints UID on success. |
| `dump_uid` | Create account and print its UID (hex). |
| `add_pal` | Add a pal to an existing account. |
| `join_group` | Join a named group with a password. |
| `dwycobg` | Background group sync daemon. |

```bash
# Create an account (exits immediately with 3 args, waits 2 min with 2)
./create_account /tmp/dwyXYZ dummy

# Dump UID
./dump_uid /tmp/dwyXYZ

# Add a pal
./add_pal /tmp/dwyXYZ aabbccddeeff00112233

# Join a group
./join_group /tmp/dwyXYZ mygroup mypassword
```

## Shell Scripts

### `test_roundtrip.sh` — Round-Trip Message Tests

Coordinated end-to-end test: starts a receiver and sender, verifies message delivery. Tests simple text, no-forward, special characters, and attachments (small inline + large out-of-line).

```bash
./test_roundtrip.sh <user_a_dir> <user_a_uid_hex> <user_b_dir> <user_b_uid_hex>
```

Requires both accounts to exist and be pals on the server.

### `check_group_key_identical.sh` — Group Key Consistency Check

Verifies that the `dhg.sql` private key records are identical across 500 clients in `/tmp/dwy0` through `/tmp/dwy499`.

```bash
./check_group_key_identical.sh [group_name]
```

### `create_500_join_group.sh` — Bulk Client Creation

Creates 500 client accounts and has each join a group. Requires an existing group member to be alive.

```bash
./create_500_join_group.sh
```

### `create_and_enter_group.sh` — Single Client Creation + Group Join

Creates one account and joins a group. Used by `create_500_join_group.sh`.

```bash
./create_and_enter_group.sh [user_dir] [group_name]
```

### `start_group_bg.sh` — Background Group Sync

Starts `dwycobg` in group sync mode on a client directory. Prints PID and port.

```bash
./start_group_bg.sh <client_dir> <group_name> [password] [build_dir]
```

### `stress_create_acct.sh` — Account Creation Stress Test

Creates 500 accounts in parallel.

```bash
./stress_create_acct.sh
```

## Prerequisites

- CMake >= 3.14
- C++17 compiler (g++ or clang++)
- Running dwyco server (for all tests except the list helper tests)
- `sqlite3` CLI (for `check_group_key_identical.sh`)
- `python3` (for attachment generation in `test_roundtrip.sh`)

## Directory Layout

```
test_dwyco/
├── CMakeLists.txt              # Build configuration
├── test_main.cpp               # dwytest_local source
├── test_peer.cpp               # dwytest_peer source
├── test_common.h               # Shared harness: uid codec, service loop, peer/seed slots
├── test_creation_destruction.cpp
├── test_append_operations.cpp
├── test_lists_12.cpp
├── list_test_suite_part1.cpp
├── list_helpers_test.cpp
├── dwytest_list.cpp            # list copy/serialize/print coverage
├── dwytest_utils.cpp           # eze2/ezd2/gen_pass/load_file_e/random_string2
├── dwytest_settings.cpp        # set/get_setting, codec, state flags, contacts
├── dwytest_media_state.cpp     # audio/pause state, vfw, external driver callbacks
├── list_readback.h             # List readback helpers
├── create_account.cpp
├── dump_uid.cpp
├── add_pal.cpp
├── join_group.cpp
├── dwyco_new_msg.cpp/h         # New message handling
├── dlli.h                      # dwyco API header
├── dumpxfer.lh                 # Script to dump dwyco xfer files
├── app/                        # App files (pub key, servers2, no_img.png)
├── test_roundtrip.sh
├── check_group_key_identical.sh
├── create_500_join_group.sh
├── create_and_enter_group.sh
├── start_group_bg.sh
└── stress_create_acct.sh
```

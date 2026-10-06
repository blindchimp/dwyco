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
| `server` | `dwytest_local`, `dwytest_settings`, `dwytest_media_state`, `dwytest_users`, `dwytest_tags`, `dwytest_msg`, `dwytest_attach`, `dwytest_profile`, `dwytest_calls` | a running server; a writable client dir |

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

### `dwytest_users` — Users, Presence, UID Resolution, Trash, SQL

Covers `dwyco_get_user_list2`, `dwyco_load_users2`, `dwyco_load_users_internal`,
`dwyco_get_updated_uids`, `dwyco_uid_status`, `dwyco_uid_online`, `dwyco_uid_g`,
`dwyco_uid_to_info`, `dwyco_uid_to_ip` / `_2`, `dwyco_map_uid_to_uids`,
`dwyco_map_uid_to_representative`, `dwyco_name_to_uid`, `dwyco_delete_user`,
`dwyco_clear_user`, `dwyco_fetch_info`, the trash API and `dwyco_run_sql`.

Uses its own client dir at `/tmp/dwytest_users`. The uid-resolution test needs
a server login and skips without one.

```bash
./dwytest_users
```

#### Documentation bug: `dwyco_uid_status` never returns 0 or 1

`dlli.h:780` documents:

```
// 0 for offline
// 1 for online, not available
// 3 for online, available
```

The implementation is `uid_online_display(v) | 2`, and
`uid_online_display()` only ever returns 0 or 1. So **0 and 1 are unreachable**
— the real values are 2 (offline) and 3 (online). The bit meanings still hold:
bit 0 is online-ness, bit 1 is always set.

`dwyco_uid_status_never_returns_0_or_1` pins this. If the header is corrected
to match reality, or the `| 2` is dropped, the test fails.

#### `dwyco_run_sql` is only safe with valid SQL

The library's `sql_run_sql` wraps statements in try/catch and rolls back, and
`dwyco_run_sql` maps a nil result to 0 — but a statement SQLite rejects makes
the SQL layer call `user_panic()` → `exit(1)` before the catch can help, and
an empty statement segfaults:

| statement | result |
|-----------|--------|
| `select 1` | returns 1 |
| `create table ...`, `insert ...`, `select ...`, `drop table` | return 1 |
| `this is not sql` | `exit(1)` |
| `select * from no_such_table` | `exit(1)` |
| `""` (empty) | `SIGSEGV` |

So the 0/1 return only distinguishes "ran fine" from "did not get that far".
Valid SQL including bound `?1`/`?2`/`?3` is tested in-process; the failure
cases are tested out-of-process. Note `dwyco_run_sql` binds at most **three**
positional arguments.

#### Other contracts this test pins

- `dwyco_uid_to_info` falls back to the uid's **ASCII hex** in the
  `DWYCO_INFO_HANDLE` column when the uid cannot be resolved, and returns a
  1-row × 6-column record either way.
- `dwyco_uid_to_ip` hands back a pointer straight out of `inet_ntoa()`, i.e.
  static storage: copy it out, never free it. `dwyco_uid_to_ip2` by contrast
  allocates an `"ip:port"` string that **does** need `dwyco_free_array` — and
  on failure it returns 0 **without writing `*str_out` at all**.
- `DWYCO_SE_IDENT_TO_UID` carries the queried handle in the **value**
  parameter; the name parameter is always null for this event. The resolved
  uid is in the uid parameter, empty when the handle does not resolve.
- `dwyco_uid_g` returns 1 unconditionally for one hardcoded uid
  (`5a098f3df49015331d74`) — a backdoor, pinned so nobody is surprised.
- `dwyco_get_user_list2`'s `nelems_out` always equals the row count.

### `dwytest_tags` — Message Tagging

Covers the whole tag API, and fixes the fact that the existing tag tests in
`test_main.cpp` were crash-checks at best.

```bash
./dwytest_tags
```

#### Why the old tag tests proved nothing

`test_main.cpp` tagged mids like `"test_mid_001"` that exist in no index, and
the comment there admitted `get_tagged_mids` returns nothing for them. The
reason is a real and testable split in the SQL layer:

| API | reads |
|-----|--------|
| `dwyco_count_tag`, `dwyco_get_tagged_mids_older_than` | `mt.gmt` only |
| `dwyco_valid_tag_exists`, `dwyco_get_tagged_mids`, `dwyco_uid_has_tag`, `dwyco_uid_count_tag`, `dwyco_all_messages_tagged` | `mt.gmt` **inner-joined to `gi`** |

So a tagged mid with no global-index row is *counted* yet invisible to
everything else. `synthetic_mids_are_invisible_to_the_gi_joins` pins exactly
that, and `test_main.cpp`'s tag tests have been updated to assert it.

To cover the **positive** path without depending on a second live process,
this binary inserts a row straight into `gi` with `dwyco_run_sql` — the
documented debugging escape hatch. The cost is that these tests know about
two table layouts; if either schema changes the fixture stops working and says
so.

#### Remaining defect: `dwyco_get_tagged_mids2` always terminates the process

Its body is a bare `oopanic("broken")` with the real code commented out
behind it, so calling it kills the process for *any* tag. The header documents
it as an ordinary function with no such restriction.
`get_tagged_mids2_terminates` pins this out-of-process and will fail if it is
ever implemented. Callers needing uid/mid pairs should use
`dwyco_get_tagged_mids`, whose uid column really is the hex form.

#### Other contracts this test pins

- `dwyco_get_tagged_mids` returns 2 columns, and `DWYCO_TAGGED_MIDS_HEX_UID`
  really is ASCII hex as the header warns.
- `dwyco_get_mid_tag_payload` returns 0 for an empty mid or tag **without
  touching `payload_out`**; otherwise 1 plus a 1-element list that is
  `DWYCO_TYPE_NIL` when no payload is stored.
- `dwyco_all_messages_tagged` is the complement of `dwyco_uid_has_tag` — "are
  **all** messages tagged" vs "is **any** tagged". Its query reads as the
  inverse (it selects untagged messages and returns "there was one"), but the
  observable result matches the name.
- Setting the same `(mid, tag)` twice is idempotent; `gmt` is keyed on
  `(mid, tag, uid, guid)`.
- `dwyco_get_tagged_mids_older_than` filters on the tag's own timestamp and
  does **not** join `gi`. Note its window compares against SQLite's
  `strftime('%s','now')`, which is UTC, while `time(NULL)` is local — the
  test keeps its boundaries far apart so that skew cannot make it flaky.

### `dwytest_msg`, `dwytest_attach`, `dwytest_profile` — Two-Client Tests

These three need a real message to work with, so each re-executes itself as
a second dwyco client: the parent is the receiver and spawns a child running
`--peer-send` / `--peer`, which is a separate process with its own account.
Same binary on both sides so the init sequence is identical.

| Binary | Covers |
|--------|--------|
| `dwytest_msg` | `get_saved_message3`, `get_body_text`, `get_body_array`, `authenticate_body`, `is_special_message`(`2`), `is_delivery_report`, `get_new_message_index`, `get_message_bodies`, `save_message`, `clear_user_unfav`, `qd_message_to_body`, `cancel_message_fetch` |
| `dwytest_attach` | `make_zap_composition_raw`, `set_special_zap`, `is_forward_composition`, `make_forward_zap_composition2`, `zap_send4`, `zap_cancel`, `zap_still_active`, `zap_composition_chan_id`, `copy_out_file_zap2`, `copy_out_file_zap_buf2`, `make_zap_view2`, `make_zap_view_file`(`_raw`), `delete_zap_view`, `zap_quick_stats_view`, `zap_stop_view`, `zap_play_view_no_audio`, `zap_create_preview`(`_buf`) |
| `dwytest_profile` | `create_bootstrap_profile`, `make_profile_pack`, `set_profile_from_composer`, `get_profile_to_viewer`, `get_profile_to_viewer_sync` |

`dwytest_attach` proves the **forward round trip**: the receiver forwards a
message back to the sender, and the sender confirms the message it received
has 2 body components instead of 1. Nothing tested that before.

#### The sender's directory is a deliberate cache

Each of these keeps its **sender** data directory between runs, and wipes only
its own. This is load-bearing, not laziness: a brand new account registers
with the server asynchronously, and messages sent during that window are
accepted locally (`dwyco_zap_send6` returns nonzero and
`DWYCO_SE_MSG_SEND_SUCCESS` fires) but never actually routed — so nothing
arrives and the test hangs for its full timeout. With the sender's directory
in place the tests are reproducible run to run. The receiver may be fresh
every time; sending outbound from a new account is fine.

#### How a received message actually shows up

The single most important thing learned here: **a message that arrives
peer-to-peer never appears in `dwyco_get_unfetched_messages()`**. It goes
straight into the local message table tagged `"_inbox"`. A receiver that only
polls the unfetched queue sees nothing at all, even with the message sitting
right there. The working loop is:

1. if the rescan flag is set, clear it and process the unfetched queue — this
   is how *server-queued* messages get fetched
2. **every** iteration, call `dwyco_new_msg2()`, which reads the `_inbox` tag

Step 2 is the one that catches direct messages. `dwytest_peer` already did
this; the first version of these tests did not, and hung silently.

Second gotcha: `install_app_files()` must run before `dwyco_init()`, or the
client has no `servers2`, silently falls back to the compiled-in production
server list, logs in successfully, and then never receives anything.

#### Defect: `dwyco_get_user_payload` faults on any non-special message

The function's job is to return 0 when a message carries no user payload, but
the check that would make that safe is commented out in
`bld/cdc32/dlli.cpp`:

```cpp
vc sv = body[QM_BODY_SPECIAL_TYPE];
//  if(sv[0] != vc("user")) return 0;     <-- disabled
vc msg_type_vec = sv[1];                 <-- faults when sv is nil
```

For an ordinary text message `SPECIAL_TYPE` is nil, so `sv[1]` indexes a nil
container and the process dies:

```
runtime error: can't do set operation on atomic (4)
```

So the function is only safe on a message that actually *is* a user-defined
special message, and the header does not say so. Pinned out-of-process.

#### Defect: `dwyco_make_zap_view_file` always terminates the process

```cpp
m->actual_filename = newfn(filename);
```

`newfn()` → `filename_modify()` maps a filename onto one of dwyco's known file
types via a perfect hash on the trailing suffix, and for anything else does
`oopanic("<fn> didn't match anything")` — `[[noreturn]]`, `exit(1)`. So the
function kills the process for *any* filename that is not one of dwyco's own
types: a `.png`, a `.jpg`, anything a caller would plausibly want to view.
Verified with a plain PNG in `/tmp` and with the bundled app PNG; both exit.

`dwyco_make_zap_view_file_raw` is the escape hatch — it assigns
`actual_filename` directly and works on any path. `dlli.h` documents no such
restriction on either function.

#### Other contracts these tests pin

- `dwyco_make_profile_pack` writes into a **function-static** buffer. The
  pointer can even move between calls, so the value must be copied out
  immediately — and must **not** be freed, because it is neither a `new[]`
  buffer nor a borrowed pointer. Every other string-returning call in this API
  is one or the other.
- `DwycoProfileCallback`'s `s1` is the profile **handle**, not the
  description. `get_peer_profile` asserts the handle we expect comes back.
- `dwyco_make_zap_composition_raw` only accepts a filename ending in `.dyc`
  or `.fle` — it checks the extension and does **not** validate the contents, so
  a renamed text file is accepted.
- `dwyco_make_zap_view_file`/`_raw` do not return 0 on failure; they hand back
  `DwVP`'s invalid-cookie sentinel `0x55555555`. Every consumer validates it,
  so it is safe to pass on — but a caller testing `if (viewid > 0)` will treat
  it as success.
- `DWYCO_QM_BODY_FROM` is a **binary** uid column, while the index's
  `DWYCO_MSG_IDX_ASSOC_UID` and `dwyco_get_tagged_mids`' uid column are
  **hex**. Easy to get wrong; both are asserted.
- `DWYCO_MSG_IDX_IS_SENT` is nil (not `"0"`) for a received message.

### `dwytest_calls` — Call / Channel State

Covers selective chat, pals-only filtering, the channel/call lookups with an
unknown id, keyboard input, and all the call/chat callback registrations.
Needs no peer and no media hardware.

```bash
./dwytest_calls
```

#### Defect: `dwyco_set_zap_appearance_callback` always terminates

```cpp
oopanic("zap appearances not supported anymore");
//zap_appearance_callback = cb;
```

Zap appearances were removed but the declaration was left in `dlli.h` as an
ordinary setter, so a client that still installs one dies at startup with a
confusing message rather than getting a link error or a no-op. Pinned
out-of-process.

#### Not covered here, and why

`dwyco_connect_uid`, `dwyco_connect_all4`, `dwyco_connect_msg_chan`, the
`DWYCO_CSC_ACCEPT`/`DEFER`/`REJECT` screening matrix, and the
`DWYCO_CALLDISP_*` dispositions all need two endpoints that can actually
establish a media session. This build reports no audio hardware
(`dwyco_get_audio_hw` gives no input and no output), so those are untestable
here. They are the natural next chunk and would have to run against a live
peer with working capture devices.

#### Other contracts this test pins

- `dwyco_zap_accept` and `dwyco_zap_reject` are hardcoded `return 0` — not
  broken, just unimplemented. A test asserting 1 would be wrong.
- **Selective chat needs a live private chat session.** Both halves bail out
  with 0 when there is no "message xmitter" channel, so they always report
  failure and their state is unobservable. `dlli.h` documents no such
  precondition. The positive path belongs with the chat-server work.
- `dwyco_set_pals_only` is backed by the `zap/ignore` setting, and
  `dwyco_get_pals_only` reads it back, so the pair round-trips.
- Every channel/call-keyed function validates its id and returns 0 (or −1 for
  `dwyco_chan_to_call`) for an unknown one, leaving out parameters untouched.
- `dwyco_channel_create` to an unreachable address reports
  `DWYCO_CALLDISP_STARTED` synchronously. That is fine; a synchronous
  `DWYCO_CALLDISP_ESTABLISHED` would not be, and is asserted against.

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

#### Setting groups and the `user` group

`dwyco_set_setting` rejects the whole `user/` group ("user settings cant be
set this way anymore"); those settings are only reachable through the profile
API. Reading `user/*` back with `dwyco_get_setting` does work. There is no
`display` setting group — nothing is ever registered under it.

Both of these used to be wrong in the header, which listed `display` and
`user` among the valid groups and gave `dwyco_set_setting("user/email", ...)`
as the worked example. The header now lists `net`, `call_acceptance`,
`raw_files`, `video_format`, `video_input`, `zap`, notes that `user` is
profile-managed, and uses `zap/save_sent` as the example.

Beyond the documented groups, these also exist: `rate`, `auth`, `group`,
`sync`, `server`, `app`, and a typo'd `vid_input`.

#### Other contracts this test pins

- **`dwyco_get_setting` / `dwyco_set_setting` abort on an unknown name.**
  They reach `get_settings_value` / `set_settings_value`, which call
  `oopanic("bad setting")` — `[[noreturn]]`, `exit(1)`. Only the two names
  handled *before* the lookup fail safely: a name with no `/` at all, and
  anything under `user`. Verified in a subprocess.
- **`dwyco_get_setting`'s `*value_out` must be copied out immediately and
  must not be freed.** Like the `dwyco_list_get` family, it hands back a
  pointer into the value's own storage. This is the opposite of
  `dwyco_get_authenticator` and `dwyco_get_aux_string`, which *do* return
  `new[]` buffers that need `dwyco_free_array`.
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

#### Remaining defect: `dwyco_ezd2` off-by-one at exactly 8 bytes

`dwyco_ezd2` used to have no length check at all and computed
`len_str - 8`, which went negative and threw an uncaught `std::bad_alloc`
out of the `DwString` constructor, terminating the process.

That is fixed, but the new guard is off by one:

```cpp
if(len_str < 8) { *str_out = 0; return; }   // should be <= 8
...
vc es(VC_BSTRING, str + 8, len_str - 8);   // len_str == 8 -> es is EMPTY
```

At exactly 8 bytes the guard passes, `es` is empty, and the blowfish
decoder's `USER_BOMB` fires — `user_panic()` → `exit(1)`, printing
`runtime error: BF-xfer-dec arg must be vector(iv, string)`.

Measured behavior across lengths:

| `len_str` | result |
|-----------|--------|
| 0–7 | `*str_out = 0`, `*len_out` untouched |
| **8** | **`exit(1)` via `user_panic`** |
| 9 and up | `*str_out = 0`, `*len_out` untouched |

The shortest string `dwyco_eze2` can produce is 16 bytes (8 IV + one block),
so the real precondition is `len_str > 8`. `ezd2_exactly_iv_length_still_exits`
pins this out-of-process and **will fail once the guard is corrected** — that
is deliberate.

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

### Functions that cannot be called at all

`dlli.h` declares **18** functions that have no definition anywhere in the
library. Calling any of them is a link error, so they are excluded from the
tests entirely. A further 15 are inside `#if 0` blocks and are not declared to
callers at all.

Both groups are intentional dead code (pal auth was retired, most of the rest
are Windows-only or superseded), but the visible ones are a trap for anyone
reading the header.

**Declared but undefined — calling these will not link:**

| Function | Header | Why |
|----------|--------|-----|
| `dwyco_update_profile`, `dwyco_remove_profile` | `:593`, `:592` | marked `// not impl.`; zero definitions repo-wide |
| `dwyco_set_group_profile_from_composer`, `dwyco_get_group_profile`, `dwyco_get_group_profile_sync`, `dwyco_get_group_profile_by_name` | `:609`–`:628` | absent from `bld/cdc32/dlli.h` *and* from `dlli.cpp` — `test_dwyco/dlli.h` advertises intended-but-unfinished work |
| `dwyco_set_auto_reply_msgNA` | `:1034` | the whole auto-reply feature is `#if 0`'d out in `dlli.cpp` |
| `dwyco_get_lobby_name_by_id` | `:1450` | commented out; superseded by `dwyco_get_lobby_name_by_id2` |
| `dwyco_inhibit_chat` | `:1353` | commented out |
| `dwyco_set_login_password` | `:1340` | commented out |
| `dwyco_set_chat_server_status_callback` | `:411` | commented out |
| `dwyco_set_main_msg_window`, `dwyco_handle_msg` | `:1596`, `:1597` | Windows-only, and not defined for any platform |
| `dwyco_is_capturing_video` | `:1041` | Windows-only, `return 0` stub where it is defined |
| `dwyco_request_singleton_lock` | `:1639` | Android-only |

**Inside `#if 0` — not declared at all:** the pal-auth family
(`dwyco_get_pal_auth_state`, `dwyco_set_pal_auth_state`,
`dwyco_get_my_pal_auth_state`, `dwyco_get_pal_auth_warning`,
`dwyco_pal_auth_granted`, `dwyco_handle_pal_auth`, `dwyco_handle_pal_auth2`,
`dwyco_revoke_pal_auth`, `dwyco_clear_pal_auths`,
`dwyco_set_pal_auth_callback`), the visibility family
(`dwyco_always_visible`, `dwyco_never_visible`, `dwyco_is_always_visible`,
`dwyco_is_never_visible`), and `dwyco_set_video_display_init_callback`.

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
├── dwytest_users.cpp           # user list, presence, uid resolution, trash, run_sql
├── dwytest_tags.cpp            # message tagging, incl. hand-built gi fixtures
├── dwytest_msg.cpp             # received-message API (spawns a 2nd client)
├── dwytest_attach.cpp          # attachments, views, forwarding (spawns a 2nd client)
├── dwytest_profile.cpp         # profiles (spawns a 2nd client)
├── dwytest_calls.cpp           # call/channel state, pals-only, callback registration
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

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
```

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

These do not require a server — they test the list data structure in isolation.

```bash
./test_creation_destruction
./test_append_operations
./test_lists_12
./list_test_suite_part1
./list_helpers_test
```

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
├── test_common.h               # Shared test utilities
├── test_creation_destruction.cpp
├── test_append_operations.cpp
├── test_lists_12.cpp
├── list_test_suite_part1.cpp
├── list_helpers_test.cpp
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

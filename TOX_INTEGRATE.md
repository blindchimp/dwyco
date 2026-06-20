# Integration Guide: Adding a New Messaging System to Dwyco

Based on the Tox messaging integration work across `ai-tox-experiment` and `ai-change-course` branches.

## Architecture Overview

The integration follows a **3-layer architecture**:

```
UI Layer (QML/C++)        phoo/*.qml, dwyco_top.cpp, toxfriendmodel.cpp
Public API Layer           bld/cdc32/dlli.cpp, dlli.h
Bridge/Integration Layer  bld/cdc32/toxbridge.h, toxbridge.cpp
Protocol Plugin Library   toxd/toxd_plugin.h, toxd/toxd.cpp
                         (links the actual messaging library, e.g., libtoxcore)
```

**Key decision**: The `ai-change-course` branch refactored away from a separate-process model (fork/exec toxd binary, RPC over pipes) to a **directly-linked in-process library**. The protocol library (`toxd.cpp`) is compiled directly into `cdc32` via `CMakeLists.txt`. This is the pattern to follow.

## UIDs: How They Are Mapped

### The Pseudo-UID Concept

The messaging system has its own identity format (e.g., 32-byte NaCl public key for Tox). Dwyco UIDs are compact binary strings. The integration creates a **pseudo-UID** by taking a prefix of the native identity:

```cpp
pseudo_uid = first_10_bytes(native_public_key)
```

File: `toxbridge.cpp:41-47`

This pseudo-UID is used everywhere the system expects a UID. The mapping is **deterministic** (always derivable from the full public key) and **one-way** (you can't recover the full key from the pseudo-UID alone).

### Resolution Paths

When a pseudo-UID needs to be resolved to a display name, the system tries these paths in order:

1. **In-memory `Friend_cache`** (static `vc` vector in `toxbridge.cpp`): Full friend list with `{friend_number, pubkey, name, status, user_status}`. Updated each poll cycle.
2. **`Session_infos` map** (in-memory): Maps `pseudo_uid -> {name, ...}`. Populated from friend cache rebuild and from CRDT tag imports.
3. **CRDT `_tox_friend` tags** (SQLite message DB): Tags stored as `hex(pseudo_uid) + "_" + hex(name)`, synced across devices. Allows non-tox clients to see tox friend names.
4. **Fallback**: Display the hex-encoded pseudo-UID if nothing else found.

### Forward/Reverse Lookups

- `pseudo_uid -> friend_number`: Iterates `Friend_cache`, computes `tox_pubkey_to_pseudo_uid(pubkey)` for each entry until match (`toxbridge.cpp:479`)
- `friend_number -> pseudo_uid`: Iterates `Friend_cache`, looks up by friend_number field (`toxbridge.cpp:509`)
- `is_tox_uid(pseudo_uid)`: Checks the `tox_uid_type` SQL table + searches `Friend_cache` (`toxbridge.cpp:576`)

## Send Messaging Flow

```
User types message in QML
  -> SimpleChatBox.qml calls top_dispatch.send_message(uid, text)
  -> DwycoCore::send_message() in dwyco_top.cpp
  -> dwyco_zap_send6() in dlli.cpp
     -> Checks tox_bridge_is_tox_uid(uid)
     -> If yes: tox_bridge_send_message_by_uid(uid, text, 0)
     -> If no: normal Dwyco send path
```

### `tox_bridge_send_message_by_uid()` (toxbridge.cpp:824)

1. Resolves `pseudo_uid -> friend_number` via `tox_pseudo_uid_to_friend_number()`
2. Generates a `local_mid = to_hex(gen_id())` (unique identifier)
3. Constructs a full QQM message body (Dwyco's standard message format) with recipient, text, timestamp
4. **Enqueues** the serialized QQM blob to `tox_outbox` SQL table with `status=0` (pending)
5. Returns immediately to the caller

### `tox_bridge_send_queued()` (called from `tox_bridge_poll()`, toxbridge.cpp:861)

1. On each poll cycle, dequeues oldest pending message from `tox_outbox`
2. Resolves friend_number again
3. Deserializes QQM blob, extracts text
4. Calls `tox_bridge_send_message()` -> `toxp_message_send()` -> `tox_friend_send_message()` (the actual toxcore API)
5. On success: marks status=1 (in-progress), stores toxcore's message ID
6. On permanent failure: marks status=3 (failed), emits `SE_MSG_SEND_FAIL`
7. **Stale timeout**: Messages stuck in status=1 for >30 seconds are reset to status=0 for retry

### Read Receipt

When toxcore reports delivery: `process_tox_event("read_receipt")`
1. Queries `tox_outbox` for matching `recipient_pseudo + tox_mid`
2. Deserializes QQM blob, saves to `.q` file via `save_info()` (converts to a permanently stored message)
3. Deletes row from `tox_outbox` (status=sent)
4. Emits `SE_TOX_READ_RECEIPT`

## Receive Messaging Flow

```
toxcore callback fires
  -> toxd_on_friend_message() in toxd.cpp:115
     -> Packages {fn, pubkey, text, msg_type} into vc args
     -> Calls p->event_cb("message", args, p->event_userdata)
```

### `process_tox_event("message", args)` (toxbridge.cpp:93)

1. Extracts pubkey from args, computes `pseudo_uid = tox_pubkey_to_pseudo_uid(pubkey)`
2. Adds pseudo_uid to `tox_uid_type` SQL cache
3. Constructs a full QQM message body: from-uid, text, timestamp, logical clock, local_id
4. Calls `store_direct(0, qqm, 0)` to store the message directly in the main message database
5. Emits `SE_TOX_MESSAGE` system event

### System Event Processing

Events are dispatched through `se.cpp` -> `se_process()` -> `DwycoCore::dwyco_sys_event_callback()` in `dwyco_top.cpp`:

| Event | Handler |
|---|---|
| `SE_TOX_MESSAGE` | Generic unread-count update, conversation refresh |
| `SE_TOX_FRIEND_NAME` | `sys_uid_resolved(huid)` signal -> display name refresh |
| `SE_TOX_FRIEND_STATUS` | `sys_uid_resolved(huid)` signal |
| `SE_TOX_TYPING` | `sc_rem_keyboard_active(huid, typing)` -> typing indicator |
| `SE_TOX_SELF_CONNECTION_STATUS` | Updates `tox_connected` property |
| `SE_TOX_FRIEND_REQUEST` | Auto-accepts via `dwyco_tox_accept_friend_request()` |
| `SE_TOX_READY` | Signals bridge initialized, gets self-address |

## Message Queue / Caching

### SQLite Outbox: `tox_outbox` table

```sql
CREATE TABLE tox_outbox (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    qqm_blob BLOB,                    -- serialized QQM message
    recipient_pseudo BLOB,            -- target pseudo-UID
    status INTEGER DEFAULT 0,         -- 0=pending, 1=in-progress, 3=failed
    tox_mid INTEGER,                  -- protocol message ID (from send)
    local_mid TEXT,                   -- local unique ID (hex string)
    created_at INTEGER                -- unix timestamp
);
```

### Integration with QD (Queued Delivery) API

The `ToxQueue` class exposes two methods that integrate into the existing `dwyco_get_qd_messages()` and `dwyco_qd_message_to_body()` APIs:

- `get_qd_msgs(pseudo_uid)`: Returns pending/in-progress/failed messages from `tox_outbox` in the same format as `load_q_files()`, so they appear in the "pending messages" UI
- `load_qd_body(local_mid)`: Loads the full QQM body for a specific queued message

### Stored Messages

Received messages go through `store_direct()` (the normal Dwyco message storage path). Read-receipted outbound messages are converted to `.q` files via `save_info()`.

## Friend List Caching

### In-Memory `Friend_cache`

- Populated by `tox_bridge_rebuild_friend_cache()` called from `tox_bridge_poll()`
- Each entry: `{friend_number, pubkey, name, status (online/offline), user_status (none/away/busy)}`
- Used for all lookups (pseudo-uid -> name, pseudo-uid -> friend_number, etc.)

### UI-Side Model: `ToxFriendModel`

- C++ class in `phoo/toxfriendmodel.cpp`
- Calls `dwyco_tox_get_friends_model()` every 5 seconds (driven by a Timer in `ToxPage.qml`)
- Returns rows: `{friend_number, pubkey, name, status, user_status}`
- `QQmlObjectListModel<ToxFriend>` provides data to QML
- Uses `update_counter` to detect stale entries

### CRDT Tags for Cross-Device Sync

- `_tox_friend` tag: Composite value `hex(pseudo_uid) + "_" + hex(name)` replicates friend name mappings to all paired devices
- `_tox_device` tag: Value `hex(My_UID)` identifies the Dwyco UID that has tox enabled
- These tags are registered as static CRDT tags in `qmsgsql.cpp` and parsed in `import_remote_tupdate()`
- Allows non-tox clients on the same account to see tox friend names

## System Event Definitions

Defined in `se.h` and `dlli.h`:

| Enum | Constant | Args | Trigger |
|---|---|---|---|
| `SE_TOX_FRIEND_REQUEST` | `DWYCO_SE_TOX_FRIEND_REQUEST` | pseudo_uid, message, pubkey | Incoming friend request |
| `SE_TOX_MESSAGE` | `DWYCO_SE_TOX_MESSAGE` | pseudo_uid | Message received |
| `SE_TOX_READ_RECEIPT` | `DWYCO_SE_TOX_READ_RECEIPT` | - | Message delivered |
| `SE_TOX_FRIEND_STATUS` | `DWYCO_SE_TOX_FRIEND_STATUS` | pseudo_uid, "online"/"offline" | Friend connectivity change |
| `SE_TOX_FRIEND_NAME` | `DWYCO_SE_TOX_FRIEND_NAME` | pseudo_uid, name | Name resolved/changed |
| `SE_TOX_TYPING` | `DWYCO_SE_TOX_TYPING` | pseudo_uid, "on"/"off" | Typing indicator |
| `SE_TOX_FRIEND_USER_STATUS` | `DWYCO_SE_TOX_FRIEND_USER_STATUS` | pseudo_uid, status | Away/busy change |
| `SE_TOX_SELF_CONNECTION_STATUS` | `DWYCO_SE_TOX_SELF_CONNECTION_STATUS` | My_UID, status | Self connection change |
| `SE_TOX_READY` | `DWYCO_SE_TOX_READY` | - | Bridge initialized |
| `SE_TOX_CRASHED` | `DWYCO_SE_TOX_CRASHED` | - | Bridge crashed |

## The Protocol Plugin (`toxd_plugin.h`)

The plugin boundary exposes a minimal C-style API:

```c
ToxPlugin* toxp_init(const char *data_dir, ToxpEventCB cb, void *userdata);
void toxp_shutdown(ToxPlugin *p);
void toxp_save(ToxPlugin *p);
void toxp_iterate(ToxPlugin *p);
int toxp_friend_add(ToxPlugin *p, const vc &addr, const vc &msg);
int toxp_friend_add_norequest(ToxPlugin *p, const vc &pubkey);
int toxp_friend_delete(ToxPlugin *p, uint32_t fn);
int toxp_message_send(ToxPlugin *p, uint32_t fn, const vc &text, int is_action, uint32_t *mid, int *err);
int toxp_set_typing(ToxPlugin *p, uint32_t fn, int typing);
void toxp_set_name(ToxPlugin *p, const char *name, int len);
void toxp_set_status_message(ToxPlugin *p, const char *msg, int len);
vc toxp_self_get_address(ToxPlugin *p);
vc toxp_friend_list(ToxPlugin *p);
int toxp_friend_exists(ToxPlugin *p, uint32_t fn);
```

Events are delivered through the `ToxpEventCB` callback with string event types: `"friend_request"`, `"message"`, `"read_receipt"`, `"friend_name"`, `"friend_status"`, `"friend_user_status"`, `"typing"`, `"self_connection_status"`.

## Key Files to Create/Modify

### New files (one per messaging system):
- `toxd/toxd_plugin.h` - Plugin API header
- `toxd/toxd.cpp` - Protocol library wrapper (implements the plugin API using the new messaging library)
- `toxd/toxd.pro` - Build file (keep `TOXD_STANDALONE` option for testing)
- `bld/cdc32/toxbridge.h` - Bridge class declarations
- `bld/cdc32/toxbridge.cpp` - Bridge implementation (message flow, caching, event translation)
- `phoo/toxfriendmodel.h` - UI friend list model
- `phoo/toxfriendmodel.cpp` - UI friend list model implementation
- `phoo/ToxPage.qml` - Friend list UI
- `phoo/ToxStatusIndicator.qml` - Online/away/busy indicator

### Files to modify:
- `bld/cdc32/CMakeLists.txt` - Add `toxbridge.cpp` and `toxd/toxd.cpp` compilation
- `bld/cdc32/dlli.cpp` - Add public API functions, integrate into `zap_send6`, `dwyco_get_qd_messages`, `dwyco_uid_to_info`, service channels poll loop
- `bld/cdc32/dlli.h` - Declare public API functions and event constants
- `bld/cdc32/se.h` - Add event enum values
- `bld/cdc32/se.cpp` - Add event dispatch entries
- `bld/cdc32/qmsgsql.cpp` - Register static CRDT tags, import `_friend` tags
- `phoo/dwyco_top.cpp` - Handle system events, expose properties to QML
- `phoo/dwyco_top.h` - Declare new signals/slots
- `phoo/CMakeLists.txt` - Add `toxfriendmodel.cpp`
- `phoo/main.qml` - Add ToxPage to drawer
- `phoo/AppDrawer.qml`, `phoo/AppDrawerForm.ui.qml` - Add tox button
- Various browse QML files - Apply `core.format_message()` for HTML-safe display

## Evolution Between Branches (lessons learned)

| `ai-tox-experiment` (first) | `ai-change-course` (refined) |
|---|---|
| Separate toxd process (fork/exec) | In-process library (direct link) |
| RPC over pipes with vcxstream serialization | Direct function calls through `toxd_plugin.h` |
| Custom pipe read/write protocol | Callback-based event delivery |
| Process restart logic with exponential backoff | None needed (in-process) |
| SQL tables for contacts in `profiledb.cpp` | CRDT tags for cross-device sync |
| `Tox_sk`/`Tox_nospam` in auth file | Managed internally by toxcore save |
| Standalone toxd binary | Combined into cdc32 library |
| `fromLatin1()` with linux crash hack | `fromUtf8()` + `format_message()` |
| Ad-hoc HTML in QML | Centralized `core.format_message()` |

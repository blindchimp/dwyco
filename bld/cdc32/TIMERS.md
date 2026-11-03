# Software Timer Catalog

Complete catalog of software timers in this codebase: cdc32 core (`DwTimer`),
phoo (Qt/QML), toxd, plus ad-hoc wall-clock timeouts. Line numbers refer to the
working tree as of generation.

---

## 1. How timers fire (architecture)

**Nothing in the core is callback-driven.** Every `DwTimer` is a polled object:

- `class DwTimer` — `dwtimer.h:28`, `dwtimer.cc`.
  - `start(Type, first_ms, following_ms)` (`dwtimer.cc:63`) — `Type` is
    `ONESHOT = 0` or `REPEATING = 1` (`dwtimer.h:31`). `start()` always calls
    `stop()` first, so every start is also a restart. For `REPEATING`, the
    `following` interval is only applied at ack time (via `timer_reset`).
  - `is_expired()` (`dwtimer.cc:83`) — pure predicate; returns 0 if disabled,
    otherwise `timer_expired(&timer)`. It does **not** re-arm and does **not**
    invoke anything. Sets `expired = 1` and records `actual_interval`.
  - `ack_expire()` (`dwtimer.cc:106`) — REPEATING ⇒ re-arm at `start+interval`
    (phase-locked; falls back to restart if >1 period was missed);
    ONESHOT ⇒ `stop()`.
  - `stop()`, `get_time_left()`, `is_running()`, `set_interval()`,
    `time_now()` (static, `dwtimer.cc:175`), `next_expire_time()` (static).
- Low-level layer: `bld/dwcls/dwcls_timer.h` / `dwcls_timer.cpp` (Contiki-derived).
  `struct dwyco::timer { start, interval }` indexed in a global tree keyed by
  absolute expire time (`dwcls_timer.cpp:89`), so `timer::timer_next_expire()`
  (`dwcls_timer.cpp:145`) can return the earliest deadline. Clock is
  `CLOCK_BOOTTIME` (Linux) / `timeGetTime()` (Windows), milliseconds.

**The driving loops** — who calls `is_expired()`:

- `dwyco_service_channels()` — `dlli.cpp:2107`. The main grinder; must be called
  at least every 100 ms. Its **return value** is ms until the next timer expiry:
  `dlli.cpp:2205-2216` (`DwTimer::next_expire_time() - time_now()`), documented
  at `dlli.cpp:2083-2096`.
  Call chain: `dwyco_service_channels` → `dirth_poll_timeouts()` `:2131`,
  `pal_tick()` `:2132`, `MMChannel::service_channels()` `:2133`,
  `db_reconnect()` `:2134`, entropy `:2154`, `handle_deferred_msg_send()` `:2184`,
  `tox_bridge_poll()` `:2185`, `crank_activity_timer()` `:2187`,
  `sync_call_setup()` `:2188`, `dump_timer` `:2191-2203`.
- `MMChannel::service_channels()` — `mmchan.cc:4118`. Polls `Bw_adj_timer`,
  `SKID_cleaner_timer`, per-channel `timer1`/`ctrl_timer`, `ctrl_send_watchdog`,
  `sync_pinger`, then per channel: `tick()` `:4337` (`mmchan.cc:3832`) and
  `handle_channels()` `:4291`/`:4407` → `sproto::crank()` (`mmchan2.cc:132`).
  `tick()` also polls `nego_timer`, `sync_timer`, `pinger_timer`,
  `keepalive_timer`, `ref_timer`, `frame_timer`, `drop_timer`, the rate monitors,
  `AudioOutput::tick()` `:3925`, `DwRTLog::tick()` `:3930`, `tube->tick()` `:3844`.
- Background daemon loop — `bgapp.cpp:770` (`dwyco_background_processing`) and
  `bgapp.cpp:1207` (`dwyco_background_sync`): sleep `snooze` ms (clamped ≥100 ms,
  `bgapp.cpp:854-882`), re-enter.
- phoo GUI: QML `service_timer` (`main.qml:1531`) → `core.service_channels()`
  (`dwyco_top.cpp:3875`) → `dwyco_service_channels()`. Its interval is rewritten
  every tick from the returned next-expire (`main.qml:1560-1567`), clamped to
  1–100 ms.

---

## 2. Interval constants

| Constant | Value (ms) | Defined at |
|---|---|---|
| `CALLLIVE_DIRECT_TIMEOUT` | 4000 | `calllive.h:34` |
| `CALLLIVE_SERVER_TIMEOUT` | 30000 | `calllive.cpp:88` |
| `CALLLIVE_STUN_TIMEOUT` | 10000 | `calllive.cpp:92` |
| `CHANNEL_SETUP_TIMEOUT` (directsend) | 4000 | `directsend.cpp:28` |
| `XFER_WATCHDOG_TIMEOUT` (directsend) | 20000 | `directsend.cpp:29` |
| `CHANNEL_SETUP_TIMEOUT` (qsend) | 15000 | `qsend.cpp:34` |
| `XFER_WATCHDOG_TIMEOUT` (qsend) | 60000 | `qsend.cpp:35` |
| `NEGO_TIMEOUT` | 120000 | `mmchan.cc:86` |
| `PACKET_DROP_INTERVAL` | 10000 | `mmchan.cc:87`, `mmbld.cc:43` |
| `SECONDARY_CHANNEL_DROP_TIMEOUT` | 120000 | `mmchan.h:1152` |
| `VIDEO_IDLE_TIMEOUT` | 120000 | `mmchan.h:1154` |
| `AUDIO_IDLE_TIMEOUT` | 120000 | `mmchan.h:1155` |
| `CALLQ_POLL_TIME` | 3000 | `callq.cpp:29` |
| `CALLQ_SLOW_POLL_TIME` | 60000 | `callq.cpp:30` |
| `WORKTIMER` (Android) | 480000 | `bgapp.cpp:762` |
| `RTLOG_DEFAULT_TIME` (seconds) | 30 | `dwrtlog.h:26` |
| `DEFAULT_INACTIVITY_TIME` (seconds) | 300 | `dlli.cpp:288` |
| default `rate/max_fps` | 20 fps ⇒ 50 ms frame timer | `ezset2.cpp:86` |
| default `net/broadcast_interval` (seconds) | 60 | `ezset2.cpp:48` |
| sproto watchdog (literal) | 60000 | `sproto.cpp:114` |
| sproto/media idle (literal) | 120000 | starts use `VIDEO/AUDIO_IDLE_TIMEOUT` |
| `ctrl_send_watchdog` (literal, also `set_interval`) | 600000 | `mmchan.cc:464`, `:4727` |
| sync pinger (literal, `2 * VIDEO_IDLE_TIMEOUT`) | 240000 | `mmchan_sync.cpp:689` |
| callq per-call timeout (literal, `5 * 60 * 1000`) | 300000 | `callq.cpp:220` |
| dirth request default (seconds) | 30 | `qdirth.cc:68` |

**Warning:** `CHANNEL_SETUP_TIMEOUT` / `XFER_WATCHDOG_TIMEOUT` are file-local
`#define`s with *different* values in `directsend.cpp` and `qsend.cpp`.

---

## 3. cdc32 core library (`dlli.cpp`)

| Timer | Decl | Type | First / repeat | Poll site (function) | On fire |
|---|---|---|---|---|---|
| `Db_timer` ("db_timer") | `dlli.cpp:428` ns-`dwyco` | ONESHOT | `dlli.cpp:1119` 200 (resume); `:1914` 1 (first attempt); `:1919` **10000–54000 random** (backoff); `bgapp.cpp:982` 1 (post-suspend) | `dlli.cpp:1921` in `db_reconnect()` ← `dwyco_service_channels:2134` | `start_database_thread()` (`:1924`). Reconnect logic: while `Database_id == -1`, re-arm a random 10–54 s delay (thundering-herd avoidance), then connect. |
| `Activity_timer` ("activity") | `dlli.cpp:1365` static | ONESHOT | `:1373` `timeout * 1000`, default **300000** | `dlli.cpp:1413` in `crank_activity_timer()` ← `:2187` | stop + activity callback (`:1415-1417`) → default `internal_activity` sends chat state `"idle"`; re-armed by `update_activity()` from ~18 API entry points |
| `send_qd_msg_timer` ("send_qd") | `dlli.cpp:2059` fn-static | REPEATING | `:2063` and `:2070` **10000 / 10000** | `dlli.cpp:2073` in `handle_deferred_msg_send()` ← `:2184` | `qd_send_one()`; stops itself when the outqueue drains (`:2078`) |
| `dump_timer` ("dump") | `dlli.cpp:2191` fn-static, `#ifdef DW_RTLOG` | REPEATING | `:2195` **5000 / 5000** | `dlli.cpp:2198` in `dwyco_service_channels` | `dwyco_debug_dump()` (`:2201`) |

## 4. Background loops (`bgapp.cpp`)

| Timer | Decl | Type | First / repeat | Poll site | On fire |
|---|---|---|---|---|---|
| `bu_poll` | `bgapp.cpp:530` fn-static | REPEATING | `:538` **3600000 / 3600000** (1 h) | `bgapp.cpp:543` in `check_background_backup()` ← `:779` | `background_android_backup()` (`:547`), latched by `already_checked` |
| `worktimer` | `bgapp.cpp:763` local, `#ifdef ANDROID` | ONESHOT | `:764` and `:830` **480000** (`WORKTIMER`) | `bgapp.cpp:788` in the `while(1)` loop | `inactivity_exit = true` → suspend (`:963-973`); re-armed on any message activity (`:827-830`) |

## 5. Logging and rate monitors

| Timer | Decl | Type | First / repeat | Poll site | On fire |
|---|---|---|---|---|---|
| `DwRTLog::flush_timer` | `dwrtlog.h:51`, `#ifdef DW_RTLOG` | REPEATING | `dwrtlog.cc:95` **30000 / 30000** (`time * 1000`) | `dwrtlog.cc:123` in `DwRTLog::tick()` ← `mmchan.cc:3930` | `flush_to_file()` → write `rtlog.out` |
| `DwRateMonitor::timer` ×7 (`fps_send`, `fps_recv`, `bps_send`, `bps_recv`, `bps_audio_send`, `bps_audio_recv`, `bps_file_xfer`) | `dwrate.h:38`; instances `mmchan.h:372-378` | REPEATING | `dwrate.cc:15-17` ctor interval: **1000** each, `bps_file_xfer` **500** | `dwrate.cc:67` via `DwRateMonitor::is_expired()` ← `mmchan.cc:3911-3922` (inside `MMChannel::tick`) | recompute `unit_rate` over actual elapsed ms → set `rate_update` flags → status text / send-recv indicators |

Note: the whole of `dwrate.cc`/`dwrate.h` is behind `#ifdef DWYCO_RATE_DISPLAY`,
which is **not defined anywhere** — in a default build these are no-op stubs
(`dwrate.h:45-57`) and `mmchan.cc:3911-3922` does nothing.

## 6. Crypto / entropy (`qauth.cc`)

| Timer | Decl | Type | First / repeat | Poll site | On fire |
|---|---|---|---|---|---|
| `timer` ("esample") | `qauth.cc:230` fn-static | REPEATING | `:235` **60000 / 60000** | `qauth.cc:253` in `add_entropy_timer()` ← `dlli.cpp:2154` | `add_entropy()` — mix caller bytes into SHA pool (`:256`) |
| `save_timer` ("esave") | `qauth.cc:231` fn-static | REPEATING | `:239` **7200000 / 7200000** (2 h; was 15 s) | `qauth.cc:258` same function | `save_info(vclh_sha(Entropy), "entropy")` (`:261`) |

## 7. LAN broadcast (`aconn.cc`)

| Timer | Decl | Type | First / repeat | Poll site | On fire |
|---|---|---|---|---|---|
| `Broadcast_timer` ("broadcast") | `aconn.cc:160` static | ONESHOT (re-armed each tick) | `:260` **1** (first announce); `:410` **60** (fast retry after send failure); `:438` **`interval * 1000`**, default **60000** | `aconn.cc:423` in `broadcast_tick()` ← `poll_listener()` `:515` ← `mmchan.cc:4149` | `broadcast_announcement()` (`:427`) — UDP broadcast of uid/ip/ports/name; `net/broadcast_interval ≤ 10` is forced to 60 (`:429-435`) |

## 8. Media pacing

| Timer | Decl | Type | First / repeat | Poll site | On fire |
|---|---|---|---|---|---|
| `AudioOutput::output_timer` | `audout.h:99` member | ONESHOT | device-derived, not constants: `audout.cc:351` `time_until_play`; `:391`, `:432`, `:587`, `:724` `device_one_buffer_time() * bufs_to_buffer`; `:579` extends a running timer | `audout.cc:288` in `AudioOutput::tick()` ← `mmchan.cc:3925` | `stop(); play_now();` (`:293-294`) — the DwTimer is the audio output clock (pacing instead of a device callback) |
| `FileTube::qtimer` | `filetube.h:68` member | ONESHOT | `filetube.cc:506` **`m.time - t0`** (inter-block stream-timestamp delta) | `filetube.cc:523` in `FileTube::tick()` ← `mmchan.cc:3844` | move block `q → pickupq` (`:526-527`) then load/re-arm next block — rate-paced playback |
| `FileTube::kludge_timer` | `filetube.h:85` member | ONESHOT | `filetube.cc:168` **`auto_stop_delay + 400`** (`auto_stop_delay` set by callers: `-300`, `1000 * Audio_delay`, `-400`; 0 = disabled) | `filetube.cc:157` in `FileTube::has_ctrl()` ← `mmchan.cc:4784` | `kludge_done = 1` (`:160`) → end-of-control-stream → auto-stop playback |

## 9. MMChannel core (stream/channel timers)

| Timer | Decl (mmchan.h unless noted) | Type | First / repeat | Poll site (all in `MMChannel::tick` unless noted) | On fire |
|---|---|---|---|---|---|
| `nego_timer` | `:182` | ONESHOT | **120000** — `mmconn.cc:343`, `mmchan.cc:1345`, `:2430`; interval preset `mmchan.cc:465` | `mmchan.cc:3834` | `schedule_destroy()` — negotiation/ringing timed out |
| `pinger_timer` | `:687` | REPEATING | `mmchan.cc:565` **60000 / 60000** | `:3881` | increments `pinger` sync var; remote seeing divergence > 3 destroys the channel (`:3869-3878`) |
| `keepalive_timer` | `:809` | REPEATING | `mmchan.cc:616` **300000 / 300000** (server ping) | `:3887` | `keepalive_processing()` → `send_ctrl("!")` (`mmserv.cc:589`) |
| `sync_timer` | `:686` | ONESHOT | `mmchan.cc:3855` **5000** (only when `sync_manager.update_available()`) | `:3857` | `sync_send()` — flush queued sync-state updates |
| `ref_timer` | `:351` | REPEATING | `mmbld.cc:480` **10000 / 10000**; `mmctrl.cc:53` 10000; `mmctrl.cc:48` **first 0**, repeat 10000 when loss > 2% (immediate ref frame) | `:3893` | `ready_for_ref = 1` → next coded frame is a reference frame |
| `frame_timer` | `:352` | REPEATING | `mmbld.cc:479` **`intval / intval`** = `1000/max_fps` ≈ **50 ms** at default 20 fps | `:3899` | `frame_send = 1` → `send_frame()` (`:3659`) actually transmits video |
| `drop_timer` | `:353` | REPEATING | `mmbld.cc:516` **10000 / 10000** (only while packet-drop reporting enabled) | `:3906` | `drop_send = 1` → sends `"drop"` ctrl msg with loss % (`:4684-4703`) |
| `timer1` | `:720` (cb `:721`) | ONESHOT | generic slot — every call/xfer/secondary-db timeout (see §10-11) | `mmchan.cc:4201` in `service_channels` **per channel, before anything else** | `(*mc->timer1_callback)(...)` (`:4205`) |
| `ctrl_timer` | `:728` (cb `:729`) | ONESHOT | only start: `dmdsrv.cc:213` **120000** | `mmchan.cc:4209` in `service_channels` | `(*mc->ctrl_timer_callback)(...)` (`:4213`) |
| `ctrl_send_watchdog` | `:736` | ONESHOT | `mmchan.cc:4727` **600000** (re-armed after each ctrl-queue write; `set_interval` `:464`) | `mmchan.cc:4764` in `service_channels` | log `"ctrl watchdog timeout"` + `schedule_destroy()` — ctrl queue stuck |
| `Bw_adj_timer` (static) | `:820`, def `mmchan.cc:124` | REPEATING | `mmchan.cc:4140` and `:4178` **60000 / 60000**; **stopped** at `:4182-4183` when no net channels | `mmchan.cc:4164` in `service_channels` | `adjust_outgoing_bandwidth()` + `adjust_incoming_bandwidth()` |
| `SKID_cleaner_timer` (static) | `:1078`, def `mmchan.cc:126` | REPEATING | `mmchan.cc:4145` **86400000 / 86400000** (24 h) — only armed if `Current_alternate` is set on the *first ever* `service_channels()` call | `mmchan.cc:4170` | `clean_gj()` (`grpmsg.cpp:82`) — purge stale protocol runs |
| `sync_pinger` | `:1042` | ONESHOT | `mmchan_sync.cpp:689` **240000** — re-armed on every incoming sync packet | `mmchan.cc:4865` in `service_channels` | log `"sync pinger timeout"` + `schedule_destroy(HARD)` |
| `downstream_timer` | `:1043` | REPEATING | `mmchan_sync.cpp:546` **1000 / 1000** | `mmchan_sync.cpp:617` in `process_outgoing_sync()` | `package_downstream_sends()`; stops itself when nothing to send |
| `eager_pull_timer` | `:1062` | REPEATING | `mmchan_sync.cpp:524` **10000 / 10000** (gated by `eager_pull_timer_active`) | `mmchan_sync.cpp:526` in `eager_pull_processing()` | `assert_eager_pulls()` — queue background pulls |

## 10. Call setup and the call queue

| Timer | Decl | Type | First / repeat | Poll site | On fire |
|---|---|---|---|---|---|
| `MMChannel::timer1` — server/proxy connect | cb `calllive.cpp:422` | ONESHOT | `calllive.cpp:421` **30000** (`CALLLIVE_SERVER_TIMEOUT`) | `mmchan.cc:4201` | `timer1_stun_expired` (`calllive.cpp:340`) → `MMCALL_FAILED` — callee/proxy never answered |
| `MMChannel::timer1` — STUN media setup | cb `calllive.cpp:243` | ONESHOT | `calllive.cpp:242` **10000** (`CALLLIVE_STUN_TIMEOUT`) | `mmchan.cc:4201` | `stun_setup_timeout` (`calllive.cpp:164`) → `"Can't contact STUN. Call failed."` |
| `MMChannel::timer1` — direct call setup | cb `mmcall.cpp:166` | ONESHOT | `mmcall.cpp:165` **4000**; `:162` **2000** if uid in `No_direct_msgs`; `:134` re-boost **4000** after low-level connect | `mmchan.cc:4201` | `timer1_expired` (`calllive.cpp:573`) → `schedule_destroy()` + `TRACK_ADD(CL_direct_timeout)` → falls back to server-assisted call |
| `CallQ::call_q_timer` | `callq.h:37` | REPEATING | `callq.cpp:110` **3000 / 3000**; `reset_poll_time()` `:133` switches to **60000 / 60000** when idle (`:342`), back to 3000 on call activity (`:190`, `:225`); lazy re-arm `:269` | `callq.cpp:272` in `CallQ::tick()` ← `callq_tick()` `:101` ← `mmchan.cc:4185` | sweep dead/stale calls, enforce `max_established` (4) and ≤3 concurrent connects, start queued calls (`:363`) |
| `struct callq::timeout` (per queued call) | `callq.cpp:53` | ONESHOT | `callq.cpp:220` **300000** (5 min) at enqueue | `callq.cpp:312` in `CallQ::tick()`, only for `CQ_WAITING` | entry `delete`d — queued call silently dropped if never started in 5 min; stopped at `:367` once `start_call` succeeds |
| `local_connect_timer::connect_timer` | `synccalls.cpp:277` | REPEATING | `:274` and `:283` **1000 / 1000**; `throttle_down()` `:291` → **60000 / 60000** | `synccalls.cpp:340` in `sync_call_setup()` ← `dlli.cpp:2188` | run the sync-call body — rate-limits outgoing device-group sync connection attempts |

## 11. File / message transfer

All of these reuse the generic `MMChannel::timer1` slot (§9).

| Purpose | Start site | Type | Interval | Callback → action |
|---|---|---|---|---|
| Direct-send channel setup (proxy path) | `directsend.cpp:377` | ONESHOT | **4000** | `DirectSend::xfer_chan_setup_timeout` (`:252`) → `eo_direct_xfer` + `schedule_destroy(HARD)`, `TRACK_ADD(DS_xfer_setup_timeout)` |
| Direct-send channel setup (direct path) | `directsend.cpp:436` | ONESHOT | **4000** | same as above |
| Direct-send progress watchdog | `directsend.cpp:247` (on channel established) and `:223` (re-armed on every status callback) | ONESHOT | **20000** | same handler — 20 s without progress ⇒ abort transfer |
| QSend channel setup | `qsend.cpp:441` | ONESHOT | **15000** | `DwQSend::xfer_chan_setup_timeout` (`:362`) → `eo_qd_xfer` (requeue / mark undeliverable) + `schedule_destroy(HARD)` |
| QSend progress watchdog | `qsend.cpp:357` and `:331` (re-armed per status) | ONESHOT | **60000** | same handler — 60 s stall ⇒ abort + requeue |
| Secondary-DB connect | `dmdsrv.cc:299` | ONESHOT | **6000** | `connect_timeout` (`:258`) → `schedule_destroy(HARD)`; stopped on success (`:210`) or failure (`:244`) |
| Secondary-DB idle drop (`ctrl_timer`) | `dmdsrv.cc:213` | ONESHOT | **120000** | `drop_connection_timeout` (`:169`) → `schedule_destroy(HARD)` before routers silently kill idle TCP |

## 12. Protocol state machines (`sproto`)

| Timer | Decl | Type | First / repeat | Poll site | On fire |
|---|---|---|---|---|---|
| `sproto::watchdog` ("sproto-watchdog") | `sproto.h:63` | ONESHOT | `sproto.cpp:114` **60000**; re-armed in `start()` `:108` and `crank()` `:219` whenever a handler makes progress; **stopped** once media is up (`mmchan3.cpp:393,407,541,978,984,1052`) | `sproto.cpp:149` in `sproto::crank()` ← `handle_channels()` `mmchan2.cc:120` ← `service_channels` `mmchan.cc:4291/4407` | handler result `fail` → subchannel dropped, `schedule_destroy()`; `TRACK_ADD(SPROTO_watchdog_timeout)` |
| `sproto::timeout` ("sproto-timeout") | `sproto.h:61` | ONESHOT | **120000** (`VIDEO/AUDIO_IDLE_TIMEOUT`): `mmchan2.cc:285`, `mmchan3.cpp:395,409,422,979,985,990,1051`, `mmchan_sync.cpp:670` — re-armed on each media frame/ping | `sproto.cpp:93` (`quick_transition`, wakes the bg loop) and `:161` (`crank()`) | only acts if the state table has a `'t'` event: `ping` → `send-ping` (`mmchan3.cpp:106,138`) → `send_media_ping()` sends `"!"` keep-alive — **keeps the media NAT mapping alive while idle** |

---

## 13. phoo — C++ `QTimer` objects (8)

| Timer | Decl | Interval | Type | Start trigger | On timeout |
|---|---|---|---|---|---|
| `DwycoCore::m_tox_plink_timer` | `phoo/dwyco_top.h:526`, created `dwyco_top.cpp:2771-2779` | **700** | single-shot, coalesced (started only `if(!isActive())` `:2782`) | `schedule_tox_plink()` `:2768` from `DWYCO_SE_TOX_FRIEND_USER_STATUS` (`:733`) | `emit pal_came_online()` (`:2777`) → `main.qml:1178` plays plink sound |
| `DwycoCore::m_tox_saves_timer` | `dwyco_top.h:527`, created `:2789-2797` | **700** | single-shot, coalesced | `schedule_tox_saves_changed()` `:2786` from `:675`, `:772` | `emit tox_saves_changed()` (`:2795`) → `main.qml:252`, `ToxPage.qml:207`, `ToxAcct.qml:302` refresh |
| `TrayIcon::m_timer` | `phoo/trayicon.h:37`, started `trayicon.cpp:65-67` | **3000** | repeating | ctor, immediately (desktop only) | `TrayIcon::updateIcon()` (`:150-162`) — rebuild icon/tooltip |
| `TrayIcon::m_blinkTimer` | `trayicon.h:38`, config `:126-127` | **1000** | repeating | `evaluateBlink()` `:143` when window hidden + background alert; stopped `:145` | `TrayIcon::blink()` `:164-173` — toggle normal/dimmed icon |
| `simple_call::accept_timer` | `phoo/callsm.h:65`, `start_accept_timer()` `callsm.cpp:1071-1092` | **30000** | single-shot | state entered: `callsm.cpp:278`, `:296`; stopped on exit `:279`, `:297` and at `:1404,1420,1767,1798,1807` | state transitions `:272`, `:290`; also `ui->reject` click (`:280`) — auto-reject unanswered incoming call |
| `simple_call::ask_timer` | `callsm.h:66`, `start_ask_timer()` `:1101-1108` | **30000** | single-shot | `wait_rem_response` entered `:306`, stopped `:307` | transition to `start` `:302`; also `ui->cancel_req` (`:308`) |
| `simple_call::keyboard_active_timer` | `callsm.h:86`, config `:153-154` | **2000** | single-shot | `keyboard_input()` `:1199` `start(2000)`; `resume()` `:1170`; stopped in `suspend()` `:1152` | `simple_call::keyboard_inactive()` `:1206-1211` — sends `"ki"`, clears `kb_active` |
| `simple_call::reconnect_timer` | `callsm.h:87`, `start_retry_timer()` `:1116-1123` | **30000** | single-shot | `retrying` state entered `:236`, stopped on exit `:237` | transition `retrying → connecting` `:232` |

## 14. phoo — QML `Timer { }` elements (10)

| File:lines | id | Interval (ms) | Repeat | running condition | onTriggered |
|---|---|---|---|---|---|
| `main.qml:1531-1570` | `service_timer` | **30** initially, then rewritten each tick to `1`, or `min(100, sc_next)`/`100` (`:1560-1567`) | yes | always | **master network pump**: `core.service_channels()` (`:1558`), sync online flags, recompute next interval |
| `main.qml:1521-1529` | `sync_debug` | **1000** | yes | `group_active && server_account_created && qt_application_state === 0` | `SyncDescModel.load_model()` |
| `ToxPage.qml:251-256` | — | **5000** | yes | `tox_state.state === "signedin"` | `ToxFriendModel.load_friends()` |
| `ToxAcct.qml:102-106` | `bannerTimer` | **4000** | no | restarted from `showBanner()` `:111` | clear `bannerText` (`:105`) |
| `SimpleChatBox.qml:84-90` | `_toxTypingTimer` | **2000** | no | restarted `:1163` (length changed), `:1174` ( composing changed ), for tox uids | `_toxTypingActive = false`; `core.tox_set_typing(to_uid, 0)` |
| `SimpleChatBox.qml:1396-1404` | — | **100** | yes, `triggeredOnStart` | `progbar.visible` | increment fake progress bar |
| `SimpleGroupChat.qml:1021-1029` | — | **100** | yes, `triggeredOnStart` | `progbar.visible` | same fake progress |
| `MsgView.qml:102-106` | `hideOverlayTimer` | **2500** | no | restarted `:393`, `:399` (mouse press/drag, desktop + clean button) | `exit_overlay.opacity = 0` (`:105`) |
| `BlockLoader.qml:89-106` | `timer` | **300** | yes | `run()` `:108` | step block animation; **dead file — not in `qml.qrc`/CMake resources** |
| `old/PulseLoader.qml:57-73` | — | **80** | yes, self-started | component load | loader animation; **dead — `old/` not referenced by any resource file** |

(The shipped root `PulseLoader.qml` is animation-only, no Timer.)

---

## 15. Ad-hoc wall-clock timeouts (`QckDone`, not DwTimer)

`struct QckDone` (`qdirth.h:71`) carries a `time_t timeout` (`:149`);
`set_timeout(tm)` sets `time(0) + tm` (`:145`), `expired()` compares against
`time(0)` (`:139-143`). Polled by `dirth_poll_timeouts()` (`qdirth.cc:296`) from
`dwyco_service_channels` (`dlli.cpp:2131`), which synthesizes a
`"server timeout"` response (`:307-312`).

| Request | Timeout | Site |
|---|---|---|
| All dirth server sends (when `Enable_timeout`) | **30 s** | `qdirth.cc:68` |
| …`Enable_timeout` is cleared if a request sat queued > 15 s | — | `qdirth.cc:250-259` |
| `dirth_send_prov_leave` | **5 s** | `qdirth.cc:503` |
| directsend server command | **10 s** | `directsend.cpp:157` |
| secondary-DB command (short) | **5 s** | `dmdsrv.cc:392` |
| secondary-DB command (long) | **15 s** | `dmdsrv.cc:402` |

## 16. Periodic hooks that are NOT timers

| Hook | Call site | Notes |
|---|---|---|
| `tox_bridge_poll()` | `dlli.cpp:2185`, every service pass | `toxbridge.cpp:1789` — `toxp_iterate()`, conflict check, **30 s** SQL staleness reset (`:1805-1808`, `created_at < now - 30`), `tox_bridge_send_queued()`. No timer object of its own. |
| `pal_tick()` | `dlli.cpp:2132` | **no-op** — `cdcpal2.cc:353` returns 1 (PAL subsystem disabled) |
| `se_process()` | `dlli.cpp:2186` | event pump, no timeouts |
| `dirth_poll_response()` | `dlli.cpp:2130` | response dequeue, no timeouts |
| **toxd** | driven by the above | `bld/toxd/toxd.cpp` contains **zero** timers (`timer|timeout|interval|sleep|tick` = 0 matches). `toxp_iterate()` (`toxd.cpp:1029`) → `tox_iterate()` is invoked from `tox_bridge_poll()`; in phoo it is effectively pumped by `service_timer`. Legacy `TOXD_STANDALONE` test main (`:1389`) busy-loops with no sleep. |

---

## 17. Dead / disabled / legacy timers

| Item | Location | Why it's dead |
|---|---|---|
| `MMChannel::resolve_timer` | decl commented out `mmchan.h:629`; starts `mmconn.cc:113,185`; polls `mmconn.cc:150,242` | declaration removed; uses are inside `#ifdef LIBUV_ASYNC_LOOKUPS` (never defined) or `#if 0` |
| `MMChannel::bps_file_xfer` rate monitor | `mmchan.h:378`, started `mmchan.cc:485` | never `is_expired()` / `get_rate()` / `add_units()` — its 500 ms timer runs but is never consumed |
| `SKID_cleaner_timer` arming | `mmchan.cc:4145` | inside a one-shot `static been_here` guard **and** requires `Current_alternate` on the first-ever `service_channels()` call; otherwise never armed for the process lifetime |
| `frame_timer.set_interval` backoff | `mmctrl.cc:24-33` | inside `#if 0` — frame-interval backoff on droppage is disabled |
| `sproto::timeout` on file-transfer protocols | `mmchan3.cpp:36,53,68,74,84` state tables | no `'t'` event in file tables, so it is never started/checked for them — only media subchannels use it |
| `Pal_login_timer` / `Pal_ping_timer` | `old/cdcpal.cc:45-46` | `old/` tree; uses the pre-refactor API (`set_autoreload`, `load`, no-arg `start()`) that no longer exists in `dwtimer.h` |
| `audwin.cc` / `audconv.cc` local `DwTimer t` | `audwin.cc:273,345`, `audconv.cc:117,164` | test `main()`s disabled by `#undef TEST_AUDWIN` / `#undef TEST_AUDOACM`; same removed API |
| phoo `BlockLoader.qml`, `old/PulseLoader.qml` | see §14 | not in `qml.qrc` or CMake resource list |
| Commented-out starts | `mmchan.cc:561-563`, `mmbld.cc:473`, `mmchan_sync.cpp:611-615` | historical variants (1 s sync, 5 s downstream) |

---

## 18. Master summary — shortest to longest interval

| Interval | Timer(s) | Subsystem |
|---|---|---|
| 1 ms | `Db_timer` (first connect), `Broadcast_timer` (first announce) | core / LAN |
| 30 ms | phoo `service_timer` initial interval | phoo pump |
| 50 ms | `frame_timer` (1000/max_fps, 20 fps default) | video send |
| 60 ms | `Broadcast_timer` fast retry after send failure | LAN |
| ~80 ms | `old/PulseLoader.qml` (dead) | phoo (dead) |
| 100 ms | phoo fake progress bars; `service_timer` floor | phoo |
| 200 ms | `Db_timer` after `dwyco_resume()` | core |
| 300 ms | `BlockLoader.qml` (dead) | phoo (dead) |
| 700 ms | `m_tox_plink_timer`, `m_tox_saves_timer` (coalesced) | phoo |
| 1000 ms | `downstream_timer`, sync `connect_timer` (fast), rate monitors (1 s), `sync_debug` QML, tray `m_blinkTimer` | sync / stats / phoo |
| 2000 ms | `keyboard_active_timer`, `_toxTypingTimer` | phoo |
| 3000 ms | `call_q_timer` (fast poll), tray `m_timer` | callq / phoo |
| 4000 ms | direct-call setup, directsend channel setup | call / xfer |
| 5000 ms | `sync_timer`, `dump_timer`, ToxPage friend refresh | sync / debug / phoo |
| 6000 ms | secondary-DB connect timeout | dmdsrv |
| 10000 ms | STUN setup timeout, `ref_timer`, `drop_timer`, `eager_pull_timer`, `send_qd_msg_timer` | media / core |
| 15000 ms | qsend channel setup | xfer |
| 20000 ms | directsend progress watchdog | xfer |
| 30000 ms | server/proxy connect timeout, `flush_timer`, phoo `accept/ask/reconnect` timers, dirth default request | call / log / phoo / server |
| 60000 ms | sproto watchdog, qsend watchdog, `pinger_timer`, `Bw_adj_timer`, slow callq poll, sync-call slow poll, `esample`, `Broadcast_timer` (default `net/broadcast_interval` = 60 s) | many |
| 120000 ms | `nego_timer`, `ctrl_timer` (sec-db idle), `sproto::timeout` media idle | idle / nego |
| 240000 ms | `sync_pinger` | sync |
| 300000 ms | `keepalive_timer`, callq per-call 5 min, `Activity_timer` (5 min default) | idle / queue / activity |
| 480000 ms | `worktimer` (8 min Android) | Android |
| 3600000 ms | `bu_poll` (1 h) | backup |
| 7200000 ms | `esave` (2 h) | entropy |
| 86400000 ms | `SKID_cleaner_timer` (24 h) | maintenance |

---

## 19. Counts

- **cdc32 live `DwTimer` object sites:** ~40 (many are per-instance: `timer1`,
  `ctrl_timer`, `sproto` pair, rate monitors, file-tube timers).
- **phoo:** 8 `QTimer` + 10 QML `Timer` (8 live, 2 unreachable).
- **toxd:** 0.
- **Ad-hoc `QckDone` timeouts:** 6 sites.

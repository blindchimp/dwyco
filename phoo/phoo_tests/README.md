# phoo GUI / UX test harness

Drives phoo's Qt Quick UI the way a person would -- real clicks, real
keystrokes -- and answers the three questions a UX test actually asks:

| question | how |
|---|---|
| is the UI responding? | frame counter + event loop lag (`ping`) |
| has it settled? | region-of-interest pixel stability (`settle`) |
| can it be interacted with? | visible/enabled/opacity/size/hit test (`probe`) |

## How it works

A small agent is compiled into phoo (`testagent.cpp`) but is **inert unless
the app is launched with `--test-agent <socket>`**. It serves newline
delimited JSON on a unix socket. Input is injected with
`QWindowSystemInterface`, the same entry point a physical mouse uses, so
events go through the real Qt input pipeline -- but without needing the
window to hold focus, and without needing an X server.

```
pytest process                 phoo process
     |                              |
     |  --test-agent /tmp/a.sock    |
     |  /tmp/profile-dir            |
     |----------------------------->|
     |                              |  seeds startup gates
     |  ping / probe / click /      |
     |  settle / grab               |
     |<-----------------------------|
```

### Why an in-process agent at all

phoo never goes idle. `main.qml`'s `service_timer` calls
`core.service_channels()` every 1-100ms forever, and several animations
loop infinitely (`PulseLoader.qml`, and the four call-button blinks in
`SimpleChatBox.qml`). A driver that waits for "quiet" would wait forever.

So settling is measured as **pixel stability in a region**, not event loop
idleness, and `--test-agent` additionally sets the QML `test_mode` context
property which turns off the things that legitimately never stop:
tooltips (`TipButton.qml`) and the infinite animations. Without that a
settle check would hang on any screen with a spinner or a typing pulse.

### Startup gates

`phoo_tests` boots a seeded profile, so the tests skip the first-run
profile dialog, the one-time reindex page, the PIN gate and the green
`blank_page` server-login spinner. That's done by the agent writing to
the settings map between `dwyco_register_qml()` and `engine.load()`,
which is the only point where settings are loaded but QML hasn't read
them yet.

Each run gets its own profile directory, passed as `argv[1]` (which
`setup_locations()` already treats as the profile path). `--test-agent`
is stripped out of `argv` by `main.cpp` before the app sees it, so it
can't be mistaken for the profile path.

## Running

```bash
pip install -r requirements.txt

pytest                          # offline suite
pytest -v tests/test_03_interaction.py
```

Environment:

| var | meaning |
|---|---|
| `PHOO_BIN` | path to the phoo binary (default `build/Desktop_Qt_6_12_0-Release/phoo`) |
| `PHOO_PLATFORM` | e.g. `offscreen` or `vnc` to run without a display |
| `PHOO_TEST_NO_SEED` | `1` = boot a genuinely fresh profile and let the startup gates run |
| `PHOO_TEST_LIVE` | `1` = also run the tests that need the real server |
| `PHOO_VIDEO` | `off` (default) / `on-failure` / `always` — record every test |
| `PHOO_VIDEO_FPS` | recording frame rate (default 10 for `on-failure`) |

The `x11/` suite needs a real `DISPLAY` and `xdotool`; it skips cleanly
otherwise.

## Video

Recordings are captured **inside the app** by the agent, on a timer, via
`QQuickWindow::grabWindow()`. Whole window, always. `grabWindow()` has to
run on the gui thread because it is a scene-graph operation, but PNG
encoding and the disk write happen on a dedicated writer thread, and the
queue feeding it is bounded — if the encoder falls behind, frames are
dropped and the drop count is reported rather than silently slowing the
app to a crawl.

```python
# one clip around a single interaction
with agent.record(out / "drawer-open", fps=30) as rec:
    agent.click("phoo.convlist.drawer_button")
    agent.wait_prop(DRAWER, "opened", True, timeout=5)
print(rec.result.summary())
rec.encode()                      # -> drawer-open.mp4

# a whole scripted journey, written like the user story it is
res = agent.journey(out / "onboarding", [
    lambda a: a.click("phoo.convlist.drawer_button"),
    lambda a: a.wait_prop(DRAWER, "opened", True, timeout=5),
    lambda a: a.click("phoo.drawer.settings"),
])
encode_video(out / "onboarding", fmt="gif")   # mp4 | webm | gif | apng
```

### What the recording tells you

`rec.result` (or `<dir>/meta.json`) carries the input log, and that log is
the interesting part:

```json
{"cmd": "click", "name": "phoo.convlist.drawer_button", "x": 61, "y": 41,
 "frame": 7, "paint_frame": 9, "latency_ms": 130, "painted": true}
```

- `painted: false` means the input produced **no repaint at all**. That
  is not a failure — it means the click landed on something inert, and
  it's usually the first sign that a selector has gone stale or that a
  control is covered by something.
- `frame` → `paint_frame` is the input→paint latency, measured
  in-process from injection to the `frameSwapped` that showed the result.
  The video flashes an orange border across exactly that window.

### Two overlays are burned in

- **A synthetic cursor.** In-process injection never moves the real
  pointer, so without this a recording shows the UI reacting to clicks
  with no visible pointer at all. A thin cyan crosshair sits at the last
  injected position, with a red ring flashing for ~400ms on the click
  itself. Turn it off with `cursor=False`.
- **A running clock and highlight bars**, from `encode_video()`. Real
  per-frame durations come from the recording's timestamps via the
  concat demuxer, so a 400ms pause looks like 400ms instead of being
  flattened to a constant frame interval — which is the whole point, since
  a constant-rate encode would hide exactly the sluggishness you're
  trying to see.

### Two capture paths, cross-checked

The in-process recorder is the primary path. `x11/test_x11_video_crosscheck.py`
records the same journey through `ffmpeg -f x11grab` — which reads the X
server's own output and so does **not** perturb the app at all — and
asserts the two agree on duration and both actually captured motion.
That test is the reason to trust the settle checks and the latency
numbers: if the in-process grab were lying about what was on screen, the
durations would diverge.

### The observer effect is accounted for

`grabWindow()` stalls the gui thread, so recording inflates the very
numbers the harness reports. `phoo.latency` therefore always refers to
the **clean** bucket; while a recording is in progress, samples go to
`phoo.latency_recorded` instead. `phoo.diagnostics()` prints both.

Note also that `frame_count` legitimately sits at 0 on a static screen —
QQuickWindow only renders when something is dirty. Liveness is reported
by `heartbeat_count` instead, which advances on every event loop turn
unless the gui thread is blocked.

### Automatic capture on failure

```bash
PHOO_VIDEO=on-failure pytest
```

Every test is recorded, and the clip is encoded and printed as an
`ARTIFACT:` path only when the test fails or the app stops responding.
Passing tests discard their video.

## Notes for writing tests

**Don't click the window close button.** `main.qml`'s `onClosing` starts a
3000ms "press again to exit" animation on the first close. Teardown goes
through the agent's `quit`, which calls `QCoreApplication::quit()` and
bypasses `onClosing` entirely.

**Settle a named element, not the whole window,** when the screen contains
something that legitimately never stops -- a `BusyIndicator`, the typing
pulse, the directory page's fetch spinner.

```python
agent.click("phoo.convlist.drawer_button", latency=phoo.latency)
agent.wait_prop(DRAWER, "opened", True, timeout=5)
agent.wait_settled(name="phoo.drawer.form", max_ms=4000)
```

**`probe()` returns one boolean plus a reason.** When a control isn't
interactable, `why_not()` says which of the seven conditions failed --
invisible, disabled, zero opacity, zero size, hidden ancestor, outside the
window, or occluded by another item.

**Every test is bracketed by a responsiveness check** (the `guard`
autouse fixture). A test that wedges the GUI thread fails the next test
with a clear message instead of hanging it, and writes a screenshot.

## objectName convention

Controls are addressed by `objectName`, using `phoo.<screen>.<role>`:

```
phoo.window  phoo.stack  phoo.drawer  phoo.pin  phoo.blank_page
phoo.convlist.{toolbar,multi_toolbar,drawer_button,grid_toggle,trivia,
               directory_button,contacts_button,list,grid,empty_help,
               item.<i>,griditem.<i>}
phoo.chat.{input,send,back,messages,busy,failed_toast,typing,message.<i>}
phoo.drawer.{form,update_profile,browse_favs,browse_hidden,quiet,
             invisible,settings,link_device,vid_preview,lock_and_exit}
phoo.settings.{pin_expire,unreviewed,show_hidden,show_archived,
               block_list,pin_lock,trash,load_backup,about}
phoo.directory{,.list,.busy}
phoo.pin.{digit.<i>,backspace,cancel,show}
```

Before this there was exactly one `objectName` in the whole project
(`dwyco_singleton`), so nothing was addressable. List and grid delegates
carry an index suffix. `lock_and_exit` is deliberately present but no
test should click it.
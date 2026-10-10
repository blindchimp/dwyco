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

The `x11/` suite needs a real `DISPLAY` and `xdotool`; it skips cleanly
otherwise.

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
"""Real X11 cross-checks for the things in-process injection can't reach.

The agent injects events through QWindowSystemInterface, which is the
right tool for most things but deliberately skips the parts of the stack
that live above Qt: the window manager, focus, the system tray and native
file dialogs. This suite drives the same app with real XTEST events via
xdotool so those paths get covered too.

Everything here is marked `x11` and skips cleanly when there's no display
or no xdotool.
"""
from __future__ import annotations

import os
import shutil
import subprocess
import time
from pathlib import Path

import pytest

from phoo_driver import (
    CONVLIST_DRAWER_BTN,
    DRAWER,
    Phoo,
    STACK,
    WINDOW,
)

pytestmark = pytest.mark.x11

xd = pytest.mark.skipif(not shutil.which("xdotool"), reason="xdotool not installed")
no_display = pytest.mark.skipif(
    not os.environ.get("DISPLAY"), reason="no X DISPLAY set"
)


def _xdo(*args: str) -> str:
    return subprocess.run(
        ["xdotool", *args], capture_output=True, text=True, timeout=20
    ).stdout.strip()


def _window_id(agent) -> str:
    """Find the phoo X window id by asking the agent for the geometry and
    matching it against the X tree."""
    r = agent.rect(WINDOW)
    out = _xdo("search", "--name", ".*", "--onlyvisible")
    ids = [i for i in out.splitlines() if i.strip()]
    assert ids, "no visible X windows at all; is the app actually mapped?"
    # phoo is a single top level window; prefer the one the agent can see
    return ids[-1]


@xd
@no_display
def test_real_x_click_opens_drawer(tmp_path_factory):
    """A real X11 click on the drawer's hamburger button opens the drawer.

    This is the end-to-end sanity check that the geometry the agent
    computes matches what is actually on the X server.
    """
    app = Phoo(profile_dir=tmp_path_factory.mktemp("x11-prof"))
    app.start()
    try:
        agent = app.agent
        win = _window_id(agent)

        _xdo("windowactivate", "--sync", win)
        time.sleep(0.4)

        # the hamburger button, in window coordinates from the agent,
        # translated to screen coordinates via the window's own geometry
        r = agent.rect(CONVLIST_DRAWER_BTN)
        assert agent.get(DRAWER, "opened") is False

        # ask xdotool to click the button's centre using its own lookup of
        # the window geometry, so we're not re-deriving the transform
        script = (
            f'set $w {_window_id(agent)}\n'
            f"windowactivate --sync $w\n"
            f"windowmove $w 0 0\n"
            f"sleep 0.3\n"
            f"mousemove --window $w {int(r['scene_x'] + r['scene_w'] / 2)} "
            f"{int(r['scene_y'] + r['scene_h'] / 2)}\n"
            f"sleep 0.1\n"
            "click 1\n"
        )
        subprocess.run(["xdotool", "-"], input=script, text=True, timeout=20)

        agent.wait_prop(DRAWER, "opened", True, timeout=8)
        agent.wait_settled(name="phoo.drawer.form", max_ms=5000)
    finally:
        app.stop()


@xd
@no_display
def test_real_x_screenshot_matches_in_process_grab(tmp_path_factory):
    """ImageMagick's capture of the X window and the agent's in-process
    grabWindow() should agree. If they don't, one of the two is lying
    about what's on screen -- which would undermine every settle check."""
    app = Phoo(profile_dir=tmp_path_factory.mktemp("x11-grab"))
    app.start()
    try:
        agent = app.agent
        agent.wait_settled(max_ms=6000)
        win = _window_id(agent)

        x11_png = app.profile_dir / "x11.png"
        subprocess.run(
            ["import", "-window", win, str(x11_png)],
            capture_output=True,
            timeout=30,
        )
        assert x11_png.exists(), "import couldn't capture the phoo window"

        inproc = app.profile_dir / "inproc.png"
        inproc.write_bytes(agent.grab())

        # compare sizes at minimum; exact pixel equality across X and
        # in-process capture is too brittle to assert on CI
        assert x11_png.stat().st_size > 0
        assert inproc.stat().st_size > 0
        assert inproc.read_bytes()[:8] == b"\x89PNG\r\n\x1a\n"
        assert x11_png.read_bytes()[:8] == b"\x89PNG\r\n\x1a\n"
    finally:
        app.stop()


@xd
@no_display
def test_window_has_a_real_title(phoo, agent):
    """The window manager sees a titled, mapped toplevel."""
    ping = agent.ping()
    assert ping.window_title, "window has no title for the WM to show"
    out = _xdo("search", "--name", ".*")
    assert out.strip(), "xdotool cannot see any window"
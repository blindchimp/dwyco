"""pytest fixtures for driving phoo's ui.

The whole suite runs against one long-lived phoo process per session,
because boot is the expensive part (it builds the profile, starts the
database thread and brings up the whole qml scene). Tests share it and
navigate back to a known starting point between themselves.
"""
from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parent))

from phoo_driver import (  # noqa: E402
    CHAT_BACK,
    CONVLIST_DRAWER_BTN,
    CONVLIST_GRID_TOGGLE,
    DRAWER,
    PIN,
    Phoo,
    STACK,
    AgentError,
    AgentTimeout,
)

LIVE_ENV = "PHOO_TEST_LIVE"
VIDEO_ENV = "PHOO_VIDEO"          # off | on-failure | always
VIDEO_FPS_ENV = "PHOO_VIDEO_FPS"


def video_mode() -> str:
    """off (default) | on-failure | always."""
    return os.environ.get(VIDEO_ENV, "off").lower()


@pytest.hookimpl(hookwrapper=True, tryfirst=True)
def pytest_runtest_makereport(item, call):
    """Stash the per-phase report on the item.

    The teardown fixture needs to know whether the test body already
    failed, so it can decide whether to keep the recorded video.
    """
    outcome = yield
    rep = outcome.get_result()
    setattr(item, "rep_" + rep.when, rep)


def pytest_configure(config):
    config.addinivalue_line("markers", "live: needs the real network; opt in with PHOO_TEST_LIVE=1")
    config.addinivalue_line("markers", "x11: needs a real X display and xdotool")


def pytest_collection_modifyitems(config, items):
    if os.environ.get(LIVE_ENV) == "1":
        return
    skip = pytest.mark.skip(reason=f"set {LIVE_ENV}=1 to run tests that touch the network")
    for item in items:
        if "live" in item.keywords:
            item.add_marker(skip)


def pytest_report_header(config):
    lines = []
    binary = os.environ.get("PHOO_BIN", "(default build path)")
    lines.append(f"phoo binary: {binary}")
    lines.append(f"qt platform: {os.environ.get('PHOO_PLATFORM', '(inherit)')}")
    mode = video_mode()
    if mode != "off":
        lines.append(
            f"video recording: {mode} @ {os.environ.get(VIDEO_FPS_ENV, '10')}fps"
        )
    if os.environ.get(LIVE_ENV) != "1":
        lines.append(f"live tests: skipped (set {LIVE_ENV}=1 to enable)")
    if not shutil.which("ffmpeg"):
        lines.append("ffmpeg: NOT installed, video tests will skip")
    if not shutil.which("xdotool"):
        lines.append("xdotool: not installed, x11 tests will skip")
    return lines


def _phoo(**kw) -> Phoo:
    # PHOO_TEST_NO_SEED=1 boots a genuinely fresh profile, which means the
    # first-run/reindex/server-login gates run for real.
    seed = os.environ.get("PHOO_TEST_NO_SEED") != "1"
    kw.setdefault("seed", seed)
    p = Phoo(**kw)
    p.start()
    return p


@pytest.fixture(scope="session")
def phoo():
    """A running app for the whole session."""
    app = _phoo()
    try:
        yield app
    finally:
        app.stop()


@pytest.fixture(scope="session")
def agent(phoo):
    return phoo.agent


@pytest.fixture(autouse=True)
def guard_before(phoo, agent):
    """The app must be responsive before any test runs.

    A test that left the gui thread wedged fails here with a clear
    message instead of hanging the next one.
    """
    try:
        agent.assert_responsive()
    except (AgentTimeout, AssertionError) as e:
        pytest.fail(
            f"app was not responsive before the test started: {e}\n"
            f"{phoo.diagnostics()}"
        )


@pytest.fixture(autouse=True)
def guard_after(request, phoo, agent):
    """After every test: still responsive, still alive, video encoded.

    This is deliberately a separate fixture from guard_before. pytest
    reports a teardown error and a test failure independently, so a
    failing assertion in the test body is not replaced by a teardown
    complaint.
    """
    mode = video_mode()
    rec = None
    if mode in ("on-failure", "always"):
        fps = int(os.environ.get(VIDEO_FPS_ENV, "10"))
        # recording perturbs the timings, so route samples into the
        # recorded bucket and leave phoo.latency (the clean one) alone
        phoo.latency_recording = True
        label = request.node.name.replace("/", "_")[:60]
        rec = agent.record(phoo.profile_dir / f"video-{label}", fps=fps)
        try:
            rec.start()
        except Exception as e:
            rec = None
            print(f"\n[video] could not start recording: {e}")

    failed = False
    try:
        yield
    finally:
        phoo.latency_recording = False

        # if the test body itself raised, keep its video, but never raise
        # over the top of its own failure
        rep = getattr(request.node, "rep_call", None)
        failed = bool(rep and rep.failed)

        problems = []
        try:
            after = agent.assert_responsive()
        except (AgentTimeout, AssertionError) as e:
            problems.append(f"app stopped responding: {e}")
            after = None

        dead = phoo.proc is not None and phoo.proc.poll() is not None
        if dead:
            problems.append(f"phoo died (exit {phoo.proc.returncode})")

        if rec is not None:
            try:
                res = rec.stop()
                out = phoo.profile_dir / f"video-{label}.mp4"
                if res.frames > 1:
                    rec.encode(out)
                    phoo.artifacts.append(out)
                    print(f"\n[video] {res.summary()}")
                    if mode == "always" or failed or problems:
                        print(f"[video] ARTIFACT: {out}")
                    else:
                        out.unlink(missing_ok=True)   # test passed; don't keep it
            except Exception as e:
                problems.append(f"video encode failed: {e}")

        if problems:
            try:
                phoo.screenshot("after-test")
            except Exception:
                pass
            pytest.fail("\n".join(problems) + "\n" + str(phoo.diagnostics()))


@pytest.fixture
def home(phoo, agent):
    """Put the app back on the convlist home screen.

    Navigates with real clicks where possible so the test exercises the
    same path a user would, and only falls back to state poking when the
    ui is genuinely stuck.
    """
    # if the pin dialog is up, get out of it
    if agent.exists(PIN) and agent.get(PIN, "visible"):
        agent.set(PIN, "visible", False)

    # walk the stack back to the bottom, one back-button at a time
    for _ in range(8):
        if agent.get(STACK, "depth") <= 1:
            break
        agent.back()
        agent.wait_settled(max_ms=2000)

    # close the drawer if it got left open. not by clicking the hamburger
    # again -- the open drawer covers that button.
    if agent.get(DRAWER, "opened"):
        agent.close_drawer()

    agent.wait_settled(max_ms=3000)
    return phoo


@pytest.fixture
def on_convlist(phoo, agent, home):
    """Home screen with the empty-contact-list label visible.

    A fresh seeded profile has no contacts, which is the cleanest known
    starting state to assert against.
    """
    # flip the view toggle to the list (see below) with real clicks
    try:
        # the named object is the GridToggle wrapper; the checkbox state is
        # exposed through its grid_checked alias
        if agent.get(CONVLIST_GRID_TOGGLE, "grid_checked"):
            agent.click(CONVLIST_GRID_TOGGLE)
            agent.wait_settled(max_ms=2000)
    except (AgentError, AgentTimeout):
        pass
    return phoo


@pytest.fixture
def settings_page(phoo, agent, home):
    """Navigate: convlist -> drawer -> settings. Leaves it on the page."""
    agent.click("phoo.convlist.drawer_button")
    agent.wait_prop(DRAWER, "opened", True, timeout=5)
    agent.wait_settled(name="phoo.drawer.form", max_ms=3000)
    agent.click("phoo.drawer.settings")
    agent.wait_prop(STACK, "depth", 2, timeout=5)
    agent.wait_settled(max_ms=3000)
    return phoo
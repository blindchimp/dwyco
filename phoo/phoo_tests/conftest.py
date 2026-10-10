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
    if os.environ.get(LIVE_ENV) != "1":
        lines.append(f"live tests: skipped (set {LIVE_ENV}=1 to enable)")
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
def guard(phoo, agent):
    """Before and after every test: the app must be alive and responsive.

    A test that leaves the gui thread wedged or the app dead fails the
    *next* test with a clear message instead of hanging it.
    """
    try:
        ping = agent.assert_responsive()
    except (AgentTimeout, AssertionError) as e:
        pytest.fail(
            f"app was not responsive before the test started: {e}\n"
            f"{phoo.diagnostics()}"
        )
    yield
    try:
        after = agent.assert_responsive()
    except (AgentTimeout, AssertionError) as e:
        phoo.screenshot("after-test")
        pytest.fail(
            f"app stopped responding during the test: {e}\n"
            f"frames {ping.frame_count} -> after\n{phoo.diagnostics()}"
        )
    # catch a click that blew the gui thread up entirely
    if phoo.proc and phoo.proc.poll() is not None:
        phoo.screenshot("crashed")
        pytest.fail(f"phoo died during the test\n{phoo.diagnostics()}")


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

    # walk the stack back to the bottom
    for _ in range(8):
        depth = agent.get(STACK, "depth")
        if depth <= 1:
            break
        if agent.exists(CHAT_BACK) and agent.get(CHAT_BACK, "visible"):
            agent.click(CHAT_BACK)
        else:
            agent.set(STACK, "depth", 1)
        agent.wait_settled(max_ms=2000)
    agent.wait_prop(STACK, "depth", 1, timeout=5)

    # close the drawer if it got left open
    if agent.get(DRAWER, "opened"):
        agent.click(CONVLIST_DRAWER_BTN)

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
        if agent.get(CONVLIST_GRID_TOGGLE, "checked"):
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
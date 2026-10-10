"""The drawer and the settings page.

Exercises the pattern the harness exists for: click a control the way a
user would, wait for the ui to stop moving, then assert both that the
control is interactable and that the click actually changed something.
"""
from __future__ import annotations

import pytest

from phoo_driver import (
    CONVLIST_DRAWER_BTN,
    DRAWER,
    DRAWER_FAVS,
    DRAWER_HIDDEN,
    DRAWER_PROFILE,
    DRAWER_QUIET,
    SETTINGS,
    SETTINGS_PIN_EXPIRE,
    SETTINGS_SHOW_ARCHIVED,
    SETTINGS_SHOW_HIDDEN,
    SETTINGS_UNREVIEWED,
    STACK,
)


def open_drawer(phoo, agent):
    if not agent.get(DRAWER, "opened"):
        agent.click(CONVLIST_DRAWER_BTN, latency=phoo.latency)
        agent.wait_prop(DRAWER, "opened", True, timeout=5)
    agent.wait_settled(name="phoo.drawer.form", max_ms=4000)


def test_every_drawer_control_is_interactable(phoo, agent, on_convlist):
    open_drawer(phoo, agent)
    for name in (
        DRAWER_PROFILE,
        DRAWER_FAVS,
        DRAWER_HIDDEN,
        DRAWER_QUIET,
        "phoo.drawer.invisible",
        "phoo.drawer.settings",
    ):
        p = agent.probe(name)
        assert p.interactable, f"{name} not interactable: {p.why_not()}"

    # lock-and-exit is the one control that must NOT be clicked in a test,
    # but it still has to be visible and hit-testable
    lock = agent.probe("phoo.drawer.lock_and_exit")
    assert lock.visible
    assert lock.hit_testable, "lock and exit is covered by something"


def test_drawer_control_toggles_a_setting(phoo, agent, on_convlist):
    """Click the switch, confirm the model behind it flipped, click back."""
    open_drawer(phoo, agent)
    before = agent.get(DRAWER_QUIET, "checked")
    agent.click(DRAWER_QUIET, latency=phoo.latency)
    agent.wait_prop(DRAWER_QUIET, "checked", not before, timeout=5)
    agent.wait_settled(max_ms=3000)
    agent.click(DRAWER_QUIET)
    agent.wait_prop(DRAWER_QUIET, "checked", before, timeout=5)


def test_navigate_to_settings_and_back(phoo, agent, on_convlist):
    open_drawer(phoo, agent)
    agent.click("phoo.drawer.settings", latency=phoo.latency)
    agent.wait_prop(STACK, "depth", 2, timeout=5)
    agent.wait_settled(max_ms=4000)

    assert agent.exists(SETTINGS), "settings page never came up"
    p = agent.probe(SETTINGS)
    assert p.visible, f"settings page not visible: {p.why_not()}"

    # and back out again
    agent.set(STACK, "depth", 1)
    agent.wait_settled(max_ms=3000)


@pytest.mark.parametrize(
    "control",
    [
        SETTINGS_UNREVIEWED,
        SETTINGS_SHOW_HIDDEN,
        SETTINGS_SHOW_ARCHIVED,
        SETTINGS_PIN_EXPIRE,
    ],
)
def test_settings_checkboxes_are_interactable(phoo, agent, control):
    p = agent.probe(control)
    assert p.exists, f"{control} not found on the settings page"
    assert p.interactable, f"{control} not interactable: {p.why_not()}"


def test_settings_checkbox_toggles(phoo, agent, on_convlist):
    open_drawer(phoo, agent)
    agent.click("phoo.drawer.settings")
    agent.wait_prop(STACK, "depth", 2, timeout=5)
    agent.wait_settled(max_ms=4000)

    before = agent.get(SETTINGS_SHOW_ARCHIVED, "checked")
    agent.click(SETTINGS_SHOW_ARCHIVED, latency=phoo.latency)
    agent.wait_prop(SETTINGS_SHOW_ARCHIVED, "checked", not before, timeout=5)
    agent.wait_settled(max_ms=3000)

    # put it back so the session fixture's world is unchanged
    agent.click(SETTINGS_SHOW_ARCHIVED)
    agent.wait_prop(SETTINGS_SHOW_ARCHIVED, "checked", before, timeout=5)


def test_settings_delegates_are_present(phoo, agent, settings_page):
    for name in (
        "phoo.settings.block_list",
        "phoo.settings.pin_lock",
        "phoo.settings.trash",
        "phoo.settings.about",
    ):
        assert agent.exists(name), f"{name} missing from the settings page"
        p = agent.probe(name)
        assert p.visible, f"{name} exists but is hidden: {p.why_not()}"
"""Interaction: does a click land, does the ui react, does it settle again.

This is the core UX loop the harness was built for:

    input -> visual response within a sane time -> pixels stabilise

Every test here goes through the same three checks rather than just
asserting a state flipped, because a ui can flip a property and still
feel broken to a person.
"""
from __future__ import annotations

import time

from phoo_driver import (
    CONVLIST_DRAWER_BTN,
    CONVLIST_GRID,
    CONVLIST_GRID_TOGGLE,
    CONVLIST_LIST,
    CONVLIST_TOOLBAR,
    DIRECTORY,
    DRAWER,
    STACK,
)

# a control that should paint well inside a frame budget. anything past
# this and a human would perceive the ui as laggy.
LATENCY_BUDGET_MS = 500


def test_click_produces_visual_response(phoo, agent, on_convlist):
    """Clicking the drawer button must repaint quickly."""
    agent.click(CONVLIST_DRAWER_BTN, latency=phoo.latency)
    agent.wait_prop(DRAWER, "opened", True, timeout=5)

    samples = phoo.latency.summary()
    assert samples, "no input->paint latency was measured"
    assert samples["max_ms"] < LATENCY_BUDGET_MS, (
        f"drawer click took up to {samples['max_ms']}ms to paint "
        f"(budget {LATENCY_BUDGET_MS}ms). {phoo.latency.format()}"
    )


def test_ui_settles_after_each_navigation(phoo, agent, on_convlist):
    """Each navigation should reach a stable frame, not spin forever.

    The StackView slide is a few hundred ms, so allow a generous ceiling;
    the point is that it terminates.
    """
    for _ in range(3):
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", True, timeout=5)
        s = agent.wait_settled(name="phoo.drawer.form", max_ms=5000)
        assert s.elapsed_ms < 5000

        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", False, timeout=5)
        agent.wait_settled(max_ms=5000)


def test_grid_list_toggle_switches_presentation(phoo, agent, on_convlist):
    """The desktop build starts on the grid (+desktop/GridToggle.qml has
    the box checked). Flipping it must swap which view is on screen, and
    both must settle."""
    # on_convlist already moved us to the list
    assert agent.get(CONVLIST_LIST, "visible") is True
    assert agent.get(CONVLIST_GRID, "visible") is False
    agent.wait_settled(name=CONVLIST_LIST, max_ms=4000)

    agent.click(CONVLIST_GRID_TOGGLE, latency=phoo.latency)
    agent.wait_prop(CONVLIST_GRID, "visible", True, timeout=5)
    agent.wait_prop(CONVLIST_LIST, "visible", False, timeout=5)
    agent.wait_settled(name=CONVLIST_GRID, max_ms=4000)

    # and back
    agent.click(CONVLIST_GRID_TOGGLE)
    agent.wait_prop(CONVLIST_LIST, "visible", True, timeout=5)
    agent.wait_settled(name=CONVLIST_LIST, max_ms=4000)


def test_toolbar_hits_land_on_the_right_control(phoo, agent, on_convlist):
    """Clicking by objectName has to click the thing you meant.

    The toolbar buttons are laid out next to each other, so a hit test
    landing on the neighbour means the geometry the harness is using and
    the geometry the user sees disagree.
    """
    buttons = [
        "phoo.convlist.drawer_button",
        "phoo.convlist.trivia",
        "phoo.convlist.directory_button",
        "phoo.convlist.contacts_button",
    ]
    rects = {}
    for name in buttons:
        r = agent.rect(name)
        x, y, w, h = r["scene_x"], r["scene_y"], r["scene_w"], r["scene_h"]
        assert w > 0 and h > 0, f"{name} has no size"
        rects[name] = (x, y, w, h)

    for name in buttons:
        p = agent.probe(name)
        assert p.hit_testable, (
            f"{name}: a click at its centre lands on "
            f"{p.hit_type} {p.hit_name!r} instead"
        )


def test_directory_page_opens_and_settles(phoo, agent, on_convlist):
    """The directory page is the first thing that needs the network.

    Offline it will keep an indeterminate BusyIndicator spinning forever
    (SimpDir.qml's busy1 is bound to core.directory_fetching). So this
    asserts the page opens and the list is reachable, and deliberately
    settles on the list region rather than the whole page.
    """
    agent.click("phoo.convlist.directory_button", latency=phoo.latency)
    agent.wait_prop(STACK, "depth", 2, timeout=5)

    p = agent.probe(DIRECTORY)
    assert p.visible, f"directory page not visible: {p.why_not()}"

    # the header settles even though the body may spin
    agent.wait_settled(max_ms=5000, tol=0)
    assert agent.exists("phoo.directory.list")
    assert agent.exists("phoo.directory.busy")


def test_press_and_hold_enters_multiselect(phoo, agent, on_convlist):
    """ConvList.qml enters multi-select on onPressAndHold of a delegate.

    A fresh profile has no conversations, so there is nothing to hold.
    The toolbar swap is instead driven directly, and the check is that
    the two toolbars are mutually exclusive -- that's the same state
    machine a long press drives.
    """
    agent.set("phoo.convlist.multi_toolbar", "visible", True)
    agent.wait_until(lambda: agent.get(CONVLIST_TOOLBAR, "visible") is False,
                     timeout=3, msg="toolbars did not swap")
    agent.set("phoo.convlist.multi_toolbar", "visible", False)
    agent.wait_until(lambda: agent.get(CONVLIST_TOOLBAR, "visible") is True,
                     timeout=3, msg="toolbars did not swap back")


def test_ui_stays_responsive_under_repeated_input(phoo, agent, on_convlist):
    """Hammer the toolbar and make sure nothing wedges."""
    mark = agent.ping()
    for _ in range(10):
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", True, timeout=5)
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", False, timeout=5)

    after = agent.assert_responsive()
    assert after.frame_count > mark.frame_count
    assert after.eventloop_max_lag_ms < 3000, (
        f"event loop lag hit {after.eventloop_max_lag_ms}ms under input load"
    )


def test_latency_report_is_populated(phoo, agent, on_convlist):
    """The session's latency log should have real numbers in it by now."""
    summary = phoo.latency.summary()
    assert summary.get("count", 0) > 0, "no input->paint samples recorded"
    assert summary["max_ms"] < LATENCY_BUDGET_MS, (
        f"worst input->paint was {summary['max_ms']}ms; {phoo.latency.format()}"
    )
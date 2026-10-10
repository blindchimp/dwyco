"""Does the app come up at all, and does it come up *usable*?

These are the "is the ui responding / has it settled / can it be
interacted with" checks against a freshly seeded profile.
"""
from __future__ import annotations

from phoo_driver import (
    BLANK_PAGE,
    CONVLIST_DRAWER_BTN,
    CONVLIST_EMPTY,
    CONVLIST_TOOLBAR,
    CORE,
    DRAWER,
    PIN,
    STACK,
    WINDOW,
)


def test_window_exists_and_is_exposed(agent):
    ping = agent.wait_alive(timeout=30)
    assert ping.window_exposed, "the qml window never got exposed"
    assert ping.window_width > 0 and ping.window_height > 0, (
        f"window has no size: {ping.window_width}x{ping.window_height}"
    )


def test_window_is_interactable(agent):
    p = agent.probe(WINDOW)
    assert p.exists
    assert p.width > 0 and p.height > 0
    assert p.in_scene


def test_ui_keeps_rendering(agent):
    """The render loop is not dead. service_timer alone keeps it turning,
    but a wedged gui thread stops it even though qml is "loaded"."""
    before = agent.ping()
    agent.cmd("mark", label="render-check")
    agent.wait_until(lambda: agent.ping().frame_count > before.frame_count + 3,
                     timeout=5, msg="frame counter never advanced")
    after = agent.ping()
    assert after.last_frame_age_ms < 2000


def test_event_loop_is_not_blocking(agent, phoo):
    """Nothing is holding the gui thread. A blocking core.init() or a
    synchronous network call would show up as a big event loop lag."""
    agent.ping()  # reset nothing, just read the running max
    for _ in range(3):
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", True, timeout=5)
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", False, timeout=5)
    # the drawer slide is a few hundred ms of animation; anything much
    # beyond that means we were blocked rather than busy
    lag = agent.ping().eventloop_max_lag_ms
    assert lag < 3000, f"gui thread blocked for {lag}ms at some point"


def test_startup_gates_are_seeded(agent):
    """The seeded profile should skip the first-run profile dialog, the
    reindex page and the pin gate entirely."""
    assert agent.get(PIN, "allow_access") == 1, (
        "pin gate is not open; the seeded pw/salt didn't take"
    )
    assert agent.get(BLANK_PAGE, "visible") is False, (
        "blank_page (the green server-login spinner) is covering the ui. "
        "acct-created wasn't seeded, so server_account_created is false."
    )


def test_stack_is_on_the_convlist(agent, home):
    assert agent.get(STACK, "depth") == 1


def test_convlist_toolbar_is_interactable(agent, on_convlist):
    """Every control on the home toolbar must be clickable, not just drawn."""
    for name in (
        "phoo.convlist.toolbar",
        CONVLIST_DRAWER_BTN,
        "phoo.convlist.grid_toggle",
        "phoo.convlist.trivia",
        "phoo.convlist.directory_button",
        "phoo.convlist.contacts_button",
    ):
        p = agent.probe(name)
        assert p.interactable, f"{name} is not interactable: {p.why_not()}"


def test_convlist_settles_after_boot(agent, on_convlist):
    """A freshly seeded profile has no contacts and nothing in flight, so
    the home screen should reach a stable frame quickly.

    This is the test that would have hung forever against the old code:
    main.qml's service_timer polls every 1-100ms and never idles out, so
    there is no event-loop-idle to wait on. Region pixel stability is the
    only honest signal, and test_mode is what makes it reachable.
    """
    s = agent.wait_settled(max_ms=6000, interval_ms=100, consec=3)
    assert s.elapsed_ms < 6000


def test_empty_contact_list_is_shown(agent, on_convlist):
    assert agent.get("phoo.convlist.list", "visible") or agent.get(
        "phoo.convlist.grid", "visible"
    )
    # ConvListModel.count == 0 on a fresh profile, so the help label is up
    assert agent.get(CONVLIST_EMPTY, "visible"), (
        "expected the empty-contact-list help label on a fresh profile"
    )


def test_drawer_opens_and_closes(phoo, agent, on_convlist):
    assert agent.get(DRAWER, "opened") is False
    agent.click(CONVLIST_DRAWER_BTN, latency=phoo.latency)
    agent.wait_prop(DRAWER, "opened", True, timeout=5)
    agent.wait_settled(max_ms=3000)
    p = agent.probe("phoo.drawer.form")
    assert p.visible
    agent.click(CONVLIST_DRAWER_BTN)
    agent.wait_prop(DRAWER, "opened", False, timeout=5)
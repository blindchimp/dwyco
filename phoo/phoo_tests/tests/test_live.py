"""Tests that need the real dwyco server.

Skipped unless PHOO_TEST_LIVE=1, because they're slow, they depend on
the server being up, and they'd make the offline suite flaky.

Run them with:

    PHOO_TEST_LIVE=1 pytest phoo_tests/tests/test_live.py -v

and note these also want a profile that was NOT seeded, since seeding
acct-created short-circuits the very server login they're checking:

    PHOO_TEST_NO_SEED=1 PHOO_TEST_LIVE=1 pytest phoo_tests/tests/test_live.py
"""
from __future__ import annotations

import pytest

from phoo_driver import (
    BLANK_PAGE,
    CONVLIST_TOOLBAR,
    CORE,
    DRAWER,
    SETTINGS,
    STACK,
)

pytestmark = pytest.mark.live


@pytest.fixture
def live_app(tmp_path_factory):
    """A separate, unseeded app so the login path actually runs."""
    import os
    from phoo_driver import Phoo

    seed = os.environ.get("PHOO_TEST_NO_SEED") != "1"
    app = Phoo(profile_dir=tmp_path_factory.mktemp("live-profile"), seed=seed)
    app.start()
    try:
        yield app
    finally:
        app.stop()


def test_server_login_clears_blank_page(live_app):
    """On a real profile the green blank_page spinner covers the ui until
    server_login(what === 1) arrives and main.qml flips
    server_account_created. Watch that transition happen."""
    agent = live_app.agent
    agent.wait_alive(timeout=60)

    seen_blank = agent.get(BLANK_PAGE, "visible")
    agent.wait_until(
        lambda: agent.get(BLANK_PAGE, "visible") is False,
        timeout=120,
        interval=1.0,
        msg=(
            "server login never completed, so the ui stayed behind the "
            f"blank_page spinner (started visible={seen_blank}). "
            f"{live_app.diagnostics()}"
        ),
    )
    # with the gate open the toolbar becomes usable
    p = agent.probe(CONVLIST_TOOLBAR)
    assert p.interactable, f"toolbar still not usable: {p.why_not()}"


def test_database_reports_online(live_app):
    """core.is_database_online is polled by main.qml's service_timer and
    fed into up_and_running. It should reach 1 once we're really
    connected."""
    agent = live_app.agent
    agent.wait_until(
        lambda: agent.get(CORE, "is_database_online") == 1,
        timeout=120,
        interval=1.0,
        msg=f"database never came online. {live_app.diagnostics()}",
    )


def test_settings_reachable_and_settles(live_app):
    """Same journey as the offline suite, but with a genuinely logged-in
    app underneath."""
    agent = live_app.agent
    agent.wait_until(
        lambda: agent.get("phoo.convlist.drawer_button", "enabled"),
        timeout=60,
        interval=0.5,
    )
    agent.click("phoo.convlist.drawer_button")
    agent.wait_prop(DRAWER, "opened", True, timeout=10)
    agent.wait_settled(max_ms=8000)
    agent.click("phoo.drawer.settings")
    agent.wait_prop(STACK, "depth", 2, timeout=10)
    agent.wait_settled(max_ms=8000)
    assert agent.probe(SETTINGS).visible
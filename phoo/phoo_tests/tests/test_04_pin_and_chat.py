"""The pin gate and the chat send flow.

The pin dialog is the awkward one: its digit pad is a Repeater of
RoundButtons that used to have no id, no objectName and no text on the
button (the label lived in a nested contentItem), so there was no way at
all to address one. Same story for the backspace key.
"""
from __future__ import annotations

import pytest

from phoo_driver import (
    CHAT_INPUT,
    CHAT_MESSAGES,
    CHAT_SEND,
    CONVLIST_GRID_TOGGLE,
    PIN,
    STACK,
)


def test_pin_digit_pad_is_addressable(agent):
    for i in range(1, 9):
        name = f"{PIN}.digit.{i - 1}"
        assert agent.exists(name), f"{name} not found"
    assert agent.exists(f"{PIN}.backspace")
    assert agent.exists(f"{PIN}.cancel")
    assert agent.exists(f"{PIN}.show")


def test_pin_digits_are_enabled_and_hit_testable(phoo, agent):
    """The pin gate is normally hidden, so force it up the way the app's
    own pause/resume path does (main.qml drives pwdialog.state)."""
    agent.set(PIN, "visible", True)
    try:
        agent.wait_settled(name=PIN, max_ms=3000)
        for i in range(1, 9):
            p = agent.probe(f"{PIN}.digit.{i - 1}")
            assert p.enabled, f"digit {i} is disabled: {p.why_not()}"
            assert p.hit_testable, (
                f"digit {i}: click lands on {p.hit_type} {p.hit_name!r}"
            )
    finally:
        agent.set(PIN, "visible", False)
        agent.wait_settled(max_ms=3000)


def test_typing_a_pin_moves_the_entry(phoo, agent):
    agent.set(PIN, "visible", True)
    try:
        agent.wait_settled(name=PIN, max_ms=3000)
        # whatever the entry currently holds, four digits must change it
        before = agent.get(PIN, "pw")
        for i in range(4):
            agent.click(f"{PIN}.digit.0", latency=phoo.latency)
        after = agent.get(PIN, "pw")
        assert after != before, "clicking the digit buttons didn't enter anything"
        assert len(after) == 4, f"expected a 4 digit entry, got {after!r}"

        # and backspace removes one
        agent.click(f"{PIN}.backspace")
        assert len(agent.get(PIN, "pw")) == 3
    finally:
        # reset the entry so we don't leave the dialog in a locked state
        agent.set(PIN, "pw", "")
        agent.set(PIN, "visible", False)
        agent.wait_settled(max_ms=3000)


def test_pinned_digits_disable_at_four(phoo, agent):
    agent.set(PIN, "visible", True)
    try:
        agent.wait_settled(name=PIN, max_ms=3000)
        for _ in range(4):
            agent.click(f"{PIN}.digit.0")
        p = agent.probe(f"{PIN}.digit.0")
        assert p.enabled is False, (
            "digit buttons should disable once 4 digits are entered "
            "(PINDialog.qml: enabled: pw.length < 4)"
        )
    finally:
        agent.set(PIN, "pw", "")
        agent.set(PIN, "visible", False)
        agent.wait_settled(max_ms=3000)


@pytest.mark.skip(reason="needs a conversation to exist; see test_live.py")
def test_chat_send_flow(phoo, agent):
    """Type a message, the send button enables, click it, the message shows.

    Requires a conversation, which needs the server, so it lives in the
    live suite.
    """
    agent.click(CHAT_INPUT, latency=phoo.latency)
    agent.wait_prop(CHAT_SEND, "enabled", False, timeout=3)

    agent.type_text(CHAT_INPUT, "hello from the harness", latency=phoo.latency)
    agent.wait_prop(CHAT_SEND, "enabled", True, timeout=5)
    agent.wait_settled(name=CHAT_INPUT, max_ms=3000)

    agent.click(CHAT_SEND, latency=phoo.latency)
    agent.wait_settled(name=CHAT_MESSAGES, max_ms=5000)
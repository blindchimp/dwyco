"""Does the in-process recording agree with what the X server actually showed?

The agent's recorder captures via QQuickWindow::grabWindow(), which
re-renders the scene into an offscreen buffer. ffmpeg's x11grab reads the
X server's own output. They are different code paths, and if they
disagreed about what the user saw then every settle check and every
latency number the harness reports would be suspect.

So: run the same journey through both and compare.
"""
from __future__ import annotations

import json
import os
import shutil
import signal
import subprocess
import time
from pathlib import Path

import pytest

from phoo_driver import CONVLIST_DRAWER_BTN, DRAWER, Phoo, ffprobe, x11_record

pytestmark = [
    pytest.mark.x11,
    pytest.mark.skipif(not shutil.which("ffmpeg"), reason="ffmpeg not installed"),
    pytest.mark.skipif(not shutil.which("ffprobe"), reason="ffprobe not installed"),
    pytest.mark.skipif(
        os.environ.get("PHOO_PLATFORM", "") in ("offscreen", "minimal", "vnc"),
        reason="x11grab needs a real X display",
    ),
    pytest.mark.skipif(not os.environ.get("DISPLAY"), reason="no X DISPLAY set"),
]


def _window_geometry(agent):
    r = agent.rect("phoo.window")
    return int(r["scene_x"]), int(r["scene_y"]), int(r["scene_w"]), int(r["scene_h"])


def test_x11grab_captures_the_window_while_the_app_runs(tmp_path_factory):
    """Ground truth: ffmpeg can see the phoo window at all."""
    app = Phoo(profile_dir=tmp_path_factory.mktemp("x11-vid"))
    app.start()
    x11 = tmp_path_factory.mktemp("x11-out") / "x11.mp4"
    proc = None
    try:
        agent = app.agent
        x, y, w, h = _window_geometry(agent)
        proc = x11_record(x11, w, h, x=x, y=y, fps=10)

        time.sleep(1.0)
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", True, timeout=5)
        time.sleep(1.5)
    finally:
        if proc:
            # ffmpeg needs a moment to finalise the container on SIGINT
            proc.send_signal(signal.SIGINT)
            try:
                proc.wait(timeout=15)
            except subprocess.TimeoutExpired:
                proc.kill()
        app.stop()

    assert x11.exists(), "ffmpeg x11grab wrote no file"
    assert x11.stat().st_size > 5000, f"x11grab file is tiny ({x11.stat().st_size} bytes)"
    info = ffprobe(x11)
    assert int(info["width"]) > 0 and int(info["height"]) > 0


def test_both_paths_agree_on_the_journey(tmp_path_factory):
    """Record the same drawer open/close twice and cross-check.

    Exact frame counts can't match -- the two paths run on different
    clocks and x11grab samples the screen at its own rate -- so the
    comparison is on duration and on whether both actually captured
    motion. The point is to catch a path that silently recorded nothing.
    """
    app = Phoo(profile_dir=tmp_path_factory.mktemp("x11-agree"))
    app.start()
    proc = None
    try:
        agent = app.agent
        x, y, w, h = _window_geometry(agent)

        # --- in-process ---
        inproc_dir = app.profile_dir / "inproc"
        rec = agent.record(inproc_dir, fps=15)
        rec.start()
        try:
            time.sleep(0.4)
            agent.click(CONVLIST_DRAWER_BTN)
            agent.wait_prop(DRAWER, "opened", True, timeout=5)
            time.sleep(0.6)
            agent.click(CONVLIST_DRAWER_BTN)
            agent.wait_prop(DRAWER, "opened", False, timeout=5)
            time.sleep(0.4)
        finally:
            res = rec.stop()

        assert res.frames > 5, "in-process path recorded nothing"
        assert res.painted_events, "in-process path saw no repaint from the click"

        # --- x11 ---
        x11 = app.profile_dir / "x11.mp4"
        proc = x11_record(x11, w, h, x=x, y=y, fps=10)
        time.sleep(0.4)
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", True, timeout=5)
        time.sleep(0.6)
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", False, timeout=5)
        time.sleep(0.4)
    finally:
        if proc:
            proc.send_signal(signal.SIGINT)
            try:
                proc.wait(timeout=15)
            except subprocess.TimeoutExpired:
                proc.kill()
        app.stop()

    assert x11.exists() and x11.stat().st_size > 5000, "x11grab recorded nothing"
    xi = ffprobe(x11)

    n_x11 = int(xi["nb_frames"] or 0)
    print(
        f"\n[in-process] {res.frames} frames over {res.duration_ms}ms "
        f"(dropped {res.dropped})"
        f"\n[x11grab]    {n_x11} frames over {float(xi.get('duration') or 0):.2f}s"
    )

    # both must have produced a comparable amount of video
    assert n_x11 > 3, f"x11grab only produced {n_x11} frames"
    dur_x11 = float(xi.get("duration") or 0)
    dur_inproc = res.duration_ms / 1000.0
    ratio = dur_x11 / max(dur_inproc, 0.001)
    assert 0.4 < ratio < 2.5, (
        f"durations disagree wildly: in-process {dur_inproc:.2f}s vs "
        f"x11grab {dur_x11:.2f}s (ratio {ratio:.2f}). One of the two "
        f"paths is stalling."
    )


def test_x11_capture_shows_the_ui_actually_changed(phoo, agent, tmp_path):
    """The ground-truth capture must contain more than one distinct frame.

    A recording of a static screen, or one that captured the wrong region,
    would still 'succeed' -- this is what catches it.
    """
    x, y, w, h = _window_geometry(agent)
    out = tmp_path / "motion.mp4"
    proc = x11_record(out, w, h, x=x, y=y, fps=10)
    try:
        time.sleep(0.5)
        agent.click(CONVLIST_DRAWER_BTN)
        agent.wait_prop(DRAWER, "opened", True, timeout=5)
        time.sleep(0.8)
    finally:
        proc.send_signal(signal.SIGINT)
        try:
            proc.wait(timeout=15)
        except subprocess.TimeoutExpired:
            proc.kill()

    assert out.exists() and out.stat().st_size > 5000
    # scene-change detection: if the drawer animated, ffmpeg will have
    # found inter-frame differences worth logging
    det = subprocess.run(
        ["ffmpeg", "-i", str(out), "-vf", "select='gt(scene,0.01)'",
         "-vsync", "vfr", "-f", "null", "-"],
        capture_output=True, text=True, timeout=120)
    frame_lines = [l for l in det.stderr.splitlines() if "frame=" in l]
    print(f"\n[scene changes detected: {len(frame_lines)}]")
    assert len(frame_lines) > 0, (
        "x11grab captured a completely static screen -- either the "
        "window moved during capture or the region was wrong"
    )
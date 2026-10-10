"""Recording the app: does the video actually capture what happened?

The interesting assertions are not "a file appeared" but:
  - the frames on disk match what the agent reported
  - a real click produced a real repaint, with a real latency
  - a click on something inert is reported as *not* painting
  - the encode is a valid, playable video
  - the writer thread keeps up, or degrades honestly by dropping frames
"""
from __future__ import annotations

import json
import shutil
import time

import pytest

from phoo_driver import (
    CONVLIST_DRAWER_BTN,
    DRAWER,
    RecordResult,
    encode_video,
    ffprobe,
)

need_ffmpeg = pytest.mark.skipif(
    not shutil.which("ffmpeg"), reason="ffmpeg not installed"
)
need_ffprobe = pytest.mark.skipif(
    not shutil.which("ffprobe"), reason="ffprobe not installed"
)


def test_grab_returns_a_real_image(agent):
    """Everything else is built on this, so check it first."""
    png = agent.grab()
    assert png[:8] == b"\x89PNG\r\n\x1a\n", "grab is not a png"
    assert len(png) > 1000, f"grab is suspiciously small ({len(png)} bytes)"


def test_record_is_idle_by_default(agent):
    st = agent.record_status()
    assert st["recording"] is False
    assert st["frames"] == 0


def test_recording_captures_frames_and_writes_meta(phoo, agent, on_convlist):
    rec = agent.record(phoo.profile_dir / "rec-basic", fps=15)
    rec.start()
    try:
        agent.open_drawer()
        time.sleep(0.4)
        agent.close_drawer()
        time.sleep(0.3)

        mid = agent.record_status()
        assert mid["recording"], "recording stopped on its own"
        assert mid["frames"] > 5, f"only {mid['frames']} frames so far"
    finally:
        res = rec.stop()

    assert isinstance(res, RecordResult)
    assert res.frames > 5

    # on-disk frames must match what we were told
    frames = sorted((rec.path / "frames").glob("*.png"))
    assert len(frames) == res.frames, (
        f"{len(frames)} png files but agent reported {res.frames}"
    )
    assert all(f.stat().st_size > 200 for f in frames), "a frame file is empty"

    meta = json.loads((rec.path / "meta.json").read_text())
    assert meta["frames"] == res.frames
    assert len(meta["frame_log"]) == res.frames
    assert meta["width"] > 0 and meta["height"] > 0

    ts = [e[1] for e in meta["frame_log"]]
    assert all(b >= a for a, b in zip(ts, ts[1:])), "frame timestamps went backwards"


def test_recording_captures_the_click_and_its_latency(phoo, agent, on_convlist):
    """The whole point: the log has to say where the input went and how
    long the ui took to respond."""
    rec = agent.record(phoo.profile_dir / "rec-events", fps=15)
    rec.start()
    try:
        agent.open_drawer()
        time.sleep(0.5)
    finally:
        res = rec.stop()

    painted = res.painted_events
    assert painted, f"no input was recorded as painting. events={res.events}"

    ev = next(e for e in painted if e["name"] == CONVLIST_DRAWER_BTN)
    assert ev["cmd"] == "click"
    assert "x" in ev and "y" in ev, "the click has no scene coordinates"
    assert ev["paint_frame"] >= ev["frame"], (
        f"paint frame {ev['paint_frame']} precedes input frame {ev['frame']}"
    )
    assert ev["latency_ms"] >= 0, "no input->paint latency recorded"


def test_every_input_gets_an_explicit_painted_flag(phoo, agent, on_convlist):
    """An input that caused no repaint must be reported as such.

    This matters more than it looks: a click that changes nothing and a
    click that changed the screen look identical if you only record
    "the click happened". painted=false is the difference between "the ui
    ignored this" and "the ui responded", and it is what tells you a
    selector has gone stale.
    """
    rec = agent.record(phoo.profile_dir / "rec-inert", fps=15)
    rec.start()
    try:
        # a hover over the toolbar: real input, but nothing to repaint
        agent.hover(CONVLIST_DRAWER_BTN)
        time.sleep(0.5)
    finally:
        res = rec.stop()

    inputs = [e for e in res.events if e.get("event") == "input"]
    assert inputs, "the hover was not logged"
    for e in inputs:
        assert "painted" in e, f"input event has no painted flag: {e}"
        if not e["painted"]:
            # and it must not have invented a latency for it
            assert e["latency_ms"] == -1
            assert e["paint_frame"] == -1


def test_recording_can_run_twice(phoo, agent, on_convlist):
    """No leaked writer thread or recorder state between runs."""
    for i in (1, 2):
        rec = agent.record(phoo.profile_dir / f"rec-twice-{i}", fps=10)
        rec.start()
        time.sleep(0.4)
        res = rec.stop()
        assert res.frames > 2, f"run {i} captured {res.frames} frames"
        st = agent.record_status()
        assert st["recording"] is False


def test_max_ms_auto_stops_the_recorder(phoo, agent, on_convlist):
    rec = agent.record(phoo.profile_dir / "rec-auto", fps=10, max_ms=700)
    rec.start()
    # no explicit stop; the agent should stop itself
    agent.wait_until(
        lambda: agent.record_status()["recording"] is False,
        timeout=6, msg="max_ms never auto-stopped the recorder")
    res = rec.stop()
    assert res.frames > 2, "auto-stopped recording wrote no frames"
    # the drain must have completed, otherwise the last queued frames are
    # still sitting in the writer thread when meta.json is written
    on_disk = list((rec.path / "frames").glob("*.png"))
    assert len(on_disk) == res.frames, (
        f"{len(on_disk)} frames on disk but agent reported {res.frames}; "
        "the writer queue was not drained"
    )


def test_recording_context_manager_drains_on_exception(phoo, agent, on_convlist):
    """If the body raises we must still drain the recording, or the
    writer thread and the output directory are left dangling."""
    rec = agent.record(phoo.profile_dir / "rec-ctx", fps=10)
    with pytest.raises(ValueError):
        with rec:
            time.sleep(0.3)
            raise ValueError("boom")
    assert rec.result is not None, "context manager did not drain on exception"
    assert rec.result.frames > 1


@need_ffmpeg
def test_encode_produces_a_playable_mp4(phoo, agent, on_convlist):
    rec = agent.record(phoo.profile_dir / "rec-mp4", fps=15)
    rec.start()
    try:
        agent.open_drawer()
        time.sleep(0.6)
    finally:
        rec.stop()

    out = rec.encode()
    assert out.exists(), f"encode produced nothing at {out}"
    assert out.stat().st_size > 2000, f"mp4 is tiny ({out.stat().st_size} bytes)"
    assert out.suffix == ".mp4"

    if shutil.which("ffprobe"):
        info = ffprobe(out)
        assert int(info["width"]) > 0 and int(info["height"]) > 0
        # constant-rate encoding duplicates frames to honour the real
        # per-frame durations, so the encoded count is >= the source count
        assert int(info["nb_frames"]) >= rec.result.frames


@need_ffmpeg
def test_encode_gif_is_downscaled_and_valid(phoo, agent, on_convlist):
    rec = agent.record(phoo.profile_dir / "rec-gif", fps=10)
    rec.start()
    time.sleep(0.6)
    rec.stop()
    out = rec.encode(fmt="gif")
    assert out.exists() and out.stat().st_size > 1000
    assert out.read_bytes()[:6] in (b"GIF87a", b"GIF89a")


@need_ffmpeg
def test_journey_records_a_scripted_sequence(phoo, agent, on_convlist):
    """A journey reads like the user story, and produces one clip."""
    res = agent.journey(
        phoo.profile_dir / "rec-journey",
        [
            lambda a: a.open_drawer(),
            lambda a: a.close_drawer(),
        ],
        fps=15,
    )
    assert res.frames > 8, f"journey captured only {res.frames} frames"
    assert res.painted_events, "the journey's clicks painted nothing"

    out = encode_video(phoo.profile_dir / "rec-journey", fmt="mp4")
    assert out.exists()


@need_ffmpeg
@need_ffprobe
def test_video_matches_the_window_size(phoo, agent, on_convlist):
    rec = agent.record(phoo.profile_dir / "rec-size", fps=10)
    rec.start()
    time.sleep(0.4)
    res = rec.stop()
    out = rec.encode()
    info = ffprobe(out)
    assert int(info["width"]) == res.meta["width"]
    assert int(info["height"]) == res.meta["height"]


@need_ffmpeg
def test_recording_does_not_wedge_the_app(phoo, agent, on_convlist):
    """The recorder must not be able to hang the app. This is the whole
    risk of capturing frames on the gui thread."""
    mark = agent.ping()
    rec = agent.record(phoo.profile_dir / "rec-stress", fps=25, max_ms=0)
    rec.start()
    time.sleep(1.5)
    res = rec.stop()
    assert res.frames > 10
    after = agent.assert_responsive()
    assert after.heartbeat_count > mark.heartbeat_count
    # the heartbeat should still be running near 1khz; a big max lag
    # would mean the grabs stalled the gui thread badly
    assert after.eventloop_max_lag_ms < 2000, (
        f"recording stalled the gui thread for "
        f"{after.eventloop_max_lag_ms}ms"
    )


def test_latency_buckets_are_separate(phoo, agent, on_convlist):
    """Tests must not accidentally measure instrumented timings."""
    before_clean = len(phoo.latency_clean.samples)
    before_rec = len(phoo.latency_recorded.samples)

    # while recording, `phoo.latency` hands out the recorded bucket, so a
    # click made now lands there and NOT in the clean numbers
    phoo.latency_recording = True
    try:
        agent.click(CONVLIST_DRAWER_BTN, latency=phoo.latency)
        assert len(phoo.latency_recorded.samples) > before_rec, (
            "a click during recording should land in the recorded bucket"
        )
        assert len(phoo.latency_clean.samples) == before_clean, (
            "the clean bucket must not be polluted by a recorded click"
        )
    finally:
        phoo.latency_recording = False
        agent.close_drawer()
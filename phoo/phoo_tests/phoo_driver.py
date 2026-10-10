"""Client for the in-process phoo GUI test agent (see ../testagent.cpp).

The agent answers the three questions a UX test actually asks about a UI:

    is it responding?   ping()    -> frame counter + event loop lag
    has it settled?     settle()  -> region-of-interest pixel diff
    can it be clicked?  probe()   -> visible/enabled/opacity/size/hit test

Input is injected in-process through QWindowSystemInterface, which is the
same entry point a physical mouse uses, so events go through the real Qt
input pipeline -- but the window doesn't need focus and no X server is
needed.

Everything here is synchronous and talks newline-delimited JSON over a
QLocalServer (a unix socket on linux).
"""
from __future__ import annotations

import base64
import json
import os
import socket
import statistics
import subprocess
import tempfile
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Iterable, Sequence

REPO = Path(__file__).resolve().parent.parent

# QML objectNames, so tests don't sprinkle string literals around.
WINDOW = "phoo.window"
STACK = "phoo.stack"
DRAWER = "phoo.drawer"
PIN = "phoo.pin"
BLANK_PAGE = "phoo.blank_page"
CORE = "dwyco_singleton"

CONVLIST = "phoo.convlist"
CONVLIST_TOOLBAR = "phoo.convlist.toolbar"
CONVLIST_MULTI = "phoo.convlist.multi_toolbar"
CONVLIST_DRAWER_BTN = "phoo.convlist.drawer_button"
CONVLIST_GRID_TOGGLE = "phoo.convlist.grid_toggle"
CONVLIST_TRIVIA = "phoo.convlist.trivia"
CONVLIST_DIRECTORY_BTN = "phoo.convlist.directory_button"
CONVLIST_CONTACTS_BTN = "phoo.convlist.contacts_button"
CONVLIST_LIST = "phoo.convlist.list"
CONVLIST_GRID = "phoo.convlist.grid"
CONVLIST_EMPTY = "phoo.convlist.empty_help"

CHAT_INPUT = "phoo.chat.input"
CHAT_SEND = "phoo.chat.send"
CHAT_BACK = "phoo.chat.back"
CHAT_MESSAGES = "phoo.chat.messages"
CHAT_BUSY = "phoo.chat.busy"
CHAT_FAILED = "phoo.chat.failed_toast"
CHAT_TYPING = "phoo.chat.typing"

SETTINGS = "phoo.settings"
SETTINGS_PIN_EXPIRE = "phoo.settings.pin_expire"
SETTINGS_UNREVIEWED = "phoo.settings.unreviewed"
SETTINGS_SHOW_HIDDEN = "phoo.settings.show_hidden"
SETTINGS_SHOW_ARCHIVED = "phoo.settings.show_archived"
SETTINGS_BLOCK_LIST = "phoo.settings.block_list"
SETTINGS_PIN_LOCK = "phoo.settings.pin_lock"
SETTINGS_TRASH = "phoo.settings.trash"
SETTINGS_ABOUT = "phoo.settings.about"

DRAWER_SETTINGS = "phoo.drawer.settings"
DRAWER_PROFILE = "phoo.drawer.update_profile"
DRAWER_FAVS = "phoo.drawer.browse_favs"
DRAWER_HIDDEN = "phoo.drawer.browse_hidden"
DRAWER_QUIET = "phoo.drawer.quiet"
DRAWER_INVISIBLE = "phoo.drawer.invisible"
DRAWER_LOCK_EXIT = "phoo.drawer.lock_and_exit"

DIRECTORY = "phoo.directory"
DIRECTORY_LIST = "phoo.directory.list"
DIRECTORY_BUSY = "phoo.directory.busy"


class AgentError(RuntimeError):
    """The agent replied, but with ok:false."""

    def __init__(self, cmd: str, payload: dict):
        self.cmd = cmd
        self.payload = payload
        super().__init__(f"{cmd} failed: {payload.get('error', payload)}")


class AgentTimeout(RuntimeError):
    pass


@dataclass
class Ping:
    frame_count: int
    last_frame_age_ms: int
    eventloop_max_lag_ms: int
    heartbeat_count: int = 0
    eventloop_p99_lag_ms: int | None = None
    eventloop_median_lag_ms: int | None = None
    window_bound: bool = False
    window_exposed: bool = False
    window_width: int = 0
    window_height: int = 0
    window_title: str = ""
    last_input_latency_ms: int | None = None
    uptime_ms: int = 0

    @property
    def alive(self) -> bool:
        """The gui thread is turning."""
        return self.heartbeat_count > 0


@dataclass
class Probe:
    name: str
    exists: bool
    visible: bool
    enabled: bool
    opacity: float
    width: float
    height: float
    ancestors_visible: bool
    in_scene: bool
    hit_testable: bool
    interactable: bool
    hit_name: str = ""
    hit_type: str = ""
    scene_rect: tuple[float, float, float, float] = (0, 0, 0, 0)

    def why_not(self) -> str:
        """Human readable reason this element can't be clicked."""
        reasons = []
        if not self.visible:
            reasons.append("not visible")
        if not self.enabled:
            reasons.append("not enabled")
        if self.opacity <= 0:
            reasons.append(f"opacity {self.opacity}")
        if self.width <= 0 or self.height <= 0:
            reasons.append(f"size {self.width}x{self.height}")
        if not self.ancestors_visible:
            reasons.append("an ancestor is hidden")
        if not self.in_scene:
            reasons.append("outside the window")
        if not self.hit_testable:
            reasons.append(
                f"point is covered by {self.hit_type or 'nothing'}"
                f" {self.hit_name!r}".strip()
            )
        return ", ".join(reasons) or "interactable"


@dataclass
class Settle:
    settled: bool
    elapsed_ms: int
    samples: int
    max_diff_pixels: int
    frame_count: int
    eventloop_max_lag_ms: int


@dataclass
class LatencyReport:
    """input -> paint latency, collected across a test."""

    samples: list[tuple[str, int]] = field(default_factory=list)

    def record(self, label: str, ms: int) -> None:
        self.samples.append((label, ms))

    def summary(self) -> dict[str, Any]:
        if not self.samples:
            return {}
        vals = sorted(ms for _, ms in self.samples)
        return {
            "count": len(vals),
            "min_ms": vals[0],
            "p50_ms": int(statistics.median(vals)),
            "p95_ms": vals[min(len(vals) - 1, int(len(vals) * 0.95))],
            "max_ms": vals[-1],
        }

    def format(self) -> str:
        s = self.summary()
        if not s:
            return "no latency samples"
        return (
            f"n={s['count']} p50={s['p50_ms']}ms "
            f"p95={s['p95_ms']}ms max={s['max_ms']}ms"
        )


class Agent:
    """Synchronous JSON client for the test agent socket."""

    def __init__(self, socket_path: str | os.PathLike, timeout: float = 15.0):
        self.socket_path = str(socket_path)
        self.timeout = timeout
        self._sock: socket.socket | None = None
        self._buf = b""
        self._next_id = 1

    # ---- connection ----

    def connect(self, deadline_s: float = 30.0) -> "Agent":
        end = time.monotonic() + deadline_s
        last: Exception | None = None
        while time.monotonic() < end:
            try:
                s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                s.settimeout(self.timeout)
                s.connect(self.socket_path)
                self._sock = s
                return self
            except (FileNotFoundError, ConnectionRefusedError, OSError) as e:
                last = e
                time.sleep(0.05)
        raise AgentTimeout(
            f"could not connect to agent socket {self.socket_path}: {last}"
        )

    def close(self) -> None:
        if self._sock:
            try:
                self._sock.close()
            except OSError:
                pass
            self._sock = None

    def __enter__(self) -> "Agent":
        return self.connect()

    def __exit__(self, *exc) -> None:
        self.close()

    # ---- protocol ----

    def cmd(self, cmd: str, expect_ok: bool = True, **kw) -> dict:
        if self._sock is None:
            raise AgentTimeout("not connected")
        req = {"id": self._next_id, "cmd": cmd}
        req.update({k: v for k, v in kw.items() if v is not None})
        self._next_id += 1
        self._sock.sendall(json.dumps(req).encode() + b"\n")

        end = time.monotonic() + self.timeout
        while b"\n" not in self._buf:
            if time.monotonic() > end:
                raise AgentTimeout(f"no reply to {cmd!r}")
            chunk = self._sock.recv(65536)
            if not chunk:
                raise AgentTimeout(f"agent closed the connection during {cmd!r}")
            self._buf += chunk
        line, self._buf = self._buf.split(b"\n", 1)
        resp = json.loads(line.decode())
        if expect_ok and not resp.get("ok"):
            raise AgentError(cmd, resp)
        return resp

    # ---- liveness ----

    def ping(self) -> Ping:
        p = self.cmd("ping")
        return Ping(
            frame_count=p["frame_count"],
            last_frame_age_ms=p["last_frame_age_ms"],
            eventloop_max_lag_ms=p["eventloop_max_lag_ms"],
            heartbeat_count=p.get("heartbeat_count", 0),
            eventloop_p99_lag_ms=p.get("eventloop_p99_lag_ms"),
            eventloop_median_lag_ms=p.get("eventloop_median_lag_ms"),
            window_bound=p.get("window_bound", False),
            window_exposed=p.get("window_exposed", False),
            window_width=p.get("window_width", 0),
            window_height=p.get("window_height", 0),
            window_title=p.get("window_title", ""),
            last_input_latency_ms=p.get("last_input_latency_ms", -1),
            uptime_ms=p.get("uptime_ms", 0),
        )

    def wait_alive(self, timeout: float = 30.0) -> Ping:
        """Block until the window exists and the gui thread is turning.

        Deliberately does not wait for frames: QQuickWindow only renders
        when something in the scene is dirty, so frame_count legitimately
        sits at 0 on a static screen. heartbeat_count is the signal that
        works regardless.
        """
        end = time.monotonic() + timeout
        p = self.ping()
        while time.monotonic() < end:
            if p.window_bound and p.heartbeat_count > 0:
                return p
            time.sleep(0.05)
            p = self.ping()
        raise AgentTimeout(f"app never became alive: {p}")

    def assert_responsive(self, max_frame_age_ms: int = 2000) -> Ping:
        """The ui thread is not wedged.

        The heartbeat check is the real assertion: it advances on every
        event loop turn, so a blocking core.init(), a deadlock or a
        synchronous network call stops it dead. The frame check is
        secondary and only applies once the app has painted something --
        on a static screen frames legitimately stop.
        """
        before = self.ping()
        time.sleep(0.25)
        after = self.ping()

        assert after.heartbeat_count > before.heartbeat_count, (
            f"gui thread stopped turning: heartbeat went "
            f"{before.heartbeat_count} -> {after.heartbeat_count} over 250ms. "
            "Something is blocking the event loop."
        )
        assert after.window_bound, "no window bound"

        if before.frame_count > 0:
            assert after.frame_count >= before.frame_count, (
                f"frame counter went backwards: "
                f"{before.frame_count} -> {after.frame_count}"
            )
            assert after.last_frame_age_ms < max_frame_age_ms, (
                f"last frame was {after.last_frame_age_ms}ms ago "
                f"(limit {max_frame_age_ms}ms)"
            )
        return after

    def max_lag_since(self, mark: Ping) -> int:
        return self.ping().eventloop_max_lag_ms - mark.eventloop_max_lag_ms

    # ---- lookup ----

    def find(self, name: str) -> dict | None:
        r = self.cmd("find", name=name, expect_ok=False)
        return r.get("object") if r.get("ok") else None

    def exists(self, name: str) -> bool:
        return self.find(name) is not None

    def rect(self, name: str) -> dict:
        return self.cmd("rect", name=name)["rect"]

    def probe(self, name: str, expect_exists: bool = True) -> Probe | None:
        r = self.cmd("probe", name=name, expect_ok=False)
        if not r.get("ok"):
            if expect_exists:
                raise AgentError("probe", r)
            return None
        return Probe(
            name=name,
            exists=True,
            visible=r["visible"],
            enabled=r["enabled"],
            opacity=r["opacity"],
            width=r["width"],
            height=r["height"],
            ancestors_visible=r["ancestors_visible"],
            in_scene=r["in_scene"],
            hit_testable=r["hit_testable"],
            interactable=r["interactable"],
            hit_name=r.get("hit_name", ""),
            hit_type=r.get("hit_type", ""),
            scene_rect=tuple(r["scene_rect"]),
        )

    def tree(self, depth: int = 3, root: str | None = None) -> dict:
        return self.cmd("tree", depth=depth, name=root)["tree"]

    # ---- properties ----

    def get(self, name: str, prop: str) -> Any:
        return self.cmd("get", name=name, prop=prop)["value"]

    def set(self, name: str, prop: str, value: Any) -> None:
        self.cmd("set", name=name, prop=prop, value=value)

    def call(self, name: str, method: str, *args: Any) -> Any:
        r = self.cmd("call", name=name, method=method, args=list(args), expect_ok=False)
        if not r.get("ok"):
            raise AgentError("call", r)
        return r.get("return")

    # ---- pixels ----

    def grab(self, roi: Sequence[int] | None = None, name: str | None = None) -> bytes:
        r = self.cmd("grab", rect=list(roi) if roi else None, name=name)
        return base64.b64decode(r["data_b64"])

    def settle(
        self,
        roi: Sequence[int] | None = None,
        name: str | None = None,
        interval_ms: int = 100,
        consec: int = 3,
        max_ms: int = 3000,
        tol: int = 0,
    ) -> Settle:
        """Has the region stopped changing pixels?

        `name` restricts the check to one element's box, which is how you
        settle a control without waiting on something that legitimately
        never stops (a BusyIndicator, a typing pulse).
        """
        r = self.cmd(
            "settle",
            rect=list(roi) if roi else None,
            name=name,
            interval_ms=interval_ms,
            consec=consec,
            max_ms=max_ms,
            tol=tol,
        )
        return Settle(
            settled=r["settled"],
            elapsed_ms=r["elapsed_ms"],
            samples=r["samples"],
            max_diff_pixels=r["max_diff_pixels"],
            frame_count=r["frame_count"],
            eventloop_max_lag_ms=r["eventloop_max_lag_ms"],
        )

    def wait_settled(self, **kw) -> Settle:
        """settle() but a timeout is a failure, not a returned object."""
        s = self.settle(**kw)
        assert s.settled, (
            f"never settled after {s.elapsed_ms}ms "
            f"({s.samples} samples, up to {s.max_diff_pixels} pixels changing)"
        )
        return s

    # ---- waiting ----

    def wait_prop(
        self, name: str, prop: str, expected: Any = True, timeout: float = 10.0
    ) -> Any:
        end = time.monotonic() + timeout
        last = None
        while time.monotonic() < end:
            last = self.get(name, prop)
            if last == expected:
                return last
            time.sleep(0.05)
        raise AgentTimeout(
            f"{name}.{prop} was {last!r}, expected {expected!r}, after {timeout}s"
        )

    def wait_until(self, predicate, timeout: float = 10.0, interval: float = 0.05, msg=""):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            v = predicate()
            if v:
                return v
            time.sleep(interval)
        raise AgentTimeout(f"condition never became true after {timeout}s. {msg}")

    def mark(self, label: str) -> None:
        self.cmd("mark", label=label)

    def drain_events(self) -> list[dict]:
        return self.cmd("events")["events"]

    # ---- input ----

    def _after_input(self, label: str, latency: LatencyReport | None) -> Ping:
        """Pick up the input->paint latency the agent measured for us."""
        p = self.ping()
        if latency is not None and p.last_input_latency_ms is not None and p.last_input_latency_ms >= 0:
            latency.record(label, p.last_input_latency_ms)
        return p

    def click(
        self,
        name: str | None = None,
        x: int | None = None,
        y: int | None = None,
        shift: bool = False,
        ctrl: bool = False,
        alt: bool = False,
        latency: LatencyReport | None = None,
    ) -> Ping:
        r = self.cmd("click", name=name, x=x, y=y, shift=shift or None,
                     ctrl=ctrl or None, alt=alt or None)
        return self._after_input(f"click:{name or (x, y)}", latency)

    def press(self, name: str, latency: LatencyReport | None = None) -> Ping:
        self.cmd("press", name=name)
        return self._after_input(f"press:{name}", latency)

    def release(self, name: str, latency: LatencyReport | None = None) -> Ping:
        self.cmd("release", name=name)
        return self._after_input(f"release:{name}", latency)

    def hover(self, name: str | None = None, x=None, y=None) -> Ping:
        self.cmd("hover", name=name, x=x, y=y)
        return self.ping()

    def drag(self, name: str, tox: int, toy: int) -> Ping:
        self.cmd("drag", name=name, tox=tox, toy=toy)
        return self.ping()

    def press_hold(self, name: str, ms: int = 800,
                   latency: LatencyReport | None = None) -> Ping:
        self.cmd("presshold", name=name, ms=ms)
        return self._after_input(f"presshold:{name}", latency)

    def type_text(self, name: str, text: str,
                  latency: LatencyReport | None = None) -> Ping:
        self.cmd("type", name=name, text=text)
        return self._after_input(f"type:{name}", latency)

    def key(self, keycode: int, shift: bool = False, ctrl: bool = False,
            latency: LatencyReport | None = None) -> Ping:
        self.cmd("key", keycode=keycode, shift=shift or None, ctrl=ctrl or None)
        return self._after_input(f"key:{keycode}", latency)

    # ---- lifecycle ----

    def quit(self) -> None:
        try:
            self.cmd("quit", expect_ok=False)
        except (OSError, AgentTimeout):
            pass

    # ---- recording ----

    def record(self, path: "Path | str", fps: int = 15, max_ms: int = 0,
               cursor: bool = True) -> "Recording":
        """Start an in-app recording. Use as a context manager."""
        return Recording(self, path, fps=fps, max_ms=max_ms, cursor=cursor)

    def record_status(self) -> dict:
        return self.cmd("record", sub="status")

    def journey(self, path: "Path | str", steps, fps: int = 15,
                settle_between: bool = True) -> RecordResult:
        """Record a scripted sequence and return the result.

        `steps` is any iterable of callables taking this agent, so a
        journey reads like the user story it is:

            agent.journey(out, [
                lambda a: a.click("phoo.convlist.drawer_button"),
                lambda a: a.wait_prop(DRAWER, "opened", True, timeout=5),
                lambda a: a.click("phoo.drawer.settings"),
            ])
        """
        rec = Recording(self, path, fps=fps)
        rec.start()
        try:
            for step in steps:
                step(self)
                if settle_between:
                    self.settle(max_ms=1500)
        finally:
            result = rec.stop()
        return result


# ---------------------------------------------------------------------------
# recording
# ---------------------------------------------------------------------------

DEFAULT_FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"


@dataclass
class RecordResult:
    dir: Path
    frames: int
    dropped: int
    duration_ms: int
    events: list[dict]
    meta: dict

    @property
    def painted_events(self) -> list[dict]:
        """Inputs that actually caused a repaint.

        An input with painted=False landed on something inert -- the ui
        genuinely did not respond. That distinction is the most useful
        thing in the log, so it gets its own accessor.
        """
        return [e for e in self.events if e.get("event") == "input" and e.get("painted")]

    @property
    def inert_events(self) -> list[dict]:
        return [e for e in self.events if e.get("event") == "input" and not e.get("painted")]

    def latencies(self) -> list[int]:
        return [e["latency_ms"] for e in self.painted_events if e.get("latency_ms", -1) >= 0]

    def summary(self) -> str:
        lat = self.latencies()
        return (
            f"{self.frames} frames, {self.dropped} dropped, "
            f"{self.duration_ms}ms, {len(self.painted_events)} painted / "
            f"{len(self.inert_events)} inert inputs"
            + (f", latency {lat}" if lat else "")
        )


def encode_video(
    recording: "Recording | Path | str",
    out: "Path | str | None" = None,
    *,
    fmt: str = "mp4",
    highlights: bool = True,
    clock: bool = True,
    font: str = DEFAULT_FONT,
    scale: "int | None" = None,
    crf: int = 20,
    timeout: int = 300,
) -> Path:
    """Turn a recording's frames into a playable video.

    Uses the concat demuxer with a per-frame duration taken from the
    recording's real timestamps, so a 400ms pause actually shows up as
    400ms of still video instead of being flattened to a constant frame
    interval. That is the whole point -- a constant-rate encode would
    hide exactly the sluggishness you're trying to see.
    """
    rec_dir = Path(recording.dir if isinstance(recording, Recording) else recording)
    meta = json.loads((rec_dir / "meta.json").read_text())
    frame_log = meta["frame_log"]
    if not frame_log:
        raise AgentTimeout(f"recording in {rec_dir} has no frames")

    out = Path(out) if out else rec_dir.with_suffix(f".{ 'gif' if fmt=='gif' else fmt}")
    out.parent.mkdir(parents=True, exist_ok=True)

    # --- concat manifest with real per-frame durations ---
    concat = rec_dir / "frames.ffconcat"
    lines = ["ffconcat version 1.0"]
    n = len(frame_log)
    for i, entry in enumerate(frame_log):
        lines.append(f"file '{rec_dir / 'frames' / f'{i:06d}.png'}'")
        if i + 1 < n:
            dur = max(frame_log[i + 1][1] - entry[1], 1) / 1000.0
        else:
            # last frame: hold it briefly so the final state is visible
            dur = 0.1
        lines.append(f"duration {dur:.4f}")
    # concat needs the last file repeated, or the final duration is ignored
    lines.append(f"file '{rec_dir / 'frames' / f'{n - 1:06d}.png'}'")
    concat.write_text("\n".join(lines) + "\n")

    # --- filters ---
    vf: list[str] = []

    # flash a border across each click->paint window, so you can see the
    # measured latency in the video rather than having to trust the log
    if highlights:
        for e in meta.get("events", []):
            if e.get("event") != "input" or not e.get("painted"):
                continue
            start = int(e.get("frame", 0))
            end = int(e.get("paint_frame", start))
            vf.append(
                f"drawbox=x=0:y=0:w=iw:h=ih:color=orange@0.55:t=5:"
                f"enable='between(n,{start},{max(end, start)})'"
            )
            # and a marker at the input frame itself
            vf.append(
                f"drawbox=x=iw-6:y=0:w=6:h=ih:color=red@0.8:t=fill:"
                f"enable='eq(n,{start})'"
            )

    if clock and Path(font).exists():
        # NOTE the escaped colon: inside a filtergraph arg, %{pts\:hms}
        # must be written with a backslash or ffmpeg refuses the filter.
        vf.append(
            f"drawtext=fontfile={font}:text='t\\=%{{pts\\:hms}}':"
            f"x=8:y=6:fontsize=13:fontcolor=white:box=1:boxcolor=black@0.45:"
            f"boxborderw=4"
        )

    if scale:
        vf.append(f"scale={scale}:-1:flags=lanczos")

    if fmt == "gif":
        vf.append("fps=12")
        vf.append("scale=640:-1:flags=lanczos")
        vf.append("split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse")
    else:
        # yuv420p needs even dimensions; force rather than hope
        vf.append("scale=trunc(iw/2)*2:trunc(ih/2)*2,format=yuv420p")

    cmd = ["ffmpeg", "-y", "-loglevel", "error", "-f", "concat", "-safe", "0",
           "-i", str(concat)]
    if vf:
        cmd += ["-vf", ",".join(vf)]
    if fmt == "mp4":
        cmd += ["-c:v", "libx264", "-preset", "veryfast", "-crf", str(crf),
                "-movflags", "+faststart"]
    elif fmt == "webm":
        cmd += ["-c:v", "libvpx-vp9", "-b:v", "0", "-crf", str(crf)]
    elif fmt == "apng":
        cmd += ["-plays", "0"]
    cmd.append(str(out))

    r = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
    if r.returncode != 0 or not out.exists():
        raise AgentError("ffmpeg", {"error": r.stderr.strip()[:500], "cmd": " ".join(cmd)})
    return out


def ffprobe(path: "Path | str") -> dict:
    """Stream facts about an encoded video, for tests to assert on."""
    r = subprocess.run(
        ["ffprobe", "-v", "error", "-select_streams", "v:0",
         "-show_entries", "stream=nb_frames,width,height,avg_frame_rate,duration",
         "-of", "json", str(path)],
        capture_output=True, text=True, timeout=60)
    if r.returncode != 0:
        raise AgentError("ffprobe", {"error": r.stderr.strip()})
    data = json.loads(r.stdout)
    return data.get("streams", [{}])[0]


class Recording:
    """An in-app recording. Use as a context manager, then encode()."""

    def __init__(
        self,
        agent: Agent,
        path: "Path | str",
        fps: int = 15,
        max_ms: int = 0,
        cursor: bool = True,
    ):
        self.agent = agent
        self.path = Path(path)
        self.fps = fps
        self.max_ms = max_ms
        self.cursor = cursor
        self.result: RecordResult | None = None
        self.started = False

    def start(self) -> "Recording":
        r = self.agent.cmd(
            "record", sub="start", path=str(self.path), fps=self.fps,
            max_ms=self.max_ms, cursor=self.cursor,
        )
        self.started = True
        return self

    def stop(self) -> RecordResult:
        r = self.agent.cmd("record", sub="stop")
        meta = json.loads((self.path / "meta.json").read_text())
        self.result = RecordResult(
            dir=self.path,
            frames=r.get("frames", 0),
            dropped=r.get("dropped", 0),
            duration_ms=r.get("duration_ms", 0),
            events=meta.get("events", []),
            meta=meta,
        )
        return self.result

    def encode(self, out=None, **kw) -> Path:
        if self.result is None:
            self.stop()
        return encode_video(self.path, out, **kw)

    def __enter__(self) -> "Recording":
        return self.start()

    def __exit__(self, *exc) -> None:
        # always drain, even if the body raised, or the writer thread and
        # the recording directory are left dangling
        if self.started and self.result is None:
            try:
                self.stop()
            except Exception:
                pass


def x11_record(
    out: "Path | str",
    width: int,
    height: int,
    x: int = 0,
    y: int = 0,
    fps: int = 15,
    display: str | None = None,
) -> subprocess.Popen:
    """Capture the real X server with ffmpeg.

    Unlike the in-process recorder this does not perturb the app at all --
    it reads what the X server is putting on screen. The cost is that it
    needs a real display and captures whatever else happens to be visible.
    """
    cmd = [
        "ffmpeg", "-y", "-loglevel", "error",
        "-f", "x11grab",
        "-video_size", f"{width}x{height}",
        "-framerate", str(fps),
    ]
    if display:
        cmd += ["-display", display]
    else:
        cmd += ["-i", os.environ.get("DISPLAY", ":0")]
    cmd += ["-x", str(x), "-y", str(y), str(out)]
    return subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


class Phoo:
    """A running phoo under test: process + agent + artifact capture."""

    def __init__(
        self,
        binary: Path | None = None,
        profile_dir: Path | None = None,
        platform: str | None = None,
        seed: bool = True,
        env: dict | None = None,
    ):
        self.binary = Path(binary or os.environ.get(
            "PHOO_BIN", REPO / "build/Desktop_Qt_6_12_0-Release/phoo"))
        if not self.binary.exists():
            raise FileNotFoundError(
                f"phoo binary not found at {self.binary}. Build it, or set PHOO_BIN."
            )
        self._tmp_profile = profile_dir is None
        self.profile_dir = Path(profile_dir or tempfile.mkdtemp(prefix="phoo-prof-"))
        self.profile_dir.mkdir(parents=True, exist_ok=True)
        self.socket_path = str(self.profile_dir / "agent.sock")
        self.platform = platform if platform is not None else os.environ.get("PHOO_PLATFORM")
        self.seed = seed
        self.extra_env = env or {}
        self.proc: subprocess.Popen | None = None
        self.agent: Agent | None = None
        # Two buckets, because recording perturbs the very numbers we
        # measure. `latency` is always the clean one, so tests that pass
        # `phoo.latency` to click() get untainted timings even when the
        # test happens to be recording.
        self.latency_clean = LatencyReport()
        self.latency_recorded = LatencyReport()
        self.latency_recording = False
        self.artifacts: list[Path] = []
        self._screenshot_n = 0
        self._stderr_path = self.profile_dir / "phoo-stderr.log"

    @property
    def latency(self) -> LatencyReport:
        """Clean input->paint samples. Flipped to the recorded bucket
        while a recording is in progress."""
        return self.latency_recorded if self.latency_recording else self.latency_clean

    # ---- lifecycle ----

    def start(self, timeout: float = 60.0) -> "Phoo":
        # the profile directory has to come through as argv[1] (or anywhere
        # -- setup_locations reads arguments()[1]) and --test-agent gets
        # stripped out of argv by main.cpp before the app sees it.
        cmd = [str(self.binary), "--test-agent", self.socket_path]
        if not self.seed:
            cmd.append("--test-no-seed")
        cmd.append(str(self.profile_dir))

        env = dict(os.environ)
        env.setdefault("QT_LOGGING_RULES", "qt.qml.binding.removal.info=false")
        if self.platform:
            env["QT_QPA_PLATFORM"] = self.platform
        env.update(self.extra_env)

        self._errfh = open(self._stderr_path, "wb")
        self.proc = subprocess.Popen(
            cmd, env=env, stdout=self._errfh, stderr=subprocess.STDOUT
        )
        self.agent = Agent(self.socket_path)
        self.agent.connect(deadline_s=timeout)
        self.agent.wait_alive(timeout=timeout)
        return self

    def stop(self, hard: bool = False) -> None:
        """Shut the app down.

        Deliberately does not click the window close button: main.qml's
        onClosing starts a 3000ms "press again to exit" animation on the
        first close, so clicking it would leave the app running. `quit`
        goes through QCoreApplication::quit(), which bypasses onClosing.
        """
        if self.agent:
            self.agent.quit()
        if self.proc:
            try:
                self.proc.wait(timeout=10 if not hard else 2)
            except subprocess.TimeoutExpired:
                self.proc.kill()
                self.proc.wait(timeout=5)
            self.proc = None
        if self.agent:
            self.agent.close()
            self.agent = None
        try:
            self._errfh.close()
        except Exception:
            pass

    def __enter__(self) -> "Phoo":
        return self.start()

    def __exit__(self, *exc) -> None:
        self.stop()

    # ---- diagnostics ----

    def screenshot(self, label: str = "shot") -> Path:
        assert self.agent
        self._screenshot_n += 1
        out = self.profile_dir / f"{self._screenshot_n:03d}-{label}.png"
        out.write_bytes(self.agent.grab())
        self.artifacts.append(out)
        return out

    def diagnostics(self) -> dict:
        """Everything you want in a failure message."""
        d: dict[str, Any] = {
            "profile_dir": str(self.profile_dir),
            "stderr": str(self._stderr_path),
            "latency_clean": self.latency_clean.format(),
        }
        if self.latency_recorded.samples:
            d["latency_under_recording"] = self.latency_recorded.format()
        if self.agent:
            try:
                d["ping"] = vars(self.agent.ping())
            except Exception as e:
                d["ping_error"] = str(e)
            try:
                d["events"] = self.agent.drain_events()
            except Exception:
                pass
        if self.proc and self.proc.poll() is not None:
            d["exit_code"] = self.proc.returncode
        try:
            tail = self._stderr_path.read_text(errors="replace").splitlines()[-60:]
            if tail:
                d["stderr_tail"] = tail
        except Exception:
            pass
        return d
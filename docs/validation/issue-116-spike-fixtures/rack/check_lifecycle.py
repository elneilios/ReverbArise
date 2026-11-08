"""Analyze the separately observed high-gate, reset and power actions."""
import argparse
import json
from collections import Counter
from pathlib import Path

from check_timing import PERIOD, read_trace


def check(path, actions):
    rows = read_trace(path)
    capture = [row for row in rows if row["stream"] == "capture"]
    startup = capture[0]
    assert startup["value"] == 1 and startup["sample"] < startup["frames"]
    left = [row for row in rows if row["stream"] == "left"]
    assert any(row["value"] == 1 and row["sample"] == startup["sample"] for row in left)

    reset_time = actions["reset"]
    before_reset = [row for row in rows if row["wallMs"] < reset_time - 200]
    duplicated = Counter((row["sample"], row["value"]) for row in before_reset if row["stream"] == "cue")
    cue_reference = Counter((row["sample"], row["value"]) for row in before_reset
                            if row["stream"] == "right" and row["sample"] > 0)
    assert duplicated == cue_reference and duplicated and set(duplicated.values()) == {1}

    phrase = [row for row in rows if row["stream"] == "phrase" and row["value"] == 1]
    pre_reset = max(row["sample"] for row in phrase if row["wallMs"] < reset_time)
    post_reset = min(row["sample"] for row in phrase if row["wallMs"] >= reset_time)
    assert post_reset - pre_reset != PERIOD

    removed = next(row for row in capture if row["value"] == 0
                   and actions["remove"] <= row["wallMs"] < actions["remove"] + 1000)
    assert [row for row in left if row["sample"] <= removed["sample"]][-1]["value"] == 1

    off, on = actions["suspend"], actions["resume"]
    heartbeats = [row for row in rows if row["stream"] == "callback"]
    assert not [row for row in heartbeats if off + 1500 < row["wallMs"] < on - 500]
    previous = [row for row in heartbeats if row["wallMs"] < off + 1500][-1]
    resumed = next(row for row in heartbeats if row["wallMs"] >= on)
    assert resumed["sample"] - previous["sample"] <= 12288
    assert resumed["wallMs"] - previous["wallMs"] > 1000
    life = [row for row in rows if row["stream"] in ("prepare", "release", "reset", "bypass")]
    assert all(row["sample"] == 0 for row in life), life
    phrase_before = max(row["sample"] for row in phrase if row["wallMs"] < off + 1500)
    phrase_after = min(row["sample"] for row in phrase if row["wallMs"] >= on)
    assert phrase_after - phrase_before != PERIOD

    return {
        "trace": path.name, "startup_held_note_on_sample": startup["sample"],
        "duplicate_cue_edges_delivered_once": sum(duplicated.values()),
        "patch_reset_interval_frames": post_reset - pre_reset,
        "capture_note_off_after_unplug_sample": removed["sample"], "dry_gate_still_high": True,
        "suspension_heartbeat_wall_gap_ms": resumed["wallMs"] - previous["wallMs"],
        "suspension_heartbeat_frame_gap": resumed["sample"] - previous["sample"],
        "suspension_phrase_interval_frames": phrase_after - phrase_before,
        "processor_lifecycle_events_after_start": len([row for row in life if row["sample"] > 0]),
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("trace", type=Path)
    parser.add_argument("actions", type=Path)
    args = parser.parse_args()
    print(json.dumps(check(args.trace, json.loads(args.actions.read_text(encoding="utf-8"))), indent=2))

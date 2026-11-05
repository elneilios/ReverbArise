"""Read-only CSV summaries. Markers bracket user actions, not exact mute timestamps."""
import collections
import csv
import json
import pathlib
import sys


def analyse(path):
    stages = collections.defaultdict(lambda: {"events": collections.Counter(), "callbacks": 0,
        "input_peak_max": 0.0, "output_peak_max": 0.0, "max_callback_gap_ms": 0.0,
        "input_enabled": set(), "output_enabled": set(), "playing": set(), "offline": set()})
    dropped = 0
    previous = None
    with path.open(newline="", encoding="utf-8-sig") as stream:
        for row in csv.DictReader(stream):
            stage = stages[int(row["marker"])]
            kind = row["event"]
            stage["events"][kind] += 1
            dropped = max(dropped, int(row["dropped_total_at_flush"]))
            if kind in ("audio", "bypass"):
                stage["callbacks"] += 1
                for key in ("input_enabled", "output_enabled", "playing", "offline"):
                    stage[key].add(int(row[key]))
                stage["input_peak_max"] = max(stage["input_peak_max"], float(row["input_peak"]))
                stage["output_peak_max"] = max(stage["output_peak_max"], float(row["output_peak"]))
                stamp = float(row["qpc_seconds"])
                if previous is not None:
                    stage["max_callback_gap_ms"] = max(stage["max_callback_gap_ms"], (stamp - previous) * 1000)
                previous = stamp
    for stage in stages.values():
        for key, value in stage.items():
            if isinstance(value, set):
                stage[key] = sorted(value)
    return {"file": str(path), "dropped_rows": dropped,
        "no_reported_queue_loss": dropped == 0, "stages": dict(stages)}


if __name__ == "__main__":
    root = pathlib.Path(sys.argv[1])
    paths = [root] if root.is_file() else sorted(root.glob("host-*.csv"))
    if not paths:
        raise SystemExit("No host CSV found; no host result claimed")
    print(json.dumps({"interpretation": "Observed events only; a correlation is not a mute contract. "
        "Gaps include stopped/disabled scheduling; marker clicks are not exact toggle timestamps.",
        "instances": [analyse(p) for p in paths]}, indent=2))

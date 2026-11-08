"""Check actual Host-FX trace events against voltage references and fixture schedule."""
import argparse
import csv
import json
from collections import Counter
from pathlib import Path

PERIOD = 49153
CYCLES = 32
CONTROL = ("capture", "phrase", "cue", "stop")
SCHEDULE = {
    "phrase": [(0, 1), (48, 0)],
    "cue": [(97, 1), (98, 0), (193, 1), (241, 0), (4099, 1), (4147, 0),
            (PERIOD - 1024, 1), (PERIOD - 976, 0)],
    "stop": [(4099, 1), (4147, 0), (8197, 1), (8198, 0)],
}


def read_trace(path):
    with path.open(newline="", encoding="utf-8") as handle:
        lines = handle.readlines()
    assert not any(line.startswith("# dropped") for line in lines), path
    return [
        {key: value if key == "stream" else int(value) for key, value in row.items()}
        for row in csv.DictReader(line for line in lines if not line.startswith("#"))
    ]


def pulse_rows(rows, stream, start, end):
    return [row for row in rows if row["stream"] == stream and start <= row["sample"] < end]


def edges(rows, stream, start, end):
    return Counter((row["sample"], row["value"]) for row in pulse_rows(rows, stream, start, end))


def find_stable_window(rows):
    starts = sorted({row["sample"] for row in rows if row["stream"] == "phrase" and row["value"] == 1})
    for i in range(1, len(starts) - CYCLES):
        start = starts[i]
        if starts[i:i + CYCLES + 1] == [start + k * PERIOD for k in range(CYCLES + 1)]:
            return start, start + CYCLES * PERIOD
    raise AssertionError(f"Missing {CYCLES} stable intervals. Phrase starts: {starts[:10]}")


def analyze(path):
    rows = read_trace(path)
    start, end = find_stable_window(rows)
    observed = {stream: edges(rows, stream, start, end) for stream in CONTROL}
    for stream, pattern in SCHEDULE.items():
        expected = Counter((start + cycle * PERIOD + offset, value)
                           for cycle in range(CYCLES) for offset, value in pattern)
        assert expected == observed[stream], (path.name, stream,
                                               expected - observed[stream], observed[stream] - expected)
    expected_capture = [(start + cycle * PERIOD, value) for cycle in range(CYCLES)
                        for value in [((cycle + observed["capture"][(start, 1)]) & 1)]]
    assert Counter(expected_capture) == observed["capture"], (path.name, "capture length")
    left = edges(rows, "left", start, end)
    right = edges(rows, "right", start, end)
    pairs = [("capture", "phrase"), ("cue", "stop")]
    matches = [pair for pair in pairs if left == observed[pair[0]] and right == observed[pair[1]]]
    assert len(matches) == 1, (path.name, "MIDI and dry audio edges differ", len(left), len(right),
                               {key: len(value) for key, value in observed.items()})
    commands = [row for row in rows if row["stream"] in CONTROL and start <= row["sample"] < end]
    assert {row["rate"] for row in commands} == {48000}
    frames, = {row["frames"] for row in commands}
    assert all(0 <= row["sample"] - row["blockStart"] < frames for row in commands)
    assert {row["rate"] for row in rows if row["stream"] == "callback" and start <= row["sample"] < end} == {48000}
    return {
        "trace": path.name, "callback_frames": frames, "reference_pair": list(matches[0]),
        "start_sample": start, "end_exclusive": end, "period_frames": PERIOD, "cycles": CYCLES,
        "gate_high_frames": PERIOD, "matching_reference_edges": sum(left.values()) + sum(right.values()),
        "command_counts": {stream: sum(events.values()) for stream, events in observed.items()},
        "simultaneous_cue_stop_rises": CYCLES, "single_frame_cues": CYCLES, "single_frame_stops": CYCLES,
        "distinct_phrase_callback_offsets": len({row["sample"] - row["blockStart"]
                                                 for row in commands if row["stream"] == "phrase" and row["value"]}),
        "max_abs_reference_offset_frames": 0,
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("traces", nargs="+", type=Path)
    args = parser.parse_args()
    results = [analyze(path) for path in args.traces]
    assert {(result["callback_frames"], tuple(result["reference_pair"])) for result in results} == {
        (size, pair) for size in (128, 256) for pair in (("capture", "phrase"), ("cue", "stop"))
    }
    print(json.dumps(results, indent=2))


if __name__ == "__main__":
    main()

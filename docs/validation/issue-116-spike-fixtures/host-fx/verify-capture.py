"""Verify a probe WAV against its Rack MIDI edge trace and optional binary input."""

import argparse
import csv
import json
import math
import struct
from pathlib import Path


def read_wav(path):
    raw = path.read_bytes()
    assert raw[:4] == b"RIFF" and raw[8:12] == b"WAVE", "Expected RIFF WAVE"
    position = 12
    chunks = {}
    while position + 8 <= len(raw):
        name, size = struct.unpack_from("<4sI", raw, position)
        position += 8
        assert position + size <= len(raw), "Truncated WAV chunk"
        chunks[name] = raw[position:position + size]
        position += size + size % 2
    fmt = chunks[b"fmt "]
    encoding, channels, rate, _, frame_size, bits = struct.unpack_from("<HHIIHH", fmt)
    if encoding == 0xfffe:
        encoding, = struct.unpack_from("<I", fmt, 24)
    assert (encoding, channels, frame_size, bits) == (3, 2, 8, 32), "Expected stereo float32 WAV"
    payload = chunks[b"data"]
    assert len(payload) % frame_size == 0, "Incomplete WAV frame"
    return rate, payload


def verify(path, binary_reference):
    trace_id, start_text = path.stem.removeprefix("take-").split("-", 1)
    start = int(start_text)
    trace_path = path.with_name(f"trace-{trace_id}.csv")
    with trace_path.open(newline="", encoding="utf-8") as trace:
        rows = list(csv.DictReader(line for line in trace if not line.startswith("#")))
    rate, payload = read_wav(path)
    frames = len(payload) // 8
    end = start + frames
    start_row, = [row for row in rows if row["stream"] == "take_start" and int(row["sample"]) == start]
    end_row, = [row for row in rows if row["stream"] == "take_end" and int(row["sample"]) == end]
    assert int(start_row["sampleRate"]) == int(end_row["sampleRate"]) == rate
    for row, level in [(start_row, "1"), (end_row, "0")]:
        assert any(event["stream"] == "gate_midi" and event["sample"] == row["sample"]
                   and event["high"] == level for event in rows), "Take boundary disagrees with MIDI gate"

    channels = [[], []]
    if binary_reference:
        for name, changes in zip(("gate_audio", "pulse_audio"), channels):
            changes.extend((int(row["sample"]), float(row["value"]))
                           for row in rows if row["stream"] == name)
            assert changes and changes[0][0] <= start, "Missing reference baseline"
    indices = [0, 0]
    peaks = [0.0, 0.0]
    energies = [0.0, 0.0]
    stereo_differs = False
    for offset, frame in enumerate(struct.iter_unpack("<ff", payload)):
        stereo_differs |= frame[0] != frame[1]
        for channel, value in enumerate(frame):
            assert math.isfinite(value), "Non-finite audio sample"
            peaks[channel] = max(peaks[channel], abs(value))
            energies[channel] += value * value
            if binary_reference:
                changes = channels[channel]
                while indices[channel] + 1 < len(changes) and changes[indices[channel] + 1][0] <= start + offset:
                    indices[channel] += 1
                expected = changes[indices[channel]][1]
                assert value == expected, f"Channel {channel + 1}, frame {offset}: {value} != {expected}"
    assert all(peak > 0 for peak in peaks) and stereo_differs, "Missing or duplicated input"
    return {
        "wav": path.name, "trace": trace_path.name, "rate": rate, "frames": frames,
        "start": start, "end_exclusive": end,
        "start_callback_offset": start - int(start_row["blockStart"]),
        "end_callback_offset": end - int(end_row["blockStart"]),
        "callback_size": int(start_row["blockSize"]),
        "peak": peaks, "rms": [math.sqrt(energy / frames) for energy in energies],
        "all_stereo_samples_match_reference": True if binary_reference else None,
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wav", type=Path)
    parser.add_argument("--binary-reference", action="store_true")
    args = parser.parse_args()
    print(json.dumps(verify(args.wav, args.binary_reference), indent=2))

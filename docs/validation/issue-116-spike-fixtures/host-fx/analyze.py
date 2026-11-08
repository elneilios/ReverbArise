"""Summarize callback-level CV edges against the same signal on the audio input."""

import argparse
import csv
import json
import statistics
from pathlib import Path


def read_trace(path):
    rows = []
    warnings = []
    with path.open(encoding="utf-8", newline="") as stream:
        reader = csv.reader(stream)
        if next(reader, None) != [
            "sample", "blockStart", "blockSize", "sampleRate", "stream", "high", "value"
        ]:
            raise ValueError(f"{path}: unexpected trace header")
        for row in reader:
            if row and row[0].startswith("#"):
                warnings.append(row[0])
            elif len(row) == 7:
                sample, start, block, rate, name, high, value = row
                rows.append((int(sample), int(start), int(block), int(rate),
                             name, bool(int(high)), float(value)))
            elif row:
                warnings.append("incomplete row, possibly being written")
    return rows, warnings


def summarize_edges(audio, parameter, tolerance, first_sample, last_sample):
    # The first level of each stream is an initial observation, not a transition.
    audio, parameter = audio[1:], parameter[1:]
    if not audio:
        return None
    trusted_start = first_sample + tolerance
    trusted_end = last_sample - tolerance
    references = [(sample, high) for sample, high in audio
                  if trusted_start <= sample <= trusted_end]
    unused = set(range(len(parameter)))
    offsets = []
    matched_offsets = {}
    missed = 0
    for reference, high in references:
        candidates = [
            (abs(candidate - reference), index)
            for index in unused
            for candidate, level in [parameter[index]]
            if level == high and abs(candidate - reference) <= tolerance
        ]
        if candidates:
            _, index = min(candidates)
            unused.remove(index)
            offsets.append(parameter[index][0] - reference)
            matched_offsets[reference] = parameter[index][0] - reference
        else:
            missed += 1
    widths = [end - start for (start, rise), (end, fall) in zip(audio, audio[1:])
              if rise and not fall and trusted_start <= start <= trusted_end
              and end <= trusted_end]
    width_errors = [
        matched_offsets[end] - matched_offsets[start]
        for (start, rise), (end, fall) in zip(audio, audio[1:])
        if rise and not fall and start in matched_offsets and end in matched_offsets
    ]
    return {
        "reference_edges": len(references),
        "matched": len(offsets),
        "unmatched_reference_edges": missed,
        "unmatched_parameter_edges": sum(
            trusted_start <= parameter[index][0] <= trusted_end for index in unused
        ),
        "parameter_minus_audio_samples": (
            [min(offsets), statistics.median(offsets), max(offsets)] if offsets else None
        ),
        "reference_high_width_samples": (
            [min(widths), statistics.median(widths), max(widths)] if widths else None
        ),
        "observed_high_width_error_samples": (
            [min(width_errors), statistics.median(width_errors), max(width_errors)]
            if width_errors else None
        ),
    }


def summarize(rows):
    sections = []
    current = None
    for sample, start, block, rate, name, high, _ in rows:
        if name == "callback":
            if current is not None:
                sections.append(current)
            current = {
                "sample_rate": rate, "block_size": block, "first": start,
                "last": start, "streams": {},
            }
        elif current is not None:
            current["streams"].setdefault(name, []).append((sample, high))
            current["last"] = max(current["last"], sample)
    if current is not None:
        sections.append(current)

    result = []
    for section in sections:
        block = section["block_size"]
        streams = section["streams"]
        result.append({
            "sample_rate": section["sample_rate"],
            "block_size": block,
            "first_sample": section["first"],
            "last_event_sample": section["last"],
            "matching_window_samples": block * 2,
            **{
                kind: summarize_edges(
                    streams.get(f"{kind}_audio", []),
                    streams.get(f"{kind}_midi", streams.get(f"{kind}_parameter", [])),
                    block * 2, section["first"], section["last"],
                )
                for kind in ("gate", "pulse")
            },
            "pulse_route": "midi_note" if "pulse_midi" in streams else "parameter",
            "gate_route": "midi_note" if "gate_midi" in streams else "parameter",
        })
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", nargs="+", type=Path)
    args = parser.parse_args()
    results = {}
    for path in args.trace:
        rows, warnings = read_trace(path)
        results[str(path)] = {"warnings": warnings, "sections": summarize(rows)}
    print(json.dumps(results, indent=2))

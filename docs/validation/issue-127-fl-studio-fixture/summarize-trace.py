"""Summarize complete FL Studio VST3 callback traces for the Loop fixture."""

import argparse
import csv
from collections import Counter
from pathlib import Path


def int_field(row, key):
    return int(row[key]) if row[key] else None


def summarize(path):
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines or not lines[-1].startswith("# dropped="):
        raise ValueError("Close the traced plug-in instance to finalize the CSV.")
    footer = dict(field.split("=") for field in lines[-1][2:].split(","))
    if int(footer["dropped"]) or int(footer["midi_overflow"]):
        raise ValueError(f"Incomplete trace: {lines[-1]}")

    rows = list(csv.DictReader(lines[:-1]))
    callbacks = [row for row in rows if int(row["midi_size"]) == 0]
    notes = [row for row in rows if (int(row["status"]) & 0xF0) in (0x80, 0x90)]
    if not callbacks:
        raise ValueError("No audio callbacks recorded.")

    print("Callbacks by offline and playing flags:")
    for offline, playing in sorted({(row["offline"], row["playing"]) for row in callbacks}):
        sizes = [int(row["frames"]) for row in callbacks
                 if (row["offline"], row["playing"]) == (offline, playing)]
        print(f"  offline={offline} playing={playing}: {len(sizes)} callbacks, "
              f"{min(sizes)} to {max(sizes)} frames")

    print("Host timing omissions in playing callbacks:")
    for key in ("sample", "ppq", "bpm", "numerator", "denominator"):
        missing = sum(row["playing"] == "1" and not row[key] for row in callbacks)
        print(f"  {key}: {missing}")

    print("Playing sample discontinuities:")
    for previous, current in zip(callbacks, callbacks[1:]):
        before = int_field(previous, "sample")
        after = int_field(current, "sample")
        if before is None or after is None:
            continue
        delta = after - before - int(previous["frames"])
        if previous["playing"] == current["playing"] == "1" and delta != 0:
            print(f"  block {current['block']}: {before} -> {after}, deviation {delta}")

    print("Notes and releases:")
    duplicates = Counter()
    for row in notes:
        status = int(row["status"])
        positive = (status & 0xF0) == 0x90 and int(row["data2"]) > 0
        offset = int(row["midi_offset"])
        sample = int_field(row, "sample")
        absolute = None if sample is None else sample + offset
        print(f"  offline={row['offline']} block={row['block']} offset={offset} sample={absolute} "
              f"ch={(status & 15) + 1} note={row['data1']} {'on' if positive else 'off'}")
        duplicates[(row["block"], offset, status, row["data1"], row["data2"])] += 1
    print("Identical events in one callback:", sum(count - 1 for count in duplicates.values()))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    summarize(parser.parse_args().trace)

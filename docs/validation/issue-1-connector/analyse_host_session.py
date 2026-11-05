"""Read-only streaming analysis of the native spike's CSV and PCM16 WAV evidence.

Receipt fields are not mouse-click latency or an atomic timestamp protocol.
Use snapshots: the host may still append to the original callback logs.
"""
import argparse
import csv
import hashlib
import json
import math
import statistics
import struct
import wave
from collections import Counter
from pathlib import Path


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(65536), b""):
            digest.update(block)
    return digest.hexdigest()


def stats(values):
    if not values:
        return None
    values = sorted(values)
    return {"n": len(values), "min_ms": values[0], "median_ms": statistics.median(values),
            "p95_ms": values[math.ceil(.95 * len(values)) - 1], "max_ms": values[-1]}


def observation(row):
    return {"block": int(row["block"]), "qpc_seconds": float(row["qpc_seconds"]),
            "sequence": int(row["sequence"]), "mode": int(row["mode"]),
            "fresh": bool(int(row["fresh"])), "playing": bool(int(row["playing"])),
            "frames": int(row["frames"]), "rate": float(row["rate"]),
            "host_sample": int(row["host_sample"]),
            "query_age_ms": float(row["query_to_callback_ms"]),
            "write_age_ms": float(row["write_to_callback_ms"])}


def receiver(path):
    count = malformed = fresh = changed = 0
    last_packet = None
    query_ages, write_ages = [], []
    state_counts = Counter()
    passes = []
    current = None
    last = None
    with Path(path).open(newline="") as source:
        for row in csv.DictReader(source):
            if any(row.get(k) in (None, "") for k in ("packet", "mode", "fresh", "offline", "probe_signal_peak")):
                malformed += 1
                continue
            count += 1
            fresh += int(row["fresh"])
            state_counts[(row["mode"], row["fresh"], row["playing"], row["offline"])] += 1
            last = observation(row)
            packet = int(row["packet"])
            if packet and packet != last_packet:
                changed += 1
                q, w = float(row["query_to_callback_ms"]), float(row["write_to_callback_ms"])
                # Exclude stale/setup samples from realtime receipt-field statistics.
                if row["fresh"] == "1" and row["offline"] == "0" and 0 <= w <= q + .125 and q < 750:
                    query_ages.append(q)
                    write_ages.append(w)
                last_packet = packet
            if row["offline"] == "1":
                if current is None:
                    current = {"first": observation(row), "last": None, "callbacks": 0,
                               "processed_frames": 0, "new_packets": 0, "states": Counter(),
                               "max_input_peak": 0, "max_output_peak": 0, "max_signal_peak": 0,
                               "first_packet": packet, "last_packet": packet}
                    passes.append(current)
                current["callbacks"] += 1
                current["processed_frames"] += int(row["frames"])
                current["states"][(row["mode"], row["fresh"], row["tone_enabled"], row["playing"])] += 1
                current["new_packets"] += packet != current["last_packet"]
                current["last_packet"] = packet
                current["last"] = observation(row)
                for key, column in (("max_input_peak", "input_peak"), ("max_output_peak", "output_peak"), ("max_signal_peak", "probe_signal_peak")):
                    current[key] = max(current[key], float(row[column]))
            else:
                current = None
    for item in passes:
        item["wall_duration_seconds"] = item["last"]["qpc_seconds"] - item["first"]["qpc_seconds"]
        item["states"] = [{"mode": int(k[0]), "fresh": bool(int(k[1])),
                           "tone_enabled": bool(int(k[2])), "playing": bool(int(k[3])),
                           "callbacks": n} for k, n in sorted(item["states"].items())]
    return {"file": Path(path).name, "sha256": sha256(path), "callbacks": count,
            "malformed_rows": malformed, "fresh_callbacks": fresh,
            "changed_packet_receipts": changed, "latest": last,
            "realtime_first_receipt_query_age": stats(query_ages),
            "realtime_first_receipt_write_age": stats(write_ages),
            "states": [{"mode": int(k[0]), "fresh": bool(int(k[1])),
                        "playing": bool(int(k[2])), "offline": bool(int(k[3])),
                        "callbacks": n} for k, n in sorted(state_counts.items())],
            "offline_passes": passes}


def pcm16(path):
    peak = square_sum = nonzero = samples = 0
    with wave.open(str(path), "rb") as source:
        if source.getsampwidth() != 2 or source.getcomptype() != "NONE":
            raise ValueError("Expected uncompressed 16-bit PCM; no implicit conversion")
        channels, rate, frames = source.getnchannels(), source.getframerate(), source.getnframes()
        while True:
            block = source.readframes(4096)
            if not block:
                break
            values = struct.unpack("<%dh" % (len(block) // 2), block)
            samples += len(values)
            peak = max(peak, max(map(abs, values), default=0))
            square_sum += sum(value * value for value in values)
            nonzero += sum(value != 0 for value in values)
    if samples != channels * frames:
        raise ValueError("Incomplete WAV data")
    return {"file": Path(path).name, "sha256": sha256(path), "channels": channels,
            "bits": 16, "sample_rate": rate, "frames": frames,
            "duration_seconds": frames / rate, "peak": peak / 32768,
            "rms": math.sqrt(square_sum / max(1, samples)) / 32768,
            "nonzero_sample_values": nonzero, "digital_silence": nonzero == 0}


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("receiver_directory", type=Path)
    parser.add_argument("--wav", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    paths = sorted(args.receiver_directory.glob("receiver-*.csv"))
    if not paths:
        raise SystemExit("No native host CSV evidence")
    report = {"date": "2026-11-05", "wav": pcm16(args.wav),
              "receivers": [receiver(path) for path in paths],
              "limitations": ["Owner listening checks are distinct from PCM evidence.",
                  "Receipt ages require timestamp coherence; no mouse-click or zero-leakage guarantee.",
                  "No fresh packet consumed does not distinguish absent script callbacks from host delivery suspension.",
                  "Native test logs are excluded by filename; only host receiver-*.csv files are analysed."]}
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"wav": report["wav"], "receivers": [
        {"file": item["file"], "callbacks": item["callbacks"],
         "receipt_write_age": item["realtime_first_receipt_write_age"],
         "offline_passes": item["offline_passes"]} for item in report["receivers"]]}, indent=2))

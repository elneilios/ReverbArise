"""Summarise native evidence; no host/config/asset mutation. Empty data is not success."""
import argparse
import csv
import json
from collections import Counter
from pathlib import Path


def summary(directory):
    reports = []
    for path in sorted(Path(directory).glob("receiver-*.csv")):
        with path.open(newline="") as source:
            rows = list(csv.DictReader(source))
        seen = set()
        consumed = []
        for row in rows:
            packet = int(row["packet"])
            if packet and (not seen or packet != consumed[-1]["packet"]):
                consumed.append({"packet": packet, "query_ms": float(row["query_to_callback_ms"]),
                                 "write_ms": float(row["write_to_callback_ms"])})
                seen.add(packet)
        writes = sorted(r["write_ms"] for r in consumed if 0 <= r["write_ms"] < 2000)
        queries = sorted(r["query_ms"] for r in consumed if 0 <= r["query_ms"] < 2000)
        def stats(values):
            return None if not values else {"n": len(values), "min": values[0],
                "median": values[len(values)//2], "p95": values[int((len(values)-1)*.95)], "max": values[-1]}
        reports.append({"file": str(path), "callback_rows": len(rows), "consumptions": len(consumed),
                        "modes": dict(Counter(row["mode"] for row in rows)),
                        "offline_rows": sum(row["offline"] == "1" for row in rows),
                        "stale_rows": sum(row["fresh"] == "0" for row in rows),
                        "max_signal_peak": max((float(row["probe_signal_peak"]) for row in rows), default=0),
                        "query_to_callback_ms": stats(queries), "write_to_callback_ms": stats(writes),
                        "excluded_clock_rows": len(consumed)-len(writes)})
    return reports


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("receiver_directory")
    parser.add_argument("--script-directory")
    args = parser.parse_args()
    result = {"receivers": summary(args.receiver_directory),
              "warning": "Native-test rows are not FL evidence. Timestamp ordering/coherence and script clock must be checked; no zero-leakage claim."}
    if args.script_directory:
        events, flags = Counter(), Counter()
        for path in Path(args.script_directory).glob("script-*.csv"):
            with path.open(newline="") as source:
                for row in csv.DictReader(source):
                    events[row["event"]] += 1
                    if row["event"] == "refresh":
                        flags[row["flags"]] += 1
        result["script_events"], result["refresh_flags"] = dict(events), dict(flags)
    print(json.dumps(result, indent=2))
    if not result["receivers"]:
        raise SystemExit("NO RECEIVER EVIDENCE: delivery has not been established")

"""Reanalyse retained production measurements; does not run the current DSP."""
import hashlib
import json
import math
import pathlib
import sys

EXPECTED = "050b931fa9f4698568a9fc4db256fca3ba98e072aefce36619a7352bb7ceeb34"


def analyse(path):
    blob = pathlib.Path(path).read_bytes()
    assert hashlib.sha256(blob).hexdigest() == EXPECTED, "Unexpected listening evidence"
    report = json.loads(blob)
    selected = {"preverb-hall", "monumental", "whisper", "imported-chamber",
                "imported-chamber-hot", "imported-sparse", "imported-chamber-quiet"}
    rows = []
    for response in report["responses"]:
        if response["id"] not in selected:
            continue
        for fixture in response["fixtures"]:
            if fixture["name"] not in {"quiet chord", "strong chord"}:
                continue
            candidate = next(c for c in fixture["candidates"] if c["candidate"] == "energy-plus-6")
            # Historical default_wet_peak is before calibration/protection.
            raw_peak = candidate["default_wet_peak_dbfs"]
            unprotected = raw_peak + candidate["baseline_db"]
            relative = candidate["wet_to_dry_energy_db"] - candidate["wet_protection_db"]
            rows.append({"ir": response["id"], "source": fixture["name"],
                         "source_pcm_sha256": fixture["source_pcm_sha256"],
                         "historical_measurement": candidate,
                         "derived": {
                             "musical_peak_at_minus12_dbfs": unprotected - 12,
                             "musical_guard_at_minus12_db": min(0, -1 - (unprotected - 12)),
                             "musical_attack_relative_at_minus12_db": relative - 12,
                             "musical_attack_relative_at_minus24_db": relative - 24,
                             "legacy_peak_at_minus12_before_guard_dbfs": raw_peak - 12,
                             "legacy_guard_at_minus12_db": min(0, -6 - (raw_peak - 12))}})
    assert len(rows) == 14
    assert all(math.isfinite(v) for r in rows for v in r["derived"].values())
    return {"analysis_date": "2026-11-06", "source_metrics_sha256": EXPECTED,
            "source_evidence": "gain-staging-evidence.md#source-energy-extension--2026-11-02",
            "method": "Analytical Level offsets on retained 2026-11-02 production measurements; no new DSP run or listening approval",
            "rows": rows}


if __name__ == "__main__":
    result = analyse(sys.argv[1])
    pathlib.Path(sys.argv[2]).write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"Verified source hash; wrote {len(result['rows'])} analytical rows")

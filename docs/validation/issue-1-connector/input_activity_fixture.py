"""Explore-only causal activity-gate counterexamples. Synthetic samples, not FL/JUCE DSP.

Run with Python 3.10+; no dependencies, assets, installations or production state.
The 1 kHz fixture rate gives one frame per millisecond for exact counts.
"""
import json

RATE = 1000


def source(length, notes, amplitude=.2, floor=0.0):
    data = [floor] * length
    for first, end in notes:
        data[first:end] = [amplitude] * (end-first)
    return data


def activity_gate(data, threshold=1e-4, hold_ms=0, startup_ms=0):
    deadline = startup_ms
    result = []
    for index, sample in enumerate(data):
        if abs(sample) > threshold:
            deadline = max(deadline, index + hold_ms + 1)
        result.append(index < deadline)
    return result


def admitted_fraction(gate, first, end):
    return sum(gate[first:end]) / (end-first)


def run():
    length = 14000
    enabled = source(length, [(4000, 4100)])
    muted = source(length, [])
    # Actual host buffers before the future attack can be identical in both cases.
    assert enabled[:4000] == muted[:4000]
    policies = []
    for hold in (0, 250, 5000):
        good = activity_gate(enabled, hold_ms=hold)
        off = activity_gate(muted, hold_ms=hold)
        assert good[2000:4000] == off[2000:4000]
        assert admitted_fraction(good, 2000, 4000) == 0
        assert admitted_fraction(off, 2000, 4000) == 0
        policies.append({"hold_ms": hold,
                         "legitimate_first_lead_in_admitted_fraction": admitted_fraction(good, 2000, 4000),
                         "muted_first_lead_in_admitted_fraction": admitted_fraction(off, 2000, 4000),
                         "legitimate_post_arrival_tail_admitted_fraction": admitted_fraction(good, 4000, 4500)})
    grace_good = activity_gate(enabled, hold_ms=250, startup_ms=5000)
    grace_off = activity_gate(muted, hold_ms=250, startup_ms=5000)
    assert admitted_fraction(grace_good, 2000, 4000) == 1
    assert admitted_fraction(grace_off, 2000, 4000) == 1
    late = source(length, [(12000, 12100)])
    grace_late = activity_gate(late, hold_ms=250, startup_ms=5000)
    assert admitted_fraction(grace_late, 10000, 12000) == 0
    sparse = source(length, [(1000, 1100), (12000, 12100)])
    sparse_gate = activity_gate(sparse, hold_ms=5000)
    assert admitted_fraction(sparse_gate, 10000, 12000) == 0
    # A long hold preserves the sparse lead-in but cannot promptly follow a mute.
    long_hold = activity_gate(source(length, [(1000, 1100)]), hold_ms=11000)
    assert admitted_fraction(long_hold, 10000, 12000) == 1
    quiet = activity_gate(source(length, [(4000, 4100)], amplitude=5e-5), hold_ms=250)
    assert not any(quiet)
    noise = activity_gate(source(length, [], floor=1e-7), threshold=0, hold_ms=0)
    assert all(noise)
    # Missing expected dry can be recognised at the expected onset, after the
    # associated reverse lead-in has already passed. Stored dry is the same in
    # both cases, so consulting it alone does not disclose an upstream mute.
    assert next(i for i, (a, b) in enumerate(zip(enabled, muted)) if a != b) == 4000
    return {"date": "2026-11-05", "sample_rate": RATE,
            "evidence_kind": "synthetic activity policy fixture; no FL/JUCE/native callback execution",
            "first_dry_note_ms": 4000, "prepared_lead_in_ms": [2000, 4000],
            "identical_enabled_and_muted_input_prefix_frames": 4000,
            "policies": policies,
            "startup_grace_ms": 5000,
            "startup_grace_enabled_lead_in_fraction": 1,
            "startup_grace_muted_leak_fraction": 1,
            "startup_grace_late_first_note_lead_in_fraction": 0,
            "sparse_second_lead_in_fraction_with_5000ms_hold": 0,
            "muted_future_lead_in_fraction_with_11000ms_hold": 1,
            "quiet_note_detected_at_minus80dBFS_threshold": any(quiet),
            "nonzero_noise_keeps_exact_zero_detector_open": all(noise),
            "first_possible_expected_source_mismatch_ms": 4000,
            "result": "PASS: counterexamples reproduced; no mute-identification claim"}


if __name__ == "__main__":
    print(json.dumps(run(), indent=2))

"""Independently compare the live VST3's dry input and mixed output."""
import argparse
import json
import math
import struct
from pathlib import Path

P, LEAD, FADE = 49153, 2048, 240
MARK, CUE, STOP, LOCKED, QUEUED, PREROLL, PLAYING, FADING = (1 << i for i in range(8))


def check(path, wet):
    frames = list(struct.iter_unpack("<ffffI", path.read_bytes()))
    assert len(frames) == 19 * P
    callback = int(path.with_suffix(".txt").read_text(encoding="utf-8"))
    origin = next(i for i, frame in enumerate(frames) if frame[4] & MARK)
    assert 0 <= origin < 4096
    cues = {2*P+97, 5*P+P-1024, 9*P+97, 11*P+97, 12*P+512, 13*P+97}
    stops = {5*P+512, 12*P+512}
    markers = {cycle*P + (cycle == 14) for cycle in range(19) if cycle != 8}
    actions = {MARK: markers, CUE: cues, STOP: stops}
    for mask, expected in actions.items():
        observed = {i - origin for i, frame in enumerate(frames) if frame[4] & mask}
        assert observed == expected, (mask, observed - expected, expected - observed)
    dry_reference = [
        (0.6, 0.5) if phase < 48 else
        (0.1 * math.sin(phase * 0.0575958653), -0.08 * math.sin(phase * 0.0575958653))
        for phase in range(P)
    ]
    repeat = list(wet[LEAD:]) + [(0, 0)] * (P - 2*LEAD) + list(wet[:LEAD])
    # The expected launch and interruption positions are fixed by the fixture,
    # independently of the probe's implementation or reported state flags.
    windows = [(3*P, 5*P+512), (7*P, 8*P), (12*P, 12*P+512), (14*P, 14*P)]
    max_mix_error = max_dry_error = 0
    wet_frames = dry_frames = 0
    for i, (left, right, out_left, out_right, _) in enumerate(frames):
        time = i - origin
        dry = (left, right)
        out = (out_left, out_right)
        expected_dry = dry_reference[time % P] if time >= 0 else (0, 0)
        max_dry_error = max(max_dry_error, *(abs(value-ref) for value, ref in zip(dry, expected_dry)))
        for bar_one, cut in windows:
            if bar_one - LEAD <= time < cut + FADE:
                relative = time - bar_one
                expected_wet = wet[relative+LEAD] if relative < 0 else repeat[relative % P]
                fade = 1 if time < cut else 1 - (time-cut)/FADE
                max_mix_error = max(max_mix_error, *(abs(o-d-fade*w) for o,d,w in zip(out,dry,expected_wet)))
                wet_frames += 1
                break
        else:
            assert out == dry, (path.name, "wet escaped Standby", time, dry, out)
            dry_frames += 1
    assert max_dry_error < 1e-7 and max_mix_error < 1e-7
    state = lambda time: frames[origin+time][4]
    assert state(2*P) & LOCKED
    assert state(2*P+97) & QUEUED
    assert state(3*P-LEAD) & PREROLL
    assert state(3*P) & MARK and state(3*P) & PLAYING
    assert state(5*P+512) & FADING and not state(5*P+512+FADE) & FADING
    assert state(6*P) & QUEUED and state(7*P-LEAD) & PREROLL
    assert not state(8*P) & LOCKED and state(8*P) & FADING
    assert state(9*P+97) & CUE and not state(9*P+97) & QUEUED
    assert state(11*P) & LOCKED and not state(11*P) & (QUEUED | PLAYING | PREROLL)
    assert state(12*P+512) & (CUE | STOP) == CUE | STOP
    assert state(12*P+512) & FADING
    assert not state(14*P) & LOCKED and state(14*P) & FADING and state(14*P+1) & MARK
    assert state(17*P) & LOCKED and not state(17*P) & (QUEUED | PLAYING | PREROLL)
    return {
        "trace": path.name, "callback_frames": callback, "verified_frames": len(frames),
        "dry_only_frames_exact": dry_frames, "prepared_wet_window_frames": wet_frames,
        "dry_reference_max_abs_error": max_dry_error, "mixed_output_max_abs_error": max_mix_error,
        "lead_in_frames": LEAD, "stop_fade_frames": FADE,
        "late_cue_deferral_frames": P, "bar_one_error_frames": 0,
        "missing_marker_ends_wet": True, "one_sample_shift_ends_wet": True,
        "requires_fresh_cue_after_lock": True,
    }


if __name__ == "__main__":
    cli = argparse.ArgumentParser()
    cli.add_argument("wet", type=Path)
    cli.add_argument("traces", nargs="+", type=Path)
    args = cli.parse_args()
    wet = list(struct.iter_unpack("<ff", args.wet.read_bytes()))
    assert len(wet) == 2*LEAD and max(abs(sample) for frame in wet[:LEAD] for sample in frame) > .1
    results = [check(trace, wet) for trace in args.traces]
    assert {result["callback_frames"] for result in results} == {128, 256}
    print(json.dumps(results, indent=2))

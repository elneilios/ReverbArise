"""Summarize and check a hosted Rack timing trace."""

import argparse
import csv
import json
from collections import Counter, defaultdict
from fractions import Fraction
from pathlib import Path

NOTES = dict(capture=14, phrase=12, bar=19, beat=21, cue=16, stop=17)
NOTES["reset-signal"] = 22


def alignment(audio, markers, expected_offset):
    # The initial state report can start high. The final timer flush can stop
    # between the two paths of the masked fixture.
    start = max(audio[0], markers[0]) + 2
    end = min(audio[-1], markers[-1]) - 2
    assert start <= end, "No shared comparison window"
    audio = {sample for sample in audio if start <= sample <= end}
    adjusted = {sample + expected_offset for sample in markers
                if start <= sample + expected_offset <= end}
    return dict(matched=len(audio & adjusted), audio_minus_midi=expected_offset,
                without_marker=len(audio - adjusted), without_audio=len(adjusted - audio))


def analyze(path, tempo, masked):
    lines = path.read_text().splitlines()
    assert not any(line.startswith("#") for line in lines), "Trace lost events or hit its limit"
    streams = defaultdict(list)
    for row in csv.DictReader(lines):
        sample = int(row["sample"])
        frames = int(row["frames"])
        offset = sample - int(row["blockStart"])
        assert 0 <= offset < frames, row
        assert int(row["rate"]) == 48000, row
        assert row["stream"] not in {"invalid-offset", "midi-overflow"}, row
        streams[row["stream"]].append((sample, int(row["value"]), offset,
                                       frames, int(row["note"]), int(row["channel"])))

    def rises(role):
        return [sample for sample, high, *_ in streams[role] if high]

    roles = {}
    for role, expected_note in NOTES.items():
        events = streams[role]
        assert events and {event[4] for event in events} == {expected_note}, role
        assert {event[5] for event in events} == {1}, role
        offsets = [event[2] for event in events]
        roles[role] = dict(note=expected_note, channel=1, edges=len(events),
                           rising=len(rises(role)), callback_offset=[min(offsets), max(offsets)])

    for role, count in (("cue", 4), ("stop", 1), ("reset-signal", 1)):
        edges = streams[role]
        assert len(edges) == 2 * count, role
        for rise, fall in zip(edges[::2], edges[1::2]):
            assert rise[1] == 1 and fall[1] == 0 and fall[0] - rise[0] == 48, role

    bar_samples = set(rises("bar"))
    assert bar_samples <= set(rises("beat")), "Bars do not coincide with beat markers"
    assert {sample - int(masked) for sample in rises("phrase")} <= bar_samples
    phrase = alignment(rises("left"), rises("phrase"), -int(masked))
    bar = alignment(rises("right"), rises("bar"), 0)
    assert bar["matched"] > 0 and not bar["without_marker"] and not bar["without_audio"], bar
    assert phrase["matched"] > 0 and not phrase["without_audio"], phrase
    assert bool(phrase["without_marker"]) == masked, phrase

    nominal = Fraction(48000 * 60 * 16, tempo)
    # The reset begins a new epoch. Dry continues through the marker mask.
    dry = [sample for sample in rises("left") if sample > rises("reset-signal")[-1]]
    intervals = Counter(b - a for a, b in zip(dry, dry[1:]))
    assert len(dry) >= 5, "Too few complete phrases after reset"
    residuals = [sample - dry[0] - i * nominal for i, sample in enumerate(dry)]
    return dict(trace=path.name, tempo=tempo, rate=48000, roles=roles, trace_losses=0,
                callback_frames=sorted({event[3] for event in streams["callback"]}),
                processed_seconds=round(streams["callback"][-1][0] / 48000, 3),
                phrase_alignment=phrase, bar_alignment=bar, intervals=dict(sorted(intervals.items())),
                residual_samples=[round(float(min(residuals)), 4), round(float(max(residuals)), 4)],
                reset_samples=rises("reset-signal"),
                gap_samples=max(b[0] - a[0] for a, b in zip(streams["callback"], streams["callback"][1:])))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trace", type=Path)
    parser.add_argument("--tempo", type=int, required=True)
    parser.add_argument("--mask-phrase", action="store_true")
    args = parser.parse_args()
    print(json.dumps(analyze(args.trace, args.tempo, args.mask_phrase), indent=2))

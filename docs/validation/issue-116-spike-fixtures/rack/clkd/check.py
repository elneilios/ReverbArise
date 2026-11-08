import csv
import json
import statistics
from bisect import bisect_left
from collections import Counter
from pathlib import Path

here = Path(__file__).resolve().parent
known = set((here / 'previous-traces.txt').read_text().splitlines())
inputs = sorted(p for p in (here.parent / 'traces').glob('timing-*.csv') if p.name not in known)
if len(inputs) != 4:
    raise SystemExit(f'Expected four trace lanes, got {len(inputs)}')


def check_trace(path):
    lines = path.read_text().splitlines()
    assert not any(line.startswith('#') for line in lines), path
    rows = list(csv.DictReader(lines))

    def rises(stream):
        return [int(row['sample']) for row in rows if row['stream'] == stream and row['value'] == '1']

    def intervals(edges):
        return dict(sorted(Counter(b-a for a, b in zip(edges, edges[1:])).items()))

    def compare(clock, audio):
        # A VCO square's transition can give additional crossings inside the
        # click. Only use the first threshold crossing in each 1 ms window.
        onsets = []
        for sample in audio:
            if not onsets or sample - onsets[-1] > 100:
                onsets.append(sample)
        offsets = []
        lower = max(onsets[0], clock[0]) + 4
        upper = min(onsets[-1], clock[-1]) - 4
        for sample in clock:
            if not lower <= sample <= upper:
                continue
            index = bisect_left(onsets, sample - 3)
            assert index < len(onsets) and abs(onsets[index]-sample) <= 3, (path, sample)
            offsets.append(onsets[index]-sample)
        for sample in onsets:
            if not lower <= sample <= upper:
                continue
            index = bisect_left(clock, sample - 3)
            assert index < len(clock) and abs(clock[index]-sample) <= 3, (path, sample)
        assert len(offsets) >= 5, (path, offsets)
        return {'edges': len(offsets), 'audio_minus_midi_samples': dict(sorted(Counter(offsets).items()))}

    bars, phrases, stops = map(rises, ('bar', 'phrase', 'capture-stop'))
    assert len(phrases) >= 6, path
    for previous, current in zip(phrases, phrases[1:]):
        assert previous in bars and current in bars, (path, previous, current)
        assert bisect_left(bars, current) - bisect_left(bars, previous) == 4, (path, previous, current)

    bar_intervals = intervals(bars)
    phrase_intervals = intervals(phrases)
    callback_sizes = sorted({int(row['frames']) for row in rows})
    rate = {int(row['rate']) for row in rows}
    assert rate == {48000}, rate
    first = pending = None
    durations = []
    capture_lengths = []
    non_silent = []
    offsets = set()
    for row in rows:
        label = row['stream']
        sample, value = int(row['sample']), int(row['value'])
        if label == 'capture-start':
            assert first is None and sample in bars and sample in phrases, (path, row)
            first = sample
        elif label == 'stop-pending':
            assert first is not None and pending is None and sample in stops, (path, row)
            assert value == sample-first, (path, row)
            pending = sample
            offsets.add(sample-int(row['blockStart']))
        elif label == 'capture-end':
            assert first is not None and pending is not None, (path, row)
            assert sample == bars[bisect_left(bars, pending)], (path, row)
            assert value == sample-first, (path, row)
            durations.append(sample-pending)
            capture_lengths.append(value)
            first = pending = None
        elif label == 'non-silent-frames':
            assert capture_lengths and 0 < value < capture_lengths[-1], (path, row)
            non_silent.append(value)
        elif label == 'capture-cancelled':
            raise AssertionError((path, row))

    assert len(durations) == len(non_silent) and len(durations) >= 5, path
    assert len({duration == 0 for duration in durations}) == 1, path
    return {
        'trace': path.name, 'callback_frames': callback_sizes,
        'nominal_tempo': round(48000*240/statistics.median(b-a for a, b in zip(bars, bars[1:]))),
        'bar_intervals_samples': bar_intervals, 'phrase_intervals_samples': phrase_intervals,
        'bar_audio': compare(bars, rises('left')), 'phrase_audio': compare(phrases, rises('right')),
        'capture_case': 'exactly_on_bar' if durations[0] == 0 else 'between_bars',
        'verified_captures': len(durations),
        'capture_lengths_samples': dict(sorted(Counter(capture_lengths).items())),
        'stop_to_boundary_samples': dict(sorted(Counter(durations).items())),
        'stop_callback_offsets': sorted(offsets),
        'non_silent_frames_range': [min(non_silent), max(non_silent)],
    }


results = sorted(map(check_trace, inputs), key=lambda row: (row['nominal_tempo'], row['capture_case']))
assert {(r['nominal_tempo'], r['capture_case'], tuple(r['callback_frames'])) for r in results} == {
    (120, 'between_bars', (128,)), (137, 'between_bars', (128,)),
    (137, 'exactly_on_bar', (256,)), (280, 'between_bars', (256,)),
}, results
summary = json.dumps(results, indent=2)
(here / 'results.json').write_text(summary+'\n')
print(summary)

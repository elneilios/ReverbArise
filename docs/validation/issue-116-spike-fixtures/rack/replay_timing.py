"""Offline replay of measured Rack command timestamps through a rough wet mixer."""
import argparse
import json
from pathlib import Path

from check_timing import PERIOD, analyze, read_trace

LEAD = 2048
FADE = 240


def rises(rows, stream, start, end):
    return sorted({row["sample"] for row in rows if row["stream"] == stream
                   and row["value"] == 1 and start <= row["sample"] < end})


class Replay:
    def __init__(self, first, markers, cues, stops, lead, body):
        self.first = first
        self.markers = set(markers)
        self.cues = set(cues)
        self.stops = set(stops)
        self.lead = lead
        self.body = body
        self.anchor = None
        self.intervals = 0
        self.launch = None
        self.fade = None
        self.accepted_launches = []

    def stop(self, sample):
        if self.launch is None or self.fade is not None:
            return
        if sample < self.launch - len(self.lead):
            self.launch = None
        else:
            self.fade = sample

    def step(self, sample):
        if self.anchor is not None and sample == self.anchor + PERIOD and sample not in self.markers:
            self.stop(sample)
            self.anchor = None
            self.intervals = 0
        if sample in self.markers:
            if self.anchor is None:
                self.intervals = 0
            elif sample - self.anchor == PERIOD:
                self.intervals += 1
            else:
                self.stop(sample)
                self.intervals = 0
            self.anchor = sample
        if sample in self.stops:
            self.stop(sample)
        elif sample in self.cues and self.intervals >= 2 and self.launch is None:
            deadline = sample + len(self.lead)
            cycles = max(1, (deadline - self.anchor + PERIOD - 1) // PERIOD)
            self.launch = self.anchor + cycles * PERIOD
            self.accepted_launches.append(self.launch)
        if self.launch is None or sample < self.launch - len(self.lead):
            return 0
        if self.fade is not None and sample >= self.fade + FADE:
            self.launch = None
            self.fade = None
            return 0
        position = sample - self.launch
        if position < 0:
            wet = self.lead[position + len(self.lead)]
        else:
            phase = position % PERIOD
            wet = self.body[phase]
            if phase >= PERIOD - len(self.lead):
                wet += self.lead[phase - (PERIOD - len(self.lead))]
        return wet if self.fade is None else wet * (1 - (sample - self.fade) / FADE)

    def render(self, end):
        return [self.step(sample) for sample in range(self.first, end)]


def verify(path):
    result = analyze(path)
    rows = read_trace(path)
    first, end = result["start_sample"], result["end_exclusive"]
    markers = rises(rows, "phrase", first, end)
    cues = rises(rows, "cue", first, end)
    stops = rises(rows, "stop", first, end)
    after_lock = first + 2 * PERIOD
    early, repeated, simultaneous, late = [
        next(cue for cue in cues if cue == after_lock + offset)
        for offset in (97, 193, 4099, PERIOD - 1024)
    ]
    assert simultaneous in stops
    lead = [((i % 31) + 1) / 64 for i in range(LEAD)]
    body = [((i % 17) + 1) / 32 for i in range(PERIOD)]

    def model(cue_times, stop_times=(), reference=markers):
        return Replay(first, reference, cue_times, stop_times, lead, body)

    launch = after_lock + PERIOD
    clean = model([early, repeated])
    signal = clean.render(launch + 3 * PERIOD)
    assert clean.accepted_launches == [launch]
    assert signal[launch - LEAD - 1 - first] == 0
    assert signal[launch - LEAD - first:launch - first] == lead
    assert signal[launch - first] == body[0]
    expected = body.copy()
    for i, value in enumerate(lead):
        expected[PERIOD - LEAD + i] += value
    assert signal[launch - first:] == expected * 3

    deferred = model([late])
    deferred.render(launch + PERIOD)
    assert deferred.accepted_launches == [launch + PERIOD]

    queued_stop = model([early], [simultaneous])
    assert not any(queued_stop.render(launch + PERIOD))
    conflict = model([simultaneous], [simultaneous])
    assert not any(conflict.render(launch + PERIOD)) and not conflict.accepted_launches

    playing_stop = next(stop for stop in stops if stop == launch + 4099)
    stopped = model([early], [playing_stop])
    signal = stopped.render(playing_stop + FADE + 1)
    assert signal[playing_stop - first] == body[4099]
    assert signal[playing_stop + FADE - first] == 0
    assert 0 < signal[playing_stop + FADE - 1 - first] < body[4099 + FADE - 1]

    missing = model([early], reference=[marker for marker in markers if marker != launch])
    signal = missing.render(launch + FADE + 1)
    assert signal[launch - LEAD - first:launch - first] == lead
    assert signal[launch + FADE - first] == 0

    shifted = model([early], reference=[markers[0], markers[1], markers[2] + 1])
    assert not any(shifted.render(launch + PERIOD)) and not shifted.accepted_launches
    return {
        "trace": path.name, "callback_frames": result["callback_frames"],
        "whole_lead_in_frames": LEAD, "bar_one_offset_samples": 0,
        "repeats_checked": 3, "overlap_summed_exactly": True,
        "late_cue_deferral_frames": PERIOD, "duplicate_cue_kept_first_launch": True,
        "queued_and_simultaneous_stop_silent": True, "wet_fade_frames": FADE,
        "missing_marker_fades_wet": True, "mismatch_blocks_cue": True,
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("traces", nargs="+", type=Path)
    args = parser.parse_args()
    print(json.dumps([verify(path) for path in args.traces], indent=2))

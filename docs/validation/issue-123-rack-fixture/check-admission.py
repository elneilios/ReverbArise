"""Run contract scenarios and optionally replay a complete Rack trace."""

import argparse
import csv
from dataclasses import replace
from fractions import Fraction
from pathlib import Path

from admission import Authority, FreshGate, PhrasePhase, PhraseTiming, State
from admission import capture_end_request, capture_end_sample
from admission import musical_grid, valid_offsets


def check_scenarios():
    timing = PhraseTiming(21, Fraction(120), (4, 4), 1)
    phase = PhrasePhase(timing, 1)
    unrelated = PhrasePhase(replace(timing, beats=2), 1)
    unavailable = Authority(State.NOT_CONFIGURED, "Connect the external command input")
    song = Authority(State.NOT_VERIFIED, "Host did not supply absolute samples")
    ready = Authority(State.READY)
    assert not phase.cue(0, timing, 10)
    for sample in (0, 10, 21):
        phase.observe(sample)
    assert phase.authority == ready and phase.cue(21, timing, 10)
    assert phase.for_take(replace(timing, beats=4), 42).state == State.NOT_VERIFIED
    assert unrelated.authority.state == State.NOT_VERIFIED
    assert song.state == State.NOT_VERIFIED and unavailable.state == State.NOT_CONFIGURED
    assert musical_grid(ready, None).state == State.NOT_CONFIGURED
    assert musical_grid(ready, (4, 4)) == ready
    gate = FreshGate()
    assert not gate.observe(True) and not gate.observe(True)
    assert not gate.observe(False) and gate.observe(True)
    assert not gate.observe(False) and not gate.observe(True)
    assert capture_end_request(ready, False, unavailable).state == State.UNSUPPORTED
    assert capture_end_request(ready, False, ready) == ready
    assert capture_end_sample(21, [21, 42]) == 21
    assert capture_end_sample(22, [21, 42]) == 42
    assert capture_end_sample(None, [21, 42]) is None
    assert capture_end_sample(22, [21, 42], interruption=33) is None
    assert capture_end_sample(22, [21]) is None

    phase.observe(32)
    # The last possible marker is at 43. No heartbeat arrives between
    # the marker at 32 and the Cue commands at 43 and 44.
    assert phase.cue(43, timing, 10)
    assert not phase.cue(44, timing, 10)
    assert phase.authority.state == State.SYNC_LOST and not phase.cued
    for sample in (50, 61, 71):
        phase.observe(sample)
    assert phase.authority == ready and not phase.cued
    assert phase.cue(71, timing, 11)
    phase.revoke("Reset")
    assert phase.authority.state == State.SYNC_LOST and not phase.cued

    # Each interval rounds to a legal integer, but the cumulative drift
    # violates the fixed epoch. A rolling interval check would accept it.
    for sample in (100, 111, 122):
        phase.observe(sample)
    assert phase.authority == ready
    phase.observe(133)
    assert phase.authority.state == State.SYNC_LOST

    for sample in (150, 160, 171):
        phase.observe(sample)
    assert phase.authority == ready
    phase.observe(172)
    assert phase.authority.state == State.SYNC_LOST

    assert valid_offsets(128, [0, 127]) and valid_offsets(128, [0] * 128)
    assert not valid_offsets(128, [-1]) and not valid_offsets(128, [128])
    assert not valid_offsets(128, [0] * 129)


def check_measured_take_lengths():
    timing = PhraseTiming(48000, Fraction(137), (4, 4), 16)
    phase = PhrasePhase(timing, 1)
    for sample in (0, 336350, 672701):
        phase.observe(sample)
    assert phase.cue(672701, timing, 336350)
    assert phase.cue(672701, timing, 336351)
    assert phase.for_take(timing, 336349).state == State.NOT_VERIFIED
    assert phase.for_take(timing, 336352).state == State.NOT_VERIFIED
    assert phase.for_take(replace(timing, beats=4), 84088).state == State.NOT_VERIFIED
    # An identical period can come from a different clock or meter.
    assert phase.for_take(replace(timing, meter=(8, 8), beats=32), 336350).state == State.NOT_VERIFIED
    assert phase.for_take(replace(timing, sample_rate=96000, tempo_bpm=274), 336350).state == State.NOT_VERIFIED


def replay(path, timing, phase_error, masked):
    phase = PhrasePhase(timing, phase_error)
    reset_sample = None
    markers = 0
    losses = 0
    cued_at_loss = False
    fresh_cue_after_loss = False

    def aligned_sample(row):
        # The masked patch has one extra engine sample on the phrase path.
        return int(row["sample"]) - int(masked and row["stream"] == "phrase")

    # Trace streams can be written out of sample order within a callback.
    with path.open(newline="") as stream:
        order = {"reset-signal": 0, "phrase": 1, "callback": 3}
        rows = sorted(csv.DictReader(stream), key=lambda row: (
            aligned_sample(row), order.get(row["stream"], 2)))
    for row in rows:
        sample = aligned_sample(row)
        if row["stream"] == "reset-signal" and row["value"] == "1":
            phase.revoke("Reset")
            reset_sample = sample
            markers = 0
            continue
        if reset_sample is None or sample <= reset_sample:
            continue
        previously_ready = phase.authority.state == State.READY
        previously_cued = phase.cued
        if row["stream"] == "phrase" and row["value"] == "1":
            phase.observe(sample)
            markers += 1
            if not masked and markers >= 3:
                assert phase.authority.state == State.READY, row
        elif row["stream"] == "callback":
            phase.advance(sample)
        elif row["stream"] == "cue" and row["value"] == "1":
            # This probe records markers and commands, so replay uses a
            # reference take with the shorter legal length for this lane.
            admitted = phase.cue(sample, timing, phase.lower)
            fresh_cue_after_loss |= losses > 0 and admitted
        elif row["stream"] == "stop" and row["value"] == "1":
            phase.cued = False
        if previously_ready and phase.authority.state == State.SYNC_LOST:
            losses += 1
            cued_at_loss |= previously_cued
            assert not phase.cued
    assert markers >= 5 and not phase.cued and bool(losses) == masked
    assert fresh_cue_after_loss == masked
    assert phase.authority.state == State.READY
    print(f"{path.name}: {markers} phrase markers checked after reset, {losses} sync losses"
          f", Cue cleared on loss: {cued_at_loss}, fresh Cue after loss: {fresh_cue_after_loss}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trace", type=Path)
    parser.add_argument("--tempo", type=int, choices=(137, 280))
    parser.add_argument("--phase-error", type=Fraction, default=Fraction(1))
    parser.add_argument("--mask-phrase", action="store_true")
    args = parser.parse_args()
    check_scenarios()
    check_measured_take_lengths()
    print("Admission scenarios passed")
    if args.trace is not None:
        if args.tempo is None:
            parser.error("--trace requires --tempo")
        timing = PhraseTiming(48000, Fraction(args.tempo), (4, 4), 16)
        replay(args.trace, timing, args.phase_error, args.mask_phrase)

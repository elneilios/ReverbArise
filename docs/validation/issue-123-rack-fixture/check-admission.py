"""Run contract scenarios and optionally replay a complete unmasked Rack trace."""

import argparse
import csv
from fractions import Fraction
from pathlib import Path

from admission import Authority, FreshGate, PhrasePhase, State, capture_end_request, capture_end_sample
from admission import musical_grid, valid_offsets


def check_scenarios():
    phase = PhrasePhase(Fraction(21, 2), 1)
    unrelated = PhrasePhase(21, 1)
    unavailable = Authority(State.NOT_CONFIGURED, "Connect the external command input")
    song = Authority(State.NOT_VERIFIED, "Host did not supply absolute samples")
    ready = Authority(State.READY)
    assert not phase.cue(phase.period)
    for sample in (0, 10, 21):
        phase.observe(sample)
    assert phase.authority == ready and phase.cue(phase.period)
    assert phase.for_take(42).state == State.NOT_VERIFIED
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
    phase.advance(43)
    assert phase.authority == ready
    phase.advance(44)
    assert phase.authority.state == State.SYNC_LOST and not phase.cued
    for sample in (50, 61, 71):
        phase.observe(sample)
    assert phase.authority == ready and not phase.cued
    assert phase.cue(phase.period)
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


def replay(path, period, phase_error, masked):
    phase = PhrasePhase(period, phase_error)
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
        if row["stream"] == "phrase" and row["value"] == "1":
            previously_ready = phase.authority.state == State.READY
            previously_cued = phase.cued
            phase.observe(sample)
            markers += 1
            if previously_ready and phase.authority.state == State.SYNC_LOST:
                losses += 1
                cued_at_loss |= previously_cued
                assert not phase.cued
            if not masked and markers >= 3:
                assert phase.authority.state == State.READY, row
        elif row["stream"] == "callback":
            previously_ready = phase.authority.state == State.READY
            previously_cued = phase.cued
            phase.advance(sample)
            if previously_ready and phase.authority.state == State.SYNC_LOST:
                losses += 1
                cued_at_loss |= previously_cued
                assert not phase.cued
        elif row["stream"] == "cue" and row["value"] == "1":
            admitted = phase.cue(period)
            fresh_cue_after_loss |= losses > 0 and admitted
        elif row["stream"] == "stop" and row["value"] == "1":
            phase.cued = False
    assert markers >= 5 and not phase.cued and bool(losses) == masked
    assert fresh_cue_after_loss == masked
    assert phase.authority.state == State.READY
    print(f"{path.name}: {markers} phrase markers checked after reset, {losses} sync losses"
          f", Cue cleared on loss: {cued_at_loss}, fresh Cue after loss: {fresh_cue_after_loss}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trace", type=Path)
    parser.add_argument("--period", type=Fraction)
    parser.add_argument("--phase-error", type=Fraction, default=Fraction(1))
    parser.add_argument("--mask-phrase", action="store_true")
    args = parser.parse_args()
    check_scenarios()
    print("Admission scenarios passed")
    if args.trace is not None:
        if args.period is None:
            parser.error("--trace requires --period")
        replay(args.trace, args.period, args.phase_error, args.mask_phrase)

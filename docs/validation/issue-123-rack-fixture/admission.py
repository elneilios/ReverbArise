"""Executable reference for the measured Rack admission contract."""

from dataclasses import dataclass
from enum import Enum
from fractions import Fraction


class State(Enum):
    UNSUPPORTED = "Unsupported"
    NOT_CONFIGURED = "Not configured"
    NOT_VERIFIED = "Not verified"
    SYNC_LOST = "Sync lost"
    READY = "Ready"


@dataclass(frozen=True)
class Authority:
    state: State
    reason: str = ""


def musical_grid(beat, meter):
    if meter is None:
        return Authority(State.NOT_CONFIGURED, "Declare the meter to use Bars and Beats")
    return beat


class FreshGate:
    def __init__(self):
        self.saw_low = False
        self.started = False

    def observe(self, high):
        if self.started:
            return False
        if not high:
            self.saw_low = True
            return False
        self.started = self.saw_low
        return self.started


def capture_end_request(gate, qualified_release, independent_end):
    if gate.state != State.READY:
        return gate
    if not qualified_release and independent_end.state != State.READY:
        return Authority(State.UNSUPPORTED, "Use Duration or a verified capture end input")
    return Authority(State.READY)


def capture_end_sample(request, verified_bars, interruption=None):
    if request is None:
        return None
    for bar in verified_bars:
        if interruption is not None and bar >= interruption:
            return None
        if bar >= request:
            return bar
    return None


def valid_offsets(frames, offsets):
    return len(offsets) <= 128 and all(0 <= offset < frames for offset in offsets)


class PhrasePhase:
    """Only accepts samples from one verified, explicitly configured reference."""

    def __init__(self, period, phase_error):
        self.period = Fraction(period)
        self.phase_error = Fraction(phase_error)
        self.lower = self.period.numerator // self.period.denominator
        self.upper = -(-self.period.numerator // self.period.denominator)
        self.anchor = self.previous = None
        self.intervals = 0
        self.authority = Authority(State.NOT_VERIFIED, "Wait for matching phrase markers")
        self.cued = False

    def revoke(self, reason):
        state = self.authority.state
        if state in (State.READY, State.SYNC_LOST):
            state = State.SYNC_LOST
        self.authority = Authority(state, reason)
        self.anchor = self.previous = None
        self.intervals = 0
        self.cued = False

    def observe(self, sample):
        if self.previous is not None:
            interval = sample - self.previous
            expected = self.anchor + (self.intervals + 1) * self.period
            if not self.lower <= interval <= self.upper or abs(sample - expected) > self.phase_error:
                self.revoke("Phrase markers no longer match the verified cadence")
                return
            self.intervals += 1
        else:
            self.anchor = sample
        self.previous = sample
        if self.intervals >= 2:
            self.authority = Authority(State.READY)

    def advance(self, sample):
        # Consume events at the current sample before checking the deadline.
        if self.previous is not None and sample > self.previous + self.upper:
            self.revoke("Phrase marker missing. Restore the clock, then Cue again")

    def for_take(self, expected_period):
        if self.authority.state == State.READY and expected_period != self.period:
            return Authority(State.NOT_VERIFIED, "Connect a reference matching the captured phrase")
        return self.authority

    def cue(self, expected_period):
        self.cued = self.for_take(expected_period).state == State.READY
        return self.cued

# Loop host timing and admission

[Issue #123](https://github.com/elneilios/ReverbArise/issues/123) defines the
host contract used by Loop capture, playback and UI. See the
[dated evidence](../validation/2026-11-09-issue-123-rack-timing.md) for the
measured configurations, limitations and owner decision. The
[isolated probe](../validation/issue-123-rack-fixture/README.md) exercises
the input path and the reference policy. Production Loop integration
follows in #124–#126.

## Timing and input authority

Rack owns the clock. Drive the audible phrase and the beat, bar and phrase
references from the same stable CLKD source and reset. Declare the meter
and the relationship between the musical origin and the reference. The
fixture uses 4/4, a /4 bar and a /16 four-bar phrase. The dry and reference
paths must be aligned. A single extra Rack module can add one sample.
Check signed offsets at the hosted VST3 inputs, including after reset.

| Note | Input | Interpretation |
| ---: | --- | --- |
| 14 | Capture gate | Observe low before a fresh rise. A release needs independent qualification to request capture end. |
| 12 | Phrase | Recurring phase marker. |
| 19 | Bar | Musical boundary for the declared meter. |
| 21 | Beat | Beat timing for the declared clock. |
| 22 | Reset | Invalidate the current epoch when this route is verified. |
| 16 | Cue | External performance request. |
| 17 | Stop | External Perform Stop. |

The tested Host-FX Note Gate route uses MIDI channel 1. Preserve the
callback-relative offset in `[0, frames)`. Keep simultaneous roles distinct.
A reset or other invalidation takes precedence over admission at the same
sample. A phrase marker coincident with reset does not start the next
epoch. A phrase or bar does not imply Stop. The probe bounds observation
at 128 MIDI events per callback. An invalid offset, overflow or dropped
trace record invalidates evidence from that run. Downstream code must keep
event work bounded and reject incomplete authority.

## Readiness by capability

Readiness and its cause belong to each instance. A capability consumes only
evidence from its current reference epoch.

| Capability | Required evidence | Example unavailable explanation |
| --- | --- | --- |
| Song timeline | Existing Song admission for absolute host timing | Host did not supply a valid Song timeline. |
| Loop phrase phase | Shared origin and a recurring reference matching the accepted phrase | Connect a reference that repeats at the captured length. |
| Duration capture | Verified start, beat timing, meter and the requested span's musical boundary | Declare the meter to use Bars and Beats. |
| End signal capture | Verified fresh start, attributable end request and a verified bar reference for quantised completion | Use Duration or a verified capture end input. |
| Musical grid | Verified beat timing and explicit meter | Provide beat timing to enable the grid. |
| External Cue/Stop | Verified command route for this instance | Connect the external Cue and Stop inputs. |

**Unsupported** means the condition falls outside verified support.
**Not configured** means required setup is missing. **Not verified** means
configured inputs have yet to establish authority. **Sync lost** means
previously verified authority was revoked. **Ready** applies only while
the required observations remain valid. Local Cue/Stop does not depend on
external command readiness. Song retains its existing admission even when
Loop inputs are ready. FL Studio Loop MIDI and automation verification
belongs to [#127](https://github.com/elneilios/ReverbArise/issues/127).

## Stable fractional periods and recovery

The reference policy uses two complete matching intervals to acquire
phase. For a verified fixed configuration, use its fractional period and
accept the two adjacent integer interval lengths. On the measured 48 kHz
lanes the four-bar periods are exactly `46,080,000 / 137` and
`46,080,000 / 280` samples. Bound cumulative error to one sample from
the epoch's first observed marker and the fractional cadence. This keeps
an individually valid interval from allowing drift to accumulate. The
observed marker owns the actual boundary. A one-bar take needs a matching
recurrence before live Cue, even when the host has valid four-bar markers.

Declare sync lost when a required marker is absent after the previous
marker plus the upper integer interval, arrives out of cadence, or the epoch loses required
authority. Revoke on observed reset, interrupted processing or a
reference, tempo or sample-rate change. After restore or reactivation,
start from new observations and require a fresh Cue after re-lock.
Detection is limited by what the host makes observable. The historical
power-off test halted callbacks without delivering a processor lifecycle
event. A callback pause cannot finish a pending capture. Verify the
resumed reference before granting authority.

## Capture end and disconnect

For a verified Rack musical end request, use the first verified bar at or
after the request as the exclusive end. A request on the bar closes at
that sample. If an interruption is observed before or at that boundary,
the incomplete attempt cannot replace the accepted source.

The observed Host-FX unplug emitted a note-off while the independent dry
gate stayed high. The note-off alone cannot establish a successful end
request, even if the bar clock continues. Withhold gate-release
completion on that wiring. An independent end route needs its own
verification. Duration can be available when its start, beat, meter and
boundary checks are satisfied.

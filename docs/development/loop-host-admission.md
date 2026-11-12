# Loop host timing and admission

[Issue #123](https://github.com/elneilios/ReverbArise/issues/123) defines the
host contract used by Loop capture, playback and UI. See the
[dated evidence](../validation/2026-11-09-issue-123-rack-timing.md) for the
measured configurations, limitations and owner decision. The
[isolated probe](../validation/issue-123-rack-fixture/README.md) exercises
the input path and the reference policy. Production Loop integration
follows in #124–#126.

## Timing and input authority

The simplified capture route uses channel 1 positive notes 23 (Clock), 24
(Start/Arm), and 25 (Stop). Use one pulse per beat, 4/4, and an audio path
aligned with those pulses. Learn can supply Start/Arm and Stop locally. The
first captured beat is the local downbeat, and subsequent observed beats
establish bar ends. The route measures cadence from sample offsets and
requires enough observed intervals to verify the take. A one-beat candidate
may wait for another beat to verify its clock, while retaining the original
exclusive capture end. Rounded pulse distances must admit a common period
with strictly less than one sample of rounding error per distance. Clock loss,
reset, or conflicting markers before the required boundary invalidate the
attempt. A local capture grid grants no recurring phase authority for Perform.
The Phrase pulse on note 12 may remain connected alongside the simplified
Clock, including on the same sample. Duration and End Signal leave it to
Perform. End on repeat uses Phrase coincident with Clock to open a take and
close it at the next coincident pulse after a whole number of bars. A mix
of simplified Clock and legacy Beat or Bar remains conflicting capture
timing.

See the [manual and sequenced Rack fixtures](../validation/issue-133-rack-fixture/README.md)
for measured control delivery and sample alignment. Existing marker-based
Duration projects continue to use the following contract.

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
| 17 | Stop | Finish the current pass or cancel a queued launch. |
| 18 | Stop Now | Fade wet immediately or cancel a queued launch. |

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
| Loop phrase phase | Observed wet origin and a recurring reference matching the accepted phrase | Connect a reference that repeats at the captured length. |
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
is covered by the [FL Studio fixture](../validation/issue-127-fl-studio-fixture/README.md).

## FL Studio VST3

For FL Studio VST3, Loop capture can use the host's running sample position,
PPQ, fixed tempo and 4/4 meter at 48 kHz. Consecutive callbacks must cover
adjacent sample intervals and agree on the musical origin. When transport
starts on a bar line, an armed Duration take includes the first block.
The next callback must confirm continuity. When transport starts mid-bar,
or Arm is pressed during a bar, recording starts on the next downbeat.
The Duration unit sets the length. An End Signal take can use the MIDI
Start/Arm and Learn Stop notes. Start uses the same DAW downbeat rule and
Stop closes the take on the first verified downbeat after the request.
Discontinuity or loss of timing cancels an in-progress take and keeps
the prior source. The capture worker retains its existing atomic settlement
and cadence checks.

Perform defaults to the running Playlist grid. A Phrase note received while
Loop playback is idle sets beat one at the note's sample and shifts future
Cues until timing is interrupted. The first downbeat with time for the full
lead-in starts the wet Loop, which repeats at the accepted sample length.
The Phrase note does not change capture timing or a launch already queued.
Button and MIDI Cue use the same boundary rule. The plug-in preserves the
offset of a positive channel 1 note-on and combines same-sample duplicates.

With Live playback enabled, Perform follows an eligible FL Studio repeat
automatically. Admission uses the reported bounds, compatible capture timing
and a sample extent within one sample of the accepted Loop. FL Studio can
report these bounds for a Pattern or a Playlist repeat. A saved Playlist start
takes priority. Manual Cue remains available when there is no placement and
the repeat is absent or ineligible.

Set Loop Start uses the running playhead when compatible host timing is
available. The placement editor accepts explicit bar and beat while stopped or
when the playhead cannot be established. A stopped FL Studio playhead is
treated conservatively until its updates have been verified in the desktop
host. A Clock/Phrase-only Rack host leaves Set Loop Start disabled.

| MIDI number | FL Studio note | Rack note | Role |
| ---: | --- | --- | --- |
| 12 | C1 | C0 | Phrase |
| 16 | E1 | E0 | Perform Cue |
| 17 | F1 | F0 | Perform Stop |
| 18 | F#1 | F#0 | Perform Stop Now |
| 24 | C2 | C1 | Learn Start/Arm |
| 25 | C#2 | C#1 | Learn Stop |

The tested route uses MIDI Out and the effect wrapper's matching input
port on the MIDI Input event bus. Cue and Stop are also available in Perform
once live timing and prepared wet pass admission. A seek, reset or
interruption cancels queued or playing wet. Send a fresh Cue after timing
returns. The non-realtime path does not play Loop wet.

## Rack Clock and Perform Stop

The Rack Clock input serves both Learn and Perform. The first Clock pulse
counts as beat one in 4/4. Duration starts only on an observed beat one and
keeps that bar count when moving between Learn and Perform. End Signal keeps
its trigger-driven start. Perform verifies beat intervals independently of
the captured take. A Phrase or Bar pulse coincident with a Clock beat
can set the downbeat while the wet scheduler is idle. Cue forecasts the first
bar with room for the lead-in and repeats at the accepted sample length.
An aligned Phrase pulse sets the phrase origin. Cue then counts the accepted
phrase length from that origin and skips intervening bar lines.
The Rack route follows its Clock pulses and ignores the host transport
position for seek detection. Playlist playback uses its host transport.
Clock loss revokes the current run and the first returning pulse starts a
new count. A legacy Phrase-only patch can still use verified recurrence.

In both hosts, the first Stop Loop click or a MIDI Stop note suppresses later
cycles and finishes the current pass. Repeating MIDI Stop leaves that finish
in place. Stop Now in the UI or over MIDI uses the immediate short fade, as
does timing loss. Dry + Wet continues to pass the dry signal.

## Stable fractional periods and recovery

The legacy Phrase-only route uses an independently declared phrase length and the
observed marker as the wet origin. A capture grid can be ready while live phase is unconfigured, waiting,
mismatched or lost. The accepted sample extent provides a rounding check for
matching a measured capture tempo to the live reference. Live marker cadence
uses the independently verified reference tempo throughout its epoch.

The legacy automatic Phrase route supports one, two and four bars. Use the accepted
length as the expected recurrence and verify it against the actual Phrase
pulses. A wrong cadence blocks Cue. The reference policy uses two complete
matching intervals to acquire phase. For a verified fixed configuration, use its fractional period and
accept the two adjacent integer interval lengths. On the measured 48 kHz
lanes the four-bar periods are exactly `46,080,000 / 137` and
`46,080,000 / 280` samples. Bound cumulative error to one sample from
the epoch's first observed marker and the fractional cadence. This keeps
an individually valid interval from allowing drift to accumulate. The
observed marker owns the actual boundary. A one-bar take needs a matching
recurrence before live Cue, even when the host has valid four-bar markers.

Match the take using its verified sample rate, stable tempo, explicit
meter, and duration in beats of that meter. These fields must match the
reference. Derive the nominal period as
`sampleRate × 60 × beats × 4 / (tempoBpm × meterDenominator)`.
Accept a recorded frame count equal to the floor or ceiling of that
period. A four-bar take at 137 BPM and 48 kHz can therefore contain
336,350 or 336,351 frames. Keep the recorded extent and use the live marker
for the wet downbeat. The musician may place that marker independently of
incoming dry audio.

Declare sync lost when a required marker is absent after the previous
marker plus the upper integer interval, arrives out of cadence, or the
epoch loses required authority. Check the deadline at the Cue sample
before admitting the command, including between trace heartbeats. Process
a marker at the same sample before Cue. Revoke on observed reset, interrupted processing or a
reference, tempo or sample-rate change. After restore or reactivation,
start from new observations and require a fresh Cue after re-lock.
Detection is limited by what the host makes observable. The historical
power-off test halted callbacks without delivering a processor lifecycle
event. A callback pause cannot finish a pending capture. Verify the
resumed reference before granting authority.

## Editing while a Loop plays

The scheduler keeps its playback origin while the worker prepares a compatible
swell edit. Once a complete successor is available, repeating playback
crossfades the old and new mixes at the same Loop position. Excluding all
swells clears the wet mix while the Loop continues to advance. A queued
first entry still needs current wet preparation so its full lead-in fits.
Source replacement, incompatible host timing or loss of playback authority
still cancels or fades the old run.

The host must match the accepted Loop's sample rate for both Audition and
Cue. Their disabled controls report the required rate in the existing
Perform hover-help footer.

The worker reserves headroom for newly generated Loop wet at default Levels
and Live Wet 0 dB. On the fixed host grid it measures the positioned sum,
including partial pass overlap at Cue and finish. Variable phase routes use
a conservative bound. Perform uses the accepted starting gains and applies
subsequent gain edits directly. Dry + Wet adds the live dry signal at unity
and reports overload if the resulting float sum exceeds full scale.

## Capture end and disconnect

FL Studio can declare a repeating range for Pattern playback. Learn treats
the transition as continuous only when the host stays playing, the timing
map and repeat bounds match, and consecutive callbacks meet the declared end
and start exactly. Duration and End Signal count beats on the capture
timeline, so a take can finish at the wrap without appending audio from the
next pass. Early seeks and interrupted takes still cancel. Perform checks
the same boundary against its independent live observations. A verified wrap
keeps Cue and playback scheduled on the running audio cursor. An early jump
to the repeat start still cancels Cue and fades active wet.

For the simplified capture route, End Signal closes immediately before the
next downbeat strictly after Stop. Stop on a downbeat includes the bar
beginning there. Duration Stop cancels the current take, including when Stop
coincides with its configured end or arrives while the take still awaits a
corroborating beat. If processing or timing is interrupted
before the required boundary, the incomplete attempt cannot replace the
accepted source.

The observed Host-FX unplug emitted a note-off while the independent dry
gate stayed high. The note-off alone cannot establish a successful end
request, even if the bar clock continues. Withhold gate-release
completion on that wiring. An independent end route needs its own
verification. Duration can be available when its start, beat, meter and
boundary checks are satisfied.

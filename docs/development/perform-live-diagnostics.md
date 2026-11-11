# Perform live playback diagnostics

The Settings -> Recovery -> Open Diagnostics window is a read-only view of
transport admission and worker preparation. Preserve the failing project and
managed assets. Do not Clear, Recapture, reset renders, delete swells or toggle
settings to repair silence before collecting evidence; use a separate FLP copy
when changing the plugin build.

Each periodic refresh updates the Diagnostics readouts before measuring their
wrapped heights and laying out the window. Layout uses the text from that refresh.

## Reading observations

The fixed-grid section is a **UI snapshot**, separate from callback/worker
evidence. It reports the same decision used by timeline/inspector snapping:
sample-position availability, capture rate, accepted and observed timing,
division and the rejected prerequisite. Origin differences are observed minus
accepted in samples. Invalid/missing timing leaves derived origins unavailable.
The coordinate anchor remains accepted capture timing. During Play, the grid
can instead use the independently validated current run as its validation
reference. Its session/reset/run identity and validation delta appear separately
from the source/host delta. An admitted source-to-run offset is informational,
not a Snap warning. The same decision gates canvas and inspector actions.
Missing or rejected current run authority remains unavailable; a host tuple
matching the saved origin does not override a rejected run. Diagnostics merely
describe this control decision and never supply its authority. Grid availability
alone cannot establish audible output. Stop can restore grid compatibility
without proving the preceding Play was admitted.
Snap On, Ctrl bypass, Snap Off and exact-entry policies are unchanged.

Use **Copy diagnostics** in this window to share all readouts, including text
outside the visible viewport. Host offline errors retain prepared, export-run
reference and failing host timing separately. See the
[fixed-tempo export clock](offline-export-clock.md) for its admission contract;
a later parked callback cannot explain an earlier offline rejection.

Callback observations copy values from one processed block and its active
publication. `block` identifies that callback; `revision` identifies the
publication. Current and historical rejection observations are separate bounded
snapshots and need not be from the same block. An unavailable observation means
no complete snapshot was obtained, not successful playback.

The gate distinguishes workflow exclusion, disabled live playback, stopped
transport, absent publication, invalid callback range, invalid timing, rate,
tempo, meter, PPQ-zero origin mismatch, reset transition and missing coverage.
`admitted` describes eligible prepared processing; it does not assert nonzero
samples at a musically silent point. Routing and linear wet gain remain visible.

The observed/published timing tuple includes sample, PPQ, rate, BPM and meter.
Inferred origin is `sample - PPQ * rate * 60 / BPM`; delta is observed minus
published origin in samples. This source/publication delta is informational:
valid live playing transport may have a different origin from parked/source
timing. The owner recording demonstrates an approximately 16-sample difference.
No fixed 16-sample subtraction is applied and saved coordinates are unchanged.

`Live run` and its `reference` identify the callback-owned ephemeral clock used
for actual live admission. The first valid playing observation establishes the
run; later observations must match it within the existing one-sample origin
allowance. A valid Stop ends the run. Invalid timing or callback ranges do not
create a baseline or pretend that the host stopped; seeks, loops and output
controls cannot erase a rejected mapping during continuous Play. Session/audio
reset requires a new runtime observation. Source/render rate, tempo, meter and
identity checks remain independent and mandatory.

`Run reference / observed / delta samples` is the comparison used for the
playing clock. It is distinct from the source/publication comparison. `Run
clock gate` reports clock validation even if another gate (such as no publication
or disabled output) explains the callback's final result. Callback host-loop
bounds are labelled separately from the publication's preparation loop hints.

The causal section retains one fixed-size bundle: run start, last admitted
callback, first timing rejection and the admitted callback immediately preceding
that first rejection. The predecessor is unavailable if none was admitted.
Later failures, seeks, loops, publication changes and recovery do not overwrite
the first rejection/predecessor. Current and latest historical rejection remain
separate. A confirmed new run/session/audio-reset observation replaces the
bundle; Stop retains the completed run for inspection. Labels identify retained
run/session/reset and callback numbers rather than implying all rows are current.
The bundle is ephemeral and is not saved in DAW projects. Bounded snapshot reads
can return unavailable; this never means admission succeeded.

A supported source-to-playing origin delta can therefore appear alongside
`admitted`. A current `PPQ-zero origin mismatch` with a live run instead means
that the host departed from that run's reference. Values retain sufficient
precision to inspect this distinction. Unsupported/nonfinite origins remain
unavailable. The shared strict mapping helper and Capture, Audition, internal
Export and host offline validation keep their existing contracts.

## Host processing reactivation

The JUCE VST3 wrapper calls `reset()` when host processing is turned off,
independently of its release/prepare lifecycle. The processor forwards that
explicit reset to the bounded live-clock reset request. Until a new callback
publishes valid timing, worker clock authority is unavailable. The next valid
playing callback validates a fresh run and joins eligible resident wet at the
current absolute Song offset. Cold coverage follows the existing preparation
and retry rules; skipped audio is never replayed. Accepted capture, renders,
edits and saved Live preferences are retained.

This reset performs only an atomic clock invalidation; it does not clear assets,
relax rate/tempo/meter checks or reinterpret stored positions. A callback gap,
seek, loop or internal Live toggle does not itself reset the established run.
See [issue #103 evidence](../validation/2026-11-06-issue-103-host-reactivation.md)
for the synthetic lifecycle regression and separate FL Studio coverage.

Worker observations are separate from callback observations. Service and attempt
sequences distinguish eligibility checks from actual preparation work. Monotonic
service/attempt times are process-local milliseconds, not wall-clock timestamps.
Current/prepared revisions and window bounds describe the worker observation;
callback publication revision/bounds describe what that block actually consumed.
The worker live-run reference is copied from the same bounded control publication
used for eligibility, independently of developer telemetry. Publication rechecks
run/session identity and source/render dependencies; unavailable observations or
transient rejection remain retryable. An unchanged render revision/window alone
does not certify a new live clock. Verified resident PCM can be reused with its
original sample coordinates. The latest service result and last actual attempt
result distinguish an unchanged
resident window from successful publication, rejection and a later retry.

Stopping clears the play-gated incompatibility flag. A stopped prepared status
alone proves neither successful playing admission nor a new preparation. The
last rejection remains explicitly historical; compare its block to the current
observation. Observations are ephemeral and are not written to session state.

## Collecting evidence for issue #53

1. With the diagnostic build on a preserved instance/copy, open Diagnostics and
   record current callback, historical rejection and worker evidence while
   stopped, including live enable, routing and requested wet level.
2. Start Song playback before a known wet audible range and record the same
   fields through it. Include source/session and published versus observed
   timing, revisions, window bounds, callback range validity and actual gate.
3. Stop and record the resulting current/historical distinction. A working
   instance comparison is useful only if it isolates a different failed gate.

The owner report uses 48 kHz, 122 BPM, 4/4 and 61 swells. Three-decimal PPQ from
older diagnostics cannot establish a one-sample origin mismatch. Audition and
Export use distinct paths; their success alone does not prove live compatibility.
Keep deterministic injected fixture evidence separate from reproduction on the
owner's instance. Choose a behavioral fix only after identifying a causal defect.

Native verification and current host limitations are recorded in the
[issue #53 evidence](../validation/2026-11-04-issue-53-live-diagnostics.md).

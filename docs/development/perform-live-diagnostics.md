# Perform live playback diagnostics

The Settings -> Recovery -> Open Diagnostics window is a read-only view of
transport admission and worker preparation. Preserve the failing project and
managed assets. Do not Clear, Recapture, reset renders, delete swells or toggle
settings to repair silence before collecting evidence; use a separate FLP copy
when changing the plugin build.

## Reading observations

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
published origin in samples. Values are formatted with sufficient precision to
inspect the current one-sample origin tolerance. Unsupported/nonfinite origins
are unavailable rather than a fabricated zero difference. No timing tolerance
or asset-position reinterpretation is authorized by opening Diagnostics.

Worker observations are separate from callback observations. Service and attempt
sequences distinguish eligibility checks from actual preparation work. Monotonic
service/attempt times are process-local milliseconds, not wall-clock timestamps.
Current/prepared revisions and window bounds describe the worker observation;
callback publication revision/bounds describe what that block actually consumed.
The latest service result and last actual attempt result distinguish an unchanged
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

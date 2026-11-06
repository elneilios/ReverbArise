# Fixed-tempo DAW export clock

Host non-real-time Perform output schedules prepared wet at its accepted absolute
sample positions. A fresh bounce establishes a callback-owned fixed-tempo clock
independently of the captured or parked source PPQ origin. This handles FL
Studio's observed 16-sample source/export origin difference without moving audio.
There is no hard-coded offset, new latency or change to the audio/gain stage.

The source and callback must still have valid finite timing, matching sample
rate, BPM and meter. Each wet-required callback must keep the bounce reference's
origin within the existing one-sample allowance. A genuine change fails visibly;
the reference is not replaced on ordinary page publication or Playing changes.
Accepted IR fallback, asset verification, routing and gain plans remain active.
The same explicit run-clock validator governs outer preparation and the inner
mixer; changing only one gate would still produce a silent bounce.

Run boundaries retain the existing endOffline, prepare/release and backward
export snapshot restart behavior. Zero-length callbacks do not establish a run.
After all accepted wet ends, the existing host-tail PPQ-reset exception remains.
Failures stay local to each instance and sticky until a supported new run.
Live, capture, audition, internal Export and Snap retain their existing guards.
Issue #71 owns future tempo-map evidence/precision and persistent map revisions;
this repair implements only the current constant-tempo sample scheduling contract.

After a failed bounce, use Settings > Recovery > Open diagnostics > **Copy
diagnostics** and paste the text into support. It includes prepared, export-run
reference and failing host timing, source/run origin deltas, current diagnostics,
Snap/grid availability and precise live/worker observations. Copying changes no
saved state or playback setting. No swell regeneration is required.

This branch is based on main and contains no experimental output-protection
stage/settings. Existing plugin IDs, projects, captured audio and rendered asset
coordinates remain compatible. Install through the normal test-build workflow;
native passing and installed inventory are separate from FL Studio acceptance.

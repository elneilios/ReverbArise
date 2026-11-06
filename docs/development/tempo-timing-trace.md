# Opt-in tempo timing trace

This diagnostic harness supports issue #71 tasks 1.1–1.2. It records the raw
JUCE playhead query before existing timing admission. It does not implement
tempo-map support: current fixed-tempo capture/playback rejection remains active.
The trace never becomes an input to capture, Snap, render or audio admission.

Install the current Release through `scripts/install-test-vst3.ps1` with FL Studio
closed. Retain the printed prior bundle. Create a separate scratch project; do
not change the owner's saved arrangement to run the experiment. Launch using:

```powershell
& {
    ./scripts/start-timing-trace-host.ps1 `
        -Project 'C:\absolute\scratch.flp' `
        -TraceDirectory 'C:\absolute\trace-directory'
}
```

The launcher sets `REVERBARISE_TIMING_TRACE_DIR` only in the child host's
environment and restores its own prior value. No registry or user environment
setting is changed. The directory must exist for the plugin's opt-in; the
launcher creates it. A normal host launch with this variable absent creates no
trace. Each newly constructed instrumented plugin instance creates its own
UUID-named CSV, including scan/verification instances: identify the experiment
file by its actual rows, not file order. No audio, names, project state or asset
paths are stored in the CSV.

## Collection and interpretation

The fixed queue holds 65,536 records, 112 bytes each on the validated x64 build
(7 MiB plus index atomics). The callback does fixed optional-field copies and
release/acquire queue publication. The worker formats CSV, flushes batches and
writes a closing summary when the plugin/host is closed normally. No callback
allocation/deallocation, locks, waits or file access is introduced.

Records include callback ordinal, **diagnostic** run/reset/source epoch/pass,
sample position, PPQ, BPM, processing sample rate, last-bar PPQ, raw musical loop
bounds, frame count, meter and availability/playing/recording/looping/offline
flags. Diagnostic run numbers identify trace transitions, not the engine's
validated live-run admission. Missing fields have absent flags; their numeric
slots must not be interpreted as real zero observations. Values are serialized
with 17 significant digits. Sample-range overflow and observed noncontiguity are
flagged without synthesizing replacement coordinates or PPQ.

Retain the first parked observation per authority and each playing/offline
callback plus the confirmed Stop edge. Repeated parked callbacks are deliberately
omitted, so the file is not a continuous log of stopped tempo/position changes.
Missing PositionInfo during a run is retained, not treated as confirmed Stop.
The 65,536-row prefix cap is per plugin lifetime: roughly 43.7 s at 32 frames,
5 min 49.5 s at 256, or 11 min 39.1 s at 512, at 48 kHz, excluding suppressed
idle callbacks. The diagnostic cap differs from the **proposed** production
evidence cap and does not control source acceptance.

On overflow, subsequent eligible records are counted as omitted, never silently
overwritten. Disk failures are reported in the closing summary when writable;
missing summary or `omitted>0`/`io_failed=1` means the trace is incomplete and
cannot establish a complete sequence. Close the scratch host normally to drain
the worker and obtain the final summary; do not kill it. Analyze with:

```powershell
& { ./scripts/summarize-timing-trace.ps1 -Path 'C:\absolute\timing-uuid.csv' }
```

The summary is descriptive, not a map validator. Keep the original CSV and the
exact build/host/audio-device/block-size/PDC/project settings with each result.
Distinguish measured callback values from guarantees: neither sampled BPM nor
PPQ endpoint agreement proves exact ramp interiors, hidden intra-block events,
future-map access or unchanged timing at unobserved positions.

## Experiment matrix and checkpoint

Use a fixed 4/4 scratch arrangement with an existing tempo step, a linear ramp
and a curved ramp in separate labelled passes. Record the configured automation
and actual displayed BPM, run from origin and from inside transitions, seek and
loop, repeat with another actual buffer length, and bounce the same regions.
Retain raw callback boundaries and the host's offline flag/Playing behavior.
Record sample rate, audio device, host version, plugin version/commit, enabled
processing options and known plugin-delay compensation. Include an automation
transition inside a callback if reproducible; otherwise label that case untested.
Zero source input remains sufficient to inspect host timing if the DAW processes
the effect; do not infer wet acceptance from such a trace.

Task 1.2 is complete only when the requested host matrix is actually measured.
Unavailable automation or host control must be reported as missing coverage,
not replaced by the synthetic fixture. Task 1.3 is an owner checkpoint covering
measured capabilities, proposed admission/comparison/conflict rules, local grid
limitations and production resource limits. Production timing changes require
that checkpoint's approval. The final feature host smoke test remains separate.

## Why

Automatic capture and swell preparation now work, but manual Audition cannot demonstrate the musical result during normal FL Studio playback. The roadmap's M4 Live FX playback is the next step. Recent feedback also leaves extreme envelope drags unresolved; that regression must remain visible while progressing to transport playback.

## What Changes

- Add prepared wet playback at absolute Song positions, including seeking into swells, loops, overlaps, placement offsets, enabled/mute/solo and edited envelopes.
- Provide independent live playback enable, Dry + Wet/Wet Only routing and wet level controls. Default to enabled Dry + Wet with neutral user gain; live dry remains unchanged in that mode. Manual Audition retains separate settings and is unavailable during Song playback to prevent duplicate wet output.
- Feed playback from bounded, worker-prepared immutable pages; expose preparation, cold-seek misses, stale assets and unsupported timing instead of pretending missing audio played.
- Preserve dry-only capture, safe output transitions, instance settings and resource bounds. Display the host playhead and actual live playback readiness in the existing workspace.
- Reproduce and fix extreme envelope-handle bounds/gesture failures with focused regressions; retain existing edits and wet/dry alignment semantics.
- Follow-up feedback: prevent identical loop captures and temporary timing-read gaps from requeuing settled swells. Fit untouched final automatic tails to completed pass boundaries using real captured samples, preserving manual ownership.
- Performance feedback: tolerate small source variations against the retained baseline, independently of marker sensitivity; add deterministic host offline bounce of prepared swells without asynchronous window omissions.

## Capabilities

### New Capabilities
- `transport-swell-playback`: Song-positioned prepared wet mixing, lifecycle, routing, readiness, persistence and real-time resource guarantees.

### Modified Capabilities
- `reverse-convolution`: qualify manual audition independence when enabled Song playback takes priority, retaining the predecessor changes' contextual audition and recovery behavior. The focused envelope regression enforces the existing in-flight workspace contract rather than introducing another editing capability.

## Impact

Extends SwellEngine, processor timing/mixing, session metadata and the current workspace; adds a real-time transport renderer and worker-side playback preparation. Adds deterministic DSP, lifecycle, resource and processor tests plus a separate FL Studio acceptance checklist. Depends on completed M3 editing/persistence and the already synced/archived automatic workspace. Reconcile and sync/archive M3 while preserving newer workspace contracts. This change's final centre-line endpoint host check remains open before its archive.

No anticipatory playback of uncaptured future input, DAW transport control, tempo-map support, standalone export/portability work or broad UI redesign. Prepared audio before the host's starting position cannot be heard retroactively. Silent-insert callback behavior requires FL Studio validation.

- Expose an independently saved Regeneration sensitivity knob and per-swell Freeze render. Frozen renders retain verified wet audio across source changes and reopen; shape, gain and placement remain editable. Source/IR/support changes require unfreeze.

- Fix final-marker rounding at fractional loop boundaries and verify multiple independent instances during offline export, with prominent per-instance failure diagnostics.

- Avoid misclassifying an offline PPQ-origin reset after all wet has completed as an incomplete wet export; preserve dry routing and genuine timing failures.

- Refine envelope handles to true-position z stacking; offset only intrinsically coincident controls and use zoom to resolve visual overlap.

- Keep Start/End handles on the waveform centre line independently of fade values, naturally separating them from unity-level zero-fade controls.

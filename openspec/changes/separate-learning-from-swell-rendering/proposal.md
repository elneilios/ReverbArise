# Proposal

## Why

The current Learn detection policy also suppresses sound regeneration for existing swells. Issue #8 recovery exposed this coupling: re-learning valid source with detection off leaves existing swells stale unless a separate generation path intervenes. The owner wants one swell engine that continuously maintains valid sounds from material dependencies, with no recovery-only renderer.

## What Changes

- Separate source capture and optional marker discovery/application from existing-swell render maintenance. Learn owns source/detection; Perform owns sound editing/playback; the worker engine owns render readiness regardless of selected page/mode.
- **BREAKING behavior:** existing unfrozen swells automatically reconcile accepted source, assigned IR and compatible material-timing changes even when Detect events, Generate on Stop or legacy refresh preferences are off. Retain those saved values; they no longer suppress maintenance of existing definitions. Ordinary source replacement still requires explicit Recapture.
- Keep the same minimum-work pipeline for ordinary edits, file restoration, cache misses and re-learning. Reuse valid exact renders; never invent source or substitute IRs. Envelope/level/placement-only edits reuse full wet audio.
- Preserve frozen results, immutable history, protected marker edits, bounded worker jobs, latest-request publication and the callback's no-I/O/no-blocking contract.
- Keep the missing-asset dialog modal with Locate/Restore and normal Learn guidance. It resolves dependencies; it does not request special DSP jobs or duplicate Learn/Generate controls.
- Prepare the architectural review in `docs/development/swell-engine-architecture.md` before implementation. This proposal is an architectural dependency of `portable-project-assets` task 3.10, not authorization for association, cleanup or compression.

## Capabilities

### New Capabilities

None; reuse the existing source/marker and render capability boundaries.

### Modified Capabilities

- `automatic-swell-processing`: detection/application controls govern marker creation rather than suppressing maintenance of accepted swells; replace the existing dry-only/no-convolution coupling.
- `swell-regeneration`: one mode-independent dependency reconciler owns full-wet readiness, exact reuse, invalidation, bounded publication and recovery through ordinary inputs.

## Impact

Affected areas: CaptureWorker accepted-source notifications; SwellEngine, AutomaticCoordinator and LearnGeneration scheduling; source/IR/timing dependency fingerprints; waveform shaping/playback consumers; legacy preference semantics; recovery diagnostics, tests and guidance. Existing DSP algorithms, persistent identities, saved formats, default storage, durable-retention rules and manual mode/output rules remain protected. No canonical spec sync/archive or release is part of planning. Implementation must replace the superseded unshipped recovery-only experiment, not layer another scheduler over it.

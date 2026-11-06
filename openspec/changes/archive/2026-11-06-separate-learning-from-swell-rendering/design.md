# Design

## Context

See proposal.md for motivation and [the architecture review](../../../../docs/development/swell-engine-architecture.md) for ownership tables, component/data-flow and lifecycle diagrams. Current `AutomaticCoordinator::captureCommitted` defers existing work and `dispatchAutomatic` blocks Capture-mode maintenance; current canonical dry-only Learn requirements intentionally enforce that behavior. Existing immutable assets, fingerprints, input/shape generations and job publication guards should be reused.

## Goals / Non-Goals

**Goals:** Make accepted definitions plus verified material dependencies sufficient to maintain sound, independently of UI mode and marker policy; preserve exact saved sound reuse and separate shaping from convolution.

**Non-Goals:** New DSP algorithms, automatic marker creation when detection is off, source replacement without Recapture, inferred upstream DAW edits, new host identities/state formats, working-folder/writer migration, durable cleanup or compression. Timeline coordinate semantics and explicit engine-upgrade behavior remain authoritative.

## Decisions

1. Learn/capture and marker application publish accepted snapshots. Existing-swell reconciliation subscribes to accepted material and asset-availability events, including when the editor is closed. Provisional source cannot become render authority. Reject mode/screen-owned render admission and a special recovery-generation callback because they duplicate policy and create different behavior for the same inputs.
2. Reuse existing fingerprints, immutable stores and per-swell generations. Distinguish a required full-wet material key from derived envelope/playback identity. A crop extension that really adds source support remains a material footprint change; envelope-only edits within support do not convolve.
3. Reconcile affected IDs, compare verified assets, queue bounded/coalesced jobs, pin inputs and publish only matching latest identity/generation. Active playback page service and reader retirement keep their current priority and safety. Do not use UI polling or whole-profile scans as scheduling.
4. Freeze holds the saved render; missing frozen audio requires exact restoration or explicit unfreezing. Compatible old renders retain exact audio until an explicit upgrade or genuine input change. Source-rate/mapping incompatibility waits instead of silently resampling captured source.
5. Keep saved detection, marker Generate on Stop and legacy flags readable/unchanged. Their former ability to suppress existing render maintenance is an intentional behavioral change, documented and regression-tested. Changing a future Default IR does not reassign existing swells.
6. Missing-asset UI resolves dependencies and gives normal Learn guidance. Remove the superseded duplicate Storage controls/recovery renderer. Native modal behavior and bindings retain issue #8's review gate; global engine behavior is proven through ordinary and recovery inputs using the same tests.

## Risks / Trade-offs

- [More background DSP with marker detection off] -> changed-dependency-only reconciliation, coalescing, existing memory limits and no work for zero existing swells or identical input.
- [Legacy off preferences previously prevented maintenance] -> preserve serialized values/identities and explain the intentional ownership change; no implicit marker or capture admission.
- [Full wet identity differs from UI shape] -> test both zero-convolution shaping and genuine support extension; preserve existing timeline semantics.
- [Restoration accidentally changes sound] -> exact compatible render reuse and rerun the separate-profile zero-work checks before delivery.
- [Stale publication or reader races] -> retain all session/ID/input-generation/fingerprint guards and audio acknowledgement pins.

## Migration Plan

First approve the architecture review. Apply this change through the existing workflow, replacing the recovery-only experiment rather than layering it. Update focused capture/marker/engine/persistence/UI tests together; validate ordinary and missing-asset flows and final native Release. Keep issue #8 task 3.10 open until the modal warning, common-engine recovery and native review are verified. Preserve pre-change binaries/projects/assets for rollback; no data migration or canonical sync/archive is part of this proposal.

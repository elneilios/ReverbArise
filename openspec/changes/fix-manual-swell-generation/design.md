# Design

## Context

See proposal.md for motivation and the automatic-swell-processing delta for acceptance. At inspected revision 939f656, `SwellEngine::addTarget` sets Stale but only inserts `forceRefresh` when automaticProcessing or autoRegenerate is enabled. The accepted input-edit path has the same gate. `dispatchAutomatic` already accepts explicit forced requests independently of the master and dry-only capture policy, subject to timing, frozen-state, IR and coverage checks. Existing tests in swell_engine_test explicitly call generate after insertion; its writePackedIR helper and wavpack_ir_test cover real WavPack decoding but do not prove this reported manual-only workflow.

`PluginInspector::refreshValues` initializes bounds a/b to zero, computes them only with a raw render, then calculates arrival-a and b-arrival even when shaped is false. This explains how an unprepared target can display song-position-sized values; it is a code-level explanation, not reproduction of the owner's exact 77.705-second screenshot.

## Goals / Non-Goals

**Goals:** Route explicit manual intent through existing bounded background dependency checks and keep feedback consistent with actual selected render availability.

**Non-Goals:** Change onset detection, source replacement, automatic backfill, transport playback, convolution algorithms, default IR semantics or #38's future candidate workflow. No broad control extraction or redesign.

## Decisions

1. Accepted creation and render-affecting Perform edits enqueue explicit work regardless of automatic flags. Reuse forceRefresh and the worker's existing edit/session fencing, debounce and minimum-work reuse. Audit accepted IR import completion as well as arrival/source changes so asynchronous assignment also queues the intended ID. Requests must survive waiting for coverage without turning unrelated swells or analysis on. Enabling the master would violate saved user intent; adding a Generate control would add UI scope where the existing worker supports direct intent.
2. Preserve dry-only Learn source notifications and workflow admission. Only accepted manual commands grant this preparation. Retain frozen, timing, source integrity, memory and immutable publication validation. New preparation must never start audition automatically or put DSP/I/O on the callback.
3. Keep existing selected-swell KnobValue controls, notice/status presentation, semantic roles and geometry. Gate derived pre-roll/end-offset values on valid selected wet bounds and show the existing unavailable representation when absent. Do not expose zero-origin fallback as a measured value. Reuse existing caption/help/state policies; any new control family, token or visual exception needs explicit owner review before rollout. Run UI standards and binding tests; if appearance/interaction changes, launch one prepared native preview for owner review before wider use.
4. Extend engine and processor fixtures using committed non-silent source at sample zero and late-song coordinates, with master/markers/legacy refresh all off. Exercise valid WAV and encoded .wv default/selected import, eventual readiness and non-zero audition output; assert flags and detector counts stay off. Cover edits, gaps, missing/unsupported IR, session/deletion races and unchanged dependency reuse through focused existing suites.

## Risks / Trade-offs

- Explicit edits now trigger preparation in manual mode → coalesce work, preserve minimum-work shape semantics and assert unaffected swells do not reconvolve.
- Existing tests may depend on insertion staying stale → replace incidental assumptions with explicit manual-intent and dry-only admission assertions, retaining safety coverage.
- UI values may belong to another selection or old dependency → verify identity and validity at bindings and test stale/no-render/selection transitions.
- Exact owner IR/source are unavailable → verify supported imported .wv fixtures and record the difference between code findings, deterministic reproduction and owner host observations.
- Concurrent #38/#53 work may overlap → operate only in this issue worktree and reconcile latest main before implementation without modifying other worktrees.

## Migration Plan

No persisted format or identity migration. Deliver an issue branch and separate PR for owner merge. Roll back the scoped scheduling/binding change without deleting source or immutable assets. Record native wrapper results separately from fixture checks and untested host/DPI coverage in dated docs/validation evidence.

## Open Questions

The owner's exact IR duration/audio and screenshot timing remain unavailable; implementation will establish the deterministic fallback case and record any additional independent timing cause rather than infer it from the reported number.

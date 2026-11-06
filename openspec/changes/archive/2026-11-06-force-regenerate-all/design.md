# Design

## Context

See proposal.md for the reported no-op and the owner's requested force semantics. The current engine uses an `all` flag: Selected queues selected unfrozen swells; All excludes current Ready renders. Both already share forced-refresh, input-generation, background rendering and guarded publication. The existing source-edge test explicitly expects All to skip current work. The action is exposed on the Settings Recovery surface, whose existing static helper is far from the rendering buttons.

## Goals / Non-Goals

**Goals:** use the existing forced-render path for every eligible All target; return truthful admission counts to its UI binding; show clear feedback beside the action using existing helper styling.

**Non-Goals:** change convolution, calibration, automatic dependency invalidation, passive restore, frozen ownership, source capture, memory policy or selected-swell command semantics. No extra force toggle or new styling family.

## Decisions

### Force is an explicit command property

Remove All's current-conditioning/Ready exemption. Preserve confirmed-target scope, frozen/import guards, creative-mode admission, input-generation increments and the existing forced-refresh queue. Disabled/muted eligible definitions remain included. Reuse background execution and stale-result rejection rather than creating a second renderer. A label-only explanation was rejected because the owner explicitly wants fresh renders.

### The engine reports command admission

Return a small nonpersistent regeneration result containing accepted/rejected status and queued/skipped counts. Compute it under the same ownership/lock as admission, rather than inferring eligibility again from a possibly stale UI snapshot. Existing callers may ignore the result; Selected retains its behavior. Queued means accepted for work, not successful or complete audio. Waiting/missing dependencies and render failures retain their established per-swell status.

### Feedback belongs beside the command

Reuse existing Label/helper roles for a result row immediately below the Recovery regeneration buttons. Retain ordinary recovery guidance and higher-priority notices. Shift only the following rows by a named feedback-band allocation and verify the complete page in the existing Settings viewport at 100%/125%. Add tooltip/context help explaining that All forces eligible fresh renders and skips frozen/importing targets. Clear obsolete feedback on session replacement; repeated commands replace the previous result. Do not synthesize speech or write feedback into the musical session.

## Risks / Trade-offs

- More work for an explicit All click → retain the bounded worker/memory policy and latest-generation publication guards; never render on the audio callback.
- Fast renders can finish before polling → retain the immediate command summary independently of transient queue progress; never promise completion early.
- A skipped batch can still appear unresponsive → explicitly report no eligible targets and the frozen/importing or mode reason.
- Failed/superseded work → preserve previous complete assets and source, with existing readiness/failure semantics.
- Extra feedback row → reuse named geometry and the existing auxiliary overflow/focus policy; check bounds at both supported scales.

## Migration Plan

Update the two existing regeneration requirements, their current-processing regression expectations and the UI binding together. Extend mixed-batch, repeated-force, Selected-isolation, frozen/import, source-failure and supersession coverage. Update the source-edge guide and offline guide. Run focused native checks and the supported Release gate before a review-ready PR. No saved data migration is needed; rollback restores the prior explicit-command policy without deleting assets.

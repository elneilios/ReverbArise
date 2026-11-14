# Design

## Context

`beginIRReplacementLocked` invalidates the interrupted render token and can leave the live definition in Checking. The worker skips ordinary rendering for a target owned by a pending replacement. The headroom pass can then wait on that Checking phase while the replacement waits for ordinary renders to publish. The source-edge test holds publication to exercise this interleaving.

## Goals / Non-Goals

**Goals:** Let both requests settle, keep their publication fences and preserve the original intent of all five failing targets.

**Non-Goals:** Change callback behavior, introduce a saved-state format, redesign existing controls or change packaging criteria.

## Decisions

- Use the verified accepted render as the contribution for a pending replacement during ordinary headroom calculation when its accepted dependencies still match. A superseded worker phase alone must not make the replacement target block unrelated work. Retain generation, source, Freeze and session guards and the existing staged commit. Removing the wait for every Checking swell could admit an incomplete mix.
- Update the processor fixture to Arm explicitly and still assert the exact dry PCM interval. Select Timing, Envelope and Sound separately for readability coverage and verify the fixed Reset strip. Exercise an unsupported creative state version above v14 while retaining the unchanged-state assertion after rejection.
- Distinguish recapture settlement from analysis dispatch in the list fixture. Verify the accepted source revision, changed render and preserved protected swell. Require an analysis count increment only in the scenario that owns an analysis request.
- Reproduce the Export drag-to-save transition with native peer events and record each counter and button transition. Repair fixture event ordering only if the production release and next-click behavior is correct. If the product mishandles stale pointer state, fix the existing binding without changing the control family.

## Risks / Trade-offs

- A retained render with incompatible dependencies must never enter the headroom mix. Exercise accepted-render retention, supersession and failed preparation using existing guarded paths.
- Native focus and pointer tests depend on desktop state. Run the failing targets in isolation and in the full suite under the repository build lock.
- If inspection identifies a new control contract or visual variant, take that decision through the existing owner review gate before adopting it.

## Migration Plan

The repair uses existing assets and state. Review on the issue branch, then rerun the normal alpha.4 packaging gate on the integrated candidate.

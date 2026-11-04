# Proposal

## Why

GitHub issue: [#38](https://github.com/elneilios/ReverbArise/issues/38). Real-song testing produced 61 committed swells without a practical sensitivity/refinement loop. Learn must let users inspect candidate markers before deliberately replacing automatic definitions, with the approved optional automatic Stop journey retained.

## What Changes

- Separate asynchronous candidate analysis from committed definitions and wet rendering; show provisional, updating and settled markers on the existing Learn dry timeline.
- Add Generate Swells and the persistent manual-placement helper beneath sensitivity, reusing shared controls and the approved two-column composition.
- On the initial successful generation handoff, select the earliest committed swell and reveal its lead-in/arrival through existing editor selection/navigation, without waiting for wet renders. Preserve restored and later editing positions; empty visits and unsuccessful generation do not consume the handoff.
- Use one guarded, atomic replacement transaction for explicit Generate and eligible Auto-generate on Stop; preserve manual/edited/frozen and outside-scope work, stable matched IDs, suppression and immutable assets.
- Default Auto-generate on Stop OFF for fresh projects; persist explicit values and restore ON for older state lacking the field. Enter Perform only after a nonempty changed generation commits, without waiting for wet rendering.
- Allow sensitivity refinement and current-default IR assignment on repeated Generate without recapture. Preserve accepted capture contexts as historical provenance, rather than allowing them to override explicit generation intent.
- Cover realistic dense and sparse arrangements, cancellation/races, compatibility, native UI acceptance and one consolidated host smoke.

## Capabilities

### New Capabilities

None; extend the existing ownership boundaries.

### Modified Capabilities

- `workflow-modes`: replace unconditional useful Stop entry with optional guarded generation and success-only Perform entry.
- `automatic-swell-processing`: candidate lifecycle, generation snapshot, full-coverage atomic reconciliation and current-default assignment.
- `swell-session`: persisted Auto-generate preference, conservative provenance migration and transient-job exclusion.
- `swell-workspace`: Learn candidate overlay, Generate/helper/preference bindings, accessible readiness and the initial chronological Perform selection/reveal.
- `song-timeline-audio`: keep accepted capture provenance immutable while distinguishing the current explicit generation snapshot from arrival-pass defaults.

## Impact

Targets `WorkflowState`, processor capture settlement, `AutomaticCoordinator`/`SwellEngine`, `AutomaticTargets`, `SwellSession` codecs and Learn/editor bindings. Existing onset detection, coverage readers, immutable asset handling and shared UI families remain the foundations. #11 is merged in this baseline; no redesign or changes to its archived evidence are required.

This deliberately revises #10/#11 Stop and pass-default behavior, including contradictory scenarios in the affected specifications and living workflow/source-context guidance. No VST3 identity or numeric mode migration, new DSP, Perform bulk editing (#9), cache cleanup/portability (#8), capture-failure fix (#54), wet-routing fix (#53), or unrelated waveform optimization. Coordinate shared seams with open Perform #46 and waveform work without absorbing their scope. Planning only; owner review and a subsequent Apply request are required before implementation.

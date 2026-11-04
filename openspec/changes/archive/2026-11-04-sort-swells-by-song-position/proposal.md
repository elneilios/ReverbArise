# Proposal

## Why

GitHub issue: [#4](https://github.com/elneilios/ReverbArise/issues/4).
The Perform Swells list follows session insertion order, so capturing an earlier song section after a later one puts its swells at the bottom. Chronological ordering must follow canonical target positions while selection and edits continue to follow persistent identities.

## What Changes

- Display Swells in ascending absolute `arrival` sample order, with persistent swell ID as the deterministic same-position tie-breaker.
- Derive a list-only projection from accepted model snapshots; names, wet placement and display labels never determine ordering.
- Update automatically after add, target movement, recapture/reanalysis and project restoration, preserving primary/multiple selection and row-action ownership by ID.
- Preserve the reader's scroll context on background changes and keep an actively moved/explicitly selected primary swell visible without jumping on routine refresh.
- Reuse existing row appearance and shared UI policies; consolidate affected list interaction/binding setup into a focused reusable list control and obtain native owner interaction approval before wider rollout.
- Cover numeric ordering, ties, arbitrary names, persistence, capture reconciliation, selection and scrolling with focused automated tests.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: chronological Swells presentation with identity-safe interactions and stable scrolling across model updates.

## Impact

Expected implementation touches the list bindings in `Source/Plugin/PluginEditor.cpp/.h`, a focused reusable control/projection under `Source/UI/Controls/`, and existing UI/session/automatic-target or processor tests. `Source/Core/SwellSession.h` already supplies `arrival`, `id`, selected IDs and primary ID; no new persistent coordinate, schema migration, dependency or host parameter is needed. Session vector order, captured source, render/IR assets, generation rules and DSP scheduling remain unchanged. Learn keeps its list hidden. This is independent of #3's larger workflow and #9's future bulk operations; `bound-waveform-paint-work` remains a separate active change. Planning only: Apply requires a new user request.

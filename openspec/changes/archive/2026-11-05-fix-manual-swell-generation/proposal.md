# Proposal

## Why

GitHub issue: #55 reports that a manually added Perform swell remains stale and cannot be auditioned after dry-only Learn capture. Explicit manual work needs a usable generation path even when automatic discovery and refresh are off.

## What Changes

- Treat accepted manual creation and render-affecting Perform edits as explicit background preparation requests, independently of automatic processing, marker and legacy refresh preferences.
- Preserve dry-only Learn admission, manual identity, immutable assets, edit/session guards and existing minimal-work shape handling.
- Show actionable preparation failures and unavailable timing without presenting an unprepared render as valid.
- Verify imported WavPack IRs and late-song timing in deterministic native/processor regressions, followed by one brief owner FL Studio check.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `automatic-swell-processing`: make explicit manual preparation independent of disabled automatic policy and require truthful readiness/timing feedback for this workflow.

## Impact

SwellEngine manual commands, AutomaticCoordinator admission, existing IR import/render workers, selected-swell timing/status bindings and focused engine/processor/UI tests. No new control family, styling variant, session schema, dependencies or broad UI redesign is planned. #38 candidate generation and #53 DAW transport playback remain separate; #46's merged Perform controls are reused. Exact owner IR/audio and screenshot are unavailable, so their reported timing cause remains unconfirmed until reproduced.

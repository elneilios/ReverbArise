# Proposal

## Why

GitHub issue #208 tracks five failing targets from the full alpha.4 Release run. A managed IR replacement can stall alongside bulk regeneration, and several validation fixtures still assume earlier capture, inspector or state-format behavior.

## What Changes

- Restore independent completion of a pending IR replacement and regeneration of other swells, preserving the accepted sound until handover.
- Update capture, inspector and state compatibility fixtures to exercise shipped behavior.
- Diagnose the recapture and native Export event failures separately and retain assertions for their intended outcomes.
- Verify the repaired tests before retrying the existing alpha.4 packaging gate.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-regeneration`: Define completion when an IR replacement and regeneration of other swells overlap.

## Impact

The change affects the background swell worker, Release fixtures and guide currency review. Preserve existing UI controls, saved-state formats and release gates.

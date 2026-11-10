# Proposal

## Why

Export can refresh its ready WAV before the pointer crosses the drag threshold. It also scans the prepared file again before starting an OS drag, delaying a gesture on an already verified WAV. P8 pins the file at invocation and uses background preparation as the readiness boundary for immediate drag. [Issue #107](https://github.com/elneilios/ReverbArise/issues/107)

## What Changes

- Bind Save WAV, the drag tile's Save action and external drag to the ready WAV, session and Export pass at invocation. Reject stale or superseded work without targeting a newer WAV.
- Hand the ready, background-verified WAV to Windows at the drag threshold, with no second full-file scan. Require the original press at handoff and suppress Save after a drag attempt.
- Show the approved WAV cursor on accepted targets and a stop sign on rejected targets through the native drag. Restore the dotted tile border after drop or cancellation.
- Retain the native Save chooser, overwrite confirmation, the existing drag tile treatment, and the current Save fallback for unsupported external drag.
- Start P8 with Export's file-delivery boundary. Learn and Perform canvas interactions, IR chooser coverage and auxiliary windows have their own P8 scope. P7 lists and scrolling remain separate under #107.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `song-export` defines how Save and external drag retain the current WAV and invocation identity across delayed callbacks.

## Impact

Export workspace bindings, the specialized file tile, the engine's ready-file lookup, and the Windows external drag adapter are affected. Keep Save on the guarded native chooser and worker. Reuse the existing background preparation, verification and immutable publication path for drag readiness.

# Proposal

## Why

A Loop saved in a host project needs to reopen with the same audio and swell edits, including after its asset folder moves or is restored from backup. Capture and playback commands must start fresh after reopening so a saved project cannot overwrite the take or start wet playback on its own.

GitHub issue: [#129](https://github.com/elneilios/ReverbArise/issues/129).

## What Changes

- Verify that a current-build save preserves the accepted Loop's source identity, revision, exact audio and timing, swell edits, and verified asset references independently of the next-capture preference.
- Verify that restore discards any pending take and command state, disables Recapture Audio, and starts live Loop playback in dry Standby. Require fresh timing and a fresh Cue before wet playback.
- Extend the existing Storage paths where needed so a current-build Loop project can locate a moved folder or restore a verified backup. Missing or invalid dependencies must remain visible and unavailable.
- Verify save, reopen, and recovery using fresh projects in VCV Rack and FL Studio, with a current-build Song project as a regression baseline. Legacy project migration is outside this issue's scope.

## Capabilities

### New Capabilities

### Modified Capabilities

- `loop-phrase-capture`: Preserve current-build Loop state across host saves while resetting transient recording and playback state.
- `project-asset-portability`: Specify exact Loop recovery through the existing asset verification and association flow.

## Impact

The affected paths are the processor's host-state save and restore, capture manifests and publication fences, Loop transport reset, and existing Storage snapshot and recovery flows. The plan uses managed audio references, existing VST3 identity, and existing Storage controls.

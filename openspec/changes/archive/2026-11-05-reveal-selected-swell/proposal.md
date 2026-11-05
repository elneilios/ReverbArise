# Proposal

## Why

GitHub issue: #93. Selecting a swell in Perform's chronological list can leave its editing region outside the waveform viewport. Explicit list navigation should frame the complete swell through the existing Fit behavior.

Owner review on 5 November 2026 supersedes the issue's original zoom-preservation acceptance criterion: after clicking and navigating, automatically apply the same pan/zoom as Fit so the entire swell region is in view.

## What Changes

- Apply the existing contextual Fit action to the accepted primary after explicit list selection, including re-clicking the selected row after manual navigation.
- Preserve stable-ID and absolute-time ownership, modifier-selection conventions, lane/ruler/handle/Overview synchronization and navigation persistence.
- Keep incidental refresh, render publication and mode return from triggering automatic Fit.
- Retain selection/enablement independence and existing creative/audio/asset state.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: Explicit list selection invokes complete-context Fit automatically; incidental selection refresh retains the view.

## Impact

The production list binding, focused list/Fit/editor-binding tests, living navigation guidance and offline guide currency. Reuse existing SwellList and complete-context timeline Fit with the accepted Zoom/Fit, ruler and Overview controls under production standards revision 5. No new visual variant, control, DSP, dependency or state schema. #9 bulk-editing work remains separate.

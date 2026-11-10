# Proposal

## Why

Learn, Export and Perform use different mouse gestures for similar waveform
actions. This P8 slice of [issue #107](https://github.com/elneilios/ReverbArise/issues/107)
adopts the shared interaction tested and approved in the native JUCE preview.

## What Changes

- On the main waveforms, left-click or drag positions the Learn inspection pin,
  the ready Export preview playhead or the local Perform playhead. A Perform
  handle takes precedence when pressed.
- The wheel zooms at the pointer. Pressing and dragging the middle button pans.
  Right-click and right-drag are reserved for a future context menu.
- The compact Perform Overview uses left-drag alone to pan the detail viewport.
  The visible range follows the drag without moving a playhead or handle.
- Use the shared keyboard/accessibility focus treatment. Preserve the existing
  keyboard actions, capture and preview eligibility, and guarded Perform edits.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace` specifies the shared pointer gestures, the per-viewer
  position or edit, and the drag-only Perform Overview.

## Impact

The affected controls are Learn’s inspection canvas, Export’s
`TimelineNavigation` and preview binding, and Perform’s `SwellTimeline` and
`SwellOverview`. The WAV Save/Drag slice and the remaining P8 native surfaces
and auxiliary windows are outside this change. DSP, host transport, persistent
formats and source/render/WAV identities stay outside the gesture layer.

Production edits require [PR #143](https://github.com/elneilios/ReverbArise/pull/143)
to have merged. It merged into `feature/loop-mode`, so apply must check the
intended base and account for the overlapping Learn and Perform changes.

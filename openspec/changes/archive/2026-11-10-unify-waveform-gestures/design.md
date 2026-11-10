# Design

## Context

The production baseline is standards revision 23. The approved waveform slice
adopts revision 24 with the existing geometry, colour and text tokens.

Learn's `LearnWorkspace` owns read-only inspection and a capture-driven
`LearnDryView`. Export's `TimelineNavigation` owns bounded zoom and pan, while
`ExportWorkspace` authorizes seeks against the prepared preview identity.
Perform's `SwellTimeline` owns viewport and guarded edits, and its compact
`SwellOverview` currently recentres the detail on a click. The approved JUCE
preview compared all three with sample data and exercised the Overview exception.

## Goals / Non-Goals

Share mouse intent and pointer-anchored zoom across the main waveforms. Keep
domain geometry, sample mapping, bounds and actions with their current owners.
Retain the existing waveform paint pipeline, view preference schema, selection
Fit, and Export file workflow. Leave context menus, other P8 surfaces and Loop
feature delivery in their own scope.

## Decisions

| Decision | Rationale and alternatives |
| --- | --- |
| Share a small input policy for button and wheel interpretation. Reuse `SwellViewport` for bounded coordinate math. | A common viewer class would combine unrelated responsibilities. Leaving button interpretation entirely separate risks another divergence. Retire legacy mouse mappings in the affected viewers while keeping the shared Zoom/Fit controls and keyboard commands. |
| Fix the gesture owner at pointer-down and clear it on release or invalidation. | Hit-testing again during movement could switch from a handle edit to a playhead or vice versa. Preserve Perform's handle drag baseline, selection/epoch/clock guards and Ctrl/Shift policy. Existing intentional Perform double-click commands remain left-button-only and guarded. Export Fit remains on its existing control and keyboard shortcut. |
| Reuse accepted bounds and readiness at each action. | Learn retains its capture-driven full extent separately from the current view, so a manual zoom can survive routine refresh and coverage classification can use absolute samples. Export positioning continues through the existing ready-identity callback. Perform uses a display-only absolute playhead separate from the DAW and audition cursors. |
| Pan the Perform Overview by the drag delta in full-song coordinates from its down-time viewport. | Click-to-recentre would jump the visible range on press. Relative dragging lets the highlighted range follow the pointer while preserving zoom. The detail waveform keeps middle-drag at its own scale. |
| Reuse `FocusVisibility`, aqua corner brackets and existing cursor and help roles. | This applies the approved family focus policy without introducing a visual token. Keep handle hover/held feedback separate from focus. The Overview's pointer cue and contextual help identify its left-drag exception. |

## Risks / Trade-offs

- Refresh could refit a manual Learn view, rebase a drag unexpectedly or report
  the wrong sparse coverage at a zoom boundary. Keep accepted extent and view
  separate, clamp on bound changes, and exercise refresh during active capture.
- A previous Export waveform is still navigable. The ready-identity guard must
  continue to gate positioning, including when readiness changes mid-gesture.
- PR #143 merged into `feature/loop-mode` at `fa1153d`. Compare the affected
  Learn layout and Perform Loop overlays before editing production. Follow the
  normal mainline workflow, reconcile against the actual implementation base,
  and keep the Loop behavior intact when the changes meet. Do not pull Loop
  implementation into this change to satisfy the merge prerequisite.

## Migration Plan

Verify the dependency and base first, then migrate the four production surfaces
together. Keep existing persisted navigation data and user settings. Rollback is
a source revert with no project-data conversion.

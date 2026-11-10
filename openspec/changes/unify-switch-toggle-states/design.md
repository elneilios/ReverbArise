# Design

## Context

The [C6 contract](../../../docs/development/ui-control-contracts.md#c6--switches-and-toggles) and [P6 inventory](../../../docs/design/ui-control-inventory.md#delivery-packets) define the family. Learn already uses a track and thumb. Perform and Settings still use theme-drawn checkboxes, with some bindings using an invocation hook to capture a workflow pass. On 9 November 2026 the owner approved the standalone Windows comparison and chose switches where there is space, with inline checkboxes for the three Swell Inspector flags.

## Goals / Non-Goals

**Goals.** Reuse one boolean interaction contract and two explicit presentations. Keep the inspector flags on one row and preserve the existing Learn and Settings edit paths.

**Non-goals.** Command latches, Swells list hit regions, auxiliary-window layout redesign, audio or persistence policy, and recovery work tracked by #8. The sample Include dry source option illustrated a staged choice in the preview.

## Decisions

### Choose the control form at composition time

Keep the Learn switch footprint of 44 × 24 for the track and 56 × 36 with focus clearance. Start with the existing 18-unit checkbox indicator for the compact inspector. Use existing spacing, text roles and a shared focus painter. Keep the inspector's current row and its fixed Reset placement. Associate a switch with its existing caption and any help in Learn or Settings. Give the caption and indicator one input target and one Tab stop.

| Inventory | Planned disposition |
| --- | --- |
| L4 · Learn | Keep switches for Recapture existing audio, Enable automatic swells and Auto-generate on Stop. Preserve the Learn pass rules, including the allowed turn-off path where applicable. |
| P4 · Swell Inspector | Keep Enabled, Solo and Freeze as inline checkboxes. Preserve the existing selected-swell flag edits, Freeze readiness and retained render behavior. |
| S4 · Settings | Adopt labeled switches for Live playback, Map while playing, Mute, Isolate selected, Isolate dry source and the 440 Hz test tone. Keep each binding's existing eligible state and effects. |
| S5 · Settings Processing | Adopt labeled switches for Automatic processing and Auto-refresh after recapture. Preserve their existing invocation hooks and pass checks. |
| D3 · Retained seams | Keep the hidden Exact sample positions, Auto markers and old Recapture control in their existing compatibility roles. Record this disposition in the inventory. |

### Share interaction state and keep binding authority

Extend the existing switch and invocation-toggle mechanisms where they fit. Use a shared state resolver for the track and checkbox indicator, with thumb position or a checkmark carrying the accepted value. Apply hover and held feedback to the indicator's border and background. Keep the focus brackets outside that surface and use the existing keyboard, accessibility and pointer provenance policy. Derive enabled and unavailable styling from named production colours. Review any new token or one-off variant in the scoped native checkpoint.

Keep silent model-to-control refresh and route activation through the existing domain callbacks. Preserve keyboard Return where it already works, including Learn, and exercise Space, label clicks and accessibility through the same eligible path. A pass-aware hook may still capture before an unavailable request is rejected. Preserve selected-swell identity and session authority for any deferred dispatch, following the existing captured-context patterns. Retain tooltip or context help and an accessible description for unavailable controls.

### Review the shared JUCE controls before wider adoption

Use one native fixture with the approved comparison as its reference. Include a Learn row, representative Settings switches and the compact Enabled/Solo/Freeze row. Show on and off, hover and held feedback, keyboard focus, an unavailable on state and long label/help. Check the minimum 900 × 600 editor, a larger size and 100%/125% UI scale. Open that fixture for the owner before connecting it to the production consumers. Keep native owner feedback, software-scale checks, physical DPI and FL Studio coverage separately recorded.

## Risks / Trade-offs

- A larger switch footprint could crowd the Settings processing and audition rows. Check captions, neighbors, focus clearance and clickable area in the actual layouts.
- Updating styling can accidentally create extra Tab stops or duplicate invocation through label and indicator. Verify one activation per gesture and unchanged silent refresh.
- Unavailable checked controls need readable state without appearing actionable. Verify the on and off presentations, including dry isolation during wet-only audition and Freeze eligibility.

## Migration Plan

Build and test the shared controls in the native fixture, record the owner checkpoint, then adopt them in Learn, Perform and Settings. Reconcile the inventory, style revision and user guide. Existing saved state requires no migration.

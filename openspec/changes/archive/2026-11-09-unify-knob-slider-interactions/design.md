# Design

## Context

The [C5 contract](../../../../docs/development/ui-control-contracts.md#c5--knobs-and-sliders) and [P5 inventory](../../../../docs/design/ui-control-inventory.md#delivery-packets) define the family boundary. Main includes #108 Level and P4 through PR #134. The owner approved the P5 native dial and track preview, then approved the revised Level detent on 9 November 2026. The fixture exercised sample values in a standalone JUCE window.

## Goals / Non-Goals

**Goals:** Share dial and track presentation and fine-wheel routing. Keep domain conversions, edit authority and compatibility with the existing bindings.

**Non-goals:** Changes to audio gain law, DSP, saved state, IR handling, viewport behavior outside the zoom controls, or P4 field submission. Recovery and engine work under #8 has its own scope.

## Decisions

### Reuse the existing gesture owners

Use a shared painter and explicit dial or track roles at the existing `KnobValue`, `SliderControl` and `TimelineZoomControls` entry points. The painter reads the accepted value and uses the shared focus-provenance policy for brackets. Dial hover and held feedback follow the full sweep of the value arc, with the centre and the gap between its ends clear. Slider pointer feedback follows the track body. The active track or arc stays aqua and the position indicator stays off-white. Dim the value ink and indicator together when unavailable. Preserve the preview's neutral-centered arc for bipolar dials and the independent P4 focus for a neighboring value field. Resolve the track bounds from its actual layout so neither paint nor wheel input overlaps the numeric field.

### Keep companion dial fields the same width

Use the existing 74-unit value width for every dial's exact-entry field. Cap it to the available space in narrow columns. Align inline feedback under the field and let it wrap within the value column. Rely on Target's existing hover-help footer and accessible field description for its format hint so the idle Timing row stays compact. Keep Target validation inline. Exact-entry fields retain their current editing behavior.

### Map wheel steps in the owning domain

Route wheel deltas through the existing invocation guards and value conversions. Honor direction and sign, clamp at limits and retain ordinary scrolling outside the dial or track. Tracks use one declared coarse or fine step per event. Unsnapped dials retain proportional travel for fractional wheel input. Keep the current keyboard, accessibility, stepper and drag routes. Ctrl has no drag-speed role on continuous tracks. Existing timing Ctrl-drag Snap bypass and Shift fine adjustment keep their meanings.

| Consumer | P5 disposition and wheel policy |
| --- | --- |
| Perform Level and ten other inspector dials (P7) | Use the shared dial body and focus. Level uses 1/0.1 dB on plain/Ctrl+wheel, with its existing Shift fine path. Snapped timing steps to adjacent grid points. Ctrl+wheel uses the existing unsnapped sample-coordinate step for that event, equivalent to 1 ms in the Move Wet preview. Preserve the finer Ctrl+Shift path where already supported. Curves use their existing coarse and fine tension increments. |
| Learn Event sensitivity (L6), Export Dry/Wet (E5) | Use the shared track body. Plain/Ctrl+wheel steps are 1/0.1 percentage points. Preserve captured Learn pass, the untouched Export level and accepted Character derivation. |
| Settings Live Level, Separation, Tail, Audition trim, and test tone Start/End (S8) | Adopt the compact shared track. Start with 1/0.1 dB for Live Level and Audition trim, 10/1 ms for Separation, and 0.1/0.01 s for Tail. The fine increments match the existing intervals. Use one whole-bar step for the discrete tone endpoints. Keep range, suffix, compact value boxes, and the existing pass and reset policies. Verify these Settings defaults in the integrated component check. |
| Perform and Export horizontal zoom (P6/E4) | Adopt the text-box-free track with its own focus. Start with the preview's 5/0.5 percentage points of the normalized zoom position and verify it against real viewport conversions and limits. Keep Fit, anchor, plus/minus and captured navigation semantics. |
| Timeline Height (N4) | Use the shared track with the preview's 3/1 dB wheel steps. Keep the existing 0 dB reset, range and display-only amplitude scale. |
| Retained sensitivity knobs (D6) | Keep the hidden and zero-size compatibility paths and their saved values. Record them as retained seams, with no new visible controls. |

### Apply the home detent only to raw drag candidates

Give the dial an optional configuration with a home value and attraction band in native units. Configure it for Level at 0 dB with a ±0.3 dB band and quiet tick at 12 o'clock. Keep the drag accumulator tied to the unsnapped pointer candidate so the user can pass through neutral in the same gesture. Apply the result through the existing Level edit and publication path. Keep the home option independent of the reset provider, and leave it unset on all other dials in P5.

### Verify the real composition against the approved preview

Use the reviewed fixture as the appearance and feel reference. Exercise the production component on real bindings before broad adoption, at 900 × 600 and a larger size with 100% and 125% UI scale. Preserve that evidence and take another owner checkpoint if integration materially changes the approved look or interaction. Record host and physical DPI coverage separately.

## Risks / Trade-offs

- Feeding the snapped result back into drag movement could trap the pointer at neutral. Test entry and exit on both sides, fine drag, reversal and modifier rebasing.
- Applying one numeric increment to all domains could skip a grid point, emit a no-op after rounding or introduce a fractional bar. Use each binding's legal range and step, and verify behavior at limits.
- Local slider themes and nested controls can shift focus or hit bounds at compact sizes. Check real track, field and stepper regions and preserve the fixed inspector actions.
- The old Level first-touch rule can differ from an ordinary new edit. Retain exact legacy values during display and refresh, and test first touch, detent, Reset and reopen independently.

## Migration Plan

Land the shared surfaces and gesture policies with focused coverage, then adopt them by consumer group within P5. Reconcile the full inventory, update living standards and the user guide, and run the Windows Release suite. There is no saved-state migration.

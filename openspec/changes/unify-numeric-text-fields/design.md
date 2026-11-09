# Design

## Context

The [C4 contract](../../../docs/development/ui-control-contracts.md#c4--numeric-and-text-fields) records separate lifecycles for persistent editors, temporary slider editors and read-only values. The [control inventory](../../../docs/design/ui-control-inventory.md#delivery-packets) provides the consumer IDs below. Production standards revision 17 and merged #122 provide the approved C3 metrics, error treatment, focus policy and Settings source-range reference.

## Goals / Non-Goals

**Goals:** Use shared presentation and role-specific editing adapters across all P4 consumers. Keep a feedback position and lifecycle decision for every inventory row, including retained or hidden controls.

**Non-goals:** P5 owns changes to dial and slider gestures, ranges and track appearance. P2 owns choice selectors. The established audio, session and domain validation contracts remain the authority for fields.

## Decisions

### Cover the entire family

| Consumer | Entry and existing binding to preserve | Feedback position and planned disposition |
| --- | --- | --- |
| Learn Event sensitivity (L6) | `LearnWorkspace::sensitivity` through `SliderControl` and `NumericInputControl`. Preserve percent formatting, precision, steps and captured Learn pass. | Compact value box uses the shared embedded field presentation. Keep the existing context help and accessible units. |
| Perform inspector (P7) | All 11 `KnobValue::Entry` instances. Target, Source Start, Source End, Pre-roll, End Offset, Fade In, Fade Out, Level, Curve In, Curve Out and Move Wet. Retain exact edit text, Return and typing focus-loss commit, Escape cancellation and identity guards. | Share a position under the affected field or its Timing, Envelope or Sound group as demonstrated in the native preview. Display the binding's existing actionable rejection and mark the implicated entry. Verify room for adjacent controls and the fixed Reset actions. |
| Export Dry and Wet (E5) | Both `ExportLevelsControl` embedded `SliderControl` entries and their step buttons. Preserve independent level edits, the accepted pair and derived Character. | Compact embedded entries use the shared variant and existing units and context help. |
| Settings numeric entries (S8) | Live Level, Separation, Tail, Audition trim, test tone Start and End. Retain each slider's editable value lifecycle, suffix, interval, range, relevant reset and invocation guard. | Standardize the six value boxes. Keep applicable existing page or control help associated with its input. Compact value boxes use context help and accessible descriptions where their row has no local position. |
| Settings bar range (S9) | `barStartEditor` and `barEndEditor`. Keep the accepted digits-only entry and explicit paired Return or button submission. | Existing shared adoption. Verify the caption and wrapped group message, rejection, correction and unavailable guidance as regressions. |
| Diagnostics exact values (D5) | Arrival, Source A/B, Audible A/B, Fade In/Out, Move Wet placement and the hidden Name seam. Retain precision, conditional visibility and read-only access. | Use a shared read-only variant for exposed values. Associate existing captions and units. Record the hidden Name seam without exposing an editable field. |
| Retained sensitivity values (D6) | Legacy marker and regeneration sensitivity boxes. Verify their current zero-size, hidden or disabled states and their saved preferences. | Inventory as retained seams. Preserve exposure and document the disposition explicitly. |
| Rename dialog (N1) | The Name entry and explicit OK/Return and Cancel/Escape paths. Preserve the captured swell ID and session epoch and the existing nonempty rule. | Add the shared feedback position below Name in the dialog. An empty OK or Return leaves the dialog open with the local reason, allowing correction or cancellation. Reuse reviewed command actions where required for this dialog. |
| Timeline Height (N4) | `SwellTimeline::waveZoom` editable dB box. Preserve its range, steps, reset and display-only effect. | Adopt the shared embedded value role and retain display scope help. The track and timeline remain under their existing ownership. |

The creation path audit covers the persistent editors in `PluginEditor`, the generated labels and their temporary editors in `SliderControl` and the Settings slider setup, the eleven configured `KnobValue` cells, the Rename alert's Name editor and `SwellTimeline::waveZoom`. The legacy sensitivity sliders also create text boxes, but the standard layouts hide or zero-size them. Record adoption, existing shared coverage or a named owner-approved specialization for each instance or equivalent set. Window size and UI scale are choice controls. Timeline navigation zoom sliders are configured without text boxes.

### Share appearance while preserving editing mechanisms

Extend the approved `FieldPresentation`, `FieldFeedbackLabel` and focus policy. Reuse the native text, selection and accessibility interfaces in persistent fields, `KnobValue::Entry` and slider text boxes. A reusable adapter may share paint and metadata without replacing each editor lifecycle. A universal text editor would discard behavior that differs between a temporary slider editor, a paired command and the inspector. Keep step buttons and wheel, fine-step and drag gestures attached to their existing owner.

Route existing binding outcomes to feedback, using the current validation and command path. Maintain accepted precision and each subtype's draft reconciliation. Keep accessible descriptions tied to the actual input or group. On repeated refresh, publish only a changed description. Clear stale local errors after accepted correction or context replacement and keep ordinary help readable when unavailable. A field without a local position retains the existing context help and accessibility path. Layout and copy for the proposed inspector and Rename positions are part of the native review.

The owner approved blocking letters in numeric drafts, including paste, and using the existing musical-position parser for bar.beat.fraction. Numeric boxes with steppers support one configured increment or decrement per wheel event over the value or arrows. The standalone stepper variant is available to future consumers. Settings and Height retain their existing compact boxes without arrows. The Settings bar pair and Rename validate on focus loss and still submit through their explicit actions or Return.

The native fixture also records the owner's P5 direction for slider tracks: Ctrl+wheel fine adjustment and ordinary drag with or without Ctrl. P4 adopts the field and stepper behavior only.

### One native preview for the family

Open one native window directly for the owner before any production adoption. Provide scenes for inspector timing and Level, a compact slider value and its stepper, the Settings bar pair, a read-only Diagnostics value and Rename. Exercise keyboard, pointer and accessibility focus, exact edits, cancel, unavailable and read-only states, empty Rename recovery and long wrapping. Show the minimum 900 × 600 at 100%, a larger view and supported 125% UI scale. Use the approved preview as the gate for rollout and document any scoped variant or new metric with its evidence.

## Risks / Trade-offs

- A label can create a temporary editor on demand. Exercise both the resting value and the live editor, including focus changes and accessible increments, to catch a mismatch that a static preview misses.
- Inspector sections are compact. Check wrapping, scroll and focus reveal at supported sizes, including the fixed Reset area. Resolve any new geometry in the native review.
- A common visual policy can hide incompatible value semantics. Verify independent Export levels, older saved Level values, captured edit guards and unchanged focus or refresh against real bindings.

## Migration Plan

Create the preview and shared field variants first. After owner approval, adopt them by consumer group within this change and reconcile the complete inventory before closing P4. Verify the existing source-range pair and IR composition as regression references. Update the living inventory, standards and user guide where behavior changed, and record Windows native, host and physical DPI coverage separately. The change requires no saved-state migration.

# Design

## Context

The [C3 contract](../../../docs/development/ui-control-contracts.md#c3--labels-context-help-notices-and-field-feedback) identifies shared field anatomy as the first text and feedback slice. `FieldPresentation` already provides approved IR feedback metrics and colors. The IR field also has its own reviewed height policy; adopting new wrapping in Settings does not require changing that policy or its import button.

The Settings Swell page lays out the two bar editors with side labels and fixed space below. Return in either editor and the Set source from bars button call the same editor command. The processor owns timing and range resolution and sends rejections through existing diagnostics. Settings needs the result of that command to show local feedback accurately.

## Goals / Non-Goals

**Goals:** Share measurement and layout for a single body or a pair. Put the source-range command result beside the existing inputs and make the full explanation available through their accessible descriptions.

**Non-Goals:** A universal text editor or field framework, changes to IR behavior, or migration of footer help, notices, transport readouts and unrelated Settings labels. C4 still owns editor lifecycle changes. No new validation, session state or host behavior.

## Decisions

1. **Measure and render the same wrapped text at the actual input width.** Extend the existing field presentation policy using its 12-height Inter Regular helper, 17-unit line box, 4-unit gap and existing ink roles. The host provides the visible body bounds or the union of the pair's bodies; the action is outside those bounds. Use one wrap result for measurement and paint, breaking at word boundaries when possible and wrapping long unbroken text when needed. A fixed line cap would hide the actionable part of a message, and estimating line count from total glyph width can undercount word wrapping.
2. **Let the Swell page use the measured height.** Keep the pair's group heading and put each caption immediately above its own input. Anchor the message under the pair and flow the controls below it from the measured bottom. Normal guidance explains the exclusive end bar; rejection takes its place. A frozen or otherwise unavailable action can have readable local guidance without changing the enabled state of either editor. Keep the existing Settings page fit and tab order; avoid using the dialog or button width to gain room for the message.
3. **Use the existing command as the source of truth.** Return and the action continue through the same path with their existing behavior. If needed, return a scoped accepted/rejected result and reason from that path while retaining existing diagnostics and callers. Use a reason or affected-field indication from the command boundary, never string matching, a duplicate parser or the last global status. A range error marks the implicated input or pair; a rejection due to unavailable host timing can be explained without blaming an individual input. A successful correction or replacement of the affected context clears local feedback. An unchanged refresh does not replay events or copy stale errors to a new selection. Presentation adds no live validation, new eligibility check, implicit commit, or change to accepted state or edit reconciliation.
4. **Keep focus, field state and assistive text separate.** Use the approved red body treatment for implicated inputs and retain the existing shared aqua outer brackets for keyboard or explicit accessibility focus, with pointer-only focus decoration suppressed for the scoped composition. Preserve caret and text selection behavior. Associate each caption with its editor and the group message with the relevant inputs. Use existing accessibility notifications when descriptions change; no extra live announcement or tooltip-only error.

## Risks / Trade-offs

- A message can span more lines at 125% or with a long token. Exercise measurement, paint and reflow together at supported sizes and verify the actions below the pair remain visible and reachable.
- Settings shares a command with other paths, including initialization. Keep feedback scoped to the Settings submission and retain the existing diagnostic behavior, selection effects and return handling for other callers.
- A shared helper could change the already-reviewed IR row accidentally. Keep the selector and adjacent folder action as regression controls; leave its existing layout policy in place.

## Migration Plan

Build a lightweight native preview with a single field and the Settings pair, showing normal, long, rejected, cleared and unavailable states. Launch it for scoped owner approval before integrating the pair into Settings. After approval, adopt the shared composition in that one production group and record the reviewed behavior and remaining C3 inventory. The change has no persisted data to migrate; rollback is a code revert of this adopter and its scoped presentation.

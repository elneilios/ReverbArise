# Design

## Context

`PluginEditor` owns `chosenCandidate` as a sample position. Refresh currently rebuilds the ComboBox from viewport-visible suggestions plus the chosen sample. The source lane can also choose a sample. `confirmButton` invokes the existing engine command separately.

## Goals / Non-Goals

Reuse C2 presentation in the Recovery page while preserving its existing sample selection and confirmation flow. The change is scoped to this field and its binding, including the source lane's selection callback.

## Decisions

- Use `ChoiceSelector` and its existing popup, focus, accessibility and no-wheel policy. Use the existing body geometry and Settings layout tokens for the outside focus brackets. Retain the Onset suggestions caption and expose the current empty guidance in visible and accessible form.
- Use the sample position as the domain identity. Maintain a bounded displayed ID-to-sample mapping that can represent the full sample-position range. Preserve the viewport admission rule and selected out-of-view sample. Resolve source-lane and dropdown choices through that same identity and update the field silently.
- Compare rows and accepted selection before replacing the selector contents. `ChoiceSelector::clear` dismisses the popup, so unconditional refresh would disrupt inspection. A changed mapping or accepted selection invalidates the old popup. Capture the source/session context at invocation and validate it and the current sample mapping at completion, including changes that arrive before editor refresh. Use existing lifetime and refresh guards.
- Reuse the existing Confirm command and eligibility path. Automated native checks cover the Settings integration and the shared selector's popup states. The owner authorized adoption using the existing dropdown approval.

## Risks / Trade-offs

- Row IDs can be reused after reorder or source replacement. Exercise both an equivalent refresh and a stale invocation that would otherwise point at a different sample.
- Layout margins or scroll routing can hide focus decoration or absorb wheel input. Verify the native Recovery page at compact bounds and supported user scales.

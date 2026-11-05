# Design

## Context

See proposal.md for motivation and the swell-workspace delta for the contract. `PluginEditor.cpp` creates `hostInfo` as a centred JUCE Label, assigns the fixed `PerformLayout::info` region above context help, and rebuilds one combined string on refresh. The component bounds already stay fixed; centring variable-width combined text moves the visible group and internal separators. There is no separate production red highlight to reproduce from the issue's annotation.

`PerformText.h` owns display-only rounding (three fractional beat digits, two tempo decimals with trailing zeros removed); `MusicalPosition.h` owns exact musical conversion. Existing `PerformReadabilityCases.h` and `swell_ui_test.cpp` inspect the label and unavailable strings. The editor supports a 1100x850 logical minimum; the production guide owns Inter typography and forbids solving density by shrinking fonts. Host timing has no small product-wide upper bar/tempo/meter limit, so a finite visual envelope must not become a new host-domain clamp.

## Goals / Non-Goals

**Goals:** Keep group and field geometry independent of refresh values, preserve truthful formatting, and make size/overflow policy measurable in a focused reusable component.

**Non-Goals:** Changing transport interpretation, core musical conversion, precision of stored values, DSP, other workspace layouts, or extracting unrelated status controls.

## Decisions

1. **A specialized read-only `TransportReadout` under `Source/UI/Controls/`.** It owns stable slot/separator layout, painting, static-text accessibility and context-help metadata. A presentation structure carries independent bar, beat, tempo and meter strings plus availability and full description. The editor binds existing snapshots and retains the stable `hostInfo` component name and mode visibility lifecycle. Reuse theme font resolution, semantic ink and existing spacing/Perform band policies. `HeaderInformation` is a header accent primitive, not an appropriate footer layout owner. Separate raw editor labels would scatter policy; padding one proportional-font string or switching fonts would not establish reliable boundaries.

2. **Geometry derives from font metrics and fixed display budgets, never live values.** Reserve labels, units, three separators, spacing and value boxes, centre the resulting complete group once within the info band, and right-align numeric values. Recompute only for component bounds or font/theme changes. Use worst-case digit widths in embedded Inter so proportional digits cannot exceed budget. Keep bar and beat distinct; a centred position substring still shifts its internal separator. No new rectangle, highlight, color or typography variant is proposed. Owner preview may refine slot spacing within existing policies.

3. **Explicit normal display envelope and bounded overflow.** Measure negative/six-digit bars, three-digit beats plus three decimals, four-digit BPM plus two decimals and three-digit meter parts, along with full unavailable copy. This is a GUI sizing budget, not a host/model limit. Remove optional spacing only through a deterministic width policy chosen from container width; do not shrink fonts or size slots from observed playback values. For exceptional values beyond the tested envelope, retain all exact formatted text in accessibility/context help and show bounded ellipsis/overflow within the affected slot. Preserve current rounding, independent availability and exact musical-position help. Tests must establish that the chosen budget fits at minimum width; a fit failure blocks rollout for explicit design correction rather than silently clipping normal values.

4. **Review the family before production rollout.** Build the component and native synthetic preview first, reusing the existing swell UI harness. Launch one prepared preview directly with representative playback/seek sequences, fractional tempos, unusual meters and unavailable states. Obtain scoped owner look/feel approval before editor rollout. Any new token/variant needs an explicit rationale, scope and native evidence; publish approved standards revision and tests together. This plan does not preapprove a visual exception or wider migration.

5. **Verify geometry and integration separately.** Component tests compare every slot/separator/group rectangle across digit transitions and availability changes, and check actual glyph fit against value bounds. Extend the existing readability/editor tests to exercise real refresh bindings and mode return at supported logical sizes, preserving session serialization, capture revision and render counts. Reuse native software-scale rendering at 100/125/150/200%; label it separately from physical host/DPI. Automated cases cover edge conditions so owner UAT remains a short focused review and final FL smoke if needed for unresolved host behavior.

## Risks / Trade-offs

- Wide reserves leave more whitespace at common positions -> native preview reviews the real minimum-width composition; keep grouping visually coherent using existing spacing.
- Unbounded finite host values cannot all fit on one line -> explicit normal envelope, per-slot bounded overflow and full inspection text; no new host clamp or group movement.
- Existing tests depend on `juce::Label` casts -> migrate only affected assertions to the component's inspection API and retain full accessible semantic text.
- Offscreen scaling does not prove physical Windows DPI or FL rendering -> record tested environments honestly and request additional host checks only for a concrete unresolved behavior.

## Migration Plan

No saved-state, asset or parameter migration. After native approval, replace only the footer consumer and redundant setup, retain mode/help behavior, update living guidance and run the guide-maintenance review. Rollback reverts the GUI control/binding and accompanying docs/tests; captured source, renders and DAW projects remain compatible.

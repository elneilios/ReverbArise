# Proposal

## Why

Numeric and text fields across ReverbArise use several editor mechanisms. They need consistent field presentation and accessible feedback while preserving the edit rules that each binding relies on. P4 applies the shared field pattern proven in Settings source range to the complete family under [#107](https://github.com/elneilios/ReverbArise/issues/107).

## What Changes

- Adopt shared field presentation across Learn, Perform, Export, Settings, Diagnostics, Rename and the timeline Height numeric entry. Cover standalone inputs, embedded numeric entries and read-only values through explicit variants.
- Give each applicable field or group its help or validation within its feedback position, using the shared C3 layout. Show real binding rejections beside the input where that position exists. Keep compact controls readable and preserve the distinction between read-only, unavailable and invalid states.
- Preserve each subtype's precision, units, keyboard behavior, clamp or reject rules and edit ownership. Make an empty Rename submission visibly recoverable in the dialog while keeping its existing nonempty-name rule.
- Present one native family preview for approval before production rollout. Account for every consumer through adoption, existing shared coverage or an owner-approved specialized variant. Verify the full inventory before declaring P4 complete.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system`: Define observable presentation, feedback and edit behavior for the numeric and text family across the plugin.

## Impact

The rollout touches the field portions of the Learn and Export numeric controls, the Perform inspector entries, Settings sliders and bar editors, Diagnostics read-only values, the Rename dialog and the timeline Height input. P5 owns the dial and slider surfaces and their gestures. Existing model validation, persistence and host timing remain the authorities for edits.

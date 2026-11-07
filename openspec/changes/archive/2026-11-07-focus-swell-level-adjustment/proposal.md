# Proposal

## Why

GitHub issue: #108. The owner wants one clear Level control with fixed -12 and +12 dB limits and 0 dB at 12 o'clock. This owner-confirmed revision replaces the earlier extended-editing contract while preserving existing projects' sound until Level is deliberately changed or reset.

## What Changes

- Use a linear-in-dB -12 to +12 dB editing range and 0 dB at 12 o'clock in the standard knob. Keep the standard appearance and drag response, 0 dB creation/reset and 1/0.1 dB wheel/key steps.
- **BREAKING (editing only):** reject newly entered values below -12 dB. Show older values down to -60 dB in the ordinary input field. A primary press on the dial brings an older value to -12 dB. Valid entry and reset apply their chosen values directly.
- Keep the existing per-swell Level definition, calibration, raw convolution and path-specific output controls. Learn's future IR choice does not gain a speculative Level setting.
- Separate saved-value compatibility from new-edit validation; update help and tests for consistent hard limits, real pointer behavior, legacy round trips, gain-path agreement and independent settings.
- Verify the standard appearance and first-touch behavior with the native control before rollout.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-gain-staging`: one bounded Level editing range and retention of older saved values until the user interacts with Level.

## Impact

The correction touches the existing Level policy/binding, affected shared-control presentation, authoring validation, focused model/component/audio tests and current help/guide text. Retain the durable Shape domain for compatibility. No parameter ID, host normalized mapping, session schema, calibration revision or asset-format change is proposed. Reuse the existing shared controls and #80's scope separation; #79/#9 remain integration contracts. Coordinate affected control work with #107 before Apply.

The [investigation](../../../../docs/validation/2026-11-06-issue-108-ir-level.md), [extended-editing integration](../../../../docs/validation/2026-11-06-issue-108-integration.md) and [labeled candidate](../../../../docs/validation/2026-11-07-issue-108-fixed-level.md) record earlier work. The owner asked to retain the standard knob appearance after reviewing the labeled candidate.

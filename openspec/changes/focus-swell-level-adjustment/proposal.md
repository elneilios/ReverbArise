# Proposal

## Why

GitHub issue: #108. The owner wants one clear Level control with fixed -12 and +12 dB limits and 0 dB at 12 o'clock. This owner-confirmed revision replaces the earlier extended-editing contract while preserving existing projects' sound until Level is deliberately changed or reset.

## What Changes

- Use a linear-in-dB -12 to +12 dB editing range for drag, wheel, keys, typing and accessibility, with clearly visible low/high labels and 0 dB physically at 12 o'clock. Keep 0 dB creation/reset and 1/0.1 dB coarse/fine steps.
- **BREAKING (editing only):** reject newly entered values below -12 dB. Retain supported older saved values down to -60 dB without load-time clamping, audio changes or quantization; identify their actual saved value until an accepted bounded edit or explicit reset replaces it.
- Keep the existing per-swell Level definition, calibration, raw convolution and path-specific output controls. Learn's future IR choice does not gain a speculative Level setting.
- Separate saved-value compatibility from new-edit validation; update help and tests for consistent hard limits, real pointer behavior, legacy round trips, gain-path agreement and independent settings.
- Review the revised endpoint/center presentation natively before rollout. This invocation updates planning only; use Apply separately to implement the correction.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-gain-staging`: one bounded Level editing range, explicit low/high and center presentation, and sound-preserving retention of older saved values until edited.

## Impact

The correction touches the existing Level policy/binding, affected shared-control presentation, authoring validation, focused model/component/audio tests and current help/guide text. Retain the durable Shape domain for compatibility. No parameter ID, host normalized mapping, session schema, calibration revision or asset-format change is proposed. Reuse the existing shared controls and #80's scope separation; #79/#9 remain integration contracts. Coordinate affected control work with #107 before Apply.

The [investigation](../../../docs/validation/2026-11-06-issue-108-ir-level.md) and [541b8ba integration evidence](../../../docs/validation/2026-11-06-issue-108-integration.md) describe the preceding contract and results. They do not verify this revision. At the planning update, code still implements extended editing; the new unchecked correction tasks must be applied and verified before claiming the simpler contract is delivered.

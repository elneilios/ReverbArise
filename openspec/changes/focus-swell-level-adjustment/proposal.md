# Proposal

## Why

GitHub issue: #108. The owner finds the current -60 to +12 dB Level sweep unnecessarily wide. A symmetric everyday sweep gives more useful adjustment around the approved 0 dB baseline, while deeper attenuation remains necessary for saved sound, subtle balances and uncalibrated imports.

## What Changes

- Propose a linear-in-dB -12 to +12 dB main dial sweep, 0 dB at its midpoint, retaining 0 dB creation/reset, 1 dB coarse and 0.1 dB fine steps.
- Retain finite exact numeric entry and absolute accessibility edits across -60 to +12 dB. Show saved extended values accurately without changing sound or snapping them to the main sweep.
- Keep the existing per-swell Level definition, calibration, raw convolution and path-specific output controls. Learn's future IR choice does not gain a speculative Level setting.
- Consolidate affected range/mapping constants and binding validation; document extended entry and test legacy round trips, gain-path agreement and independent settings.
- Require scoped native control review before rollout. This issue delivers investigation and planning only; a new Apply request is required for implementation.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-gain-staging`: focus the main Level adjustment sweep while retaining the existing typeable range, calibration and compatibility contract.

## Impact

Future implementation touches `Source/Plugin/PluginInspector.cpp`, `Source/UI/KnobValue.h` only where needed for explicit absolute-accessibility intent, shared Level constants near `Source/Core/SwellSession.h`, focused component/model/audio tests and current help/guide text. No parameter ID, host normalized mapping, session schema, calibration revision or asset-format change is proposed. #80 is merged in baseline `717c890`; consume its existing scope separation. #79 and #9 remain integration contracts, not additional implementation scope. Coordinate any shared-control changes with #107 before Apply.

Evidence and limitations: [investigation](../../../docs/validation/2026-11-06-issue-108-ir-level.md). No production code changes are part of this proposal.

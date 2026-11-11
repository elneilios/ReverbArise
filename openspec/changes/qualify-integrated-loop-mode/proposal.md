# Proposal

## Why

Loop Mode's component deliveries have been merged into the integration branch. One integrated candidate needs end-to-end host, audio and physical DPI evidence before the owner can judge readiness.

GitHub issue: [#130](https://github.com/elneilios/ReverbArise/issues/130).

## What Changes

- Qualify the same Windows Release VST3 in VCV Rack and FL Studio, covering capture, live timing and commands, audible swells, WAV delivery and project recovery.
- Check the approved 900 × 600 Song and Loop Learn/Perform layouts at actual Windows 100% and 125% display scaling. Record the physical DPI evidence transferred from #126.
- Consolidate versioned evidence and a supported-host matrix. Reconcile the offline guide and its host appendices with the measured routes, limitations and recovery steps.
- Record owner acceptance and outstanding blockers before proposing the integration branch for wider release review.

## Capabilities

### New Capabilities

### Modified Capabilities

## Impact

Validation, fixture and guide work uses the existing Loop capture, host admission, WAV export and asset portability contracts. Qualification starts from `feature/loop-mode` at `866c84e` and retains the [approved #116 design](https://github.com/elneilios/ReverbArise/issues/116#issuecomment-6707861945) and subsequent approved previews. The plan introduces no product behavior or spec changes. A discovered behavior defect needs a scoped fix and affected evidence refreshed before qualification.

# Tasks

Planning for #108 only. No task below is implemented or approved merely by creating this proposal. Begin only after a new Apply request. Preserve task-level Apply instruction refreshes and owner control-review boundaries.

## 1. Shared Level contract and compatibility

- [x] 1.1 Reconcile current #80/#107 control work and any concurrent parameter exposure, then consolidate Level semantic constants without changing [-60,12] validation or 0 dB creation/reset; verify model tests reject nonfinite/out-of-range values and accept -60, -30, -12, 0, +12 and a fractional non-step value; document actual semantic/UI domain separation in the living gain guide.
- [x] 1.2 Extend session/processor fixtures for Legacy and Musical -60/-30/fractional values, frozen reopen with missing source and unchanged unrelated fields; verify save/reopen preserves actual float, policy/calibration and raw asset identities, publishes no load-time edit and starts no playback; assert the host interface still exposes no Level parameter/mapping and document rollback without migration.

## 2. Focused control candidate and owner gate

- [x] 2.1 Implement the scoped -12/+12 linear main mapping, 24 dB span and 0 midpoint in the existing binding/control candidate, retaining 1/0.1 dB steps and exact full-range entry; add explicit absolute-accessibility intent only where needed. Verify component/binding tests cover endpoints, Shift/wheel/key, exact fractional entry, rejection/cancel, -30 relative edits without jumps, absolute -24 accessibility edit, full reported domain, Level-only reset and stale selection/session/render guards; add honest extended-range help and update affected shared-control standards/tests.
- [x] 2.2 Launch one prepared native preview directly for the owner and retain it until review is complete; show ordinary -12/0/+12 and extended -30/-60 states, precise entry/reset and compare -12/-24 on a bounded retained factory/imported quiet/strong fixture set. Record owner approval of this revised control family's look/feel and listening balance, or revise only the candidate before rollout. Verify the dated evidence distinguishes new feedback from the prior calibration approval.

## 3. Integration, audio and documentation

- [x] 3.1 After task 2.2 approval, integrate the accepted Level candidate into Perform without adding a Learn Level default or changing the shared IR editor. Verify native binding tests preserve future IR defaults, next-swell 0 dB creation, neighbours, other instances and frozen Level editing; verify refresh/focus changes no model generations and accepted Level changes shape generation without raw reconvolution; update the affected technical/UI guidance.
- [x] 3.2 Extend production audio integration fixtures across admitted audition, live Song, plugin WAV and host offline bounce for -60/-30/-12/0/+12 with equal content/membership and unity output controls. Verify expected dB amplitude ratios away from ramps below protection, finite/disclosed hot-overlap protection, unchanged dry/raw PCM/calibration and last-good/superseded worker behavior; record exact coverage and limitations rather than assuming all routing contexts are equal.
- [x] 3.3 Use maintain-user-guide to review and update offline Level/help wording for focused sweep versus extended entry; verify read-only guide currency, links and consistent numeric/accessibility help. Keep Live wet, audition trim and Export levels distinct and do not claim new Learn or bulk capabilities.

## 4. Final integration gates

- [x] 4.1 Run the supported `scripts/build.ps1` Debug/Release validation appropriate to affected components and native integration, `scripts/check-ui-standards.ps1`, strict OpenSpec validation and whitespace checks; record revision, exact commands, results, warm/cold cache status and skipped coverage in dated evidence. Run a brief final FL Studio smoke only for concrete unresolved host behavior using the install-test-vst3 skill if installation is needed; preserve prior passing checks and report untested DPI/host coverage.

## Workflow follow-up

- Keep the #108 planning delivery distinct from implementing this change; leave its proposal for review and require a new Apply request.
- Future implementation uses one coherent review PR and no self-merge. Sync/archive only through a separately requested standard lifecycle step.

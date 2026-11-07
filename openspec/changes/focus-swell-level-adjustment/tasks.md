# Tasks

Owner-confirmed revision: standard knob appearance and drag response, fixed -12/+12 limits, 0 dB at 12 o'clock and older values shown in the standard input until Level is operated.

Sections 1-4 below preserve the completed `541b8ba` baseline under the **superseded extended-editing contract**. Their checked boxes and dated results are historical; they do not prove the revised contract. Section 5 contains all remaining correction work. Existing proposal/design/delta requirements govern the current target behavior.

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

## 5. Owner-confirmed fixed-limits correction

- [x] 5.1 Separate the single [-12,+12] new-edit domain from the unchanged [-60,+12] saved-state/DSP compatibility domain. Verify command/model and real processor-wrapper tests preserve -60/-30/non-step saved values through load/focus/unrelated edits/save/frozen reopen, reject new out-of-range/nonfinite numeric and accessibility edits, accept valid bounded values and reset to 0; document the compatibility boundary without a state/asset migration.
- [x] 5.2 Restore the standard shared knob presentation and drag response. Verify native appearance against another knob, 0 dB at 12 o'clock, both endpoints, real pointer events, Shift, wheel/key/entry/accessibility, stale intents and reset. Verify -30 is shown in the normal input, a primary press or outward wheel/key step sets -12, a coarse inward step selects -11, and invalid or cancelled typing preserves -30.
- [x] 5.3 Launch one prepared native preview directly and retain it through review. Show the standard Level knob, neutral and the first-touch transition from an older value. Record owner approval of the revised interaction before wider rollout. Reuse the earlier calibration/listening approval.
- [x] 5.4 After task 5.3 approval, integrate the accepted fixed-limit candidate across affected Level authoring consumers and remove the old extended-edit policy/help. Verify defaults/neighbour/instance/freeze isolation, no refresh-time writes, raw/calibration reuse, legacy save/reopen before and after accepted edits, and gain agreement for new -12/0/+12 values across admitted audition/live/plugin WAV/offline paths with their existing protection policies. Use maintain-user-guide to update the relevant prose and explicit review record; verify read-only currency and affected links.
- [ ] 5.5 Complete proportionate supported Debug/Release native builds and focused component/integration/compatibility checks for the revised contract; run UI standards, strict OpenSpec and whitespace checks. Record exact revision/commands/results, source and binary identity, warm/cold status, earlier pointer coverage limits, scoped owner approval and untested host/DPI coverage in fresh dated evidence. Update the existing PR for review only after it matches this plan; request a host smoke only for concrete unresolved host behavior.

## Workflow follow-up

- Future implementation uses one coherent review PR and no self-merge. Sync/archive only through a separately requested standard lifecycle step.

# Tasks

## 1. Shared selector and popup family

- [x] 1.1 Implement the scoped typed popup inspection/activation state and one-shot completion seam described in design.md; verify component tests cover independent hover/keyboard/accepted state, hover exit into padding/outside, keyboard-hover-return, pointer click then leave and no synthetic initial highlight.
- [x] 1.2 Build the anchored JUCE popup adapter without a JUCE fork or global popup override; verify native component tests for screen clamping, owner destruction, outside/Escape dismissal, Tab traversal, Return/Space/accessibility one-shot commit and focus return from pointer/keyboard origins.
- [x] 1.3 Extend the existing selector body and shared FocusVisibility usage for idle/hover/held/disabled and outer aqua brackets; verify state rendering, steady chevrons, independent combined states, repeated/already-focused accessibility requests, pointer-click hiding and retained keyboard operation in focused control tests.
- [x] 1.4 Add long closed-value discovery, wrapped popup rows/reasons and unavailable inspection with separate activation eligibility; verify full accessible names/disabled state, no disabled invocation, accepted disabled Custom, normal text size and focus/checkmark containment in narrow/scaled fixtures.
- [x] 1.5 Document the C2 adapter/reuse API, wheel and inspect/commit policies in ui-control-contracts and the standards with exact scoped candidate metrics; verify documentation links and UI standards pass without debt-inventory regeneration, and mark native acceptance pending rather than inferred from mockups.
- [x] 1.6 Incorporate the owner-approved field-feedback amendment in the selector control and native preview: smaller help directly below the visible body, red error message/border, independent aqua brackets and readable disabled explanations. Verify error/hover/held/focus rendering, accessible error/clear behavior, unchanged selection, exact body/help alignment and supported-size/scale fixture checks; update the durable approval record without migrating text/numeric fields.

## 2. Native family checkpoint

- [x] 2.1 Prepare and directly launch the lightweight selector/menu native preview with long/empty/unavailable/error and combined states at 900x600/100%, 1000x700, 1100x850 and 1400x1000 plus supported 100%/125% user scale; record scoped owner acceptance and any approved popup tokens/geometry before wider adoption, distinguishing physical Windows DPI/FL Studio coverage.

## 3. Bounded consumer adoption

- [x] 3.1 Adopt the family in the shared Learn/Perform IR field without changing import sibling body alignment or spacing; verify ConvolutionIr component/binding and actual-editor checks for catalogue reorder, captured identity, accepted refresh, stale/pass/session rejection, deletion and Learn/default versus selected-swell isolation, and update their migration rows/evidence.
- [x] 3.2 Adopt Perform grid and Export Character, preserving existing IDs, grid timing meaning, Custom and atomic gain-pair edits; verify focused navigation/ExportLevels binding tests for inspection/cancel/no-wheel/one commit and silent accepted refresh, and update the exact inventory rows.
- [x] 3.3 Adopt Appearance size/scale plus Playback liveRoutingBox/gainPolicyBox and Swell inCurveBox/outCurveBox with explicit deliberate/no-wheel policy; verify actual Settings callbacks, disabled prototype inspection, preference refresh, focus/overflow and unchanged model outcomes. Leave Recovery candidateBox and all #8/#108 consumers outside the diff.
- [ ] 3.4 Finalize approved standards revision, contracts, consumer inventory and dated validation evidence for adopted selectors; semantically review the offline guide and record review only after any needed prose changes, then verify guide currency and links. Retain explicit debt for deferred controls and the separate action-menu child.

## 4. Combined integration and review delivery

- [ ] 4.1 Integrate current merged main without importing active #108/#8 work, run scripts/build.ps1 -Configuration Release -Test, scripts/check-ui-standards.ps1, strict validation of this change, guide currency and whitespace/link checks; record revision/environment/results and honest host/DPI gaps, with protected command/audio/persistence regressions passing.
- [ ] 4.2 Deliver a focused review-ready selector PR using Refs #107 and the recorded native approval/evidence; verify its diff contains only the named selector/shared-popup scope and prepare the separate action-menu handoff without marking its implementation delivered.

## Workflow follow-up

- Keep #107 open for remaining families; do not close #108 or #8.
- Sync/archive this change after approved implementation delivery using the existing OpenSpec workflows. These actions are outside tracked implementation progress.

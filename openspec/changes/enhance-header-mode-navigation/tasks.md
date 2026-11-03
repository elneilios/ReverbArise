# Tasks

Implementation has not started. Preserve the concurrent Learn agent's work.

## 1. Isolated glyph preparation and integration baseline

- [ ] 1.1 Implement the reusable semantic JUCE vector glyph helper on this branch; verify finite/in-bounds paths, the Perform rise/release and three outlined handles, optical sizes and stroke rendering through focused component tests.
- [ ] 1.2 Record the Learn agent's published navigation clipping-fix revision and prepare a safe integration diff; verify its border/focus geometry and regressions survive before editing overlapping selector/header code. If unpublished, document the exact pending merge points and defer those edits.

## 2. Candidate shared controls and owner checkpoint

- [ ] 2.1 Add optional semantic icon content to the existing selector against the preserved clipping-fix baseline; pass active/inactive/hover/disabled/focus, guarded intent, accessibility and complete-outline component tests, including existing no-icon consumers.
- [ ] 2.2 Prepare the accented regular description and scoped header-body appearance using existing control primitives; pass typography, short/long notice severity, no-collision and unchanged-geometry tests in the native review fixture, with non-header notice defaults unchanged.
- [ ] 2.3 Launch one prepared native control/header preview for owner look-and-feel approval before wider rollout; record requested adjustments and accepted versus untested interaction/DPI coverage, and keep the review window open until the owner finishes.

## 3. Header integration and evidence

- [ ] 3.1 Bind approved icons, description copy and header-only notice appearance into the existing header; pass WorkflowHeader/WorkspaceFeedback/notice tests for all modes, stale/rejected requests and silent refresh without new audio/state changes.
- [ ] 3.2 Update living UI standards and mode guidance for regular information-row typography and approved icon roles; verify the guidance matches implemented sizing, scoped colour and preserved clipping behaviour, with dated native screenshots and test evidence.
- [ ] 3.3 Run the repository build wrapper and relevant native component/integration checks at supported sizes, recording DPI gaps honestly; submit a focused owner-reviewable PR and consolidate any required FL Studio validation into one brief final smoke test.

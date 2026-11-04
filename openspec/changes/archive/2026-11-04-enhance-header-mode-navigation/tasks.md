# Tasks

Owner native look-and-feel approval recorded on 4 November 2026. All 8/8 tasks
complete; production integration and evidence submitted in implementation PR #40.

Preparation delivered separately: canonical Learn/Perform/Export SVG assets,
browser review evidence, exact visual specifications, file-level integration plan
and validation checklist in design.md. Integration baseline is merged PR #39,
`185f00f` (Learn source `598b998`); its border/focus regressions are preserved.

## 1. Isolated glyph preparation and integration baseline

- [x] 1.1 Embed and load/cache the exact prepared SVGs through a reusable semantic JUCE helper after implementation is authorised; verify JUCE loading/recolouring, finite/in-bounds geometry, the three outlined Perform handles and 14/16 px rendering through focused component tests. Do not redraw or replace the canonical assets.
- [x] 1.2 Record the Learn agent's published navigation clipping-fix revision and exact shared-file hunks, then prepare a safe content-only integration diff; verify its border/focus geometry and regressions survive before editing overlapping selector/header code. If unpublished, retain the design's file-level plan and defer those edits; do not merge or cherry-pick without separate authorisation.

## 2. Candidate shared controls and owner checkpoint

- [x] 2.1 Add optional semantic icon content to the existing selector against the preserved clipping-fix baseline; pass active/inactive/hover/disabled/focus, guarded intent, accessibility and complete-outline component tests, including existing no-icon consumers.
- [x] 2.2 Implement the design's exact 2×16 px accent, 8 px text gap and 14 px regular Inter information-row treatment using existing control primitives; pass typography, short/long notice severity, no-collision and unchanged-geometry tests in the native review fixture, with non-header notice defaults unchanged.
- [x] 2.3 Launch one prepared native control/header preview for owner look-and-feel approval before wider rollout; record requested adjustments and accepted versus untested interaction/DPI coverage, and keep the review window open until the owner finishes.

## 3. Header integration and evidence

- [x] 3.1 Bind approved icons, description copy and header-only notice appearance into the existing header; pass WorkflowHeader/WorkspaceFeedback/notice tests for all modes, stale/rejected requests and silent refresh without new audio/state changes.
- [x] 3.2 Update living UI standards and mode guidance for regular information-row typography and approved icon roles; verify the guidance matches implemented sizing, scoped colour and preserved clipping behaviour, with dated native screenshots and test evidence.
- [x] 3.3 Run the repository build wrapper and the design checklist at supported plugin sizes and 100/125/150/200% scale, recording native/host/DPI gaps honestly; submit a focused owner-reviewable PR and consolidate any required FL Studio validation into one brief final smoke test.

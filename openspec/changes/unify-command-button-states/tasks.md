# Tasks

Planning only: all implementation tasks remain unchecked. Use standards revision 11 and the approved C1/P1 contract; active #108 and #8 are excluded.

## 1. Shared command family candidate

- [ ] 1.1 Confirm merged baseline and the design's 20 legacy command/two latch consumer list before editing; record current revision, retained #80 ownership, #108/#8 exclusion boundaries and exact command callbacks in dated `docs/validation/YYYY-MM-DD-issue-107-buttons.md`. Verify source declarations and callbacks match the packet and stop for any material scope conflict.
- [ ] 1.2 Extend the existing shared surface/content mechanism with explicit neutral/primary/destructive and accepted-latch semantics, preserving #80's public adapter and include direction; add focused component state/combined-state, glyph/text/narrow-add, long-label, metadata-invariance and contained-focus tests. Verify neutral icon parity, primary/Export and FileDragTile regressions pass, and document candidate variants/reuse before rollout.
- [ ] 1.3 Consolidate eligible pointer/Return/Space/accessibility invocation plumbing only where required; add real callback tests for one-shot delivery, disabled input, queued lifetime/destruction, captured ownership and silent latch refresh. Verify binding guards retain existing semantics and no input path bypasses or duplicates them; document the invocation contract in the family evidence.
- [ ] 1.4 Prepare and directly launch one native family preview with neutral text/glyph, primary, destructive and latch-off/on specimens at supported compact/larger/user-scale settings; record explicit scoped owner look/feel approval of revised focus/disabled/destructive/latch treatment before production adoption. Leave this task unchecked while that review is pending; preserve the review window and prior unchanged #80 acceptance.

## 2. Bounded production migration

- [ ] 2.1 After family approval, migrate `addButton`, `auditionButton`, `stopButton`, `resetButton`, `resetMenu`, `snapButton` and `auditionWetOnly` with unchanged callbacks, names, eligibility and accepted-state refresh. Extend actual-editor add/audition/reset/navigation bindings and narrow compact glyph/focus checks; verify exactly one effect, disabled/stale rejection where currently guarded, and unchanged requested/effective latch behavior. Update their inventory rows/evidence immediately; do not touch #108 value controls.
- [ ] 2.2 Migrate the remaining 15 persistent legacy Settings/Diagnostics commands listed in the design, preserving text, shared IR default import routing, bar-range/Delete/regeneration/copy/disclosure behavior and existing callbacks. Extend affected actual-editor keyboard/accessibility/binding and comfortable auxiliary layout checks; verify no storage/recovery-window or engine changes from #8 and update only delivered inventory rows/evidence.
- [ ] 2.3 Remove name-based theme appearance and orphaned `loadIR` painting after all scoped consumers are explicit; keep one shared neutral fallback without an include cycle. Verify metadata spoofing cannot select a role, generic Rename OK/Cancel remains usable, and repository search finds no residual command-name styling. Run the UI standards guard without debt regeneration; remove only obsolete exact scoped entries and document retained exceptions.

## 3. Integration and delivery

- [ ] 3.1 Validate integrated family behavior with `scripts/build.ps1 -Configuration Release -Test`, focused existing control/IR/Export/audition/navigation/compact cases and the applicable persistence/audio oracles for changed dispatch paths. Verify no command/model/audio/identity or #80 rendering regression, record revision/environment/commands/results, and keep native MSVC/CTest distinct from software renders and host checks.
- [ ] 3.2 Directly launch the prepared actual-editor review covering 900x600/100%, 1000x700, 1100x850, 1400x1000 and supported 100%/125% user scale with long labels, disabled/error states and Settings/Diagnostics; verify complete focus/hit/accessibility access and record scoped integrated acceptance. Record physical Windows DPI/FL Studio coverage honestly; request a consolidated host smoke only for concrete unresolved host behavior.
- [ ] 3.3 Update production standards from the then-current approved revision, C1 contracts, inventory/debt, dated evidence and affected guide content/currency together using the guide-maintenance workflow. Verify strict OpenSpec, UI guard, documentation links, guide currency and whitespace checks pass; leave one owner-review PR with `Refs #107` and a consumer-scoped diff. Verify #108/#8 worktrees and excluded runtime areas are untouched; do not self-merge or close #107.

## Workflow follow-up

- Keep #107 open for remaining control families. This child is command buttons only.
- Sync/archive through the existing workflow after implementation and owner review, when explicitly requested; no canonical spec changes are part of proposal creation.

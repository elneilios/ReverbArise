# Tasks

## 1. Pin the evidence and separate the existing offline repair

- [ ] 1.1 Create `docs/validation/YYYY-MM-DD-issue-73-clock-investigation.md` using the execution date; record the tested branch/revision, original reported tuples, recoverable loaded-build/source/render identities and explicitly unavailable attribution. Verify the record distinguishes supplied host evidence from injected tests and links issue #73 plus the existing offline change's proposal, tasks and validation paths through local branch `codex/fix-offline-export-clock` and commits `1532f37`, `09b5696`, `a5b36d2`.
- [ ] 1.2 Recheck the local offline branch and current main integration status before implementation; inspect its offset/sample-equality regressions and successful owner bounce evidence, then record an acceptance matrix for unresolved grid/live behavior versus accepted offline behavior. Verify no offline implementation, output protection, #71 feature work or unrelated source-edge changes are copied into this investigation.

## 2. Explain fixed-grid availability without changing edits

- [ ] 2.1 Extend the existing grid authority with explicit failure reasons and exact decision inputs while preserving the optional factory behavior. Extend `host_grid_test` with the owner tuple, zero/positive/negative offsets, existing one-sample boundary, absent/nonfinite timing, rate/tempo/meter mismatch, invalid division and conversion overflow. Verify every old success/rejection and integer-oracle snap result remains unchanged; document reason semantics in the investigation record.
- [ ] 2.2 Bind the grid result to the existing Diagnostics text and full accessible description, including sample-position availability and UI-snapshot provenance; keep the concise main hint and all existing edit policies. Extend native component/binding tests for stopped/playing grid changes, canvas/inspector consistency, Ctrl bypass, Snap Off, exact entry and stale gestures. Verify no saved-state/source/render/job changes and readable, reachable text at minimum/larger sizes; update `docs/development/perform-live-diagnostics.md` with the distinct grid evidence contract.

## 3. Retain bounded causal live observations

- [ ] 3.1 Add callback-owned fixed-size run-start, last-admitted and first-timing-rejection evidence with its preceding admitted observation, retaining existing current/latest historical views. Extend transport tests for first rejection retention, later failures/recovery, seek/loop, invalid callbacks, confirmed Stop/new Play, reset/restore and instance isolation. Verify run/session/reset association, unavailable preceding evidence and coherent bounded reads; record payload size and fixed publication/read work in the investigation evidence.
- [ ] 3.2 Format explicit run-reference-to-host origins/deltas separately from publication/source deltas outside the callback and expose the causal observations in existing Diagnostics content. Add native text/binding tests for the exact recorded live tuple, approximately -16-sample run delta alongside near-zero publication delta, unavailable snapshots and editor lifecycle. Verify full accessible text/scroll containment, unchanged session/assets/output and no allocation, text formatting, locking, logging or I/O added to the live callback; update the living diagnostics collection guide.

## 4. Prove the rejection and audio distinctions

- [ ] 4.1 Add deterministic sequence tests for a shifted Play-entry observation followed by source-origin timing and for a stable shifted run followed by a later origin change. Verify unchanged strict admission, retained reference and correctly latched first rejection/predecessor; record which parts are injected assumptions and why these tests reproduce a predicate rather than establish FL Studio's cause.
- [ ] 4.2 Extend focused transport/processor regressions with independent positioned asymmetric wet references, zero-input playback and variable block partitions; compare nonzero real-time/offline samples at the same intended positions for supported clocks, accounting independently for declared ramps and gain. Verify rejected clocks retain explicit rejection/bounded suppression, and preserve seek/loop, retained IR, save/reopen, source/render identities and instance isolation. Record results separately from the existing offline branch's offset parity/host acceptance; do not loosen a guard or port that fix to manufacture parity.

## 5. Integration verification and investigation outcome

- [ ] 5.1 Run `scripts/build.ps1 -Configuration Debug -Test` and `scripts/build.ps1 -Configuration Release -Test` through the per-tree lock, plus `scripts/check-ui-standards.ps1`, strict validation of this OpenSpec change and `git diff --check`. Record exact revision/dirty state, toolchain, warm/fresh status, logs, native versus fixture results, fixed-payload concurrency/stress checks and actual limitations. Verify the final production diff changes only diagnostics/observation and equivalent grid-result reporting, with existing admission/audio policies preserved.
- [ ] 5.2 Apply `maintain-user-guide` to the delivered diagnostic/spec behavior, update guide prose only if needed and record semantic currency; verify `scripts/check-user-guide.ps1` passes. Review all requirement scenarios against the evidence matrix and verify no unsupported host/DPI or current-main offline acceptance is marked passed.
- [ ] 5.3 Finish the dated evidence record with independent grid, live and offline conclusions, the causal sequence established or still unknown, and a concrete minimal follow-up recommendation. If host evidence is still needed, include one stopped -> Play through known wet -> Stop FL Studio collection procedure on a preserved project copy with exact build identification; verify it requests no repeated accepted bounce or automated edge cases without a specific unresolved regression. Keep unresolved host observations and #73 closure criteria explicitly open; do not implement a repair as part of this task.

## Workflow follow-up

- Planning completion is not investigation completion. A fresh user request is required to start Apply.
- Keep all work local. No push, PR, installation, host operation, canonical-spec sync, archive or issue closure is authorized by this plan.
- The existing offline repair remains owned by `fix-offline-export-clock`; preserve its branch, accepted evidence and separate integration/review path.
- If the investigation supports a behavioral repair, revise the relevant proposal/design/specs/tasks and obtain explicit owner approval before changing timing policy or resuming a fix implementation.
- No new/revised control family or visual exception is planned. If one becomes necessary, stop for scoped owner review and launch its native preview directly before wider rollout.

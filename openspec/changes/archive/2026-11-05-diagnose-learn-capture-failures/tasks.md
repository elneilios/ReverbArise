# Tasks

## 1. Failure evidence and transaction regression

- [x] 1.1 Extend the existing disposable native capture/processor fixtures to stage multiple chunks, expose provisional progress, deterministically saturate a held worker queue, and reproduce the waveform disappearance with empty and previously committed sessions; verify accepted reference/revision, source/context, PCM/asset hashes and older saved-snapshot restoration remain unchanged, then verify a successful retry. Record code observations versus executed results and staged-file outcomes in dated `docs/validation/` evidence.
- [x] 1.2 Introduce typed callback failure causes and bounded pass/epoch-tagged first-failure publication, including attempted range, queued/dropped counts, queue depth/capacity and available host timing; verify distinct queue/timing/rate/block/range/admission reasons, coherent concurrent reads and lock-free supported atomics with focused tests. Extend the callback allocation/deallocation guard and document ownership/field validity in the living capture guide.
- [x] 1.3 Preserve bounded worker/storage error details and the pass-pinned accepted source identity before rollback; retain the historical failure through snapshot refresh and successful retry. Verify first-cause preservation, exact failure-time versus final counters, error text bounds, restore/reset fencing and no stale evidence publication with focused worker/processor tests; update the diagnostic contract documentation.

## 2. Failure injection and long-pass behavior

- [x] 2.1 Add deterministic invalid/nonfinite input, unavailable/incompatible timing, host rate change, oversized-block and range-boundary cases using production capture paths; verify each original cause, safe bounds, dry passthrough, unchanged accepted source and successful retry. Include valid seek/loop and silence controls, and record exact case outcomes in the dated evidence.
- [x] 2.2 Extend disposable asset fixtures for chunk write failure, staged corruption and manifest publication failure; contain injected recoverable worker exceptions at the pass boundary. Verify rejection after earlier chunks, retained original error/immutable assets/history and a working worker/retry; document surviving staged files and limitations without claiming physical disk-full or automatic orphan recovery.
- [x] 2.3 Cover repeated Stop, cancel/restore during staging and validation, obsolete completion and immediate retry with deterministic barriers and bounded deadlines; verify at most one acceptance, no cross-session source or diagnostic publication and no partial replacement, including explicit Recapture. Record ownership regression results alongside the diagnostic contract.
- [x] 2.4 Add and execute the design's isolated 3-/5-minute synthetic capture matrix at 44.1/48 kHz with 64/512/8192-frame blocks and selected sparse/loop/silence cases; inject late failures after at least 60 seconds of song position and verify full rollback plus successful retry. Provide a reproducible harness invocation and record song duration, actual wall time, summary/asset limits and every skipped combination separately from real-time throughput.
- [x] 2.5 Execute a separately timed native three-minute 48-kHz/512-frame capture with actual storage and controlled bounded worker delays, recording queue high-water mark, admitted/dropped counts and stage/write/validation latency; compare no-fault and injected-pressure behavior without assuming hardware-independent performance. Include a processor integration case with production bindings and record exact environment, revision, commands and outcomes in dated evidence.

## 3. User feedback and diagnostic binding

- [x] 3.1 Bind retained evidence to the existing capture Diagnostics text and concise shared NoticeBar/Learn status feedback, distinguishing rejected provisional progress, retained committed source and no earlier committed source; verify current versus historical failures, usable retry guidance and unchanged accepted coverage with component/binding tests. Reuse existing roles/controls and document the implemented feedback in current capture guidance.
- [x] 3.2 Verify failure copy at supported editor sizes, unavailable/empty/prior-source states and successful retry, and run `scripts/check-ui-standards.ps1`; deliver component/native evidence with no new visual tokens or family variants. If an exception becomes necessary, obtain scoped owner review and launch its prepared native preview before rollout; record host/DPI limits honestly.

## 4. Integration and review

- [x] 4.1 Complete relevant focused capture, processor, persistence and callback-safety regressions plus the final native Release build/tests using `scripts/build.ps1 -Configuration Release -Test` (and focused Debug validation through the wrapper); verify all delta scenarios against evidence and distinguish fixtures, native MSVC/CTest and long-harness runs. Make any demonstrated narrow recovery correction only with its regression test; surface changes to admission/storage semantics for further review.
- [x] 4.2 Consolidate the implemented contract and reproducible harness commands in living technical guidance, link dated evidence from `docs/README.md`, and perform or explicitly report the remaining single brief FL Studio smoke check for capture/Stop/retry and retained source. Submit one issue-specific PR with accurate `Refs #54` versus `Closes #54` scope, known/unknown original cause and exact validation limits; leave owner review and merge outstanding.

## Workflow follow-up

- Archive only through the standard OpenSpec workflow when explicitly requested after implementation/review; do not close the issue independently or auto-archive this change.

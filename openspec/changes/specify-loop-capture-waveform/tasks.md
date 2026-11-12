# Tasks

## 1. Reconcile the Loop specification

- [x] 1.1 Confirm the target revision includes PR #186. Trace Learn's pass-scoped display through recording, settlement, cancellation and restore. Confirm that Perform, Export, generation and project save continue to use the accepted source.
- [x] 1.2 Check the delta's waveform growth, marker visibility and coverage-limit scenarios against the Learn UI fixture and the existing capture cancellation and restore checks. Record the PR's Release verification at `38872d9` and the earlier FL Studio check with their respective scopes.

## 2. Verify the change

- [x] 2.1 Run strict OpenSpec validation and review the pending delta for scope. Confirm it changes only `loop-phrase-capture` and preserves the existing timing and settlement scenarios.

### Verification evidence

- [PR #186](https://github.com/elneilios/ReverbArise/pull/186) records passing Windows x64 Release checks at `38872d9` for Learn UI, capture failures, 20 capture-pressure cases, and all 15 standard regression tests. The Learn fixture checks progress before acceptance, accepted marker identity while armed, and viewport and waveform behavior when detailed coverage fills. The worker and processor checks cover cancellation and restore fencing. The canvas hiding of accepted markers during the provisional display was verified by inspection.
- The owner confirmed live waveform updates in both Song and Loop in FL Studio on the earlier installed Release build from the changes in `5dd53a0`. Release fixtures cover the subsequent backward-seek and coverage-limit fixes.

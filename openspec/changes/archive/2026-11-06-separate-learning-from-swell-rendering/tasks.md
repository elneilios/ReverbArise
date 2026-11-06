# Tasks

Implementation and verification are complete; checked tasks have native evidence and documented ownership. Issue #8 / PR #31 retains its separate native recovery review and all later storage safety gates.

## 1. Dependency ownership and minimum work

- [x] 1.1 Map accepted source, assigned IR, source-footprint/timing and presentation notifications to the architecture review; identify and replace superseded recovery-only scheduling. Verify focused tests distinguish accepted from provisional source and input from shape generations, and update the ownership guide with actual call paths.
- [x] 1.2 Reuse a common material-dependency comparison and exact compatible-render reuse path; verify changed source/IR/timing invalidates only affected swells, identical PCM across different chunking reuses audio, context/default-only edits do not reconvolve, and document key/compatibility semantics.
- [x] 1.3 Preserve full-wet versus shaping separation; verify fades/curves/level/name/selection/pure placement perform zero convolution/IR resampling, genuine footprint extension waits for real source and only material extension queues work, and document the boundary.

## 2. Shared engine scheduling independent of marker policy

- [x] 2.1 Route accepted source/definition/IR/availability events into one bounded worker reconciler independent of workflow screen and editor lifetime; verify existing unfrozen swells reach Ready with Detect events and marker Generate on Stop off without any Generate click, with no saved-policy mutation, and update current Learn/Perform guidance.
- [x] 2.2 Remove marker-policy/Capture-mode fences only from existing-render maintenance; verify all Recapture/Detect events/Generate-on-Stop combinations, no new markers with detection off, no mapped replacement without Recapture, no convolution for zero existing swells, and preserve marker transaction protection/empty-result semantics.
- [x] 2.3 Preserve coalescing, memory/page priority and latest-request publication; verify rapid edits, deleted targets, session switches, concurrent capture/import/render, budget failure and reader retirement cannot publish obsolete audio or put disk I/O/blocking/allocation into the audio callback, and update scheduling guidance.
- [x] 2.4 Preserve Freeze and legacy sound/state semantics; verify frozen bytes stay exact, missing frozen output waits for restoration/unfreezing, compatible older renders do not auto-upgrade, legacy saved controls and IDs round-trip unchanged and unsupported timing/rates fail honestly, and document the intentional off-policy behavior change.

## 3. Recovery becomes ordinary dependency resolution

- [x] 3.1 Replace duplicate Storage Re-learn/Generate controls and recovery-specific rendering commands with modal missing-asset diagnostics, verified Locate/Restore and normal Learn guidance; verify the dialog blocks underlying editor controls in fixed/responsive compositions, stale callbacks cannot affect another session and metadata failure never claims intact edits, then obtain the existing native owner review before rollout.
- [x] 3.2 Preserve transactional missing-source Recapture admission and complete-known-coverage publication; verify normal re-learning with detection off automatically updates existing eligible swells through the same engine as ordinary source changes, while partial/cancelled/non-finite capture, missing custom IRs, corrupt backups and failed renders preserve definitions/history and explicit frozen consent.
- [x] 3.3 Revalidate exact destination-only restoration in a separate Windows profile with originals unavailable; verify source/wet/all Export-history/context-only IR hashes, creative edits/positions/peaks and zero recapture/convolution/resampling for complete compatible snapshots. Keep that proof distinct from rebuilt missing-render recovery and update issue #8 evidence/task dependency.
- [x] 3.4 Maintain the offline guide from actual production bindings; verify no duplicate controls or manual regeneration instructions remain for successful normal recovery, disclose frozen/IR/changed-sound/export limits, record explicit semantic guide review and pass its read-only currency check.

## 4. Final integration boundary

- [x] 4.1 Run affected native MSVC/CTest capture, marker generation, dependency/render, playback/shape, persistence, recovery and UI suites plus the supported Release build wrapper; verify UI standards, strict OpenSpec, links/whitespace and dated evidence with honest host/DPI coverage.
- [x] 4.2 Update PR #31 with final ownership/behavior, tests, native approval and remaining storage gates; verify portable-project-assets task 3.10 is completed only after its own guarded acceptance checks, without enabling association, writer migration, cleanup, compression, release or merge.

## Workflow follow-up

- Canonical spec sync/archive requires a separate requested lifecycle step after implementation/review.
- Do not self-merge or close the issue. Keep pre-change projects/binaries/assets available for rollback.

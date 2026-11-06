# Shared swell engine — issue #8 / PR #31

Implemented after merge `4defc2e` (main `717c890`) on `codex/8-portable-asset-design`. Native Windows x64 MSVC Release, pinned JUCE 9.0.4; populated build tree reused without cache clearing. This is native/component evidence, distinct from the earlier scoped FL Studio smoke.

## Ownership and behavior

`CaptureWorker` accepts source transactionally; accepted source/definition/assigned IR/material timing/availability changes feed `SwellEngine::reconcileRenders` and the existing fingerprint/cache/render publication path. Mode, detection, marker Generate on Stop and legacy refresh preferences do not suppress existing unfrozen render maintenance. Marker/capture/output permissions remain separate. No recovery-specific render command or duplicate Storage Learn/Generate control remains. [Actual call paths and dependency boundaries](../development/swell-engine-architecture.md).

An editor-owned modal warns on missing assets. Locate/Restore use production Storage verification/copy/reload, preserving current edits. Normal Learn Recapture requires complete-known-coverage replacement; partial/cancelled/rejected source cannot become authority. Metadata failure replaces any earlier intact-edits warning, including in the same editor. Frozen output waits for exact restoration or explicit unfreeze. Missing custom IRs and incompatible source rates stay explicit. Exact compatible older renders are reused; no implicit source resampling or forced upgrade.

## Final native verification

`scripts/build.ps1 -Configuration Release -Test`, with `CMAKE_BUILD_PARALLEL_LEVEL=4`: **configure/build/test pass, 85/85 CTests in 517.44 s, wrapper exit 0**. Local logs: `out/build-logs/Release/20261106-182638-10010523/`.

- Shared engine: all eight Recapture/Detect/Generate-on-Stop combinations; held provisional validation; no marker creation with detection off or source replacement without Recapture; no work for zero swells. Changed source in Learn updates existing sounds without Generate or preference mutation. Selective/identical inputs, assigned/future IR isolation, material timing, shape/name/placement zero convolution/resampling, Freeze, exact saved-render reuse and ordinary missing wet-cache rebuild pass.
- Capture/marker/processor: source settlement and failure cases, marker transaction protection/empty results, numerical playback/shape/Export, multiple instances, memory/page budgets, latest-generation guards, reactivation and callback allocation checks pass.
- Recovery: actual partial/full missing-source callback passes, missing frozen hold, explicit unfreeze, missing custom IR failure/retention, ordinary availability reload and automatic rebuild. Existing non-finite/cancelled transaction, corrupt backup/closure/history and creative/reference preservation remain covered.
- UI: fixed/responsive owned modality and blocked editor, production Locate/Restore, retained scans/pinned actions, post-reload enabled buttons, stale callbacks, normal detection-off re-learning without Generate, and same-editor metadata-decode distinction pass.
- UI standards: 65 files, unchanged 59 bounded occurrences. Semantic offline-guide review/read-only currency, strict validation of both active changes, affected document links and whitespace pass.

The first broad integration run built successfully and passed 80/85 (297.10 s). Its IR/source import ownership race, protected-swell stale expectation, footer geometry fixture and Storage receipt/reload readiness were repaired; focused native checks passed 4/4 (16.40 s). The next full run passed 84/85 (276.28 s), with only the merged baseline's Windows foreground prerequisite failure. After owner interaction the unchanged real popup/reset test passed 1/1 (2.30 s). A further 84/85 run (264.37 s) passed reset but exposed a temporary-peer focus race in the exact-value readability fixture; its focused retry passed (1.71 s). That fixture now uses a normal native peer, requests/awaits real foreground focus and asserts field focus before exact precision, retaining all value/commit/state assertions. The final full run above passes both. No assertion, system focus policy or production control behavior was bypassed. The [merged baseline](2026-11-06-issue-8-merge-baseline.md) remains distinct.

## Native approval

The corrected popup was launched directly, preserving the review window until the owner finished. The owner explicitly answered **"Approve corrected popup"**. Its controls/wording/modality are accepted. The same-editor metadata guard uses the same approved controls; its stricter failure scenario passes automated fixed/responsive tests. This records owner interaction approval separately from automated checks.

## Separate Windows profile

Supported `scripts/validate-portable-profile.ps1` passes with binary SHA-256 `0F42A7CEEEB489EBA55C6B67323B724AE19C2AB48D02507C8AD8D8F9A01701BC`. The owner explicitly approved retry after an earlier elevation cancellation; that canceled attempt started no validation and is not a passing result.

A new **standard** account/token SID ending `1023` uses its own loaded profile and default Projects root; original fixture roots are unavailable. Full frozen/history and real generated unfrozen cases each verify **14 exact paths**; context-only/zero-swell/explicit-empty-default verifies **5**. Every declared source/wet/history/context IR hash, creative edit/position/context span, source sample/silence/gap and saved wet origin/peak matches. The generated unfrozen fixture has a positive measured peak.

Complete compatible restoration performs **zero capture stages, zero convolution, zero IR resampling and zero new rendered results**. This is exact reuse, distinct from the separately tested rebuilding of missing unfrozen wet audio. Child exit 0; exact SID-owned account/profile removed. Credentials stayed in memory; no system policy changes. [Structured current result](2026-11-06-shared-swell-engine-profile.json); [earlier independent profile checkpoint](2026-11-06-issue-8-fresh-profile.md).

## Delivery boundary

Architecture tasks 1.1–1.3, 2.1–2.4 and 3.1–3.4 are verified; final validation/PR checkboxes are recorded with exact Apply source-path/line/description guards and mandatory post-task refresh. Portable task 3.10 closes only after these native/profile/approval checks; later association/writer migration, durable cleanup/eviction, compression and release gates remain untouched.

Git confirms `717c890` is an ancestor of merge `4defc2e`; GitHub reports PR #31 mergeable/clean after conflict resolution. No system VST3 install, new FL Studio run, Debug suite, second-PC or physical DPI matrix is claimed. Preserve original projects/assets and the superseded prototype stash for rollback. No canonical sync/archive, PR merge or issue closure.

Implementation is committed as `ace4a22`; PR #31 is updated for owner review. Refreshed Apply reports **all_done, 13/13 architecture tasks complete**. Portable recovery task 3.10 is guarded/verified complete and `portable-project-assets` is **20/36**, retaining its later storage gates. Final tracking/evidence edits do not alter tested implementation inputs.

# Tasks

Completed checkboxes below retain the initial implementation's verified evidence.
The owner passed FL presets, alignment and save/reopen, then approved the notice
speech and continuous-preview plan refinements. Section 6 is new work; prior
passing coverage does not establish its completion. No implementation is
authorized by this planning-update invocation alone.

## 1. Baseline and state identity

- [x] 1.1 Verify #6 is merged and its review findings resolved; establish an issue-specific branch without disturbing #71 artifacts, record baseline revision and approved 2026-11-05 policy in dated evidence. Completion: branch/status and dependency evidence inspected.
- [x] 1.2 Replace single-crossfade model fields with independent finite bounded levels through ExportState, ExportFile and SongExportPlan; introduce explicit state/gain-law versions and legacy wet-only preference reset preserving old history. Verify serialization round-trip, malformed input, old-project restore, unrelated state/host identity and immutable old-file references; update song-export migration guidance in the same task.
- [x] 1.3 Thread an atomic pair setter through export requests, coalescing and immutable identity; include both levels and new gain-law revision in identity and filename. Verify presets produce one intent, no-op updates do not dirty state, old-law history cannot satisfy new readiness, pending-save/restore and stale-session completion tests pass; document request ownership changes.

## 2. Transparent audio and measurements

- [x] 2.1 Gate ExportSource/SongMix dry and wet independently, remove Export aggregate wet/sum attenuation and preserve per-swell accepted calibration/shape/edge treatment and finite-sample checks. Extend SongMixCases with 0/100, 100/0, 100/100, intermediate and zero-pair sample oracles against separately prepared comparable layers, including overlapping and antiphase samples; document the approved equation and calibration boundary.
- [x] 2.2 Measure actual summed peaks and audible bounds, expose overload without modifying PCM, and make overview/detail contributions use accepted independent gains and one disclosed display scale. Verify above-unity, full cancellation, sparse gaps, pre-arrival wet, dry post-attack, song-origin lead-in and late-tail tests plus bounded page/cancellation behavior; document display scale versus PCM and peak status.
- [x] 2.3 Preserve ExportVoice preview versus ExportFiles Float32 parity and song alignment, excluding only disclosed preview boundary ramps. Verify decoded high-peak WAV retains above-unity samples, preview interior matches it within stated tolerance, and gain edits do not reconvolve or schedule live wet. Update preview/WAV policy guidance with actual test tolerance.
- [x] 2.4 Implement explicit zero-pair silent state and independent dependency warnings while retaining last-good pending/error presentation and immutable history. Verify preview/seek/Save/drag disabled at 0/0, dry-zero missing-source handling, dry-inclusive warned gaps, stale publication and file-write failure recovery; document silent readiness semantics.

## 3. Scoped controls and owner checkpoint

- [x] 3.1 Compose reusable accessible Mix Character selector plus shared SliderControl/NumericInputControl Dry/Wet rows in a native review fixture; use production tokens, semantic roles and session-pinned invocation policies. Verify keyboard/focus/wheel, typed validation, edit cancellation, exact preset/Custom derivation, unchanged-other-fader and programmatic-refresh component/binding tests; update shared-control ownership guidance.
- [x] 3.2 Launch one prepared native preview directly for owner review at 1100x850 and larger supported sizes; obtain selector/two-fader look and interaction approval plus listening approval of candidate preset names/pairs before production integration. Record explicit scope and results in dated evidence; any new token/variant requires owner rationale approval, standards revision and tests. Leave the preview open until review finishes.

## 4. Export integration

- [x] 4.1 Integrate approved controls into the existing right column and bind pair-based legend, waveform, peak warning, layer/replacement guidance and immutable filename/status. Preserve approved waveform/ruler dominance, below-waveform Zoom/Fit, transport, delivery actions and Export live-wet bypass. Verify component/binding tests for accepted versus requested pairs, pending/error, stale invocation, exact Custom/preset matching and zero state; update current Export workspace guidance and affected links.
- [x] 4.2 Verify current-revision Save/drag and asynchronous publication across rapid pair edits, mode changes, project restore and delivery cancellation/failure using focused engine/persistence integration tests. Prove original source/render bytes and old managed WAV paths/checksums survive; record evidence and ownership limitations alongside the tests.

## 6. Owner follow-ups: quiet notices and continuous preview

- [x] 6.1 Remove forced speech from shared NoticeBar without disabling accessibility; preserve severity/title/description/help and standard changed-value events for every consumer. Verify changed/unchanged/cleared notice lifecycle and accessible metadata, confirm no explicit speech path remains, and update the shared UI interaction policy documentation with the approved scope.
- [x] 6.2 Expose verified independent layer pages through the existing SongMix reader and extend fixed ExportVoice storage for dry/wet stereo branches under the shared memory budget. Verify sample/range/channel equivalence, sparse spans, calibration, cancellation, ownership and slot reuse; assert bounded working memory and no source/render mutation. Document layer layout, budget and dependency lifetime.
- [x] 6.3 Add coherent atomic target-pair acceptance and 5 ms monitoring-only gain smoothing to an already-running voice without replacing its transport ticket or cursor. Verify exact elapsed-frame continuity at unaligned blocks/pages, transition sample oracles, steady-gain WAV parity, rapid latest-intent edits, finite end and callback allocation/deallocation/lock/I/O safety. Document the transition policy and its exclusion from delivered WAV samples.
- [x] 6.4 Separate preview session/pass/dependency ownership from WAV request generations; keep verified preview supplied during coalesced/delayed/failed publication while preserving immutable current-file readiness. Verify Stop/seek, mode exit, restore, dependency invalidation, 0/0 then nonzero, starvation and out-of-order completion races cannot restart or rewind playback, reuse stale targets or offer an old WAV as current. Extend engine/persistence/fault tests and ownership guidance together.
- [ ] 6.5 Bind the accepted preview target/revision, advancing cursor and retained/pending waveform/peak presentation independently of file readiness. Preserve view context during playing gain edits and preserve stopped behavior. Verify component/binding tests for preset/fader changes while playing, pending/error file states, zero state and stale invocations; update song-export, Export workspace and musician-facing guide instructions, and launch the affected native preview directly for focused feedback without a new visual redesign.
- [ ] 6.6 Run affected focused native regressions, UI standards, guide currency and final Debug/Release wrapper builds/tests for the follow-ups. Record exact revision, commands, failures/recovery, memory/callback/timing/sample results and native versus host/DPI limits in dated evidence. Preserve earlier accepted tests; do not claim the old full-suite runs verify the new preview implementation.

## 5. Integration validation and review delivery

- [x] 5.1 Run scripts/check-ui-standards.ps1 and scripts/build.ps1 -Configuration Debug -Test followed by Release -Test using the wrapper lock and existing warm trees. Resolve failures and record revision, commands, environment, cached/cold status, focused versus full/native/fixture results and skipped coverage once in dated validation evidence.
- [ ] 5.2 After section 6 passes, deliver the revised native/FL test build and obtain a brief targeted owner check for no forced speech and smooth preset/fader changes at the moving playhead. Retain the owner's passing presets, export alignment and save/reopen acceptance; request repeats only if affected behavior changes or a concrete failure warrants it. Record actual host/DPI coverage and unresolved limits, not inferred results.
- [ ] 5.3 Review final scope and diff, commit only #51 work on its existing branch, and update existing PR #92 with issue/change links, policy approval, migration notes, retained and new test evidence and outstanding limitations. Verify the PR targets main and remains open for owner review; do not create a duplicate, self-merge, independently close the issue or archive the change.

## Workflow follow-up

- Apply begins only after a new user request following presentation of these planning artifacts. Refresh vanilla Apply instructions and verify each task individually when completing it.
- Archive/sync only through the existing OpenSpec workflow when explicitly requested after review requirements are satisfied.

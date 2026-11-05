# Tasks

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

## 5. Integration validation and review delivery

- [ ] 5.1 Run scripts/check-ui-standards.ps1 and scripts/build.ps1 -Configuration Debug -Test followed by Release -Test using the wrapper lock and existing warm trees. Resolve failures and record revision, commands, environment, cached/cold status, focused versus full/native/fixture results and skipped coverage once in dated validation evidence.
- [ ] 5.2 Launch integrated native Export preview for final owner acceptance of approved controls and real reference audio states at minimum/larger supported sizes. Consolidate FL Studio testing into one brief pass covering preview, wet-layer versus replacement routing, song-aligned Save/Playlist drag and new-state restore; record actual host/DPI coverage and remaining limitations without repeating passing automated edge cases.
- [ ] 5.3 Review final scope and diff, commit only #51 work on its issue branch, and open/update one PR linking #51, this change, policy approval, migration notes, test evidence and outstanding limitations. Verify PR targets main and remains open for owner review; do not self-merge, close the issue or archive the change.

## Workflow follow-up

- Apply begins only after a new user request following presentation of these planning artifacts. Refresh vanilla Apply instructions and verify each task individually when completing it.
- Archive/sync only through the existing OpenSpec workflow when explicitly requested after review requirements are satisfied.

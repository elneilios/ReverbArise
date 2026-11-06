# Issue #4 — chronological Swells list

OpenSpec: `sort-swells-by-song-position`. Planning baseline: main `33f1871`,
proposal commit `edb7dff` / PR #47. The owner approved the proposal in chat on
4 November 2026 and requested Apply. This record distinguishes the standalone
list checkpoint from later production integration.

Current automated outcome: **42/42 Release CTest passed** at clean commit
`a2a014d` after the separately authorized frozen-tempo repair. Integrated owner
approval is recorded, and the owner reported the final FL Studio live test
passed on 4 November 2026 using the supplied build. Debug and a physical Windows
DPI/monitor-transition matrix were not run.

## Execution and review scope

| Tasks | Type / risk | Verification and dependency |
| --- | --- | --- |
| 1.1 | Pure UI projection / medium | Native numeric/identity ordering tests; no model mutation |
| 1.2–1.3 | UI interaction and snapshot state / medium | Compiled JUCE component event, selection, scrolling and virtualization tests |
| 1.4 | Native UI acceptance / medium | Directly launched standalone fixture; owner look/interaction approval before production rollout |
| 2.1–2.2 | Editor/session identity and restore bindings / high | After checkpoint approval: editor, capture reconciliation and persistence regressions |
| 3.1–3.2 | Final integration / medium | Wrapper Release/CTest, UI guard, native integration review and one brief FL Studio smoke test |

The reusable component preserves the existing Perform row appearance. It adds
no token or visual variant, changes no session schema and performs no audio,
capture, asset or model operation. The Perform editor now binds it by persistent
ID. Coherent snapshot epochs and lock-checked row commands reject stale restore
completions; targeted deletion preserves the existing select-then-delete policy
atomically. Existing unguarded engine overloads remain available to other callers.

## Standalone checkpoint

- `scripts/check-ui-standards.ps1`: passed with 34 governed files and the same
  61 bounded definition/debt occurrences; debt inventory unchanged.
- `git diff --check`: passed for the current tracked diff.
- This worktree started with fresh compiler outputs; pinned dependencies were cloned locally
  from the existing checkout using process-local Git URL mapping. No CMake cache
  was copied, deleted or shared and no raw build overlapped the wrapper.
- Isolated native MSVC fixture: `out/issue4-fixture/build.ps1` compiled both
  `tests/swell_list_order_test.cpp` and `tests/swell_list_test.cpp` with VS 2026,
  C++20 and Release `/MD /O2`, linking the existing checkout's cached JUCE/shared
  static libraries. Both executables passed (exit 0). Outputs are separate from
  both wrapper build trees; this is not a CTest or new product-build result.
  A first link attempt exposed the cache's missing modal-loop helper; the review
  now uses JUCE's ordinary event loop. The first component run caught an incorrect
  popup-anchor rectangle overload; corrected and both tests rerun successfully.
- Projection cases: numeric 2/10, signed coordinate limits, ordinal same-position
  IDs, shuffled definitions, arbitrary/renamed names, independent wet placement,
  unavailable musical timing and no definition mutation.
- Component cases: pointer checkbox/menu and Ctrl selection, keyboard primary
  after reorder, fractional-row scroll anchoring, insertion above viewport,
  moved visible primary, offscreen primary, cosmetic/identical refresh, deleted
  anchor, session epoch reset, empty state, 500-row virtualization and software
  raster snapshots at 100/125/150/200%. Refresh submitted no selection callbacks.
- Directly launched `out/issue4-fixture/swell_list_test.exe --review` (PID 26204)
  and verified the unique native window through the computer-use plugin. The
  window and live row accessibility names are present; initial primary `id-0009`
  is visibly selected. Review remains open for the owner. Software component
  captures were prepared at outer window sizes 1100x850 and 1400x900 under
  `out/issue4-fixture/screenshots/`; these are supporting fixture evidence.
  Preserved: [minimum fixture](2026-11-04-issue-4/list-review-1100x850.png) and
  [larger fixture](2026-11-04-issue-4/list-review-1400x900.png).
- Strict OpenSpec validation passed. Each completed task was individually
  guarded, checked and confirmed through refreshed Apply instructions.
- The owner approved the standalone list's **look and feel** in chat on
  4 November 2026. This authorizes editor integration; the preview remains open.
  It does not establish integrated editor, physical DPI or FL Studio acceptance.
- Integrated owner acceptance, Debug, physical Windows DPI and FL Studio remain
  separate from that standalone approval.

## Production integration verification

The full native editor test covers actual later-then-earlier host passes, a real
uncaptured gap, looping within a pass, explicit Recapture, manual/frozen metadata,
shuffled saved definitions, same-position ties and processor save/reopen. Exact
ID-based settings/IR/render/source/shape/placement and selection survive. Canvas
and inspector target movement reorder rows without sorting durable definitions.
Open rename/delete commands follow IDs across reordering; obsolete commands are
rejected even when a restored session reuses the same ID. Refresh emits no dirty
notification and does not increase generation counts or change saved definitions.

The component's recycled row surface also pins ID/epoch at press time: release
after reordering still addresses that ID, while deletion/session replacement
cancels the press. Tests retain normal pointer/Ctrl/keyboard selection and native
viewport scrolling; no new drag-and-drop behavior or audio action is added.

Native component/software-raster evidence covers 1100x850, 1400x900, 1400x1000
and 1600x1000 logical editor sizes, including empty, selected/restored and 500-row
states, plus 100/125/150/200% software scales. Representative captures:

- [Restored selection, minimum](2026-11-04-issue-4/ordering-restored-1100x850.png)
- [Restored selection, 1400x900](2026-11-04-issue-4/ordering-restored-1400x900.png)
- [Restored selection, 1600x1000](2026-11-04-issue-4/ordering-restored-1600x1000.png)
- [500 rows, minimum](2026-11-04-issue-4/many-1100x850.png)
- [500 rows, largest](2026-11-04-issue-4/many-1600x1000.png)
- [500 rows, largest at 200% software scale](2026-11-04-issue-4/many-1600x1000-scale-200.png)

Intermediate validation found and corrected a popup rectangle overload, an
ambiguous point in the held-press test, unsupported Home/End keys in the popup
test driver and a test fixture that froze a renderless definition (real frozen
asset preservation is covered by the host-pass test). Early builds overlapped
source editing and contained stale-library link errors; incremental wrapper
rebuilds resolved those without cache deletion. No production export, capture
admission, generation or scheduling behavior was changed to accommodate tests.

The 10:24 native wrapper run built successfully and passed every issue-specific
test, including the 27.33 s screenshot-enabled editor test and 2.23 s engine test.
It reported 41/42 overall because unchanged `PerformanceExportCases.h:99` failed
its frozen recovery transition after tempo changes. An isolated full processor
test rerun passed without source changes. This intermittent adjacent-test result
is retained here rather than represented as a clean first pass. The unchanged-
source wrapper rerun (`out/build-logs/Release/20261104-102656-b313ee2b/`) again
built successfully and passed **41/42**, failing that same assertion (51.90 s
total CTest time; exit 8). A second focused `performance-export` run also passed.
The full-suite failure remains unresolved; no all-green final gate is claimed.
The original checkout has separate in-flight issue #6 export work; none of it
was copied into this issue. The owner was asked whether to authorize adjacent
investigation/repair or leave #4 draft. Tasks 1.1–2.2 are verified complete
(6/8); final validation and integrated/host acceptance remain unchecked.

Current verification environment: Windows x64, VS 2026 Community/MSVC 14.51,
CMake/CTest bundled with VS 2026, Release, `edb7dff` plus this working diff. Later
runs reused this worktree's compiled outputs (parallel build level 4); the initial
build was cold with locally reused dependency sources. No speedup comparison is
claimed. Screenshot-enabled results are from the 10:24 run; the subsequent run
reused that unchanged-source evidence rather than regenerating the gallery.

The validated VST3 bundle is under
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`;
its x64 binary SHA-256 is
`667460d8a18543f4beaa3a5c6a23d77e1028673a748506572bb4d2979e5ce55b` (post-repair).
It has not been copied into a host's installed plugin location.

## Authorized frozen-tempo recovery repair

On 4 November 2026 the owner explicitly authorized investigation and repair of
the adjacent final-suite failure. It is a conformance repair to the existing
timing-incompatibility/render-freeze contract in `transport-swell-playback`,
separate from #4's list behavior and from the original checkout's issue #6 work.

The deterministic regression waits until the worker actually marks the frozen
render stale at 121 BPM, then restores its saved 240 BPM without changing source.
Before the repair, `preverb_processor_test performance-export` timed out at
`PerformanceExportCases.h:102`: the unchanged-capture early return prevented
the existing frozen compatibility/revalidation path from running. Build:
`out/build-logs/Release/20261104-103506-42b55656/`; focused failure log:
`out/issue4-fixture/frozen-recovery-before.log`.

The repair moves the same frozen rate/tempo/meter admission check ahead of that
early return. It schedules the existing worker's asset verification, never sets
Ready from metadata alone and adds no callback I/O, stretching, source mutation
or reconvolution. It preserves the saved render/edits and capture reference.
The regression also deletes the test render and repeats the tempo round trip:
verification reports Error, live eligibility stays false, and the frozen flag and
saved reference remain intact. Existing offline missing-asset checks still run.

Post-repair Release build passed through the wrapper
(`out/build-logs/Release/20261104-103650-fefe84af/`). The deterministic focused
native test passed in 0.72 s (exit 0), using this worktree's warm outputs, not the
earlier isolated cached-library fixture. Final full-suite verification follows.

Final gate: `scripts/build.ps1 -Configuration Release -Test` passed at **clean
`a2a014d`**; preflight/configure/build/test all pass, **42/42 CTest**, 47.19 s,
exit 0. Logs: `out/build-logs/Release/20261104-103840-1d79528c/`. The worktree's
build outputs were warm; no source/cache deletion or dependency download was
needed. `scripts/check-ui-standards.ps1` passed (34 files, 61 bounded occurrences),
strict OpenSpec validation and diff whitespace checks passed. No production
source changed after this gate. The historical 41/42 failures above are resolved
by the deterministic repair, not waived or hidden by retrying the old assertion.

The real production editor preview was directly launched with
`preverb_swell_ui_test --review-ordering` (PID 58488), then its unique
`ReverbArise control review` window and row accessibility were inspected through
the computer-use plugin. It uses a temporary synthetic project with actual
captured source and restored selections, independent of the user's DAW projects.
The standalone review window had already closed; no duplicate review was opened.
Task 3.1 was guarded, checked and confirmed through refreshed Apply instructions:
**7/8 complete**. The owner subsequently approved the integrated composition in
chat on 4 November 2026. The single FL Studio smoke remains pending: the owner
could not find the hidden worktree build path. The whole tested bundle was copied
to `D:\Users\neila\Desktop\ReverbArise-Issue4-a2a014d\ReverbArise.vst3` and its
parent folder opened in Explorer. All six bundle files were SHA-256 checked
against the original; the binary hash matches the post-repair hash above. No
existing bundle was overwritten. It is not installed into FL Studio automatically.

The owner subsequently reported **“FL Studio live test passed”** in chat on
4 November 2026, completing the requested final host smoke checkpoint after the
integrated approval. This is owner-reported host acceptance, separate from the
42/42 native automated result. Exact host/driver versions and a physical DPI
matrix were not supplied; no additional coverage is inferred. Production source
and the tested binary are unchanged. All eight implementation/review tasks are
complete; PR #47 is ready for owner review, with merge, issue closure, canonical
spec sync and archive left to the owner.

Component event tests dispatch JUCE pointer/keyboard events in a native MSVC
executable. Software-raster snapshots at multiple scales are not physical DPI
or FL Studio acceptance. The standalone review owns only synthetic row metadata
and cannot change a DAW project or start audio.

# Issue #46 — Perform integration validation, 4 November 2026

Latest-main [reconciliation evidence](2026-11-04-issue-46-main-reconciliation.md)
records the subsequent integration of owner-merged #53 and #4 changes, preserving
the exact scope of the #46 host acceptance and the original tested bundle.

Final: the owner reports **Host smoke passes** for the final presentation bundle.
All **21/21** OpenSpec tasks are complete; [final host acceptance and exact bundle
identity](2026-11-04-issue-46-presentation-adoption.md) supersede the historical
pending-host/progress notes below. No further code changes or repeated builds
were needed for this acceptance record. Actual Windows DPI details remain unspecified.

Owner acceptance: **Approve revised presentation**, 4 November 2026. The reviewed
profile is adopted in production; see [adoption validation](2026-11-04-issue-46-presentation-adoption.md)
for the new build and remaining host check. Earlier checkpoint measurements below
retain their original scope.

Current OpenSpec progress is **20/21**, with task 5.4 accepted/adopted and only
6.2 final host acceptance pending. Final Release build passes; corrected focused
native tests pass 10/10, with all 58 unique tests covered across full/focused runs.


Latest owner steering routes all hover help to the normal-font bottom footer:
[current native checkpoint](2026-11-04-issue-46-hover-footer.md). Floating tooltips
are removed. OpenSpec remains **19/21** pending 5.4 visual acceptance/adoption
and 6.2 final host acceptance; previous audio/interaction approvals remain valid.

The [latest Perform clarity checkpoint](2026-11-04-issue-46-perform-clarity.md)
addresses subsequent owner feedback with shared ruler/mode clarity, grouped
inspector/exact editing, concise annotations, footer and heading alignment.
**9/9 focused native tests pass**; visual acceptance/adoption remains task 5.4.

Latest: the owner requested a slimmer inline audition row and heading alignment.
The [isolated 52-pixel native checkpoint](2026-11-04-issue-46-audition-strip.md)
is open; production adoption awaits actual metric approval. OpenSpec is now
**19/21 complete**, pending slim adoption (5.4) and final host acceptance (6.2).

Current audition refinement is source `4e7aff9`, with **55/55 native Release
CTests passed** in 152.64 s through the full wrapper. The owner approved layout,
Snap, integrated editing, revised knob steps/focus and reset, and reported the
host editing looks/feels good. The subsequently requested audition loop/Wet only/
status refinement is implemented and its native preview is open; final host
audition listening/save-reopen remains pending. OpenSpec is **19/20 complete**.
See [current audition evidence and bundle identity](2026-11-04-issue-46-audition.md).
The initial 52-test results and pre-knob bundle identity below remain historical
evidence; the shared output bundle path now contains the final rebuilt binary.

## Baseline and approval

Issue [#46](https://github.com/elneilios/ReverbArise/issues/46), PR
[#48](https://github.com/elneilios/ReverbArise/pull/48), OpenSpec
`realign-perform-workspace`. The owner approved actual native compact revision 4
and the separate latching Snap family. Earlier measurements, requested divider/
refresh corrections and approvals remain in the
[composition evidence](2026-11-04-issue-46-perform-composition.md).

Merged Export PR #50 is main `9b8e70fd90193b67716e3368e9c7bd0cfb1fd209`.
The owner merge notice releases the final editor/serialization hold from
[Agent 1's handover](https://github.com/elneilios/ReverbArise/issues/46#issuecomment-6632200424).
This branch merged that main revision at `c04676a` before production mutations.
ExportWorkspace, its navigation adapter, mix/voice/file implementation and the
audio process callback are unchanged by this integration. Schema 10 and envelope
8 are preserved; optional Perform UI preferences use the actual merged decoder.
The `swell-workspace` delta retains merged Export's dedicated preview/mix/file
description and truthful current-WAV state. Canonical sync/archive remains a
separately requested operation.

## Implemented contracts and verification map

| Contract | Evidence |
| --- | --- |
| Approved compact composition; continuous rules; readable/scrolled inspector | `preverb_swell_ui_test`, actual editor binding checks and existing 60-case/240-render composition matrix; named profile/standards revision 3 |
| Existing list/IR/Freeze/Solo/exact values, long names and retained selection | Actual native editor regressions, `SwellListCaptureCases`, existing stale invocation/control cases; no new list/bulk model |
| Bounded logarithmic Zoom, anchors and Overview | Independent viewport oracle plus real `preverb_perform_navigation_test`; no source/model/asset changes |
| Initial/explicit Fit, pending source/request metadata, actual placed tail/overlap, sparse/empty | `preverb_perform_fit_test`; selection/background/mode-return non-refit in actual editor binding test |
| Host grid and bounded domains | Independent integer oracle in `preverb_host_grid_test`: meters, fractional/negative PPQ, half ties, loops/seeks, overflow and incompatible timing |
| Actual target/source/start/end/fade/translation gestures | `preverb_perform_gesture_test`: independent expected 6000-sample grid with nonzero placement, zero fade/no legal grid, canonical ordered setters and source revision protection |
| Requested/effective Snap and Ctrl/Shift/Ctrl+Shift | Latching component tests and real editor/gesture tests; no-motion modifier rebase, focus loss, key-only no job/dirty notification, saved Off restoration, accessible checked state |
| Stale publication | Frozen mapping/session/selection/source identities, atomic engine edit guard, actual mode switch, epoch-bound toolbar and UI-state setter tests |
| Preferences | Actual schema-10 decoder current/legacy/malformed per-field cases; processor packet save while Ctrl held, editor reopen, copy/restore, Export preservation and instance isolation |
| Learn/Perform/Export transitions and async IR/pending/frozen source | Existing full native editor/processor/workflow regressions plus new mode-return/navigation checks; no #38 Generate or #5 audition implementation |

`ActionButton.h` remains byte-identical to Agent 1's accepted version (SHA-256
`68EF2F339109F65C1044813C1CD074C5AA13E8DEE7470571452ECF2B1EEAAA16`).
Shared Zoom/Fit/ruler controls contain no Export-specific behavior. The shared
no-wheel policy moved unchanged from IrSelector to DeliberateSelector; exactly
that path changed in the existing lint inventory, with no debt count expansion.

## Environment and results

Windows x64, Visual Studio 2026/MSVC 18.11, SDK 10.0.26100, JUCE 9.0.4, native
Release configuration. Warm build tree and pinned dependency sources retained;
new merged Export targets still required compilation. No cold-build or speedup
claim. Native results below are MSVC/CTest, not substitute script fixtures.

- Focused native build passed. Seven of eight selected CTests passed initially;
  the existing list-capture setup assertion failed before Perform gesture work
  (`SwellListCaptureCases.h:33`). The unchanged executable passed a direct rerun.
  PR #50 already records this timing-sensitive fixture flake. It recurred in the
  first full suite (51/52, 165.95 seconds; `20261104-172407-dfeb8c3f`). The fixture
  now waits for an increased analysis counter and the expected swell count before
  accepting idle/ready, retaining every identity assertion. The next full run
  exposed the fixture re-requesting Learn after arming recapture, clearing the arm
  (UI timeout; 51/52, 187.22 seconds; `20261104-172952-1fc692e8`). The fixture now
  preserves an already prepared Learn pass. The strengthened native UI check then
  passed in 8.59 seconds (`20261104-list-fixture-arm-*`). No production capture
  code changed. Logs: `out/build-logs/Release/20261104-perform-final-focused-*`.
- All four new real-component tests passed: navigation, gestures, actual editor
  bindings and contextual Fit. Host-grid, shared controls and session decoder
  tests passed in the focused run.
- During development, legacy UI assertions expecting Snap On to ignore wet
  endpoints/keyboard were corrected to the new approved grid policy; exact
  one-sample assertions now explicitly use Ctrl bypass. Native test stack overflow
  from a stack-allocated processor was fixed by heap allocation, matching existing
  fixtures. Earlier logs remain available; these are test setup corrections.
- `scripts/check-ui-standards.ps1`: passed, 44 files / 61 bounded occurrences.
  `openspec validate --all --strict`: 20/20 passed; `git diff --check` passed.
  Lint debt was not regenerated.
- Final `scripts/build.ps1 -Configuration Release -Test`: **52/52 passed**, 174.77
  seconds. Configure/build/test all passed, exit 0; logs
  `out/build-logs/Release/20261104-173418-f5e907a9/`. The wrapper recorded
  `98000be (dirty)`; all implementation/test sources match `98000be`, with only
  documentation/evidence/task tracking uncommitted. Production code is in
  `a5cb0aa`; the fixture-arm correction is `98000be`. No source changes followed
  this successful run. The final 60-case/240-render matrix passed within CTest.

Actual production-editor captures from the successful native UI test are retained
under [integrated](2026-11-04-issue-46/integrated/):
[selected minimum size](2026-11-04-issue-46/integrated/ready-1100.png),
[dense minimum size](2026-11-04-issue-46/integrated/many-1100x850.png) and
[empty](2026-11-04-issue-46/integrated/empty.png).
These are native component renders, not generated mockups. The capture run passed;
its log is `out/build-logs/Release/20261104-perform-integrated-capture.log`.

## Callback and compatibility review

Optional UI publication occurs through the existing engine message-thread lock
and coalesced host-state notification, guarded by session epoch. It neither
invalidates swells nor schedules rendering. Modifier changes only update transient
component state. All grid arithmetic is bounded work outside the audio callback.
The process callback, capture admission, DSP kernels, render workers and asset
formats are untouched. Navigation/state tests preserve source manifest revision,
musical IDs, render/IR references and job counts. Existing native regressions
retain their audio/state/identity assertions.

## Owner/host coverage

The final native preview uses the actual production editor and functional
Zoom/Fit/Snap/grid, with one editor refresh timer, real captured test PCM/IR and
no audio device. Its separate review strip selects sparse/empty/dense scenes and
four supported sizes. Launch: copied validated executable at
`out/review-issue46-integrated-final/perform-integrated-review.exe --review-integrated`.
Native layout/Snap family approvals are retained, not repeated.
The owner explicitly approved this integrated native preview on 4 November 2026:
“Approve integrated native preview; FL Studio smoke pending.” Task 6.2 remains
open only for the final host smoke; native acceptance does not need repeating.
One brief final FL Studio smoke remains: usual-scale editing with Ctrl, Shift
and Ctrl+Shift; Zoom/Fit; mode return; project save/reopen. No repeat of accepted
Export delivery/mix tests is requested. Actual Windows DPI, screen-reader and
final host outcomes remain unverified until the owner reports them; software
100/125/150/200% raster checks are not Windows DPI acceptance.

Initial pre-knob validated bundle (historical hash; not installed automatically):
`C:\Users\neila\.codex\worktrees\670e\k7ma-preverb\out\build\windows-msvc\preverb_plugin_artefacts\Release\VST3\ReverbArise.vst3`.
Inner binary `Contents/x86_64-win/ReverbArise.vst3` SHA-256:
`B654D882500A4E9EA380196CCE34E8927444F5D8ACCFECF84D53E14C9947F17E`.
Preview executable SHA-256:
`0BF16B4E63A568642837FAD09B5E33D20ACFE15223BD064DE7CB5BBFD0CCA8E1`.

No Debug full build, waveform optimization, tempo-map support, new Learn
generation, bulk editor or full-arrangement audition was performed in #46.

## Approved follow-up after native acceptance

The owner subsequently approved timing-knob drag/wheel snapping with Ctrl/Shift
policy and unsnapped typed entry. OpenSpec task 4.4 records this pending addition;
the plan is now 16/18 complete. The [knob audit](2026-11-04-issue-46-knob-audit.md)
records standards/guard gaps and the owner's long-helper-text compression report.
At that checkpoint these were planning/audit updates only. The initial validated
binary above predates the knob extension and helper-text correction.

The owner then explicitly requested Apply of these changes. Task 4.4's behavior
and all audit corrections are now implemented and pass the final 53-test Release
suite. Fixed wheel/arrow steps and display precision use the owner-approved
policy; positions are musical and durations/movement are time-based. The old
sample-unit toggle is hidden from ordinary Settings. The focused revised-knob
native review is open; its approval and the final FL Studio smoke remain pending.
See the linked current evidence for exact source, hashes, captures and failed-run
recovery, rather than using the historical pre-knob hash above.

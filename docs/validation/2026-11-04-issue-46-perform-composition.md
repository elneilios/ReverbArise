# Issue #46 — native Perform composition checkpoint

## Scope and authorization

The owner reviewed PR #48 and approved `realign-perform-workspace` in principle on
4 November 2026, then explicitly authorized Apply starting with an isolated native
composition preview. Actual compact metrics and materially changed control families
remain subject to preview approval before production adoption. This record covers
that checkpoint, not completed #46 implementation.

Baseline: `f7caf5d` on `codex/46-perform-workspace-plan`, based on main `33cd54b`.
The initial checkpoint changed only the test executable/CMake registration,
planning design and evidence. Revision 2 adds the sole owner-approved header seam
described below; the production plugin does not instantiate the preview or adopt
its metrics.

## Execution and ownership map

| Tasks | Work / risk | Dependency and verification |
| --- | --- | --- |
| 1.1 | Cross-workspace ownership / high | Owner-coordinated #6 contract before shared mutations; remains open |
| 1.2–1.3 | Isolated native UI / medium | Actual borrowed components, fixture matrix, measured layout, then owner approval |
| 2.1–2.3 | Shared controls and production UI / medium | Layout/family approval, binding/geometry/standards tests before adoption |
| 3.1–3.3 | Viewport/toolbar / medium | Approved composition, independent transform oracles and component tests |
| 4.1–4.3 | Musical timing/gesture state / high | Fixed-map numerical oracles, guarded edits and modifier lifecycle tests |
| 5.1–5.2 | Persistence/workflow / high | #6 ownership, actual decoder and legacy/malformed/restore/identity cases |
| 6.1–6.3 | Integrated release/review / high | Full Release wrapper/CTest, final native acceptance, one host smoke and PR review |

The CLI reports **17** tracked tasks (the initial planning PR's count of 18 was a
description error). No completion is inferred from planning artifacts being done.

## Export coordination

At Apply entry, `feat/6-export-mixed-song-wav` still points to `33f1871`, but Agent 1
has active uncommitted Export implementation in the original checkout. The chat
`Find the best model for C++` shows Export work including a read-only
`Controls/TimelineNavigation.h`, Export preview/detail and mix ownership. These
are unmerged APIs, not a safe integration baseline.

The owner explicitly authorized coordination. A message to that chat was attempted
through Codex on 4 November; it was rejected with “already has an active writer”.
No message was delivered. A second attempt after its latest turn completed received
the same active-writer rejection. Task 1.1 remains open. The independently authorized fixture
does not edit production timeline, audition, decoder or editor composition paths.
Before those edits, record a concrete then-current revision and ownership contract.
At final integration reconcile the workspace delta with merged Export requirements.

The owner subsequently forwarded [Agent 1's handover](https://github.com/elneilios/ReverbArise/issues/46#issuecomment-6629874432).
It confirms the current Export visual approval and a coordinated fuller musical
ruler/shared Zoom row, not a competing Export toolbar or viewer refactor. Absolute
sample bounds, accepted timing, `SwellViewport` and navigation intents match the
proposed boundary. Export keeps its Fit target, pointer navigation, refresh/manual
view, accepted-mix Fit, last-good pending view and current-revision file readiness;
mix, voice and state stay with Agent 1. Merge/archive is held until integration or
an explicitly approved follow-up. Exact slider-state adapter/accessors and final
shared-file integration still need alignment; task 1.1 is not marked complete.

Read-only working snapshots on `feat/6-export-mixed-song-wav` at branch HEAD
`33f1871` (uncommitted Export implementation):

| File | SHA-256 |
| --- | --- |
| `Source/UI/Controls/TimelineNavigation.h` | `0AF651E8B96A6022FEFC30D788171D96334A54DEAE78F084C2D6FA3969ADFAAD` |
| `Source/UI/ExportWorkspace.h` | `C014E716C470D37D6004287D9E6ADC5CA478FF6DE17783326676AB529CBB3383` |
| `Source/Core/SwellSession.cpp` | `A7CC163FD31C4F96969A04CD9C9A5F43A887417ECFD04EF3DF37EEDDBBA1141D` |
| `docs/design/export-workspace.md` | `E605921BEE60CA0C99FE7D2CDBAA6E49DFD880DEEA5437DB6375266F24FF7B46` |

#4's merged `SwellList` owns sorting, selection and scrolling. #38's pending-render
entry/return requirements are preserved as integration constraints only; #5, #9,
#38 behavior and waveform caching are outside this checkpoint and issue.

## Fixture and candidate measurements

`preverb_perform_composition_preview` borrows actual components from a hidden,
living production editor: header, list, timeline, inspector, audition controls and
Overview. The fixture reparents them back before owner/editor teardown. It uses a
disposable processor root, real captured test PCM with an uncaptured interval and
a real generated IR/render. Dense rows reuse the existing stress-fixture technique
of unique IDs sharing real test render data. Empty/rendering/error are controlled
display cases, explicitly identified as fixture states; they are not host failures.

All candidate metrics are confined to the test translation unit. Inspector fields
retain production bindings while the fixture composes existing controls with named
production text roles. Unchanged internal timeline/list typography remains legacy
where painted by those components; this checkpoint does not claim their later
production-role migration complete. Zoom/Fit use existing navigation. Snap/grid are
visual specimens, explicitly unconnected to the proposed grid/Ctrl behavior, which
must be implemented progressively after layout approval.

The review window provides scene and logical-size selectors outside the product
composition. Its extra 44-pixel review strip is excluded from layout measurements.

Native measurements (logical pixels; lane rectangles are local to the timeline):

| Editor | List width | Inspector width | Detail component | Each dry/wet lane | Toolbar |
| --- | --- | --- | --- | --- | --- |
| 1100x850 | 180 | 304 | 552x378 | 548x128 | 552x48 |
| 1400x900 | 210 | 320 | 798x420 | 794x149 | 798x48 |
| 1400x1000 | 210 | 320 | 798x520 | 794x199 | 798x48 |
| 1600x1000 | 210 | 320 | 998x520 | 994x199 | 998x48 |

Column gaps are 16, list rows remain 64 and Overview height is 120. Inspector
content is 650 high and scrolls within its allotted viewport; fonts are not shrunk
to expose every field simultaneously. The Swells count uses a separate 14-pixel
metadata role so 200 fits beside the 21-pixel heading and add action. At 1200x850,
the existing larger outer inset yields 370 detail height and 124-high lanes; the
initial fixture assertion incorrectly assumed the 1100x850 height at every width.
That failed assertion was corrected, with no production geometry changed.

The final matrix covers six scenes at 1100x850, 1199/1200/1201x850,
1239/1240/1241x850 (the independent header breakpoint), 1400x900, 1400x1000 and
1600x1000: **60 native geometry/typeface/focus/selection cases and 240 software
scale renders passed**. Assertions cover finite usable lane bounds, one-row
non-overlapping controls, full focus ink containment, keyboard focus targets,
embedded Medium/SemiBold roles, header-child containment and selected-row
containment after explicit fixture reveal. This is not OS-DPI acceptance.

Retained evidence: [measurements](2026-11-04-issue-46/measurements.json),
[selected minimum](2026-11-04-issue-46/sparse-1100x850-100.png),
[dense minimum](2026-11-04-issue-46/dense-1100x850-100.png),
[empty minimum](2026-11-04-issue-46/empty-1100x850-100.png), and
[large selected](2026-11-04-issue-46/sparse-1600x1000-100.png).
All generated raster variants remain under `out/perform-composition-owner-checkpoint/`.

## Environment and commands

First build in this worktree: Windows x64, VS 2026 MSVC, Release configuration.
The unchanged wrapper `scripts/build.ps1 -Configuration Release` owns the build
directory lock. The binary tree is cold; pinned dependency source checkouts are
reused read-only from the existing issue #4 worktree through an ignored local
CMake toolchain include. No CMake cache or compiled object was copied.

- JUCE 9.0.4: `3fb5cbc0340758596dac39f7c8de47d562b4896e`.
- WavPack 5.9.0: `5803634a030e2a11dba602ba057b89cc34486c67`.
- Configure/build logs: `out/build-logs/Release/20261104-112953-d31c73ea/`.
- Final incremental build logs: `out/build-logs/Release/20261104-120049-9cad3a99/`.
- Focused command: `preverb_perform_composition_preview.exe` with
  `PREVERB_PERFORM_OUTPUT=out/perform-composition-owner-checkpoint` (absolute path).
- Interactive checkpoint: the same executable with `--review`, launched directly
  for the owner; preserve that process until review finishes.

## Results and limitations

Release wrapper builds passed; full CTest was **not requested/run** at this isolated
layout checkpoint. The final native matrix above passed, and existing
`preverb_swell_list_test.exe` and `preverb_swell_viewport_test.exe` both exited 0.
`scripts/check-ui-standards.ps1` passed (34 governed files, 61 bounded occurrences);
strict OpenSpec validation and diff whitespace checks passed. No debt inventory
was regenerated. Those focused existing results remain applicable because the
subsequent edits changed only the preview fixture and planning/evidence files.

The initial native matrix caught the height assertion above. Image review then
corrected asynchronous restored-data staging and crowded count presentation;
settled cases now await the real selected wet render, and test captures explicitly
reveal the selected row after resizing. The final matrix passed after those fixes.

Executable: `out/build/windows-msvc/Release/preverb_perform_composition_preview.exe`.
SHA-256: `B1698BD4E1E1E365297FD568E5E6EEEE4131ECE8C341980F47D235A6D51CD503`.
Source SHA-256 (`tests/perform_composition_preview.cpp`):
`4541DCDDFC4CA6867A203F69F5C77A8040E253644887C1AA3BE1B8857B2BCEDD`.
Build reports `f7caf5d (dirty)`; this PR's source diff identifies those tested changes.

The responsive native window **Issue #46 - Perform layout checkpoint** was launched
directly (PID 48812) and left open for owner review. Review-strip controls select
scene and size outside the measured product canvas. The owner selected **Request
layout changes** at this checkpoint and subsequently supplied four issues plus a
flashing video; the revision-2 follow-up below records the response.
Compact/layout approval remains open and no production metrics are adopted. No
relink or duplicate review window is permitted while it is open.
Learn/Export navigation is disabled with an isolated-review reason; the shared
header's production implementation is unchanged. No audio device is started, so
the audition buttons retain model binding but this is not an auditory acceptance test.

No actual Windows DPI or FL Studio acceptance is claimed. Software 100/125/150/200%
rendering is a separate test scope. No speedup, DSP, persistent preference support,
new grid snapping or production layout adoption is claimed by this checkpoint.

## Revision 2 — live flashing, divider clearance and polling work

The owner reported flashing, touching border lines, nonfunctional Snap/grid and
UI lag, then supplied `20261104-1205-03.3459414.mp4`. Frames inspected locally show
the Error/Rendering specimen alternating with real ready wet data and Learn/Export
availability. The clip is not redistributed in this PR. The initial static image
matrix did not cover this interleaved live event-loop behavior.

The fixture had two competing 100 ms refresh owners: its own timer and the hidden
production editor's timer. Only the fixture applies specimen phases and restricted
review navigation; the editor's independent poll temporarily replaced them with
real model state. The fixture also called its complete layout/font setup each poll.

The owner explicitly approved **Allow review-only refresh seam** after the shared
file exception was explained. `PluginEditor.h` adds only
`stopAutomaticRefreshForReview()`; this fixture calls it and owns polling. Normal
plugin construction retains automatic refresh. No timeline, audition, serializer,
DSP or production layout implementation changed. Borrowed list/envelope callbacks
complete through the fixture refresh so deliberate actions retain specimen state.

Idle refresh no longer invokes `resized()` or repaints the whole composition;
unchanged geometry/fonts stay in place, with existing component data refresh and
repaint behavior retained. This removes redundant fixture work, not a waveform
cache or a claim of measured whole-plugin speedup. Real interactive smoothness
still requires the owner's follow-up on this concrete reported regression.

Vertical rules now stop 12 logical pixels before the work area's horizontal rules.
Audition content and the Overview component have 8-pixel vertical insets, keeping
controls/viewport outlines clear of adjoining dividers. The overall column profile
is unchanged; the 120-high Overview allocation now contains a 104-high component.
Snap and grid are disabled with explanatory tooltips while they remain visual
specimens; no placeholder presents itself as a working musical edit.

Verification:

- `--test-competing-refresh` intentionally leaves the hidden timer running and
  **fails** on changing header availability, reproducing the video mechanism.
- `--test-refresh` passes with the approved seam: three scenes, 90 event-loop
  observations each, stable specimen/header/handle state and no idle layout passes.
- The tests also caught an asynchronous initial review-dropdown selection that
  could reset the tested scene. Both initial selections now use silent refresh;
  the timed test asserts the requested scene remains selected.
- Final Release wrapper build passed; logs:
  `out/build-logs/Release/20261104-122457-b5385cb0/`.
- Revised native matrix passed **60 geometry cases, 240 software-scale renders,
  divider-gap pixel assertions and the live refresh checks**. Focus checks cover
  enabled actions; disabled Snap/grid are explicitly verified unavailable.
- Existing `preverb_swell_ui_test.exe` passed (exit 0), exercising the normal
  editor/header, Learn, Perform, selection, bindings, gestures and recovery paths.
  UI standards, strict OpenSpec and diff checks passed. Full CTest and host/DPI
  acceptance remain unperformed at this checkpoint.

Retained revision-2 evidence: [selected](2026-11-04-issue-46/revision-2/sparse-1100x850-100.png),
[error](2026-11-04-issue-46/revision-2/error-1100x850-100.png),
[empty](2026-11-04-issue-46/revision-2/empty-1100x850-100.png) and
[measurements](2026-11-04-issue-46/revision-2/measurements.json).
All software-scale images are in `out/perform-composition-revision-2/`.

Updated executable SHA-256:
`C8E42E641210396AF961DD56BD5EA36C1F499879A040F38F1D192B49F99A8B92`.
Preview source SHA-256:
`AF43D5771A053BB13FE131455949BBF937403EC7548760BAAEDC5C16788679DF`.
Header source SHA-256:
`790701162007AF1ADCCA2C914D6355983785DD72394EED9B10EC9E9FF8DB6959`.
Build identity is `2e5a87e (dirty)` plus this PR's reviewed source diff.

The original #46 window was closed after feedback and seam authorization, and the
corrected responsive window was launched directly (PID 38696). Agent 1's Export
preview was not touched. The new window is preserved for layout/flashing/feel
review; approval and production adoption remain pending.

## Read-only decoder check for the later persistence stage

At this baseline, `Source/Core/SwellSession.cpp` accepts versions 1–9 and reads
specific named properties without rejecting unknown ones. Its `Parser::field`
marks missing fields invalid; numeric/boolean accessors are deliberately strict.
`Source/Plugin/PluginProcessor.cpp` accepts envelope versions 1–8 and rejects a
session when `m3::decode` fails. Thus an unknown optional UI object can be ignored
by this old reader, but malformed optional UI preferences in the new reader must
be validated independently of the required musical-state parser's failure flag.
Missing UI values must not become required fields or reject an otherwise valid
legacy project. This is source inspection, not a persistence test pass. Task 5.1
still requires actual decoder/round-trip/malformed/Ctrl-bypass tests after the
layout and shared-ownership gates.

The owner's subsequent composable-waveform request prompted a bounded read-only
audit of Agent 1's current `TimelineNavigation.h`, `ExportWorkspace.h` and decoder.
Export already uses `SwellViewport` and `makeSongBarRuler`, has a read-only
navigation owner with `setRange/zoom/pan/fit/onChange`, and writes session schema
**10**, retaining processor envelope **8**. The design now explicitly preserves
that future merged schema rather than forcing the planning baseline's version 9.
Its proposed stateless `TimelineZoomControls` boundary and deferred renderer
extractions are in design decision 7. No shared production code or unmerged Export
file was copied or edited. Interface agreement and actual integrated decoder tests
remain pending.
# Revision 3: divider continuity (2026-11-04)

Owner-requested dependency split: [PR #49](https://github.com/elneilios/ReverbArise/pull/49),
commit `04aa597`, independently supplies the shared Zoom/Fit row, musical ruler
and Agent 1's unchanged ActionButton. Its API guide and focused/native test
evidence live on `codex/46-shared-timeline-controls`. It adopts no Perform metrics
and completes no whole parent task. Agent 1 retains Export integration ownership;
direct chat handover was rejected because its chat has an active writer, so the
reviewable PR is the handover record. The shared family preview is open separately.

Owner comparison screenshots identified the remaining interrupted horizontal rules: the opaque timeline overlapped its parent's rule. The isolated fixture now insets timeline content and list/inspector headings, keeps vertical separators clear of horizontal rules, and removes the obsolete Perform auto-marker band. No production compact metrics have been adopted.

Release build `out/build-logs/Release/20261104-123546-e3672efb` passed. Native fixture passed 60 geometry cases and 240 software renders, including composite-pixel checks across all horizontal rules, vertical endpoint gaps and live refresh stability. Captures and measurements are in `2026-11-04-issue-46/revision-3/`. This is fixture evidence, not host/DPI acceptance. Layout approval remains pending. A copied executable under `out/review-issue46-revision3/` preserves the review window independently of subsequent shared-control builds.

## Revision 4: consume merged shared row (2026-11-04)

The owner approved PR #49's shared design/handover and merged it into main
(`12182f2`). That main revision is merged into the Perform branch. The isolated
fixture now consumes `TimelineZoomControls` instead of duplicating its buttons
and slider. In the minimum 1100x850 composition the shared row is 292 pixels,
including a 114-pixel slider and 74-pixel Fit button; all six toolbar controls
remain on one row. No production Perform metrics or controls are adopted.

The former Wave zoom label reads Height in this fixture, with a tooltip explaining
display amplitude only, independent of audio gain and horizontal zoom. Review
callbacks use the existing timeline viewport/Fit paths and silently refresh the
shared controls. The fixture's existing 1..1e12 span domain is not claimed as the
final context/pixel-derived Perform navigation bounds. Snap/grid remain disabled
specimens until layout approval; no snapping work has started.

Focused native Release builds of the composition and shared-control test targets
passed using the existing build lock/cache helpers, logs
`out/build-logs/Release/20261104-perform-shared-preview/build.log`. The native
composition matrix passed **60 geometry cases / 240 software-scale renders plus
live refresh checks**; the merged shared-control native test passed. Full-width
composite rule checks, endpoint gaps and idle-layout stability remain enabled.
UI standards passed (37 files / 61 bounded occurrences); strict OpenSpec and diff
checks passed. No full suite, actual DPI or host acceptance is claimed here.
[Selected](2026-11-04-issue-46/revision-4/sparse-1100x850-100.png),
[dense](2026-11-04-issue-46/revision-4/dense-1100x850-100.png),
[empty](2026-11-04-issue-46/revision-4/empty-1100x850-100.png) and
[measurements](2026-11-04-issue-46/revision-4/measurements.json) are retained.

The verified native window is open from `out/review-issue46-revision4/perform-review.exe`,
SHA256 `71A1D422B0391507D95408E2503C83E43AEDF4F813E58664F41BC23C8B78C013`.
Owner compact-layout approval was requested separately and remains pending;
task 1.3 and the partially delivered control-family task remain unchecked.

Read-only #6 recheck: Agent 1 remains at branch HEAD `33f1871` with uncommitted
Export work. The latest issue comments confirm left click/drag internal-preview
positioning, Shift-drag pan and stopped/playing seek semantics remain its ownership.
Its current TimelineNavigation SHA256 is
`DA5AD8F28ADF30032A21627811B06078FB45C551E578DFA238A884EF22B7951D`;
SwellSession decoder remains schema 10 and SHA256
`A7CC163FD31C4F96969A04CD9C9A5F43A887417ECFD04EF3DF37EEDDBBA1141D`.
SwellViewport text still matches after line-ending normalization. A concrete
request to coordinate shared timeline and optional preference ownership was again
rejected by Codex's active-writer restriction. No message was delivered and no
shared production timeline/serialization paths were changed. Task 1.1 remains
open rather than treating the merged stateless library as wider state ownership.

## Compact approval and standalone navigation foundations (2026-11-04)

The owner explicitly answered **Approve compact layout** for revision 4. Task 1.3
was marked complete only after refreshing Apply instructions, matching its exact
source checkbox/description and re-querying completion: **2/17 complete**.
`PerformLayout.h` now owns the approved scoped values; native fixture placement
consumes it. Standards revision 3 records the approved profile, with no change to
comfortable Learn/header tokens or the exact-occurrence debt inventory. Existing
production editor/timeline/decoder files remain untouched pending #6 coordination.

New standalone `HostGrid` reuses existing transport mapping/origin/sample authority.
Its integer oracle covers all seven divisions and 4/4, 3/4, 6/8, negative and
fractional PPQ/origins, half ties, seeks/loops, one-sample host-origin tolerance,
changed/incompatible/non-finite timing, invalid divisions and unrepresentable
grid indices. This is a numerical foundation; existing viewport/gesture wiring
is not yet extended, so task 4.1 remains incomplete.

New `LatchingActionButton` leaves accepted ActionButton unchanged, inherits its
unlatched drawing/invocation/focus and adds accepted On ink. Component tests cover
pointer, Return/Space family behavior, silent effective display, requested-On Ctrl
mask/release, saved-Off release, disabled delayed callbacks, native focus and
accessible checked state. The owner/controller still owns modifiers and preference.
The [native control specimen](2026-11-04-issue-46/snap-control/snap-control.png)
was directly launched as “Issue #46 - Snap toggle control”; its separate family
approval is pending. No production Snap/gesture behavior or persistence is claimed.

Focused native Release builds / four CTests passed in 2.74 seconds at
`out/build-logs/Release/20261104-perform-grid-control-final/` before the additional
explicit accessibility-state assertions; final focused rerun is recorded below.
No broad/full native suite or FL Studio/DPI repeat was needed for these isolated
foundations. UI standards passed with 39 governed files / 61 bounded occurrences.

The current ownership message to Agent 1 was successfully delivered through Codex
after earlier active-writer failures. It asks for the editor/timeline and optional
schema-10 Perform preference contract before shared mutations. The target chat
currently reports an interrupted/not-loaded turn; no ownership reply has arrived.
Do not infer approval from message delivery. Task 1.1 remains open.

Final verification: the extracted approved profile passed the complete native
60-case / 240-render matrix and live refresh checks, logs
`out/build-logs/Release/20261104-perform-approved-profile/`. Parsed measurement
JSON matches revision 4 exactly. Final native accessibility-state assertions and
all four focused tests passed in **2.77 seconds**, logs
`out/build-logs/Release/20261104-perform-snap-accessibility/`. Strict OpenSpec,
UI standards (39 files / 61 bounded occurrences) and diff checks passed.

The owner subsequently answered **Approve Snap button family** in the direct
native review. Its look/feel gate is satisfied. This does not complete task 2.1's
remaining selector/numeric/knob inventory and binding work, or any production
gesture/state integration. Both layout and Snap family approval are retained;
no repeat owner review is requested for those accepted visuals.

Latest read-only Export recheck during this work: Agent 1 has committed and
merged main in its branch at `13ec5be` (“Merge main with shared timeline controls
and swell-list integration”). The overlapping editor/session files have no
uncommitted diff at that revision. TimelineNavigation is now SHA256
`AEE0CF5A3EAFD287EB46FD63E359B41A551DFB4DD0830BBB27A237FF86C1C465`;
the schema-10 decoder hash is unchanged. This is a concrete integration baseline,
not an inferred ownership release or permission to absorb Export into PR #48.

## Confirmed #6 ownership handover (2026-11-04)

The owner forwarded [Agent 1's explicit reply](https://github.com/elneilios/ReverbArise/issues/46#issuecomment-6632200424).
Baseline: `feat/6-export-mixed-song-wav` at `13ec5bec36112540ec1eaa42d3e788413367d637`,
saved Export/seek implementation `d551afc`; shared-control placement remains a
working diff. Agent 1 explicitly releases isolated SwellTimeline/SwellViewport
and standalone Snap/grid work, preserving existing APIs and coordinate authority.
It is not editing those files. Final PluginEditor and SwellSession integration
must wait until #6 merges; retain Export's special layout/visibility, creative
schema-10 fields/migrations and processor envelope 8. No unmerged Export code is
absorbed. Export keeps Fit/seek/pan/file authorities and the shared #49 controls.

This resolves task 1.1's ownership contract, with the reserved-file sequencing
recorded rather than waived. Merged #4 identity/epoch/sort/scroll APIs and #38's
source-ready/pending-wet/manual/frozen-ID/return constraints remain the reviewed
integration contracts; this handover changes none of them.

## Released timeline/viewport implementation (2026-11-04)

Task 1.1 was completed through the refreshed Apply source-line/description guard
and confirmed re-query: **3/17**. Existing PluginEditor and SwellSession files
remain reserved and unchanged; #6 is not yet on main (`12182f2` at this recheck).

The existing SwellViewport now exposes opt-in bounded navigation and checked
`snapToGrid` through HostGrid. Existing Export arithmetic/API behavior remains
available. Perform uses known source/capture/render/requested-shape bounds with
4% margins, a logical-pixel/precision minimum, logarithmic fraction and exact
maximum endpoint. Target/playhead/center toolbar anchors and pointer Ctrl-wheel
use the same transform; Overview and pan share the limits. Malformed explicit
viewport assignments are ignored. Overview's one-pixel clipped highlight remains
inside its real lane at extreme margins.

Native Release builds and **6/6 CTests passed** in 12.83 seconds, logs
`out/build-logs/Release/20261104-perform-navigation-endpoints/`. The independent
integer grid oracle also checks every meter/division through SwellViewport, not
just the helper. Pure viewport tests cover anchors, negative coordinates,
min/max, non-finite arguments, overflow-safe clamping and invalid bounds.
`--test-navigation` passed real timeline target/playhead/center/pointer anchoring,
slider extremes, repeated zoom saturation, Overview containment, unchanged host
position/source capture revision, IDs/assets, shape/source/arrival/placement and
input/shape generations.

Initial focused checks exposed two intentional/corrected differences: the old
synthetic 500-sample Overview test assumed a sub-pixel 100-sample navigation span;
its oracle now verifies full short-context plus 4% margins. A new exact maximum
endpoint assertion caught ratio/multiplication rounding; the endpoint is now
assigned exactly rather than relaxing the assertion. Final focused checks pass.
Contextual Fit/return lifecycle and actual Snap gesture/modifier/state wiring are
still later work; this boundary does not claim them complete.

The complete native 60-case / 240-render matrix and live refresh checks also
passed after these changes (`out/perform-bounded-navigation-matrix/`). Task 3.1
and task 4.1 each passed the refreshed Apply source-line/description guard and
post-edit completion re-query: **5/17 tasks complete**. Final UI standards remain
39 files / 61 bounded occurrences; strict OpenSpec and diff checks passed.

Prepared task 4.2's bounded-domain numerical foundation without publishing any
gesture edits: HostGrid/SwellViewport closed-domain methods choose an actual
grid point or return unavailable, with no off-grid clamp. Independent exhaustive
small-domain oracles cover all divisions/meters, no-grid intervals, singletons,
negative and fractional origins, sub-sample intervals and unrepresentable bounds.
Four focused pure native tests passed in 0.11 seconds at
`out/build-logs/Release/20261104-perform-bounded-grid/`; final authority-wrapper
rerun is recorded below. Task 4.2 remains unchecked until actual guarded gesture
integration and its identity/source/support tests pass.

Final bounded-domain authority-wrapper rebuild and all four pure tests passed in
0.11 seconds, logs `out/build-logs/Release/20261104-perform-grid-authority-final/`.
UI standards, strict OpenSpec and diff checks passed. Existing native editor and
full composition results above remain applicable: these additional grid methods
are not yet called from production gestures. No full-suite or host run is claimed.

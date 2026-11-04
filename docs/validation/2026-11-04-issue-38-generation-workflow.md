# Issue #38 generation workflow — implementation evidence

Change: `preview-and-generate-swells`; branch `codex/38-preview-generate-swells`.
Planning baseline: `df344bd`, main implementation baseline `9b8e70f`.
This is an in-progress record, not feature acceptance. OpenSpec tasks remain
authoritative; unverified work is not checked complete.

## Execution map

| Task | Type / risk | Dependencies and verification boundary |
| --- | --- | --- |
| 1.1 | Persistence / high | Session and processor codecs; current/legacy/malformed native cases |
| 1.2 | Identity/persistence / high | Existing editor commands; session/engine/protected-target regressions |
| 2.1 | Workflow/audio / high | 1.1; state, processor and capture-transaction regressions |
| 2.2 | Concurrency/realtime / high | 2.1; reservation, cancellation and callback guards |
| 3.1 | DSP/timeline / high | Existing detector/readers; convergence, coverage and bounds |
| 3.2 | Concurrency/capture / high | 3.1; staged-data authority and stale publication races |
| 3.3 | State/persistence / high | 1.1, 3.1–3.2; no implicit generation or restore resumption |
| 4.1 | Identity/timeline / high | 1.2, 3.1; complete arrangement planner/protection tests |
| 4.2 | Transaction logic / high | 4.1; equality, empty, failure and retained-asset tests |
| 4.3 | Concurrency/realtime / high | 2.2, 3.3, 4.2; shared admission/publication races |
| 4.4 | Workflow/persistence / high | 1.1, 4.3; explicit/Stop parity and cancellation |
| 4.5 | Rendering/persistence / high | 4.3–4.4; asynchronous render/restore integration |
| 5.1 | UI bindings / medium | 4.3–4.4; component/editor ownership and accessibility |
| 5.2 | UI/timeline / medium | 3.2–3.3; coordinate, overlay and side-effect checks |
| 5.3 | Editor navigation / medium | 4.3–4.5; initial chronological selection/reveal and no delayed-render jumps |
| 5.4 | UI acceptance / medium | 5.1–5.3; native owner review and UI standards |
| 6.1 | Integration/performance / high | 4.5, 5.2; dense and sparse refinement scenarios |
| 6.2 | Native integration / high | Implementation complete; Debug/Release final gates |
| 6.3 | Host integration / high | 5.4, 6.2; one consolidated final FL Studio smoke including initial selection |
| 6.4 | Documentation/delivery / low | Evidence review; one existing PR, scope and links |

## Environment and commands

Windows worktree has its own previously absent `out/build/windows-msvc` tree.
First Release run is **cold**, not a reused native build. The installed VS 2026
CMake/CTest and MSVC are used by `scripts/build.ps1`; no raw build command or
overlapping wrapper is used. OpenSpec uses the existing installed Node v26.13.0
runtime explicitly because the PATH nvm shim has no active version configured.

- `scripts/build.ps1 -Configuration Release -Test`: cold run
  `20261104-180856-fc7981b3`, configure completed in 68.8 seconds. The VST3 and
  initial test targets built, but the serial full build was deliberately stopped
  before CTest. Its exact wrapper/CMake/MSBuild descendant tree was verified and
  stopped; no child remained before restart and no cache/object tree was cleared.
  This interrupted run establishes no completed native test result. The wrapper
  was restarted with `CMAKE_BUILD_PARALLEL_LEVEL=2`, reusing completed objects;
  resumed run `20261104-182357-54f497ae` passed configure/build and **46/46 native
  Release CTest tests**, 65.06 seconds test time. This is a partially warm restart,
  not a comparable speedup measurement.
- No Debug, UI review, FL Studio, actual Windows DPI or screen-reader acceptance
  is claimed at this stage.

## Persistence/provenance boundary

Tasks 1.1 and 1.2: four actual MSVC Release executables passed directly from the
worktree build: `preverb_swell_session_test`, `preverb_swell_engine_test`,
`preverb_automatic_targets_test`, `preverb_workflow_processor_test`. These are
focused native results, not a completed full CTest result. The wrapper was still
building independent remaining targets; no second build/configure or CTest
operation touched its tree. No callback allocation/deallocation failures were
reported by the processor/engine fixtures.

Coverage includes fresh OFF versus legacy ON (creative versions 1–10 and old
processor envelopes), strict missing/malformed new booleans, explicit ON/OFF with
Automatic Swells hidden, unchanged v8 XML mode identity, conservative legacy
origin, musical/activity/name/IR/freeze ownership and full saved-field retention
for protected reconciled swells. Native tests do not establish FL Studio or DPI
acceptance. The new candidate/planner modules are still unintegrated drafts and
are not covered by these four results.

## Source settlement and reservation boundary

Tasks 2.1 and 2.2: warm Release build-only wrapper
`20261104-183523-99f7cdd0` passed. Direct native Release workflow state, workflow
processor, capture transaction and generation configuration executables passed.
The state fixture exercises 200 Play-versus-publication races, lost pass/settings
admission, callback allocation/deallocation guards and held publication versus
off-callback reset. Source transactions retain failure/empty/silence/identical
Recapture semantics while ending Learn; manual navigation is explicit.

The full CTest suite was not run at this intermediate boundary: remaining
generation-policy fixtures still require the shared Generate API in tasks 3–4.
The earlier 46/46 result predates these workflow changes and is not represented
as final validation of them.

## Candidate analysis and generation integration

Warm Release build-only wrapper `20261104-192527-7b5256b7` passed. Direct native
Release `preverb_learn_candidates_test`, `preverb_automatic_generation_test`,
`preverb_learn_generation_test` and `preverb_initial_perform_view_test` passed.
Earlier integration iterations were also built with the wrapper; this is the
current focused boundary, not final full-suite validation.

The candidate tests exercise whole/incremental convergence, sparse and negative
coordinates, explicit silence, missing mapped assets, identity mismatches and
cancellation without replacing the previous display. The real processor fixture
checks staged provisional versus accepted candidates, coalesced sensitivity,
unchanged definitions/assets/render counts during preview, dry-only source
preview without recapture, default changes without re-detection, restored source
without implicit generation, strict source/IR failures, capture/settings/mode
cancellation, explicit/Stop parity, protection, no-change and source replacement.
Its callback allocation/deallocation guard remains enabled.

The pure navigation test verifies chronological selection independent of vector
order, deterministic equal-arrival ties, lead-in prediction before wet rendering,
minimal panning with retained useful zoom, necessary fitting, negative pre-roll
and rejection of obsolete wet timing. Native editor integration is still under
verification. UI standards lint passes with the existing 61 bounded occurrences;
the debt inventory was not regenerated.

## Integrated editor and dense arrangement boundary

Warm wrapper `20261104-193942-c799f983` built successfully. Direct native Release
`preverb_swell_ui_test` and `preverb_processor_test` passed after adapting legacy
fixtures to explicit Generate. Learn keeps definitions unchanged; existing
explicit Perform reanalysis and new Live boundary fitting remain available.
Cancelled analysis attempts are retryable on returning to the same identity.
Source settlement fences readiness until its final candidate identity matches.

The new native `preverb_learn_generation_ui_test` passes chronological initial
selection/reveal for both triggers, reversed vector order, editor-closed commit,
pending wet rendering, list/inspector/Overview binding, empty Perform visits,
restored valid selection/current viewport and later mode return. After selecting
another swell and panning, render completion preserves both. Pointer/keyboard
binding, hidden preference retention, stale invocation and injected failure checks
are included. The final full gate below supersedes intermediate results.

Dense fixture: 64 seconds of deterministic layered transient/quiet offbeat and
sustained harmonic input at 48 kHz / 120 BPM, in two 16-bar sections separated by
an uncaptured 16-bar gap (96-second extent). It is synthetic, not a listening
quality oracle. With sensitivity 0.2 / 0.5 / 0.8 it detected 96 / 128 / 128
candidates and committed 97 / 129 / 129 swells including a protected disabled
manual edit. Repeat Generate returned no-change without accumulation. Preview
retained asset and render counts. Measured preview-plus-commit was 930.7 / 1067.4 /
1046.6 ms in one warm Release run, with 28,576,328 reported resident bytes including
the conservative Learn metadata reservation; total fixture time was 6297.9 ms.
These are fixture measurements, not comparative speedup claims or guarantees for
real music. Callback allocation/deallocation guards remained enabled.

Intermediate full Release gate `20261104-194109-d2d511c2`: 49/51 passed; failures
were the new helper minimum-scale lint occurrence and a UI injected-failure
fixture racing restored-render recovery. Helper text now wraps at the existing
role size without a new presentation token or debt exception. The fixture waits
for permitted recovery before injecting the independent IR fault. No host,
Windows DPI, screen-reader or owner visual acceptance is inferred from rasters.

The repeated dense run exposed a fresh-instance race: a provisional RT Stop could
be collected against the previous empty schema-1 manifest and leave legacy
reconciliation work for a later Perform visit. Learn boundary collection now
requires successful source settlement; it never queues legacy reconciliation.
The fixture deliberately holds first-pass validation to exercise this timing.
It drains its private writer every 16 blocks because its synthetic input runs
faster than wall-clock audio; capture overflow is tested separately.

Warm Debug wrapper `20261104-200228-ae4a6322` passed the complete generation
fixture in 51.76 seconds, including this deterministic race and save during held
preparation. The dense portion produced the same 96/128/128 candidates and
97/129/129 swells; preview-plus-commit measured 5057.6/5858.0/5770.9 ms, total
33,516.7 ms, with 28,576,328 reported resident bytes. Native editor binding and
initial handoff tests also passed (12.52 seconds). Full-suite results follow below.

## Final acceptance matrix and native evidence

| Scenario | Automated coverage | Remaining owner coverage |
| --- | --- | --- |
| Fresh OFF, legacy ON, strict restore and protected identities | Session, workflow processor, engine and automatic planner tests | OFF/ON save/reopen in one FL Studio smoke |
| Read-only accepted/provisional candidates and refinement | Candidate, generation, timeline and native editor tests | Integrated marker appearance/interaction review |
| Explicit/Stop atomic replacement, no-change/fault/cancellation | Generation, planner, workflow reservation and processor tests | Brief opted-in Stop host smoke |
| Earliest first selection/reveal before wet, both triggers | Initial viewport and native generation/editor tests, including reversed vector order and editor-closed commit | Native first-swell framing, then host entry smoke |
| Restored position, later mode return and delayed rendering | Native editor binding tests with newer selection/pan and failed/no-change requests | One ordinary host reopen/return check |
| Dense full-song/gapped source, protected work, repeat/refine | Deterministic layered arrangement described above | Real music usefulness remains a listening judgement |

The final editor fix captures mode and model coherently under the publication
lock. Separate reads could otherwise consume a newly committed handoff using
pre-commit definitions. First-generation preparation is held in the editor-closed
fixture so a fast commit cannot accidentally turn that case into an open-editor
visit. Open-editor keyboard invocation is tested separately.

Final-source Release wrapper `20261104-201331-c97923b1` passed **51/51 CTest tests**
in 75.46 seconds. Its editor handoff target passed in 2.57 seconds; a subsequent
direct native run captured the supporting [Learn controls](2026-11-04-issue-38-learn.png)
and [initial selected Perform view with wet still queued](2026-11-04-issue-38-initial-perform.png).
These are JUCE-native component rasters; sizes 1100×850, 1400×1000 and 1600×1100
and software scales 1/1.25/1.5/2 were exercised. They do not establish actual OS
DPI or screen-reader acceptance. The passing Release dense portion again had
96/128/128 candidates and 97/129/129 swells, with approximately 1.1 seconds per
preview-plus-commit and about 28.6 MB of reported engine residence.

Implementation commits are `af24e58` and `3a8bdba`, following the persistence and
workflow commits recorded above. The Release wrapper records `af24e58 (dirty)`
because the coherent editor fix was committed afterward as `3a8bdba`; the tested
source is identical. Subsequent changes are evidence and task tracking only.
Final Debug results and scoped owner/host outcomes are recorded below.

Final-source warm Debug wrapper `20261104-201545-75dda709` passed **51/51 CTest
tests**, 224.10 seconds. It also records `af24e58 (dirty)` and tests the source
committed as `3a8bdba`. No source edits followed either final gate. The initial
Debug configure/build was cold; these final runs reuse its valid object/cache
tree. No cold-versus-warm speedup is claimed. UI standards passed (42 governed
files, 61 existing bounded occurrences); strict OpenSpec validation and Git diff
checks passed. The debt inventory and generated OpenSpec workflows are unchanged.

The native owner preview was directly launched from the copied Release
`out/review/Issue38NativeReview.exe --review` (initial PID 26116, title
"ReverbArise control review"). Its private source is preloaded, with one protected
disabled manual swell. It allows marker/sensitivity/default preference review,
Generate and initial Perform selection/framing, and resizing through the editor
corner. The agent did not close it. Launching it and inspecting automated rasters
do **not** establish owner approval; task 5.4 remains open pending explicit feedback.

After native acceptance, perform one brief FL Studio smoke on the final Release
bundle, recording host version, sample rate, buffer and source commit `3a8bdba`:

1. Fresh default-OFF capture/Stop stays Learn; markers refine with sensitivity.
2. Generate opens Perform with the chronological first swell selected, lead-in
   and arrival visible, and Selected Swell controls populated. Host playhead and
   transport stay put; rendering status is honest.
3. Preserve a manual edit/disabled swell through Learn/refine/Generate, and retain
   the subsequent editing position on mode return.
4. Opted-in Stop generates consistently; explicit OFF/ON and valid selection/mode
   survive save/reopen. Do not repeat automated fault/concurrency edge cases.

No FL Studio, actual OS-DPI or screen-reader acceptance has been performed or
claimed. Task 6.3 remains open. No plugin installation, main-branch merge, issue
closure, canonical spec sync or OpenSpec archive was performed.

## Owner-requested Learn layout refinement

The owner requested three refinements to the native preview: Auto-generate on
Stop beneath Enable automatic swells (before IR), contextual marker/readiness
text directly under Generate, then the manual-placement hint last with natural
width-based wrapping instead of a forced break after the question. These changes
reuse the same Switch, ActionButton, helper role and named spacing; no new tokens,
control variants, capture or DSP behavior were introduced. The active workspace
delta/design and living Learn guide now record this order. The previous Learn
raster above is retained as historical evidence; the [refined Learn raster](2026-11-04-issue-38-learn-refined.png)
shows the requested arrangement.

Native geometry checks now assert each requested relationship at minimum and
larger sizes, the manual sentence flowing through "? Add", retained helper font
size and containment. Release wrapper `20261104-204220-06390798` passed 51/51 CTest
tests in 80.17 seconds. Debug's first run `20261104-204419-e1ebb553` passed the UI
targets but timed out waiting for provisional candidates in the shared capture
fixture (50/51 overall). That accelerated fixture can outrun its 32-slot input
queue under Debug disk scheduling. It now drains its private writer every 16
blocks, as the existing dense fixture already does. No production capture path
was changed or overflow checks weakened. Final rerun results follow below.

Final refinement gates: warm Debug `20261104-204900-e930dd84` passed **51/51**
CTest tests in 217.22 seconds; warm Release `20261104-205253-81115322` passed
**51/51** in 76.30 seconds. Debug's generation and editor targets passed in
49.34 and 10.91 seconds. UI lint and strict change validation passed, with the
existing standards inventory intact. Production layout commit `7bb0a1c` and
fixture-only commit `3885058` describe the tested changes; subsequent edits are
evidence/task tracking. A fresh Release preview was launched directly after the
automated UI tests finished. Owner visual acceptance remains pending, followed
by the previously consolidated FL Studio smoke. No host or OS-DPI acceptance is
inferred from the updated native raster.

The next owner refinement keeps manual-placement guidance visible when Automatic
Swells is OFF. The hint is excluded from generation-control visibility gating,
and its layout is computed in both states. Native binding checks cover switching
OFF and reopening an editor with OFF already saved, including visible, nonempty,
contained helper bounds while Generate remains hidden. The ON ordering and
natural text flow are retained. This is Learn presentation only; no DSP, capture,
serialized preferences or generation admission behavior changed.

The OFF-hint refinement (`b559e6f`) passed warm native Release
`20261104-210019-eea5a3b2` **51/51** in 119.37 seconds and Debug
`20261104-210249-afb0175f` **51/51** in 232.56 seconds. The following owner
request superseded its native review composition before acceptance.

## Musician-focused Learn refinement

Source commit `e0ba6d6` incorporates the owner's shorter timeline, waveform,
marker, copy/status and Default IR refinements. The native waveform now fills
each existing PCM min/max bucket's time span at unchanged amplitude, within a
width-proportional band capped at half its width. No normalization or source/DSP
processing was added. Neutral guides brighten on hover or pinned inspection;
provisional/updating heads remain hollow. The full-song mapping and missing
coverage remain truthful. Hover/pin never changes generation scope or DAW state.

Detect events and Event sensitivity distinguish analysis from Generate Swells
and Generate on Stop. Default IR distinguishes the generation default from
per-swell assignments. One caption combines capture phase, duration and event
count; exact sample/coverage/totals information moved to the existing Diagnostics
view. The natural manual-placement hint remains last and visible with detection
OFF, including reopen. Shared control families and production roles/tokens are
unchanged; this composition still needs scoped native owner acceptance.

Focused native checks cover ordinary/hovered/pinned guide colours, hollow
updating heads, a 2:1 displayed amplitude ratio for 0.5 versus 0.25 PCM peaks,
shorter plot geometry at minimum/larger sizes, concise status and musical-only
visible inspection. Existing technical inspection, sparse/unavailable/zero
coverage, source timing, explicit/automatic handoff, restored navigation and
delayed-render checks remain active. Initial Perform selection remains earliest
by arrival, before wet completion, without transport/audition changes.

Supporting rasters: [Learn](2026-11-04-issue-38-learn-musician.png) and
[hover emphasis](2026-11-04-issue-38-event-hover.png). These are native synthetic
fixtures, not host or OS-DPI acceptance. Earlier rasters and failure/recovery
records above remain historical evidence.

Final warm Windows MSVC gates for the `e0ba6d6` source content (wrapper metadata
reports its parent `b559e6f` with the then-uncommitted refinement):

| Command | Build log ID | Native result |
| --- | --- | --- |
| `scripts/build.ps1 -Configuration Release -Test` | `20261104-212717-dc8ab7f1` | 51/51 CTest, 105.95 s |
| `scripts/build.ps1 -Configuration Debug -Test` | `20261104-212920-8f3ce86f` | 51/51 CTest, 252.69 s |
| `scripts/check-ui-standards.ps1` | Final refinement | Passed: 42 files, 61 bounded existing occurrences |
| `openspec validate preview-and-generate-swells --strict` | Final refinement | Passed |
| `git diff --check` | Final refinement | Passed |

The shorter-timeline fixtures required updating old shaded-silence and helper
geometry expectations; focused reruns preceded these final suites. This is a warm
build-tree result, with no claimed speedup. No production source changed after
the final suites began. Only documentation, task tracking and supporting rasters
followed. Native visual acceptance remains open; final FL Studio smoke must use
this final Release source, including first-swell selection/framing. Actual
host/DPI/screen-reader coverage remains untested.

The refined Release preview was launched directly as PID 32252, title
`ReverbArise control review`, from the copied `out/review/Issue38NativeReview.exe --review`.
It remains open for owner review; launching it does not establish acceptance.

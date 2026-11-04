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

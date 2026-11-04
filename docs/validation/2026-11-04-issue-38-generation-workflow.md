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

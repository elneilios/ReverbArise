# Issue #6 implementation evidence — 2026-11-04

Branch: `feat/6-export-mixed-song-wav`. Implementation in progress; this is not
final feature or host acceptance. Source baseline incorporates the merged header
implementation and its later documentation-only archive through `fd33ae5`.

## Foundation and visual-reference inventory

- Immutable metadata plan/dependency identity, additive Export state (creative
  JSON v10; processor envelope remains v8), independent initialization/percentage
  and compatible wet-based mode entry implemented.
- `scripts/build.ps1 -Configuration Debug`: pass, wrapper exit 0, logs
  `out/build-logs/Debug/20261104-021452-8751cc9c/`. Build-only; CTest not requested.
- Direct native Debug executables passed: `preverb_song_export_test`,
  `preverb_swell_session_test`, `preverb_workflow_state_test`,
  `preverb_workflow_processor_test` and `preverb_processor_test` (exit 0).
- Processor output: dense resource bytes 60,819,856, sparse 9,332,848, budget
  268,435,456. These are existing audio-resource checks, not final Export memory
  measurements. Full processor transcript: `out/issue6-foundation-processor.log`.
- Tests cover plan ordering/selection independence, shape/placement/version
  identity changes, rejected output preservation, v9 migration, independent
  first-use/re-entry preferences, completed-reference roundtrip, malformed/unsafe
  state, stale mix invocation, frozen-wet missing-source entry, 0% mode retention
  and unchanged Stop→Perform/audio behavior.
- `docs/design/export-workspace.md` records both owner-supplied mockups and actual
  shared-control ownership. Reference retrieval is resolved; all new control
  family/native composition approvals remain pending.

No final Release/CTest, Export waveform/file/preview checks or new FL Studio UAT
are claimed. Composition byte verification, streaming mixer, async publication,
durable file delivery and dedicated UI remain later tasks; a valid metadata plan
is not a ready WAV.


## Warned reader and shared-control checkpoint preparation

Debug build-only wrapper `20261104-022525-5bd75765` passed. The expanded
`preverb_song_export_test` passed again after explicit corrupt-file coverage was
added (build `20261104-023029-9d58f8c4`). Source cases cover quiet valid audio,
captured silence, exact gaps, missing/corrupt dry assets, wholly absent dry,
wet-only no-read behavior, memory rejection and cancellation. Perform's strict
reader remains unchanged and is exercised alongside the separate Export policy.

The shared action/drag fixture compiled after resolving a JUCE private-member
name collision and event-test access; the failed attempt is recorded at
`out/build-logs/Debug/20261104-022919-39016dc6/`. Corrected build
`20261104-023029-9d58f8c4` and focused native `--test-export-controls` passed.
Screenshots exposed literal punctuation encoding; new labels now use explicit
Unicode construction for ellipsis/bullet. An additional regression checks
keyboard Save immediately after a drag. Final checkpoint build/review pending.
No actual file drag, Save dialog, preview audio or host result is claimed by this
callback-only component fixture. Native family approval remains pending.


## Component checkpoint A — open for owner review

Debug build-only wrapper `20261104-023314-c37427ef` passed, exit 0. The complete
affected `preverb_swell_ui_test` executable passed, exit 0; transcript
`out/issue6-native-ui.log`, screenshots `out/issue6-control-checkpoint-a/`.
This includes the new action/drag controls plus existing shared-control,
header/notice, selection and lifecycle regressions. The post-drag keyboard Save
regression passes. Screenshot review confirmed corrected Unicode ellipsis/bullet
rendering and readable enabled/disabled tiles, with focus ink inside bounds.

The directly launched isolated review executable is
`out/component-previews/issue6-control-checkpoint-a/preverb_swell_ui_test.exe`
with `--review-export-controls`. It uses callback feedback only, not actual WAV
export or host drop. Owner appearance/feel approval is pending; task 5.2 remains
unchecked and the new families have not been integrated into production screens.
Verified progress: tasks 1.1, 1.2, 1.3, 2.2 and 5.1 complete (5/27). Async worker
publication/ownership, wet readers/mixing/WAV output, actual Export preview and
dedicated workspace remain unimplemented tasks, not waived acceptance.


Component checkpoint A owner response: “Approve these controls; continue”.
Action buttons and FileDragTile are accepted for integration; earlier numeric
control/header approvals remain retained. No real export/host drag is claimed.


## Mixing and file increment before trunk integration

Debug build-only wrapper `20261104-082646-da6f32a3` passed, exit 0, and
`preverb_song_export_test` passed. New shared-kernel tests compare 0/50/65/100%
output across 1/13/127/4096-frame partitions, overlapping rising wet, exact audible
bounds and unchanged raw assets. Streamed float32 WAVs decode against the sample
oracle; header/frame/rate/channel properties, immutable prior files, cancellation,
write/close/verification/publication failures and Save overwrite rejection/failure
are exercised. These are incremental worker/file tests; task-wide coverage,
concurrent publication ownership, preview and final host acceptance remain open.


## Trunk UI-guidance integration (requested by owner)

Preserved WIP in named recovery stash `17d6f123bb00372ee3dca8ab59ebe10e12e94f42`,
fast-forwarded the issue branch from `fd33ae5` to `26d16bf` (merged #37 guidance
and implementation), then restored WIP. The sole conflict was adjacent includes
in `tests/swell_ui_test.cpp`; both UI standards and Export cases were retained.
Production guard script, exact-occurrence baseline, AGENTS, theme/geometry
standards and production style guide remain identical to trunk. No guardrail
exceptions or baseline regeneration were introduced. Equivalent existing
control-label/section-heading role accessors replace direct font-policy calls
in new Export consumers without changing the accepted family appearance.
`check-ui-standards.ps1` passed (34 files / 61 bounded occurrences) and
`ui_standards_lint_test.ps1` passed all 35 assertions. Native validation after
this source integration remains to be recorded.


## Incremental worker, file and preview foundation

- Post-trunk Debug build `20261104-083427-37589fa2` passed; focused
  `preverb_song_export_test` exited 0.
- Engine integration first required an explicit bool-to-int formatting fix
  (`084700-818b8f88` build failed; `084758-d282eccb` passed). The new engine
  test then exposed a null input stream passed to JUCE on missing export history.
  A temporary symbolic stack trace identified `ExportFiles::verify`; diagnostic
  instrumentation was removed after adding explicit unavailable-stream handling.
- Debug `085256-952f8695` passed; model/mix/file and engine tests exited 0.
- Debug `085938-69bbf408` passed; `preverb_song_export_test`,
  `preverb_song_export_engine_test` and `preverb_export_voice_test` exited 0.
  Tests include bar-9 lead-in/full decoded-file comparison, nonzero origins,
  quiet tails, required timing/overflow rejection, rapid percentages, save while
  held, restore ownership, independently copied instances, mode exit, verified
  retained IR replacement and missing-file recovery to a new path.
- The bounded voice test compares 10,000 frames across 4096-frame pages, checks
  absolute cursor, stale arm after Stop, missing-page stop, manual Stop ramp and
  mode revocation. Watched callback allocations/deallocations are zero.

These are focused Debug checks, not final full CTest/Release or FL Studio #6
acceptance. The approved header/control visuals have not been redesigned.
The full processor-preview sample comparison and dedicated Export UI remain in
progress. Guard scripts, debt baseline and production tokens remain unchanged.


## Integrated callback and first workspace checks

Debug `090316-1825cb69` and `090657-1cc08f64` passed. Export engine callback
oracles now cover 0/50/65/100%, exact absolute preview cursor and decoded WAV
samples with only the documented 5 ms monitor-boundary ramps. Focused model,
file, voice and workflow processor tests exited 0. Task checkpoints 1.4, 2.1,
2.3, 2.4, 3.1 and 4.1 were verified through refreshed OpenSpec instructions.

The initial dedicated workspace compile (`091511-aab1a352`) found a constructor
parameter shadowing a member in lambdas. Renaming the parameter resolved it;
`091646-cd68deee` and `091952-ad8716c2` built successfully. The complete native
UI executable exited 0 and produced integrated 0/50/100% and supported-size
screenshots in `out/issue6-integrated/`. Learn/Perform/header regression checks
remain included. These screenshots have not yet received owner checkpoint B.

A new delivery/no-op test exposed stale readiness immediately after an Export
mode roundtrip. Readiness and action admission now require the exact workflow
pass/session, rather than accepting a previous ready pointer for a new pass.
Debug `092250-11c3a2f9` passed, as did engine delivery and workflow regressions.
Debug `092658-8476330a` passed; engine tests exited 0. Native endpoint pixel and
additional pending/error fixtures are running separately. No guardrail edits
or inventory regeneration were needed; enforcement still passes (35 files /
61 existing bounded occurrences).

## Approved nine-point refinement — native checkpoint B revision

Source: `33f1871` plus the Issue #6 working diff, 2026-11-04. The owner approved
updating this existing change; the shorter nine-point review supersedes the
extended feedback. Debug wrapper build `20261104-112412-17f69a64` passed. Focused
`preverb_song_export_engine_test` and `preverb_swell_ui_test --test-export-paint`
exited 0. The full native `preverb_swell_ui_test` also exited 0, writing
`out/issue6-refined-preview/` (minimum/larger sizes, endpoint mixes, zoom/pan,
pending/error, and 125/150/200% software raster fixtures). These are focused
executable results, not a final Release/full CTest run or actual OS DPI coverage.

Detail tests compare accepted weighted layers against sample oracles, reject
superseded view requests and retain export/render identities. The engine fixture
now previews 80,000 frames while requesting detail, exercising ring refill rather
than only an initially prefetched window. Tests cover retained previous waveform,
current-only actions, once-per-revision fit, manual-view preservation and bounded
large-coordinate navigation. A Debug stack overflow in the model test fixture
was resolved by heap-owning its additional large mixer; the earlier corrected
model test passed, without changing product callback behavior.

UI enforcement passes: 37 files / 61 existing bounded occurrences. `git diff
--check` and strict validation of `export-mixed-song-wav` pass. Guardrails,
production tokens and the debt inventory were not changed. The shorter tile,
colour legend, dimmed previous-view state and shared navigation are awaiting owner
checkpoint B approval; the new preview is launched from an isolated binary.
Final Debug/Release CTest and the one consolidated FL Studio check remain open.

## Owner acceptance and coordinated integration hold

The owner approved the current Export workspace as presented: layout, reactive
mix waveform, WAV tile, legend, readiness and working pan/zoom/Fit. Checkpoints
5.6 and 5.7 are accepted; no further visual redesign is requested. A fuller
musical ruler and #46's shared Zoom - / slider / Zoom + / Fit row are explicitly
deferred for coordination, with no Snap or editing handles. The handover is in
`docs/design/export-workspace.md` and issue #46 comment 6629874432. Merge/archive
remain held until integration or explicit owner separation into a follow-up.

The native Save chooser is safely owned by the screen and captures session/pass/
file with a Component::SafePointer; an empty cancelled selection submits no job.
Automated file/engine tests cover overwrite refusal/approval, failed publication,
revoked commit ownership, stale workflow/restore and managed-original retention.
External-drag invocation tests cover not-ready, released and unsupported outcomes;
engine tests cover current/missing/stale file verification and immutable history.
These tests do not prove real FL Studio drop acceptance. An attempted native
Save-dialog cancellation check could not run because both computer-use runtimes
failed to initialize in this environment; cancellation remains in the brief host
check, not reported as a native OS pass.

## Final automated validation and acceptance map

`scripts/build.ps1 -Configuration Debug -Test` passed at `33f1871` plus the Issue
#6 working diff: **43/43 CTest tests**, 260.91 seconds, wrapper exit 0. Logs:
`out/build-logs/Debug/20261104-115110-76fb03f5/`. Strict all-spec OpenSpec validation
passed **18/18 items**, and `git diff --check` passed. These are warm incremental
builds using existing dependency caches. Release results are recorded below only
after the matching wrapper completes. Source/build/resource/test input SHA-256s
are captured in `out/issue6-validation-inputs.json` for this working revision.

| Issue #6 acceptance area | Evidence / remaining acceptance |
| --- | --- |
| Wet-based entry, first-use audition mix, independent persisted preference | `song_export_test`, `workflow_state_test`, `workflow_processor_test`, `swell_session_test` |
| Real 0/50/65/100 audio, placement, overlap and headroom | `SongMixCases.h`, decoded-WAV and callback oracles in `song_export_engine_test`; monitor-only 5 ms edges documented |
| Complete audible fit versus song-origin WAV, negative pre-roll and late tails | Model/range/decoded-file fixtures plus native navigation/pixel cases |
| No reconvolution, capture or scheduled Export wet; live dry remains | Export engine identity/detail checks and workflow processor regressions |
| Warned missing dry, exact captured silence, immutable published history | Source-reader, file-fault, persistence and recovery fixtures |
| Pending current-only readiness, cancellation and session ownership | Held/rapid requests, stale chooser-pass, missing-file rebuild and preview Stop tests |
| Dedicated workspace, endpoints, legend, readiness and pan/zoom/Fit | Owner native approval; supported-size/software-raster fixtures, not actual Windows DPI |
| Save overwrite/failure and external drag invocation | Automated file/engine/invocation tests; native chooser cancellation and FL Studio Playlist delivery still require the brief host check |
| Avoid doubled dry/wet after Playlist import | Visible mute/layer/Export guidance; actual host routing/import remains the final FL Studio acceptance |

Issue #5's Perform audition, #38's Generate workflow, #8's portability/cleanup and
#46's Perform redesign remain separate. Learn's current Stop → Perform behavior
and captured-source provenance are retained. The fuller ruler/shared navigation
integration is recorded separately from current visual approval and holds merge/
archive; it is not being silently marked implemented.

### Validated Release candidate

`scripts/build.ps1 -Configuration Release -Test` passed: **43/43 CTest tests**,
71.07 seconds, wrapper exit 0, logs
`out/build-logs/Release/20261104-115600-d39e763e/`. Both Debug and Release include
product/built-version, shared-header/native UI, audio/workflow, persistence,
export file/mixer/engine and voice regression checks. No source inputs changed
between these final builds; the input manifest was rechecked after completion.

Validated bundle:
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
Inner Windows binary SHA-256:
`2436657A42E44C02FCFA3CF75F6CE213DF44742D5122FF2C2793D4F469FA848F`.
Source/resource/test/build input manifest SHA-256:
`92EC654CA098C570539A12533F6FDC22EE994D619D2445132BB1C6E2007F42B1`.
Machine-readable evidence: `out/issue6-release-validation.json` and
`out/issue6-validation-inputs.json`. Source remains `33f1871` plus the uncommitted
Issue #6 implementation. Creative JSON is v10 (v1–9 readable); processor envelope
stays v8. Existing VST3 IDs, empty host parameter inventory and bundled IR assets
remain unchanged. No guardrail/token/inventory changes are present.

The [brief host checklist](2026-11-04-song-export-host-checklist.md) is the only
remaining FL Studio request. Actual Playlist drag, native Save cancellation and
host save/reopen are not claimed by CTest. Actual Windows DPI remains untested.
No merge or OpenSpec archive is authorized by these automated passes.

### FL Studio owner acceptance — passed

The owner reported all three grouped checks passing with no setup changes:
usual FL Studio 26.1.7 / 48 kHz / 512 samples. This completes preview-versus-WAV
0/50/100%, Playlist delivery/song alignment, Save cancellation and successful
fallback, dry-inclusive source muting/Export wet bypass, and saved mix/reference
retention without autoplay. Actual DPI was not reported; no unsupported-drag
configuration was forced. The validated binary above remains unchanged.

The owner additionally requested waveform click/drag preview positioning after
shared navigation integration. This is deferred, not implemented or host-tested.
Current visual and functional acceptance remain passed. The #46 integration/
follow-up decision still holds merge/archive and the final closeout gate.

### Preview positioning refinement — focused checks

The owner subsequently authorized implementing click/drag preview positioning
now, without waiting for #46. Stopped seeks park; playing seeks replace the paged
preview ticket, retaining 5 ms anti-click ramps. Shift-drag pans. No new saved
field, export mix identity, file revision, DAW seeking or convolution is added.

Debug build `20261104-123138-61f62125` passed. Focused native
`preverb_song_export_engine_test`, `preverb_export_voice_test` and
`preverb_swell_ui_test --test-export-paint` exited 0. Tests cover clamped stopped
parking, no autoplay, decoded-WAV samples from a chosen start, latest playing
seek/cursor handoff, Stop revocation, stale session/pass/composition rejection,
mix/restore reset, unchanged saved export/render identities, zero callback
allocations/deallocations and mouse mapping after zoom/pan. Shift-drag pans;
right-click and pending last-good views cannot seek. Strict change validation
and UI guard passed (37 files / 61 existing bounded occurrences).

These focused results precede the final seek-refinement Debug/Release suites.
The prior FL Studio passes apply to unchanged behavior and the earlier binary;
they do not claim host acceptance of the new seek behavior. The review fixture
now advances the real engine preview using a silent timer clock, with discarded
audio and no DAW/audio-device interaction. Native gesture approval remains open.

The first full seek Debug run (`20261104-123607-e87084b8`) passed 42/43 tests:
the processor preview-start oracle intermittently failed. Targeted diagnostics
reproduced a worker timing-read miss becoming an invalid Export revision,
canceling Play before it could start. `prepareExport` now waits on an unavailable
coherent timing read, matching existing inspect/transport preparation; valid but
incompatible timing and callback timing gates are unchanged. Temporary diagnostic
prints were removed. A deterministic engine fixture injects three missing reads
during preview and requires the accepted revision and Play intent to survive.
Three focused processor rechecks pass after the fix; final suites follow below.

The seek PCM fixture now varies both channels across frames and capture chunks,
so a wrong reader offset cannot pass because dry samples happen to be constant.
UI gestures are revoked when revision ranges, session/pass, pending readiness or
mode reset changes; a held drag cannot silently become a seek in a new session.

The final mouse guard also preserves an ordinary Shift-pan through pending-view
refresh, while a left drag begun when seeking is unavailable cannot become a
seek merely because the same mix becomes Ready again. Explicit component cases
cover both transitions. Long engine sample oracles use 127-frame blocks; the
separate seek/voice oracles retain 37-frame coverage. Every output frame remains
compared, including page refill and the full 80,000-frame dry extent.

Final seek Debug wrapper `20261104-131014-e5583b22` passed **43/43 CTest tests**,
145.52 seconds, exit 0. Earlier corrected Debug `125343-2335434e` passed 43/43;
Release `125723-f8e18b55` and `130750-e18bdf52` passed 43/43 before the final
mouse-guard refinements. Those are intermediate results, not substituted for
the final Release below. Source input manifest:
`out/issue6-seek-validation-inputs.json`, SHA-256
`1FE71D5CE452825874A98481160A5E5C15B36E31847C836BAC440E0FBBAABD64`.
No new guardrail, shared-style token or debt-inventory changes were made.

Final-source Release run `20261104-131354-6b9b1927` passed 42/43: the existing
frozen-wet recovery fixture failed an immediate Perform admission after restoring
the original tempo (`PerformanceExportCases.h:99`). Its direct Release processor
recheck exited 0. This is separate from preview seeking; production frozen/live
logic and the fixture have not been changed or weakened. A full matching-wrapper
rerun follows. Record this timing-sensitive test result rather than claiming the
initial full Release was clean; no additional owner UAT is requested for it.

Release rerun `20261104-131754-8fbe1c3a` also passed 42/43, this time failing
`LearnPassPolicyCases.h:36` immediately after storage flush. Both legacy fixture
assumptions were made explicit using existing bounded `waitM3` readiness checks:
frozen recovery waits for verified eligibility after tempo restoration, and
Learn waits for the actual accepted-mode handoff after storage completion.
The original eligibility/transition assertions remain; no retries, longer
timeouts, disabled tests or production Learn/Perform changes were introduced.

### Validated seek candidate and native review

- Debug wrapper `20261104-132138-8cd483ec`: **43/43 passed**, 178.25 seconds.
- A subsequent Play-tooltip-only clarification was built through the Debug
  wrapper (`132539-6ba7443f`); focused `--test-export-paint` passed on that binary.
  The unchanged broader Debug suite was not repeated solely for tooltip copy.
- Release wrapper `20261104-132741-c1ddf608`: **43/43 passed**, 54.38 seconds,
  exit 0, including the two bounded readiness fixtures and seek regressions.
- Final source/resource/test input manifest SHA-256:
  `D37DBC21BD58F3E1253CE49CEA3C02AF5F722268714D668C9027A002F3B03148`.
- Current Release inner-binary SHA-256:
  `98F872FFDAFE24A6DB265D6C953D98D4B54BB3050323181F8682F2B5CF789E5A`.
  Bundle remains `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
  Machine-readable evidence is in `out/issue6-seek-release-validation.json`.

The native seek preview is launched from an isolated Debug executable in
`out/component-previews/issue6-playhead-final/`. It uses the real paged preview
and a silent fixture clock, not an audio device or the DAW. Only revised gesture
feel needs owner approval: stopped click/drag parks; playing seek continues;
Stop holds position; Shift-drag pans and wheel/Fit remain available. Prior visual
and FL Studio delivery/mix/persistence passes are retained. New seek behavior is
not claimed tested in FL Studio; no repetition of the old host checklist is
requested. Shared #46 ruler/navigation integration and merge/archive hold remain.

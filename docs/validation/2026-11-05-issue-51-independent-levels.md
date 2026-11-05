# Issue #51 independent Export levels

## Baseline and scope

- Branch: `codex/51-independent-export-levels`, created from locally available `origin/main` at `ae4759424cca760af65e8419d74dbd68a6094275`.
- GitHub PR #50 is merged (2026-11-04); issue #6 is closed as completed. Its final review correction `6a6aa9c` addresses publication recovery after a transient coherent timing read miss and is included in the merged dependency. Existing UI/host acceptance is recorded in PR #50 and its linked evidence; it is not a new test of #51.
- Owner approved transparent independent unity addition on 2026-11-05: retain per-swell calibration, remove Export wet-aggregate/combined attenuation, report overload, preserve above-unity Float32 samples, retain disclosed preview boundary ramps and add no output attenuation control.
- Untracked `support-existing-tempo-maps` (#71) artifacts remain untouched. Only #51 artifacts will be staged for this issue.
- Local branch setup required elevated Git access because the restricted process could not write `.git/index.lock`; branch creation then succeeded. No user audio, installed plugin or build cache was changed.

## Validation status

The staged state/model changes add independent level storage and an Export metadata version, retain legacy history with explicit gain-law provenance and invalidate the old current composition on migration. DSP and production UI changes are still pending; do not install this intermediate build for listening acceptance.

- Native Debug `preverb_song_export_test.exe`: PASS, exit 0, including added pair precision/legacy migration/malformed-state cases and existing composition/file/sample cases. Tracked working storage: overlap 67,222,352 bytes, hour-sparse 373,664 bytes. This is a focused native executable result, not full CTest.
- UI standards: PASS, 52 files / 61 bounded occurrences; inventory unchanged. Diff whitespace check passed.
- Debug wrapper first failed at configure because Windows SDK registry access was denied. Elevated retry configured and compiled state tests; its new control fixture exposed an unavailable ComboBox getter in the test, corrected to a direct wheel-event assertion. Full wrapper/CTest results remain pending.
- No control look/feel, preset listening, FL Studio or actual DPI approval is claimed.

## Shared-control candidate

`ExportLevelsControl` composes existing DeliberateSelector/SliderControl/value policies. It has no production Export consumer. Preset selection emits one pair; direct edits preserve the other level; Custom derives from the exact pair. Candidate names/levels are the issue's proposed listening values, not approved sonic settings.

Native Debug `preverb_swell_ui_test` passed (16.88 s) during wrapper run `20261105-165921-a0eb7c86`, including the new preset/fader/refresh/no-wheel/stale-binding checks, four supported geometry sizes and existing shared input/focus/typed-value coverage. This is automated component evidence, not owner native approval or actual DPI coverage. A standalone native preview is prepared under `--review-export-levels`; audio listening approval remains separate.

## Completed Debug boundary

`scripts/build.ps1 -Configuration Debug -Test` completed successfully from dirty baseline `ae47594`: **71/71 native CTest tests passed, 520.71 s**. Canonical logs: `out/build-logs/Debug/20261105-165921-a0eb7c86`. The existing build tree/dependency cache was reused, but moving from the older checkout required substantial compilation; this is not a measured speedup. The first build's corrected UI test compile failure is retained in `20261105-164400-a6b67346`; no tests ran in that failed wrapper invocation.

Strict validation of `independent-export-levels`, UI standards (52 files / 61 existing bounded occurrences), guide currency and diff whitespace checks pass. Release, independent-level DSP oracles, production integration, native owner acceptance, preset listening and FL Studio/DPI coverage remain pending. This is an intermediate state/control boundary, not completed issue #51 delivery.

The standalone 1100x850 native control review is open (window title `ReverbArise control review`, process 56148). Launch verification required desktop-level process visibility; a verification launch created a duplicate, which was closed, leaving the original review window open. Owner visual/interaction response is pending; task 3.2 remains unchecked. This fixture edits only local preview values and plays no audio.

Owner response on 2026-11-05: “yes looks good”, approving the presented selector/two-fader appearance and interaction. This does not establish preset listening approval or native review at larger sizes; task 3.2 remains incomplete until its remaining checks pass. The open window is retained for the owner.

## Independent mixing boundary after control approval

- Atomic pair edits now flow through session/engine preparation, identity and v2 pair-identifying immutable filenames. Initial Export defaults to 0/100. Legacy crossfade setters temporarily adapt to a pair while production control integration awaits listening approval.
- SongMix sums transparent independent gains, measures actual peaks, retains above-unity samples and uses weighted shared display scaling. Detail adoption rejects aggregate attenuation. Dry/wet dependencies are gated independently. Explicit 0/0 clears current readiness and offers no file; historical files survive. Existing notice roles disclose downstream clipping risk.
- Debug wrapper build PASS: `out/build-logs/Debug/20261105-174222-163e8076`. Existing warm tree reused; no speedup claimed. The preceding rebuild hit LNK1168 because the approved native review still held the UI executable; the review was then closed and rebuild succeeded.
- Focused native CTest **5/5 PASS, 40.74 s**: workflow processor, Export voice, song Export engine, song Export sample/file tests and swell UI. Independent-layer oracles include unity/intermediate/zero, high peaks, correlated overlap and antiphase; decoded WAV tolerance is 2e-6, engine preview/ramp oracle 1e-6. Callback allocation checks remain active. Engine coverage includes pair no-op/stale rejection, pending state, restore/reuse, immutable history and zero-pair disabled delivery.
- First focused run was 4/5: the old source-reader fixture changed Wet alone to imply dry bypass. Corrected it to specify Dry 0/Wet 1 and Dry 1/Wet 0, preserving its zero-read and warned-gap assertions; all five pass on rerun.
- Strict change validation, UI standards, guide review/currency and whitespace checks pass. The earlier 71/71 Debug suite applies to the preceding state/control boundary, not the subsequent audio edits. Final whole-change Debug/Release, listening, integrated controls, PR and host acceptance remain pending.
- Preset listening source is awaiting owner preference; no musical/sonic acceptance is inferred from “looks good”.

## Generated preset listening checkpoint

Owner selected generated audio and reserved FL Studio smoke testing for the end. `preverb_song_export_test --render-preset-listening <workspace-output>` creates only isolated generated assets. Native Debug build `20261105-181825-73bc26fc` and fixture execution passed. Four generated plucked notes use the bundled default IR, actual reverse convolution with source-edge conditioning, approved source-energy +6 dB calibration, and the real independent SongMix/verified Float32 publication path. Eight seconds, stereo 48 kHz, shared source/timing and accepted wet across all candidates; no output attenuation or per-file normalization.

Outputs: `out/issue51-preset-listening/generated-99ba694058e848c198967e253588c12a/`. `listening.txt` records exact published filenames, pairs and peaks. Peak levels: dry reference -24.24 dBFS; Wet Layer -17.15; Subtle -21.90; Balanced -20.01; Cinematic -17.10; Wet Forward -17.15. These are fixture measurements, not sonic acceptance or external DAW equivalence.

`preset-comparison.wav` concatenates the six verified data chunks byte-for-byte, with matching stereo 48 kHz Float32 format and no gain changes. Total 48 seconds: 0–8 dry reference, 8–16 Wet Layer, 16–24 Subtle, 24–32 Balanced, 32–40 Cinematic, 40–48 Wet Forward. Payload length, format, finite sub-unity peaks and exact concatenated bytes were checked.

Native Debug build `20261105-182017-b476f9d1` passed for the additional `--review-export-levels-large` launch path (1400x1000). The review uses a copy under `out/issue51-control-review/` so leaving it open does not lock the build target. UI standards, guide currency and whitespace checks pass. Listening and larger native acceptance remain pending; task 3.2 stays unchecked and production integration has not begun. This fixture does not read or change any real project/captured material or install a plugin.

Owner response after the generated comparison and larger preview: “seems ok but I'd like to test in FL”. This is accepted as approval of the candidate names/pairs and larger controls for integration, with final listening/host acceptance reserved for the requested FL Studio smoke test. The earlier minimum-size appearance/interaction approval remains in force. No final host acceptance is inferred.

## Production integration and final Debug checks

Production Export now binds the reviewed selector and both sliders to atomic session/pass-pinned pair edits. Its white/aqua layers, legend, retained accepted caption, readiness and file metadata use the independent pair. The original waveform/ruler, navigation, transport and delivery layout remain. Zero intent reports Silent immediately; nonzero cancelling contributions remain drawable independently of summed audible bounds. Overload notice details retain all warned missing-dry ranges.

- Full native Debug wrapper: **71/71 PASS, 533.22 s**, `out/build-logs/Debug/20261105-194031-6c05aff3`. This includes integrated preset/Custom/stale-mode binding and existing delivery, state, DSP and callback coverage.
- Small final silent-state/drawing/warning refinements were rebuilt through the wrapper (`20261105-195136-464a199a`), then the five affected native tests reran: **5/5 PASS, 78.66 s** (workflow processor, Export voice, song Export engine, song Export sample/file, swell UI). This is full-suite plus focused final coverage, not a second full 71/71 run of the final refinements.
- Both build runs reused the existing tree/caches. No cold/warm speedup comparison is claimed. Strict OpenSpec, UI standards (52 files / unchanged 61 bounded occurrences), guide review/currency and whitespace checks pass.
- Save/drag remains current-revision-only. Engine tests retain original captures/render assets and prior verified WAV histories across edits, restore, cancellation, disk faults and stale completion; actual FL Playlist delivery remains the final owner smoke check.
- Owner confirmed FL Studio closed for the pending Release/test installation. Release full CTest, system installation, final integrated native/FL acceptance and PR remain pending.

## Release gate and pending installation

Source commit: `bd481756943be42f9579792fb647b74ff7281a1d` on `codex/51-independent-export-levels`. The unrelated #71 planning directory remains untracked and untouched; build summaries report dirty state.

- Initial Release wrapper (`20261105-195616-6c3edfcd`): build passed; **70/71 tests passed, 372.73 s**. `preverb_processor_test` timed out at `tests/LiveProcessorCases.h:69` waiting for explicit regeneration after recapture. No install occurred. The isolated unchanged test then passed in **16.06 s**; no timeout or production code was changed to obtain the pass.
- Full unchanged Release retry through `install-test-vst3.ps1 -Test` (`20261105-202333-eed9570e`): warm build passed; **71/71 native CTest passed, 469.76 s**, including the previously timed-out processor test. This is a complete passing full suite, with the initial timeout retained as a validation limitation.
- Installation was then blocked by the helper's second host-closed check: FL Studio had reopened during validation. The installed system bundle was not replaced and no host was stopped. Owner asked to save/close it again; next installation will reuse the passing Release source and tests with the normal plugin-only helper build.
- Final system install, integrated native/FL acceptance and PR delivery remain pending. Release validation is complete; it is not host acceptance.

Final integrated native Export review is open from a copied Release test executable (process 13756), isolated from DAW/project assets. The [1100x850 native image](2026-11-05-issue-51-export-ready.png) was visually inspected: controls, values, actions and dominant waveform fit; accepted pair and peak warning are visible. Its deliberately hot legacy synthetic render reports +41.27 dBFS and the fixture preview clock is silent; this is warning/layout evidence, not listening material or an installed host result. The earlier generated musical comparison remains the listening reference. Actual host/DPI coverage remains pending.

## Verified FL Studio test installation

Owner confirmed FL closed again. The normal helper's Release plugin-only build passed (`20261105-210528-af58ab13`), then Windows denied the system install-lock path. The supported administrator/UAC retry completed with exit 0, without ACL changes or host termination.

- Installed complete bundle: `C:\Program Files\Common Files\VST3\ReverbArise.vst3`.
- Previous bundle retained: `C:\git\k7ma-preverb\out\vst3-install-backups\20261105-210619-32e54547f2b6441eb2bec6065fbbb55e\ReverbArise.vst3`.
- Installed inner binary SHA256: `8DFA2878615C154D4716FC8881F46506ED7B897D71CCDE2BED5B100B7F76F459`.
- Release source: `efdd2ec` (documentation-only follow-up to production `bd48175`; Source/CMake diff between those commits is empty). Build reports dirty state from the preserved unrelated #71 untracked proposal.
- Final installer build: `out/build-logs/Release/20261105-210610-7f60d8d5`; administrator output `out/issue51-install-admin.log`. Plugin-only build passed; CTest was not repeated during installation. The preceding full Release retry passed 71/71 against identical production code.
- [Draft PR #92](https://github.com/elneilios/ReverbArise/pull/92) remains open. Final FL preset/preview, Save/Playlist alignment and save/reopen acceptance is pending; installation alone is not host acceptance. Ask the owner to verify the system bundle path after a verified/rescan-previously-verified plugin scan.

## Owner FL result and requested refinements

Owner reports: “FL smoke test passes — presets, export alignment and save/reopen all look good.” This accepts those installed-build behaviors; actual host/DPI settings were not separately specified. Two follow-ups remain before final delivery: unwanted spoken Export notices and uninterrupted Mix Character changes during preview. The prior acceptance is retained and must not be repeated as a blanket host checklist.

Read-only investigation confirmed that shared NoticeBar explicitly calls `AccessibilityHandler::postAnnouncement`; bundled JUCE's Windows implementation invokes SAPI `Speak` when an accessibility client is active. No particular client or Narrator state was established. Export currently streams a completed WAV, and pair edits stop/revoke that preview; seamless moving-playhead gain changes require an approved plan revision, not just removing the Stop call. No implementation changes were made during this investigation.

## Continuous-preview and quiet-notice follow-up

Production follow-up: `4683f01`, following the approved section 6 plan. Main was merged at `703b2b2`; its Perform caution remains intact. Only the generated guide-review record conflicted and was regenerated after semantic review. Unrelated #71 planning remains untracked and untouched.

The shared NoticeBar no longer calls `postAnnouncement`; visible severity and accessible title/description/help and normal value-change events remain. Export monitoring now streams verified dry and calibrated/shaped wet branches through fixed pages. A coherent accepted target pair uses a 5 ms monitoring-only ramp while retaining the transport ticket and absolute cursor. Gain edits revoke current-file delivery; pending/failed WAV work can continue independently of playback. Source availability changes, Stop, mode/restore/timing invalidation, zero levels, finite end, starvation or non-finite mixed samples stop safely. No new saved-state or WAV gain-law version is introduced.

New coverage includes separate-branch/sample/range oracles, cancelled preparation, raw source/render hashes, 4096-frame handoffs with unaligned blocks, concurrent coherent targets, exact ramp samples, callback allocation/deallocation guards, finite-sum rejection and fixed-memory limits. The held-publication engine case advances 550 x 127 frames (69,850 frames), exceeding the entire 65,536-frame prefetch capacity, then injects a publication obstruction while asserting continued cursor advance and retained WAV integrity. UI fixtures cover current accepted target, pending measurement/file, warning/error/zero state, view retention and stale control invocations. Actual host acceptance of these follow-ups remains pending.

Development runs, retained separately from final gates:

- Debug build-only `20261105-213537-91fc42a4` and `20261105-214336-1c3e916e`: passed. Initial focused tests launched without normal Windows temporary-storage access failed capture setup/UI readiness; the escalated run passed workflow, voice, mixer and UI, and exposed the new summary reservation overcount in the engine test. Accounting was corrected before later runs.
- Debug full `20261105-214705-79472379`: 69/71, 682.91 s. Export engine and UI passed; the newly extended mixer test crashed after adding a large stack-local reader (moved to heap), and the existing processor test timed out at `LiveResourceCases.h:34`. These are failures, not final acceptance or proof of a transient failure.
- Debug `20261105-215925-83d21a5f`: build passed, test run explicitly superseded/stopped during early tests after the finite-sum safety guard was added. This is an incomplete run, not a full-suite pass. Only the owned CTest/test process was stopped; no DAW host was terminated.

Final focused and Debug/Release results follow below when completed. All builds reuse the existing warm MSVC/JUCE tree; no cache clearing or cold-build speed claim is made.

### Focused current-code result

`4683f01` production source; warm Debug build-only `20261105-220642-c3dd1b63` passed. The focused native CTest expression `^preverb_(song_export|song_export_engine|export_voice|workflow_processor|swell_ui|processor)_test$` passed 6/6 in 114.74 s. Individual results: workflow 3.26 s, voice 3.03 s, engine 12.34 s, mixer 23.17 s, UI 19.03 s, processor 53.67 s. Copy of CTest's result log: `out/issue51-followup-focused-debug.log`.

The stack-local mixer test crash is resolved by heap ownership. The processor resource assertion passed on current code without a deadline change; the earlier failure remains recorded, and this isolated result is not a replacement for the pending full-suite gates. Production Source/CMake is unchanged after `4683f01`; task/evidence edits do not change that claim. UI standards, strict change validation and explicit user-guide review/currency pass. Task 6.1–6.4 completion was guarded against the CLI task locations and each completion confirmed by a fresh Apply instruction query (16/20 complete).

Debug `20261105-221405-c300dfb5` built successfully and passed through test 37 before being explicitly superseded/stopped for the replay-status correction (not a full pass). On retrying a ready preview, Play now clears a prior worker preview-failure message. Concurrent CTest jobs were observed in two other Codex worktrees; they were left untouched. This is a shared-machine timing condition, not evidence of the earlier timeout's cause. Only this checkout's CTest and its identified child were stopped.


Debug `20261105-221819-f4a8a530` built production `08e4262`; the full run was stopped after test 46 because it already required retry and other checkouts were simultaneously running CTest/MSVC jobs. Known failures before stopping: generation UI `learn_generation_ui_test.cpp:40` (`autoGenerateOnStop` after a 30 ms dispatch); IR streaming `ir_streaming_test.cpp:45` (missing live page at sample 767744 after 1419.59 ms, Preparing live window / Reading source); Perform composition preview segmentation fault after 112.30 s. Export voice, engine, mixer and UI passed in that run (3.62 s, 44.53 s, 34.64 s, 62.56 s respectively). This is an incomplete failing run, not a full pass. Shared load/desktop contention is observed; causation is not established. Test assertions/deadlines and other worktrees were left untouched. Final checks will be retried in a quieter window.

Coordination: issue #58 compact layout will follow accepted/merged #51 changes. This branch does not adopt its pending geometry; the existing approved Export layout is preserved. PR #92 remains draft and unmerged pending native gates and the targeted FL follow-up.

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

# Issue #108: focused Level native control checkpoint

2026-11-06. Apply checkpoint for `focus-swell-level-adjustment`, **3/8 tasks complete** (1.1, 1.2, 2.1). Task 2.2 owner approval remains pending. Production construction keeps the accepted mapping; the new mapping is enabled only through `enableFocusedLevelReview()` in the native review/tests. Wider Perform integration, cross-path expanded audio tests, final guide review and Release/final gates remain unchecked.

## Candidate and compatibility

- Durable Level constants remain finite [-60,+12] with 0 dB default. Model tests accept endpoints and non-step fractions and reject out-of-range/nonfinite edits transactionally.
- The review mapping centers 0 dB in a linear [-12,+12] sweep and retains 1/0.1 dB coarse/fine steps. Extended values display their actual numeric value with the dial at its lower endpoint. Relative extended edits stay anchored to actual dB; exact entry and explicit numeric accessibility writes retain the full domain.
- The existing `KnobValue` family's optional absolute callback preserves the previous delta path for other consumers. The Level candidate uses existing controls, tokens, geometry, help delivery and invocation guards. Standards revision 11 is explicitly pending owner acceptance; revision 10 remains accepted production policy.
- Session fixtures cover Legacy/Musical -60, -30 and non-step fractional values. Real processor-wrapper round trips retain frozen definitions, policy, calibration and render/IR identities without playback. Frozen engine fixtures reopen verified renders with absent source coverage, including a nonzero saved Musical calibration, with zero reconvolution. The processor still exposes no host parameters.
- #80 is in the baseline; refreshed main had no newer commits. #107 remains the separate button-state/name-based styling follow-up, without a competing Level implementation in the inspected source. No shared control framework or Learn Level default is introduced here.

## Native verification

Tested source is the checkpoint commit containing this record, built from `b974043` plus its implementation changes. Product build stamp therefore reports that prior revision as dirty. Exact native binary hashes below identify the executables actually used.

| Check | Result |
| --- | --- |
| Supported native Debug wrapper | Pass, final incremental run `out/build-logs/Debug/20261106-173807-0a6f7c7f`; build-only, not a full CTest run |
| Focused CTest: gain staging, swell shape, swell session | **3/3 pass, 1.90 s** |
| `preverb_swell_engine_test.exe` | Pass; includes frozen extended-Level restore with absent source coverage |
| `preverb_processor_test.exe` | Pass; includes real wrapper save/reopen, no Level host parameter, existing playback/allocation/resource cases |
| `preverb_perform_composition_preview.exe --test-level` | Pass; focused endpoints/midpoint, coarse/fine/wheel/drag, extended exact entry, rejection, accessibility domain/absolute edits, Level-only reset and stale target guards |
| `preverb_perform_composition_preview.exe --test-knobs` | Pass; existing control gestures, accepted feedback, timing/selection guards, accessibility, focus and text checks |
| Existing `--test-knob-resets` full popup regression | **Not passed**: twice failed its prerequisite `peer->isFocused() && Process::isForegroundProcess()` before the popup assertions. Native peer focus existed, but Windows foreground ownership was unavailable. Do not report that suite as passed |
| Foreground native preview automation | Verified actual -30 numeric/accessible edit, visible extended value at saturated lower dial, correct Level-only reset popup label, Escape dismissal, double-click restoring 0 dB and unchanged normal UI composition at 900x600 |
| UI standards | Pass, 64 files / 59 bounded occurrences; debt inventory unchanged |
| Strict OpenSpec / whitespace | Pass at checkpoint |

The successful foreground preview smoke does not replace the entire popup regression's stale-menu/lifetime matrix. Preserve this distinction for final integration validation. No new Release build, full Debug suite, FL Studio installation/host smoke or additional OS DPI test occurred. Native owner look/feel and listening acceptance remain pending.

Commands used (CTest after the wrapper released its build lock):

```powershell
$env:CMAKE_BUILD_PARALLEL_LEVEL='4'
./scripts/build.ps1 -Configuration Debug
& 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe' --test-dir out/build/windows-msvc -C Debug -R '^preverb_(swell_session|swell_shape|gain_staging)_test$' --output-on-failure --timeout 180
./out/build/windows-msvc/Debug/preverb_swell_engine_test.exe
./out/build/windows-msvc/Debug/preverb_processor_test.exe
./out/build/windows-msvc/Debug/preverb_perform_composition_preview.exe --test-level
./out/build/windows-msvc/Debug/preverb_perform_composition_preview.exe --test-knobs
./scripts/check-ui-standards.ps1
openspec validate focus-swell-level-adjustment --strict
git diff --check
```

This worktree began without a native build tree: dependency/configuration and initial object compilation were cold. The first compile caught an invalid tooltip call on the outer component; corrected it to the actual tooltip-owning children. An initial serial wrapper was deliberately stopped by its verified process tree and confirmed idle, then resumed with four project workers. Usable objects/dependencies were preserved, no caches cleared. The final wrapper run was incremental. No comparable timing baseline or speedup is claimed.

Initial sandboxed capture fixtures failed on temporary capture operations; the native engine/processor/UI fixture runs above used approved unsandboxed execution. A separate model fixture initially marked a renderless swell frozen, which the existing validator correctly rejected; corrected the fixture while retaining genuine frozen tests on actual rendered assets. No production validation was relaxed.

## Bounded new listening comparison

`preverb_gain_review.exe <absolute-output-directory> --level-review` renders approved energy +6 calibration with Level -12/-24 through production importer, convolution, shape and audition protection. The separate mode leaves the historical default review mode available. It uses Hall, Whisper and an imported chamber, quiet/strong chords: **12 measured rows and 24 stereo 48 kHz/16-bit WAV players**. Dry + Wet is the existing fixed 50% comparison blend, explicitly distinguished from current live monitoring.

All twelve rows have approximately -6 or -18 dB wet/dry attack intensity as requested (error <0.001 dB); none needs isolated wet protection. Verified all page audio links exist and every WAV has valid stereo format/nonempty frames. [Retained native measurements](2026-11-06-issue-108-native-level-listening.json) identify actual Level, baseline, source PCM/factory hashes, protection and output peaks. This is new generated objective evidence, **not owner listening approval**; the earlier 2026-11-02 approval remains the calibration baseline only.

Local output: `out/issue-108-level-listening/index.html`. The page was opened directly for the owner. One native `--review-level` window was opened at 900x600; its top selector supplies -12/0/+12/-30/-60 presets, and its size selector supplies minimum/larger compositions. It remains open for owner review. The native fixture has no audio device: use the browser comparison for sound.

| Artifact | SHA-256 |
| --- | --- |
| Native control preview executable | `ddf1697e0116b1f10aced4ae6d2325778b6ac4d811363840bf90d69e49e8dcc5` |
| Native gain review executable | `857c39fc4e11ebcab4d3b05a1f3b3d7c223e336b2c6f753df7e3fea8287882d7` |
| Native listening metrics JSON | `acae202a04c4a464f6faaf287372b7a4fabe034d9120d2cfa65264036a4226dc` |

## Review boundary

Owner review is required by [task 2.2](../../openspec/changes/focus-swell-level-adjustment/tasks.md) before task 3.1 rollout. Review the dial's ordinary and extended feel, exact entry/reset and a brief -12/-24 listening comparison. The native automated edge cases do not need repeating manually. If the feel/range needs revision, change only the candidate and its tests until accepted. Continue on PR #110; no merge, canonical spec sync or archive is authorized by this checkpoint.

## Subsequent owner approval — 2026-11-06

The owner approved the native dial's look/feel and the new -12/-24 dB comparison,
and explicitly requested integration and remaining validation. They reported that
-24 dB was inaudible with the quiet chord but audible with the strong chord.
This is listening feedback at their monitoring level, not a digital-silence or
universal audibility threshold; source-relative Level preserves source dynamics.
The approval covers this revised Level family and bounded listening comparison.
It releases task 2.2's gate without reapproving or changing calibration, default
gain, other controls or host/DPI coverage. The checkpoint results above remain
the original candidate results; [final integration evidence](2026-11-06-issue-108-integration.md) is recorded separately.

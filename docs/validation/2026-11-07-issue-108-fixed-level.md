# Issue #108 fixed-limit Level checkpoint

The native candidate implements the owner-confirmed -12 to +12 dB edit range
with visible limits and 0 dB at 12 o'clock. The dial sits at its lower limit for
an older value below -12 dB, while the field shows the saved gain and the help
explains the edit range. An inward edit or explicit reset can replace the saved
gain. Outward movement at the lower limit, unchanged text, and cancellation
preserve it.

OpenSpec Apply completed tasks 5.1 and 5.2. The native preview was opened with
`preverb_perform_composition_preview.exe --review-level` on 2026-11-07. Task
5.3 is awaiting the owner's approval of the revised presentation and feel.
The [earlier listening approval](2026-11-06-issue-108-control-checkpoint.md)
covers calibration and source-dependent audibility.

## Candidate verification

Windows x64, Visual Studio 2026 MSVC and pinned JUCE 9.0.4. The candidate is
the source committed with this record. Its build metadata reports
`7badf842b23ff608af3f27f314745dceba3c6174` plus working-tree changes. Native
preview SHA-256:
`669a33990a82393518f6857bea2a8303c2d8c28455654e4320c25fc3753de9cc`.

- Supported `scripts/build.ps1 -Configuration Debug` passed twice with
  `CMAKE_BUILD_PARALLEL_LEVEL=4`. The first run compiled cold objects using
  cached local JUCE and WavPack sources. The candidate rebuild was incremental.
  Logs are in `out/build-logs/Debug/20261107-114843-b86daf4d` and
  `out/build-logs/Debug/20261107-115959-f2b16126`.
- Debug CTest passed `preverb_swell_session_test` and `preverb_processor_test`.
  The processor test exercises both gain policies and frozen -60, -30 and
  non-step values through wrapper load, unrelated rename, save and reopen.
  The initial focused Level test reached Escape cancellation before JUCE
  dispatched the key event. The test now waits for the field to restore.
- After rebuilding, Debug CTest passed `preverb_focused_level_test` and
  `preverb_perform_knob_test` (2/2, 10.79 s). This includes native mouse events
  in both directions and at both limits, fine movement, exact and accessibility
  entry, stale edits and Level-only reset. It checks the neutral marker in
  production paint and the labels at 900×600, 1400×900 and 125% user scale.
  CTest ran with `--timeout 240 --output-on-failure`. Local logs are
  `out/issue-108-fixed-level-debug.log` and `out/issue-108-fixed-level-candidate.log`.
- UI standards, strict OpenSpec validation and whitespace checks passed.
  Release and final integration checks follow the owner checkpoint. FL Studio
  and OS DPI coverage were outside this candidate run.

## Standard-knob revision

The owner asked to keep the standard knob appearance and show the exact older
value in the ordinary input. The revised candidate reuses the standard caption,
layout and drag response. A primary press immediately applies -12 dB for a
saved value below range. A valid typed value applies directly.

- Supported warm Debug build passed with `CMAKE_BUILD_PARALLEL_LEVEL=4`.
  Log: `out/build-logs/Debug/20261107-120840-dc4d926f`.
- Debug CTest passed the focused Level, shared Perform knob, session, and Level
  audio-path checks. The focused test covers native primary press before
  movement, passive hover/focus, wheel and keys, exact entry and stale input.
  It compares the standard control layout and verifies neutral paint at compact,
  larger and 125% presentation sizes.
- The reset regression initially lacked foreground activation. It passed
  unchanged on rerun after activating the named fixture window. Logs:
  `out/issue-108-standard-level-debug.log` and
  `out/issue-108-standard-level-reset.log`.
- UI standards, strict OpenSpec validation and whitespace checks passed.
- The preview build reports `418ac1c9544c8c41cac593b05d9e4407e768e91a` plus
  working-tree changes. Its SHA-256 is
  `333d57e2e5f6996719d3fdad5cdb24f26468f6e0552c006a9b05cc6f4e621980`.
  The standard-knob preview was opened for the owner checkpoint.

The owner approved the standard-knob candidate on 2026-11-07 and asked to
finish integration. This approval covers the existing knob appearance and
the first-touch Level behavior.

## Final integration

Tested clean source revision `305927fb26df421b07cb398bac892cedad626b1a`, which
includes `main` at `ca9f7df`. The merge retained the current selector and
Storage workflows and shared native reset fixture. Production UI standards
revision 15 records the approved Level behavior alongside those changes.

The focused native test saves and reopens a frozen swell after an actual
first press changes its old fractional gain to -12 dB. Shape, calibration,
render identity and frozen status survive the round trip. The processor
regression also reopens older values before any Level edit. The audio-path
fixture checks audition, live Song, plugin WAV and host offline processing
against the gain oracle, including -12, 0 and +12 dB, and exercises the existing
path-specific protection and raw-asset reuse.

| Check | Result |
| --- | --- |
| Supported Debug and Release wrappers | Passed on Windows x64 with Visual Studio 2026 MSVC and pinned JUCE 9.0.4. Incremental on this worktree after merging main, with cold compilation for newly introduced targets. Logs: `out/build-logs/Debug/20261107-123354-0f80b914` and `out/build-logs/Release/20261107-123815-7ecec06f`. |
| Debug focused CTest | 7/8 in 49.39 s. The full processor suite timed out in the existing Learn recapture wait at `LearnPassPolicyCases.h:52`. The identical binary passed its isolated retry in 42.33 s. Logs: `out/issue-108-merged-debug.log` and `out/issue-108-merged-debug-processor-retry.log`. |
| Release focused CTest | 14/14 in 46.54 s, including the full processor test. The separate native knob reset test passed in 2.13 s. Logs: `out/issue-108-merged-release.log` and `out/issue-108-merged-release-reset.log`. |
| UI standards, strict OpenSpec, whitespace and links | Passed. UI guard covered 73 files and 59 bounded occurrences with the existing debt inventory. |
| Offline guide | Explicit review and read-only currency passed. Native Release capture and static assembly passed with four 1400×1000 production screenshots and the embedded logo. Inspected the Perform capture. Working QA output: `out/issue-108-final-guide/preview/User-Guide.html`. |

The first pre-merge Release build compiled cold objects from cached
dependencies. The final tests ran with compilation idle. The current pointer
checks use native `mouseDown`, `mouseDrag` and `mouseUp` in both directions
and assert the first press before any movement. Earlier extended-editing
checks used synthetic value changes for part of that coverage.

```powershell
$env:CMAKE_BUILD_PARALLEL_LEVEL='4'
./scripts/build.ps1 -Configuration Debug
./scripts/build.ps1 -Configuration Release
# $ctest resolves to the Visual Studio CMake CTest executable.
& $ctest --test-dir out/build/windows-msvc -C Debug -R '^preverb_(focused_level|perform_binding|perform_knob|level_audio_paths|swell_session|processor|ir_handover|gain_staging)_test$' --output-on-failure --timeout 240
& $ctest --test-dir out/build/windows-msvc -C Debug -R '^preverb_processor_test$' --output-on-failure --timeout 240
& $ctest --test-dir out/build/windows-msvc -C Release -R '^preverb_(focused_level|perform_binding|perform_knob|level_audio_paths|swell_session|swell_engine|processor|ir_handover|gain_staging|selector_binding|responsive_scale|working_folder|storage_settings|song_export)_test$' --output-on-failure --timeout 240
& $ctest --test-dir out/build/windows-msvc -C Release -R '^preverb_perform_knob_reset_test$' --output-on-failure --timeout 240
./scripts/check-ui-standards.ps1
./scripts/check-user-guide.ps1
openspec validate focus-swell-level-adjustment --strict
git diff --check origin/main HEAD
```

| Tested artifact | SHA-256 |
| --- | --- |
| Debug native Level executable | `c1531d7884e0ad274c38137afd0d9289be80902378e6eedf8b0b1302a40c9823` |
| Release native Level executable | `022563e67bb38a15694589cf9056bf68f940dbc636addab965e14ec12fed7435` |
| Release processor test | `06b323cea1da687de1269eb38cce86f3c8fd13cdfede85fcc1936af67f216a5e` |
| Release VST3 binary | `36641fb243e019663bd9b961c3943a1352f7cf3b23023bd135a7e99cc3843432` |
| Assembled offline guide | `bda80dec845364ef0218b2aab1a1dd9023bf17f20ed462a63b042888119e5d71` |

Coverage includes native 100% and 125% user scale and simulated host
processing. Physical OS DPI, an FL Studio run and full CTest suites were
outside this focused validation.

### Storage baseline refresh

`main` advanced to `d07d139` before PR handoff. The next merge retained the
approved Level source and the new scratch-storage behavior. The guide retained
both workflows and its explicit currency review passed.

Clean source `bcfa74b4748b1720fc2721070ca264b6388f350a` passed the same
supported Debug and Release wrapper commands on the warm tree. Logs:
`out/build-logs/Debug/20261107-125154-dc3a9f84` and
`out/build-logs/Release/20261107-125256-6bc3e7a7`.

```powershell
& $ctest --test-dir out/build/windows-msvc -C Debug -R '^preverb_(focused_level|level_audio_paths|swell_session)_test$' --output-on-failure --timeout 240
& $ctest --test-dir out/build/windows-msvc -C Release -R '^preverb_(focused_level|level_audio_paths|processor|working_folder|scratch_maintenance|scratch_quarantine|scratch_removal|scratch_maintenance_callback|storage_settings|portable_recovery)_test$' --output-on-failure --timeout 240
```

Debug passed 3/3 in 13.52 s. Release passed 10/10 in 28.56 s. Logs:
`out/issue-108-storage-base-debug.log` and
`out/issue-108-storage-base-release.log`. UI standards, guide currency,
strict OpenSpec, whitespace and a fresh Release guide capture and assembly
passed. The assembled guide includes the Level first-touch instructions and
current Storage retention guidance.

| Refreshed artifact | SHA-256 |
| --- | --- |
| Debug native Level executable | `20c1694f241f9825d674991098ecd068b0f337f2e8464d4b8011de0a1b0bd841` |
| Release native Level executable | `901c918905689c782e49bbdf41dd98dc62954445e1dbddef9cd1c0c77f961b95` |
| Release processor test | `a4a74ab33529f90c94f98bd1bee15d8886c6fab68e32c39c013918b4bd142ebb` |
| Release VST3 binary | `42a2bff57fdbf92b4ceabae8ad73a80e4fb141dc1a1837e7a5e10dddcdb51783` |
| Assembled offline guide | `f608e41aab502408d020b4cb7850709ff8e8790b105d8c181e10c95d6118d852` |

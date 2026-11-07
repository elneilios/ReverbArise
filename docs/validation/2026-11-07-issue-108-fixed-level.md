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

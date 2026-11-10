# Issue #126: Loop Learn and Perform validation

[GitHub issue #126](https://github.com/elneilios/ReverbArise/issues/126) integrates Loop capture and playback with the shared Learn and Perform workspaces. The owner reviewed the native previews and approved the one-button Cue, Cancel and Stop flow, the repeating playhead and Lead-In treatment, and the final Learn layout.

| Native fixture | Evidence |
| --- | --- |
| Song and Loop Perform at 900 × 600, 100% software scale | [Comparison](song-loop-100.png) |
| Song and Loop Perform at 900 × 600, 125% software scale | [Comparison](song-loop-125.png) |
| Repeating Loop with a different swell selected | [Lead-In and playhead](loop-next-lead-in.png) |
| Empty Loop Learn in End Signal | [Compact Arm layout](learn-end-signal.png) |

The native Perform fixture uses a real accepted loop and two rendered swells. It exercises exact fields and waveform edits, Cue through lead-in and repeat, cancellation before launch, Stop, and loss and recovery of Phrase sync. The processor tests check dry passthrough, that Learn can verify Phrase pulses without playing wet, that Cue can use those observations on entry to Perform, and that a replacement clears the old reference. Learn tests cover immediate Arm cancellation with no audio callbacks, plus the compact idle and active layouts.

Windows x64 MSVC Release build passed. The first full run had one environment failure when Windows denied foreground activation to the native Perform knob reset test. The full `ctest --preset windows-msvc-release --no-tests=error` rerun used the same binaries and passed all 102 tests in 314.85 seconds. The knob reset window was brought to the foreground during the rerun. Strict OpenSpec, UI standards and offline guide currency checks also passed. Both full test logs are retained under `out/build-logs/Release/20261110-071251-3f9859cd/`.

The audible desktop handoff used VCV Rack Free 2.6.6 on Windows with Host-FX 2.1.0. The owner exercised Loop Perform and confirmed the replacement patches worked. The [Rack fixture instructions](../issue-133-rack-fixture/README.md) document the patch generator, routing, timing traces, and accepted audio for the 4-, 8- and 16-step captures. Headless Rack measurements used Fundamental 2.6.4 and Impromptu Modular 2.5.0, at 48 kHz with 128-frame callbacks. The fresh patches contain no saved plug-in state.

The desktop handoff copies and their VST3 bundle are together under `out/issue-126/arm-cancel-fix/`. The tracked generator also builds the examples against the current local Release bundle. End-to-end timing assertions, including early Phrase verification and fresh Cue after phase loss, are from native processor/UI tests and recorded Rack timing replay. The final Release build was not re-run through the entire audible desktop sequence. Physical DPI and FL Studio Loop routing remain outside this validation.

## PR #140 review verification

The Add command uses the displayed Loop playhead while it is inside the phrase, and the visible view start at other times. Add commands and waveform insertions carry the displayed session and captured-source identity into the engine. The native regression covers both placements and rejects stale session and wrong-source identities.

The Windows MSVC Release build passed. Six focused regressions passed in `out/build-logs/Release/20261110-075317-4da858a5/review-regressions.log`. The Loop Perform run in that batch crashed inside the AMD graphics driver. A direct rerun also exposed a fixture race around an edited swell. The fixture now waits for the edited render before playback and passes in `out/build-logs/Release/20261110-080035-102d5701/loop-perform-review.log`. The full 102-test pass above covers the pre-review revision.

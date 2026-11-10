# Issue #144 live output peak guard

The candidate starts from `fa1153d` on `feature/loop-mode`. It uses a linked
stereo sample-peak ceiling of −1 dBFS for the realtime Dry + Wet sum.
Attenuation starts on the peak sample. A linear release returns to unity
within 50 ms while wet remains active. The guard adds zero samples of latency.
The saved +6 dB wet calibration target and Live Wet control remain independent
of output protection.

## Native checks

- `preverb_transport_playback_test` passed its guard and Song mixer checks.
  The guard applies identical gain to both channels and returns bit-identical
  output across whole, 127-frame and single-frame partitions at 44.1, 48 and
  96 kHz. The 2.0/-1.0 stress input reached −1 dBFS with 7.02 dB of linked gain
  reduction. The test also covers repeated peaks, recovery, reset and zero
  audio-thread allocations/deallocations.
- The Song mixer check compares the live mix to an unguarded offline player
  with identical dry and wet inputs. It verifies unity under the ceiling,
  linked gain during overload and the stop fade, unity on a fresh seek, dry
  passthrough after stop, the existing Wet Only transition, and protection
  during the fade back to Dry + Wet.
- `preverb_loop_playback_test` passed its FL Studio grid and Rack Clock grid
  checks. With hot dry, the first wet lead-in starts at its original sample
  and reaches the ceiling with linked gain. Stop Now protects the wet fade and
  returns to the original dry level at the end.
- Strict OpenSpec validation and the read-only user guide currency check passed.

## Full Windows Release suite

Both runs used the warm MSVC build tree and `scripts/build.ps1 -Configuration
Release -Test`.

- Initial implementation passed 102/102 in 447.82 seconds. The subsequent
  routing-fade refinement added a specific Song regression.
- The final implementation passed 101/102 in 445.59 seconds. The remaining
  `preverb_perform_knob_reset_test` reported its native foreground-focus
  prerequisite unavailable. With this checkout's test window given foreground
  focus, its CTest rerun passed in 3.13 seconds. Every Release test has a passing
  result on the final implementation. The failed run is retained.
- Final full-run logs are under
  `out/build-logs/Release/20261110-215941-0fd17fc8/`. The isolated rerun is
  `out/issue-144/knob-reset-rerun.log`.

## Host measurements and owner listening

The repository installer installed the Release bundle into
`C:/Program Files/Common Files/VST3/ReverbArise.vst3`.
Its binary SHA-256 is
`D077B2B28B94CC2C0BA241FF3DB0FA637D28D2C5617C8CF5D1C85AF9CCCC449D`.
The previous installation is retained under
`out/vst3-install-backups/20261110-220855-d2eec2597c974367b370f35cc082be2b/`.
The installer performed a plugin-only Release build after the test run.

| Check | Sample rate | Measured sample peak | Result |
| --- | --- | --- | --- |
| Rack Dry + Wet Audition | 44.1 kHz | −1.0000 dBFS | Owner approved comparison with Cue |
| Rack ordinary live Loop | 44.1 kHz | −1.0000 dBFS over the full recording | Final two steady passes peaked at approximately −1.28 dBFS with unity output-guard gain |
| Rack louder-dry live Loop | 44.1 kHz | −1.0000 dBFS | Approximately +0.42 dBFS unguarded peak reduced by 1.42 dB |
| FL Studio Song Audition and live recording | 48 kHz | −1.0000 dBFS | Owner approved full lead-in, swell buildup and dry attack |
| FL Studio Pattern Loop Audition and Cue | 48 kHz | Separate WAV measurement was omitted | Owner approved full lead-in, swell buildup and dry attack |

Rack Free 2.6.6 recorded Host-FX output on the left and a live dry tap on the
right in 24-bit PCM. The ordinary and stress passes use the same accepted
154,511-frame Loop, prepared swells and 137 BPM Clock. The stress pass raises
the live VCA level from 42% to 68%. Neither the captured audio nor the wet
settings change. All three recordings contained zero samples above the ceiling,
allowing two PCM quantization steps for the meter comparison.

For the steady-pass gain estimate, the stored renders, saved calibration,
envelope and monitoring edges reconstruct the prepared wet. Aligning it with
the dry tap accounts for Host-FX's measured 256-frame latency. The estimated
ordinary steady-pass gain is unity and the stress reduction is approximately
1.42 dB. The wet-reference alignment RMS errors are 0.00025 and 0.00021
respectively. These are host measurement estimates, separate from the exact
native sample oracle. Startup peaks in the ordinary recording also engaged
the guard.

FL Studio 26.1.7.5654 recorded the Song comparison using an Edison instance
directly after ReverbArise on Master. The file contains stopped-transport
Audition followed by live Playlist playback. Its 32-bit float samples stay
within −1 dBFS. Edison was removed after saving the measurement. Both the
Pattern Loop and Song comparisons received explicit owner listening approval.
The final Rack listening comparison also received explicit owner approval.

Fixtures, measurement WAVs and the analysis helpers remain in
`out/issue-144/`. FL Studio has separate Song and Loop projects, and Rack has
ordinary and stress patches using the installed bundle. The active build and
fixtures are retained for review.

The ceiling applies to live sample peaks while wet is active. An inter-sample
peak may be higher, and already-hot dry-only input passes through unchanged.
Audition and host offline rendering have separate protection.

## PR #146 review F1

An offline callback can emit the existing Loop stop fade after disabling live
playback. The processor now passes its realtime protection flag through
`processLoopPlayback` to `LoopPlayback::process`. A realtime callback with
disabled playback still protects that fade. An offline callback emits the
established dry-plus-fading-wet samples.

The added `offlineFadeCases` regression failed before the fix. After the fix,
`preverb_loop_playback_test` passed all four combinations. Both stereo channels
are compared against an independent 240-frame fade oracle with 0 or 17 fade
frames already emitted. The checks include exact dry passthrough after the
fade, return to Standby and zero callback allocations/deallocations.

| Callback | Preceding fade frames | First left output |
| --- | --- | --- |
| Offline | 0 | 0.907469 |
| Offline | 17 | 0.906937 |
| Realtime | 0 | 0.891251 |
| Realtime | 17 | 0.891251 |

The first offline result matches the reviewer's base measurement. The realtime
results retain the −1 dBFS guard. Focused before/after logs are retained as
`out/issue-144/review-regression-before.log` and
`out/issue-144/review-regression-after.log`.
The earlier host approvals cover the realtime path, whose protection flag
remains enabled. The offline boundary received automated sample comparisons.

The review fix passed the complete Windows MSVC Release suite, 102/102 tests
in 417.66 seconds, using `scripts/build.ps1 -Configuration Release -Test`.
This was a warm rebuild. The two native foreground-sensitive fixtures received
foreground focus during the run and passed without a rerun. Full logs are
under `out/build-logs/Release/20261110-223646-48116ea9/`.
Strict OpenSpec validation, guide currency and diff checks also passed.

## Spec sync and archive

The owner approved PR #146 and authorized sync, archive and integration merge.
The four delta requirements were verified in the canonical
`transport-swell-playback` spec, with its existing purpose and all unrelated
requirement blocks preserved. All five tasks and all planning artifacts were
complete. The change is archived at
`openspec/changes/archive/2026-11-10-add-live-output-peak-guard/`.
Canonical spec validation passed 22/22 with existing long-text warnings.
Guide currency and diff checks passed. This closeout changes documentation
only, so the passing 102-test Release result above remains applicable.

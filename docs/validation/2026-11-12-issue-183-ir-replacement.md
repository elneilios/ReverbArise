# Issue #183 IR replacement validation

## FL Studio smoke check

On 12 November 2026, the owner reported the installed Release build at `219bde2` working in FL Studio 2026. The installer verified the binary against the Release artifact and retained the previous VST3 bundle.

A read-only inspection of `PreverbTesting_3.flp` showed the bar 3.1 swell ready with `06_Airy_Vowel.wav` selected and “Wet timing adjusted to fit this IR” in the Sound inspector. The target remained `3.1.000011111`. The editor displayed 2.61 s pre-roll and 2.98 s length. The prior inspection of the reported failure showed the same target with 3.48 s pre-roll and 3.84 s length.

The owner confirmed trying multiple IRs across multiple swells, swapping during manual Audition, and DAW playback. This completes the host playback smoke check for PR review. The exact three-file sequence, host save/reopen and host export remain unconfirmed. Native checks cover fitted-session reopen and export. The saved project has a hash-verified working copy alongside it. The agent left the original project open during validation and made no edits in FL Studio.

## Native verification

Tested the final source and fixture changes in `701670f` on Windows with Visual Studio 2026 MSVC. The first focused run found that a failed request could change the version of the still-accepted voice and restart its transition ramp. The final code keeps the accepted voice version until a replacement publishes. Some fixtures also needed to wait for the new assignment because the accepted sound stays ready while work is pending.

The 17 focused Debug CTest cases passed across the final batch and targeted reruns. Coverage included the swell model and session codec, bundled and managed IR choices, failed and superseded requests, live streaming, Wet Only and Dry + Wet handovers, fitted save/reopen, export, freeze, Loop, UI bindings and callback allocation checks. The standalone IR editor binding check also passed.

Commands used for the focused gate

- `./scripts/build.ps1 -Configuration Debug` to build the native suite. Focused batches used `ctest --preset windows-msvc-debug --no-tests=error -R <selection> --parallel 2 --output-log <log>`. The passing rerun and final selections together covered the cases below.
- `./scripts/build.ps1 -Configuration Debug -TestTarget preverb_swell_engine_test` and the same command for `preverb_ir_handover_test` to rebuild and rerun revised fixtures.
- `./out/build/windows-msvc/Debug/preverb_swell_ui_test.exe --test-ir-editor` for the dedicated IR editor bindings.

```text
preverb_{swell_session,swell_engine,swell_ui,ir_handover,ir_streaming,ir_choices,
audition_voice,audition_mix,song_export_engine,manual_preparation,source_edges,
shared_swell_engine,loop_playback,loop_perform_ui,loop_export_binding,
perform_audition,automatic_coordinator}_test
```

The installed host smoke build predates the accepted-voice version fix. The native tests exercise that fix on `701670f`.

The Release gate `./scripts/build.ps1 -Configuration Release -PrRegression` passed all 15 cases on `a43df75`. The first run exposed a Loop test fixture that forced a second render immediately after the IR had already been accepted. The fixture now waits for the accepted replacement and the playback publication. The Release rerun covered Rack and FL Studio processor fixtures, active Loop IR replacement, state, capture, transport, convolution, plugin identity and the UI standards check.

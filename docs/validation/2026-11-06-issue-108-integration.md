# Issue #108: approved Level integration and final native validation

2026-11-06. Change: `focus-swell-level-adjustment`, **8/8 Apply tasks complete**. Continues the [investigation](2026-11-06-issue-108-ir-level.md) and [native candidate checkpoint](2026-11-06-issue-108-control-checkpoint.md). The owner approved the dial's feel and -12/-24 dB comparison, then explicitly requested integration and remaining validation. They found -24 dB inaudible with the quiet chord but audible with the strong chord. This is monitoring/listening feedback, not digital silence or a universal audibility threshold. It supports keeping source dynamics and deliberate extended attenuation; no compensating gain or normalization was introduced.

## Delivered behavior

- Normal Perform construction uses the approved linear -12 to +12 dB main dial, 0 dB midpoint/default/reset, 1 dB coarse and 0.1 dB fine steps. The review-only enable switch is removed.
- Exact numeric and absolute accessibility entry retain finite [-60,+12], including non-step saved floats. An extended value is displayed accurately while the dial rests at its lower endpoint; relative edits stay anchored to the actual value. Existing session/selection/input/render/pass guards remain active.
- Only the selected swell changes. Native binding checks cover unchanged future IR defaults, the next swell's 0 dB creation, neighbours, another instance, frozen Level editing, notification-free refresh/focus and shape-only generation without raw reconvolution.
- Durable values, state schema, parameter exposure, calibration, IR/raw assets and host/session identities remain unchanged. Learn has no new Level default, and the shared IR selector/binding is unchanged. Production UI standards revision 11 records the scoped owner approval, with no new visual tokens or variants.
- New production-path fixtures verify -60/-30/-12/0/+12 dB against an independent positioned box-convolution/calibration/dB oracle in both channels. Equal admitted wet content agrees through actual manual audition, live Song processing, plugin float WAV and host offline processing, away from disclosed boundary ramps. Dry + Wet preserves incoming dry; calibration, captured source, IR and raw render identities remain unchanged, with no callback allocation/deallocation in the guarded live check.
- A real 32-member correlated overlap verifies disclosed audition/live wet protection. Plugin float WAV deliberately retains its existing unattenuated aggregate/overs policy; the fixture checks finite output and retained overs instead of imposing live protection on Export. Existing accepted-IR handover and superseded-worker coverage also passes.

## Native verification

Windows x64, Visual Studio 2026/MSVC, pinned JUCE 9.0.4. The tested integration source is in the commit containing this record; build identity reports `da243224307356841bdcfb2a4f742d88dcec7aff` plus dirty integration changes. The exact native binary hashes below identify the executables and plugin built for these checks.

| Check | Result / retained local log |
| --- | --- |
| Final supported Debug wrapper | Pass; `out/build-logs/Debug/20261106-184506-3aff2a83` |
| Final supported Release wrapper | Pass; `out/build-logs/Release/20261106-184612-34a4592f` |
| Broader affected Debug CTest run | 11/12 passed; one existing engine worker wait reported `Timeout at 103` during concurrent cold compilation; 349.83 s total |
| Final Debug CTest, compilation idle | **4/4 pass, 71.36 s**: IR handover, Level audio paths, full knob reset and engine. The unchanged engine test/deadline passed in 63.42 s |
| Final focused Release CTest | **12/12 pass, 25.50 s**: workflow processor, gain staging, IR handover, song export, Perform binding, focused Level, Level audio paths, full knob reset, shape, session, transport playback and full processor regression |
| UI standards | Pass: 64 files / 59 bounded occurrences; debt inventory unchanged |
| Strict OpenSpec / whitespace / guide currency | Pass |
| Native guide capture and static assembly | Pass: four actual 1400x1000 production screenshots plus approved logo; self-contained HTML validation; Perform capture visually inspected |

The initial Debug engine timeout did not recur with compilation idle; no production timing policy or test deadline was changed to obtain that pass. The existing popup regression's earlier foreground limitation was addressed in the **test fixture only**: a named native DocumentWindow hosts the same editor, with a bounded activation wait and all popup, reset, stale-action and lifetime assertions retained. Final Debug and Release CTests both pass that full suite without manual owner input.

The new audio fixture initially waited for the default Dry + Wet audition context beyond its short capture; it now explicitly selects the intended wet-only window first. It also initially dereferenced Export's current prepared object after leaving Export, producing a test-process access violation. Retaining the completed metrics before the mode transition corrected that fixture lifetime mistake. Final cross-path checks pass in both configurations; production DSP/export lifetime code was not changed.

Debug builds reused this worktree's existing objects/dependencies. The first Release object build was cold, reusing already populated dependency sources; final Debug/Release runs were incremental with `CMAKE_BUILD_PARALLEL_LEVEL=4`. No cache clearing or comparable speedup claim. Full Debug/Release CTest suites were not run; the focused matrices above are the actual coverage. Initial concurrent-build and candidate failures remain recorded rather than counted as passes.

Commands (tool-discovered CTest path abbreviated here as `$ctest`; no build wrapper was active for final CTests):

```powershell
$env:CMAKE_BUILD_PARALLEL_LEVEL='4'
./scripts/build.ps1 -Configuration Debug
./scripts/build.ps1 -Configuration Release
& $ctest --test-dir out/build/windows-msvc -C Debug -R '^preverb_(swell_session|swell_shape|gain_staging|swell_engine|song_export|audition_mix|transport_playback|perform_binding|perform_knob|focused_level|workflow_processor|swell_ui)_test$' --output-on-failure --timeout 240
& $ctest --test-dir out/build/windows-msvc -C Debug -R '^preverb_(swell_engine|ir_handover|level_audio_paths|perform_knob_reset)_test$' --output-on-failure --timeout 240
& $ctest --test-dir out/build/windows-msvc -C Release -R '^preverb_(swell_session|swell_shape|gain_staging|level_audio_paths|focused_level|perform_knob_reset|perform_binding|processor|ir_handover|song_export|transport_playback|workflow_processor)_test$' --output-on-failure --timeout 240
./scripts/check-ui-standards.ps1
openspec validate focus-swell-level-adjustment --strict
git diff --check
./scripts/check-user-guide.ps1
```

## Guide, identity and remaining coverage

The maintain-user-guide review updated only the relevant Perform Level prose and explicit normalized-hash currency record. It distinguishes main sweep from full exact range, states source-relative quietness and Level-only reset, and keeps Learn's default IR, Live wet and Export Dry/Wet distinct. The existing tagline, Learn capture/playback requirement, Perform/DAW rendering and optional Export workflow are preserved.

Used the existing `Invoke-ReleaseGuideCapture` / `Write-ReleaseGuide` functions for **working-tree QA only**, with identity `v0.1.0-issue108-working-tree-validation`. Output is `out/issue-108-guide/preview/User-Guide.html`; native captures/provenance are alongside it. This did not package or publish a release ZIP.

| Tested artifact | SHA-256 |
| --- | --- |
| Release processor test | `a1d18b46de5bf56abb9989a1d796663c4250db82c7b504d988ad537e19a4ff6d` |
| Release native control test | `4845900e1168143290165d1dfe26f1237f5cc77c675a32d5f20204bce74381ca` |
| Release VST3 binary | `af7314dda27bd154a1a8db31df063026385842f3a9d67a967663b85dc258fc22` |
| Assembled guide QA HTML | `4a8ed380295c039c14f9bf5d9cc888f1347ec57fc768df5c9fba7e520d41bee7` |

No new FL Studio installation/smoke or extra OS DPI coverage was performed. This control mapping introduces no host parameter, routing or timing-policy change requiring another host acceptance request; native simulated host processing is explicitly separate from FL Studio validation. Prior approvals remain scoped as recorded. Leave PR #110 open for owner review; canonical spec sync/archive and merging remain separate lifecycle actions.

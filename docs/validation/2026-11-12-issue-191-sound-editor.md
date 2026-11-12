# Shared Sound editor validation

The owner approved the revised native preview on 12 November 2026. The Windows Debug preview showed the shared control in Learn and Perform at 900×600 and expanded at 100% and 125%. The preview used fixture responses to exercise the picker, dropdown waveforms, contextual file row, invalid import, Freeze and delayed preparation. The revised picker names Metal Bloom when its controls appear and identifies Custom IR while waiting for a file.

Native preview approval covers the composition and scoped control variants. Component regression, production binding and FL Studio results are tracked separately.

## Integrated native preview

The Windows Debug fixture ran the actual Learn and Perform bindings at 900×600. It opened with a retained Custom IR in Perform, selected the bundled Monumental response, and waited for its accepted render and response peaks. The Learn default remained Preverb Hall. Entering 2.75 dB in Learn persisted the future default and preserved the assigned responses and Levels of both existing swells. The screenshot run used a disposable native capture fixture and did not open FL Studio.

## Focused native checks

`./scripts/build.ps1 -Configuration Debug -TestTarget preverb_swell_ui_test -TestFilter '^preverb_(sound_preset_ui|ir_editor_binding)_test$'` passed both checks in 8.56 s. The focused cases cover keyboard inspection and deliberate commit, wrapping, accepted checkmarks, long accessible names, pointer focus, wheel routing, stepped Echo Rate, the switch, exact entry, retained Custom IR with matching bundled PCM, stale chooser and Level ownership, failed import, Freeze, capture lock, supersession, cache limits, and the delayed overlay including static reduced-motion rendering.

The Debug Level, responsive editor, and responsive scale checks passed in 13.91 s, 48.64 s and 7.23 s respectively. The responsive fixtures include compact and expanded layouts and 100% and 125% presentation. The Perform knob Reset test passed in 9.58 s after FL Studio was closed. Its first run stopped at the native foreground-focus check. The native IR handover test passed in 32.91 s, including the shorter-response fit, active/Audition handover, failure retention, and the offline snapshot. The Learn generation UI test passed in 19.92 s and checked that inspecting Sound preserves accepted dry and wet PCM.

## Release PR regression

`./scripts/build.ps1 -Configuration Release -PrRegression` passed all 15 checks in 18.94 s CTest elapsed, including UI standards. The tested implementation was the #191 working tree based on `cdc6bfb`.

## FL Studio 2026 smoke

FL Studio 26.1.7.5654 used the Release VST3 installed at `C:\Program Files\Common Files\VST3\ReverbArise.vst3`. The build used the #191 working tree based on `cdc6bfb`. The installer verified SHA-256 `DF66CF883715C82DF7D636636A29CCE0880A100756F4383FEF9CD74EC300F36E` and retained the previous bundle under `out/vst3-install-backups/20261112-195650-0082a4df9a914c229f2a277fa3b237cb`.

- In the disposable FLP, Learn Next selected Monumental and the Level dial set 2.4 dB. A manually added Perform swell inherited both.
- In Perform, Previous selected Preverb Hall and wrapped to the contextual Custom IR picker. Import accepted the local WAV, preserved the 2.1 arrival and 2.4 dB Level, and fitted the wet lead-in to 0.20 s.
- After saving and reopening in a fresh FL Studio process, the selected swell showed the same Custom IR filename and 2.4 dB. Learn still showed Monumental at 2.4 dB. The saved state confirmed creative v12 and distinct bundled and Custom preset IDs.
- The FL Studio run used the compact editor at 100%. Expanded 100% and 125% coverage comes from the native review. Host 125% was outside this smoke.

## Reverse waveform display follow-up

The focused Debug Sound UI test passed in 3.83 s with an asymmetric response fixture that checks time direction in the closed picker and menu. The native fixture and bound Learn/Perform captures rendered successfully. The change is limited to painting the preview, so the earlier FL Studio smoke covers the unchanged audio path.

## PR #200 review follow-up

The focused Debug Sound UI and IR binding tests passed in 8.26 s. The chooser fixture exercises a successful Perform import that advances the input generation and checks focus return when the native fixture owns foreground focus. The cache fixture verifies a missing managed WAV is retried after the cooldown and produces peaks when the same file is restored in place.

The Release PR regression rerun (`./scripts/build.ps1 -Configuration Release -PrRegression`) passed all 15 checks in 10.99 s on `eb8f181` with documentation-only working-tree changes. The earlier FL Studio smoke remains the host evidence. OpenSpec validated the change in strict mode and all 24 canonical specs in standard mode. Strict canonical validation still flags pre-existing requirement-length warnings.

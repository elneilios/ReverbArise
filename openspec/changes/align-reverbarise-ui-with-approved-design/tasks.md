# Tasks

All boxes describe future implementation or acceptance. Proposal creation does not satisfy them. Use the gap IDs and traceability table in design.md; record evidence as each task is actually completed. Keep code/model dependencies in 4.3 and 6.3-6.5 focused and independently verified.

## 1. Native baseline and preservation fixtures

- [x] 1.1 Extend the native editor harness with truly fresh/no-capture, mapped-unselected and matched selected fixtures; capture baseline images and verify the empty fixture has no residual waveform/export status rather than relying on the old empty.png name.
- [x] 1.2 Record baseline host identity, parameter registration set, version-7 state values/managed asset identities and representative legacy states; verify current reopen/audio fixtures and document the exact build/harness invocation before UI edits.
- [x] 1.3 Create the UI alignment evidence/host checklist from design G01-G20 and the diagnostics inventory; verify every gap and each current diagnostic item has an assigned task, expected evidence and honest pending status.

## 2. Theme, controls and faithful brand assets

- [ ] 2.1 Centralize baseline colour/type/spacing/stroke/radius metrics and apply a native LookAndFeel to existing component types; verify uniform unoccupied #0F1418 pixels and themed fields/buttons/toggles without gradients, shadow or default raised panels.
- [ ] 2.2 Build reusable knob-plus-exact-entry and consistent simple-stroke icon controls with focus/hover/fine/reset semantics; verify valid/invalid/cancel editing and typed value/knob agreement in focused control tests, including a checked integer-delta adapter for timing.
- [x] 2.3 Integrate resources/logo/svg/reverbarise-wordmark-dark.svg as the native header Drawable without redrawing/stretching its integrated A; preserve alpha/viewBox/aspect ratio/clear space, render the tagline separately, and verify at supported scales against the actual supplied vector/prototypes. Test supplied Inter loading/styles with its OFL notice and document any Segoe UI fallback.
- [ ] 2.4 Document theme tokens as proposed baselines and typography/asset policy; verify all new colour/type roles reference the centralized metrics and any departure is recorded for review, with no unlicensed packaged font or whole-prototype UI asset.

## 3. Eight-region responsive composition

- [x] 3.1 Implement the six-band/eight-region layout with full-width globals, aligned three-column headings and main-editor-only footer/navigation; verify identical region rectangles across empty, mapped, selected and updating fixtures at 1100x850, 1400x900, 1400x1000 and 1600x1000.
- [x] 3.2 Apply thin single separators and remove major header/list/editor/inspector boxes; verify only the Overview waveform viewport has a major full rounded outline, while allowed control and selection outlines remain restrained.
- [x] 3.3 Implement independent list/inspector scrolling with a large centre editor and eleven paired controls, visible top activity/IR and discoverable Reset; verify text/units/hit bounds and all controls remain reachable without shrinking lanes or moving bands as state changes.
- [x] 3.4 Document default/minimum/maximum logical sizes, layout metrics and scale/screen-fit policy; generate native size snapshots and verify no arbitrary raster-size hardcoding or fallback to unreadably small text.

## 4. Global controls, list and default names

- [ ] 4.1 Bind Auto-marker plus both Marker and Regeneration sensitivity knobs/exact percentages in the global band; verify saved-instance independence, low/high semantics, master-paused/off states and selection changes without resetting any other preferences.
- [ ] 4.2 Restyle the virtualized list with count/manual add, enabled state, own IR, restrained selected outline and concise real status; verify selection by stable ID through background completions, row enabling, rename/delete, long labels and 500 rows.
- [x] 4.3 Replace collection-count default naming with shared checked arrival-bar naming for new manual/automatic targets; verify Swell 18 at 34 s under 120 BPM/4/4, nonzero PPQ origin/different meter, same-bar independent IDs, preserved custom/saved names and unchanged wet-placement naming.
- [x] 4.4 Update current workspace usage docs for both sensitivities, bar-based names and first-pass limitations; verify screenshots/readouts show actual engine state and unavailable timing is not fabricated.

## 5. Timeline, source range, handles and overview

- [ ] 5.1 Reuse one tested transform for ruler/source/wet/target and main-editor readouts; restyle neutral dry, aqua wet and dashed yellow target without forcing the real energy peak onto it. Verify the 120 BPM/1.00 s/+0.35 s fixture in samples and pixels, plus negative endpoints and lead-in.
- [ ] 5.2 Add matching full-height Source Start/End lines, grabbable handles and subtle flat shading between them; verify both drag directions and knob edits, arrival/source distinction, coverage-gap visibility and unchanged dry placement with only dependent invalidation.
- [x] 5.3 Restyle every actual envelope control as an outlined circle while preserving centre-line endpoints, shared hit geometry, true-time stacking, small genuine-coincidence offsets and whole-wet movement; verify zero/full/overlapping fades, tension/extreme drags and near/far zoom against accepted gesture fixtures.
- [ ] 5.4 Rework Overview title/ruler/sole waveform viewport, actual swell indicators and detail highlight/navigation; verify sparse audio/silence/gap/unavailable states, negative lead-in bounds, pan/zoom correspondence and no DAW seek.
- [ ] 5.5 Document source-range versus target/wet bounds, knob parity, display scaling and negative-time limits; verify native screenshots show real shaped/raw data and never draw waveform through unrecorded coverage.

## 6. Selected-swell bindings, IR assignment and reset

- [x] 6.1 Bind eleven grouped inspector knobs/typeable values to source start/end, arrival, pre-roll, end offset, fade lengths, level, two tensions and whole-wet placement; verify each canvas-to-field-to-knob roundtrip, checked placement inversion and no unrelated edits or unnecessary convolution.
- [ ] 6.2 Retain visible enabled, Freeze and Solo plus own IR; implement identity-pinned focus/gesture commit/cancel, coherent listener suppression and truthful unavailable/frozen states. Verify selection changes/deletion during edits, invalid/overflowing input, very large integer samples and close/reopen teardown.
- [ ] 6.3 Add the real bundled/current-IR dropdown and adjacent file loader, including a narrowly scoped explicit-ID background bundled/file assignment command reusing existing validation/assets; verify Preverb Hall despite a custom future default, two independent assignments, WAV/WavPack, freeze, invalid file, chooser cancellation and selection/deletion/supersession races.
- [x] 6.4 Add direct Reset wet shape using current creation defaults rather than Shape{}; verify 960-sample overlap/Smooth fade-out at 48 kHz, automatic start, zero fade-in/tensions/gain/placement and preservation of source/IR/name/activity/freeze/other swells, including pending support and compatible-rate gating.
- [x] 6.5 Add the adjacent explicit Reset all swell settings transaction at the current target; verify creation-policy source/current-default IR/wet/activity restoration, unchanged UUID/name/arrival/global state/other swells, no capture deletion and no partial mutation when capture/default/timing validation fails.
- [x] 6.6 Document exact units/domains, independent curve presets under Settings, both reset scopes versus Full response and default-IR versus per-swell assignment; verify every movable canvas edit has its inspector control and the reset split action has clear labels/tooltips.

## 7. Continuous audition and host information

- [ ] 7.1 Replace redundant monitoring mode buttons with one Dry-to-Wet control, exact entry and Preview only label; verify 0/50/65/100% usable-travel geometry, corresponding sample gains, true Wet Only at 100% without required dry and no changes to live routing/level or raw render identity.
- [ ] 7.2 Preserve effective/latent legacy monitoring values on editor opening and state restore; verify old Wet Only+50% displays 100%, mixed 65% displays 65%, explicit edits use the intended mode and neither restore nor options starts playback or regenerates convolution.
- [ ] 7.3 Preserve preview cursor/dry span and scope/availability feedback, visible/resettable nonzero trim and Settings isolation/tail controls; verify dry isolation, overlapping wet, missing context, short ramps, Stop/completion/replacement and enabled Song priority with independent live settings.
- [ ] 7.4 Add the informational host footer and concise processing/nondefault/warning display; verify missing host fields, stopped/seek/loop values and export warnings remain truthful, and document audition versus live/DAW transport with no fake host buttons.

## 8. Settings and purposeful dedicated diagnostics

- [ ] 8.1 Implement scoped header Settings access for existing playback, processing/default IR, audition, selected-swell presets/mute/full-response and recovery; verify all preserved musician actions are reachable, both sensitivities remain global, source/target/placement knobs and Freeze/Solo remain in the inspector, and important nondefault states remain visible.
- [ ] 8.2 Replace inline diagnostics with one editor-owned dedicated window accessible through Settings in normal Debug/Release builds; verify open/refresh/hide/close/reopen and editor destruction leave region geometry, session state and audio/capture/scheduling untouched, with current data on reopen.
- [ ] 8.3 Consolidate read-only diagnostic groups and duplicate sensitivity/status per the disposition table; verify <=5 Hz visible diagnostic refresh, no hidden periodic diagnostic string/polling work and correct listener/SafePointer teardown during active playback.
- [ ] 8.4 Retain explicit generation/analysis/candidate/recapture/Clear/asset/suppression/test-tone recovery actions with honest availability and scope; verify no action runs on window lifecycle, scheduled tone has a clear active indication/Off path and required silent-output/seek/loop/tone-state regressions still pass.
- [ ] 8.5 Retire the inline viewport/toggle allocation, unused fieldBounds, hidden duplicate curve labels and obsolete exclusive callbacks after reference checks; update affected UI tests and verify no dead side-effect timer or writable hidden-sensitivity source remains. Preserve shared scheduling/preview/state code and recorded legacy fields.
- [ ] 8.6 Update diagnostics/usage guides and the item disposition record to match actual retained/removed/moved functions; verify the documented access path and each explicit command, and correct deferred-transport UI wording without rewriting historical validation results.

## 9. Integrated native evidence and regressions

- [ ] 9.1 Run the full configured Debug build/CTest suite, including callback allocation/lock-free-path checks, convolution/capture/shape/state/automatic/freeze/loop/offline export regressions; record command/results and exact replacement VST3 bundle identity, with focused failures resolved before broad repetition.
- [ ] 9.2 Generate matched native fresh-empty, mapped-unselected and selected screenshots at minimum/default/reference sizes plus missing/stale/error/support/long-list states; compare G01-G20 against prototypes/written rules and iterate until native evidence satisfies composition, geometry, tokens, text fit and real binding assertions.
- [ ] 9.3 Extend software native captures/geometry tests to 1/1.25/1.5/2 render scales and inspect control fit, circle/rule weight, selected range and percentage travel; clearly label this as simulated rendering rather than physical FL Studio DPI acceptance.
- [ ] 9.4 Verify legacy/current saved-project identity and state fixtures after all replacements, including latent audition state, source/IR/render IDs, tone settings, freeze/activity/placement and a two-instance export regression; record unchanged parameter/host identity and lack of asset migration.
- [ ] 9.5 Run strict validation for this change and all specs, review full MODIFIED scenario preservation and traceability, and publish the consolidated native evidence/remaining host checklist; do not mark native header/host/DPI acceptance complete from compile or image snapshots.

## 10. Explicit user and FL Studio acceptance

- [ ] 10.1 Obtain visual acceptance of the integrated supplied wordmark/header and native empty/loaded composition against the written rules/current prototype pair; record any explicitly approved deviations instead of silently relaxing tokens/layout.
- [ ] 10.2 In FL Studio, verify stable resize/layout, both sensitivities, independent IRs, Freeze/Solo, all canvas/inspector knob/exact-entry edits, shaded source boundaries, reset scopes, naming, overview and Settings/diagnostics lifecycle; record actual results and leave failures open.
- [ ] 10.3 In FL Studio, verify audition mix/playhead/isolation/tail, live on/off/routing/level, seeks/loops/final swells, silent-input output, two-instance offline export and save/reopen after UI edits; record host/version/audio settings and any limitations without inferring pass from prior milestone results.
- [ ] 10.4 Record physical host usability at 100%, 125%, 150% and 200% Windows scale for minimum/default layouts where the monitor work area permits, including reopen/focus/file chooser/diagnostics. Leave unavailable scales pending or explicitly revise support policy with user review before claiming completion.

## Workflow follow-up

- User-required visual gates: pause for explicit approval after the theme/header/layout stage, after detailed editing/control composition, and after full audition/overview/diagnostics composition. Record screenshots and actual approvals in docs/ui-alignment-evidence.md; do not proceed across a gate based on silence.
- Implementation begins only after a new apply request; this proposal authorizes no source changes.
- Update task boxes as each implementation/verification item actually completes. Preserve explicit host/native-appearance pending work across usage resets.
- Sync/archive only after all required implementation, visual and host acceptance results are recorded. Do not treat a deferred or unavailable check as passed.

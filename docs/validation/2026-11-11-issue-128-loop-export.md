# Issue #128 Loop WAV export validation

## Scope and approval

Branch: `codex/128-loop-wav-export`, based on `feature/loop-mode`.
Implementation checkpoint `37e848a`; integration update `95a30b0` incorporates
the guide timeline changes from PR #147. Final validation also includes the
retained-waveform repeat-label regression and living guidance updates.

The owner approved the revised native Export preview on 2026-11-10. See
[the native preview record](2026-11-10-issue-128-export-preview.md). This uses
the existing Mix Character selector and independent Dry/Wet faders, with no
new shared control family, token or visual exception.

Loop file composition uses accepted capture-local audio and timing. Live
Clock/Phrase/Cue admission is independent and is not required to prepare, Save
or Drag a Loop WAV. Preview requires a compatible output-device rate. Song
preferences, file identity behavior and delivery remain separate.

## Automated coverage

Focused native Release checks passed during implementation: Loop plan/sample
oracles in `preverb_song_export_test`, worker/preview/file/persistence cases in
`preverb_song_export_engine_test`, actual processor/editor bindings in
`preverb_loop_export_binding_test`, and the native Loop Export component and
existing Export Levels fixtures. Existing Loop playback regression passed.
These are native/component results, not manual DAW acceptance.

The independent sample oracle covers finite wet overlap and repeated dry,
long negative lead-ins and final tails, no unscheduled next pass, calibrated
gain and level pairs 0/100, 100/0, 100/100 and 35/65. Boundary tests cover
44.1/48 kHz, nonintegral tempo, floor/ceil capture extents, three/four beats
and 64 repeats. Engine cases cover decoded preview/file comparisons, callback
allocation guards, stale completion, restore, independent Song values, silent
0/0, continuous level changes and immutable delivery history. Publication
failures include disk-space preflight, staging/rename failures and preserved
existing destination bytes. The final suite additionally checks that retained
old waveforms keep their accepted repeat labels while a replacement prepares.

`scripts/check-ui-standards.ps1`: passed, 75 files and 59 bounded occurrences;
the debt inventory was not regenerated. Strict validation of
`add-loop-wav-export`, read-only user-guide currency and `git diff --check`
passed.

The full Windows MSVC run of `scripts/build.ps1 -Configuration Release -Test`
(`20261111-002421-67f40df8`, warm tree, 602.01 seconds of CTest) built
successfully. Its first CTest pass was 100/104. It exposed an incomplete setup in
the new disk-space fault test: `TestDirectory` supplies a unique path but does
not create it. The test now creates that directory before placing the protected
destination file; production behavior was not changed for this failure. The
same run reported unavailable native foreground focus in the existing knob
reset and readability fixtures, plus a Loop Perform finishing-state assertion.

- `final-fixture-rerun.log`: `preverb_song_export_test` passed with the corrected
  Save destination setup. The Perform check still failed, making the two
  problems independent.
- `manual-focus-rerun.log`: both foreground tests passed in the interactive
  rerun, with no automation of desktop input.
- `perform-continuity-rerun.log`: both Loop Perform and the native Export
  binding passed. The earlier Perform failure reproduced with exactly one Stop
  command and a ready-to-lost sync transition after a software paint pause.
  The serial fixture now resumes simulated continuous blocks after that paint;
  production host-gap detection and all playback assertions are unchanged.
  The passing run observed the slow-paint path and also exercised real MIDI
  reference loss/re-lock within the fixture.

All 104 checks are covered by the full run and focused reruns. This is not a
single uninterrupted green CTest run. The two corrections affect fixture setup
and simulated callback timing only; the initial full run covers unchanged
production behavior. Full logs are in `out/build-logs/Release/20261111-002421-67f40df8/`,
and focused logs are in `out/issue-128/`.

## Manual host handoff

The owner requested manual approval of desktop/host checks. No automatic
desktop interaction is used for this handoff.

Prepared copies under `out/issue-128/manual/`:

- `Rack - Loop WAV Export.vcv`, from the approved #144 Ordinary listening
  patch, using the system VST3. Its accepted two-bar take is 44.1 kHz at
  136.999574696277648 BPM. The referenced capture and IR assets exist and
  their SHA256 checksums match the saved manifest. Original identities and
  durable assets are preserved. Clocked is stopped.
- `FL Studio - Loop WAV Export.flp`, copied from the approved #144 Loop
  listening project, preserving its existing source and swell state.
- `Rack - Original two-bar dry.wav`, a checksum-matching copy of the captured
  segment for Playlist alignment and wet-layer listening.
- `README.md`, the consolidated manual check: two repeats, Wet Layer and
  Dry + Wet; compare Preview, Drag and Save with FL Playlist placement,
  initial lead-in, inter-pass spacing and final tail. Mute original dry when
  listening to a dry-inclusive imported file.

Host installation and manual results are pending. Fixture preparation and
asset checksum checks do not establish host acceptance. Physical monitor/DPI
qualification remains #130; broader accepted-Loop recovery remains #129.

## Output limits

Export preserves transparent Float32 sums and reports measured peaks; it does
not bake the live output guard into files. Dry at 100% is unity. The initial
lead-in and final wet tail do not repeat dry, and a dry-only composition starts
at Loop Bar 1. Monitoring ramps are separate from the delivered WAV samples.

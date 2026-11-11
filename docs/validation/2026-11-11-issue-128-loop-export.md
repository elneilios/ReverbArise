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

Release revision `5482f0b` is installed at
`C:\Program Files\Common Files\VST3\ReverbArise.vst3`. Installed binary SHA256:
`A27E3DED826B23158ABBD91AC658C92976B7B17304D5A662105CD94AF2B01519`.
The normal installer build passed but the system install lock required Windows
elevation. The elevated repository installer passed and retained the prior
bundle at `out/vst3-install-backups/20261111-005430-44c5a8ec0c1f4734a6a00c17231df503/ReverbArise.vst3`.
Its plugin-only build did not rerun CTest; see the full and focused runs above.

Manual host results are pending. Fixture preparation, asset checksum checks
and installation do not establish host acceptance. Physical monitor/DPI
qualification remains #130; broader accepted-Loop recovery remains #129.

## Output limits

Export preserves transparent Float32 sums and reports measured peaks; it does
not bake the live output guard into files. Dry at 100% is unity. The initial
lead-in and final wet tail do not repeat dry, and a dry-only composition starts
at Loop Bar 1. Monitoring ramps are separate from the delivered WAV samples.

## Owner host feedback on Wet Layer

The owner reported that the 100% wet file from the FL Studio kick Pattern
sounded louder than Perform and distorted in the Playlist. The first analysis
used the separate Rack fixture. Its two-pass WAV and captured dry also overload
when layered at unity. The
three-pass Wet Layer and Dry 100% / Wet 50% files independently confirmed
the captured dry offset of 154,511 frames, with reconstruction error below
`1e-6` per sample.

| Aligned fixture signal | Sample peak |
| --- | --- |
| Captured dry | −6.696 dBFS |
| Two-pass 100% wet file | −1.278 dBFS |
| Two-pass 100% wet plus unity dry | +0.535 dBFS |
| Two-pass 75% wet plus unity dry | −1.155 dBFS |

Two channel samples in the unity sum exceed full scale. The largest uniform
wet gain meeting the −1 dBFS ceiling for this mix is 77.10%. Perform applies
wet overlap protection and a guard on the combined output. The current Export
gain rule measures the wet file alone in Wet Layer mode. Host acceptance
remains pending.

The owner's FL Studio screenshot identifies the exact two-pass export ending
`62bb168.wav` and displays its +3.99 dBFS warning. The matching 48 kHz capture
manifest describes one bar at 120 BPM. Its kick audio checksum verifies.
The three-bar export contains a one-bar lead-in and two scheduled passes.

| FL Studio kick fixture signal | Sample peak |
| --- | --- |
| Captured kick | −4.611 dBFS |
| Two-pass 100% wet file | +3.989 dBFS |
| Two-pass 100% wet plus aligned kick | +3.989 dBFS |
| Two-pass 75% wet plus aligned kick | +1.813 dBFS |
| Two-pass 35% wet plus aligned kick | −1.158 dBFS |

The wet file alone contains 3,880 channel samples above unity. The largest
uniform wet gain meeting −1 dBFS with the aligned captured kick is 36.805%,
including when the same kick Pattern repeats through the lead-in bar. This
measurement uses decoded WAV samples at unity, independent of Playlist routing.

The owner specified that 100% Wet should follow the wet level heard in
Perform, with the original kick level controlled in the host. The revised Loop
worker uses the same accepted wet shaping, Live Wet and overlap protection as
Perform. The fader scales that reference. The live guard applied after summing
dry and wet is outside the stem. The Loop composition identity includes the
effective gain so existing unprotected files cannot be reused for this mix.

Focused Release checks passed for the Loop playback compositor, the independent
export sample tests, worker/WAV and Preview parity with a non-unity Live Wet
setting, and native Loop Export UI and processor binding. The Windows logs are
`out/issue-128/perform-wet-test.log` and
`out/issue-128/perform-wet-and-tile-test.log`.

The native processor binding test also recorded an audible wet lead-in block
from actual Loop Perform at Live Wet −6 dB, stopped the clock, and exported at
the default Wet Layer settings. The corresponding decoded stereo WAV block
matched within `1e-6` per sample. See `out/issue-128/live-to-wav-test.log`.

The placement text now sits inside the existing drag tile below its title,
followed by the file details. Wet Layer and Dry + Wet were rendered at 900x600
and 1400x900. A single native review window was opened and the owner approved
the tile arrangement.
Screenshots are under `out/issue-128/preview-in-tile/`.

The Windows Release build of `3ba8364` passed. The full CTest run passed
103/104 in 378.58 seconds. The existing Perform knob reset fixture could
not obtain Windows foreground focus (`peer=1 foreground=0`). It passed in
3.07 seconds on the focused rerun of the same build, covering all 104 tests
across both runs. Rerun log: `out/issue-128/perform-wet-focus-rerun.log`. Full log:
`out/build-logs/Release/20261111-012742-13c42605/test.log`.

The repository installer installed Release `3ba8364` in the system VST3
folder for the manual listening check. Installed SHA256:
`A6E017FF5C8BB1E64E797A8E3C7047C3809F425AA7885BEF0A7B58FA338AAE3D`.
Rollback bundle:
`out/vst3-install-backups/20261111-013659-c26e021807994af6b6cc742266411a85/ReverbArise.vst3`.
The plugin-only install did not rerun CTest. The owner confirmed that the
fresh FL Studio kick Wet Layer export matches the wet level in Perform.
Decoded one- through four-pass Wet Layer files from that project each peak
at −1.00 dBFS, with zero samples above full scale. Each has the 96,000-frame
lead-in and the requested number of 96,000-frame passes. The owner separately
confirmed the Clock-only Rack export and FL Studio import check passed with
Drag and Save, including alignment, repeats, lead-in and tail for both mixes.
This owner check predates integration of the shared guarded file gestures.

The issue branch then integrated `feature/loop-mode` at `2aaa1cd`
(PR #150). The Loop Export workspace uses the shared guarded Save and Drag
binding and the shared waveform gestures. Its drag tile retains the approved
placement instruction and uses that instruction after a successful Loop
drop. The post-merge focused Release run passed all six Loop playback,
Perform, Export UI/binding, composition and worker tests
(`out/issue-128/integration-test.log`).

The integrated revision `8b7d419` built successfully with
`scripts/build.ps1 -Configuration Release -Test`. CTest passed 104 of 105 in
356.48 seconds. The Perform knob reset fixture timed out acquiring Windows
foreground focus and passed on its isolated rerun in 11.17 seconds. This
covers all 105 tests across the two runs. Logs:
`out/build-logs/Release/20261111-014616-8b696c9e/test.log` and
`out/issue-128/integration-focus-rerun.log`.

## PR #153 review follow-up

- F1. Export reads timing from the known accepted Loop manifest when dry
  assets are unavailable. A restored processor with a verified frozen swell
  can export Wet Layer, and including Dry reports unavailable coverage.
  The same fixture confirms Perform still requires its normal source.
- F2. At 48 kHz and 137 BPM, the lead-in boundary at −84,088 samples stays
  in place. The regression also checks the samples immediately before and
  after that boundary.
- F3. Wet Layer preview leaves the dry reader disabled. Raising Dry prepares
  it and resumes from the current position. The worker test checks that dry
  is audible after the edit and preview advances.

Focused Release tests for workflow, export composition and worker, Loop
Export UI and bindings, and the shared surface lifecycle passed across the
focused run and corrected recovery fixture rerun. The fixture uses a heap
allocation for the second processor to avoid overflowing the test stack.

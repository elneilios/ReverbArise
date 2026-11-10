# FL Studio Loop host probe

[Issue #127](https://github.com/elneilios/ReverbArise/issues/127) qualifies capture timing, live phase, and external commands independently. The fixture targets FL Studio 2026 on Windows with a Release VST3.

## Try the prepared project

Open `out/issue-127/fixture/ReverbArise Loop Ready to Cue.flp`. It already has a two-bar Loop and four prepared swells. To try the button, mute the MIDI pattern, start Song playback at bar 1 and click **Cue Loop** in **Perform**. For local preview with the transport stopped, press **Play** in the **Audition** row with **Dry + wet** selected.

The saved project retains the original host trace pattern. For the shared MIDI
map, import **Loop shared MIDI playback.mid** from `make-inputs.py` into the
MIDI Out Piano Roll and place the pattern at bar 1 in a copy of the project.
Mute the old pattern and start at bar 16. With up to two seconds of lead-in,
Phrase and Cue at 20.2 launch the two-bar Loop at 21.2. Stop at 22.3 finishes
the pass at 23.2. Cue at 24.2 launches the next pass at 25.2, and Stop Now
at 26.1 fades immediately. The Loop footer should show Bar 1, Beat 1 at each
launch, even though the Playlist is on beat 2.

To retake exactly bars 18 and 19, switch to **Learn**, choose **End on Signal**, turn on **Recapture audio**, and leave **Arm** off. Start Song playback at bar 16. The MIDI pattern starts recording at bar 18 and requests the end during bar 19. Keep FL Studio playing until **Loop accepted** appears at bar 20. Click **Generate Swells**, then go to **Perform**. For a manual take, mute the command pattern before pressing **Arm**. For **Duration**, start playback on a bar line to record from there. If playback starts mid-bar, capture waits for the next bar. For manual **End on Signal**, turn Arm off during the last bar and keep the transport running to complete it.

To record one repeat, open the Pattern 2 test copy `out/issue-127/fixture/ReverbArise Loop Ready to Cue_2.flp`. Select **PAT** and **Pattern 2**, then choose **End on repeat** in **Learn**. With playback stopped at the beginning, click **Arm** and start playback. Keep the pattern playing until **Loop accepted** appears after the first wrap. At 120 BPM and 48 kHz the one-bar take is 96,000 frames.

## Test inputs

From the repository root, run:

```powershell
python -B docs/validation/issue-127-fl-studio-fixture/make-inputs.py out/issue-127/fixture
```

Use a disposable 120 BPM, 4/4 Playlist. Place **Loop probe dry 120 BPM.wav** at bar 1. It contains a bright transient on beats 1 and 3, with a stronger downbeat on even-numbered bars. Use bar 18 as the test origin for a two-bar phrase. The intended downbeats are one bar offset from a two-bar grid anchored at bar 1.

Import **Loop probe Cue and Stop.mid** into the MIDI Out Piano Roll. The file contains positive note-on and release messages on MIDI channel 1. Its note numbers are authored inputs. The trace must establish what the effect receives.

| Playlist position | Candidate action | MIDI file note |
| --- | --- | --- |
| Bar 18, beat 4 | Cue for the next phrase | 62 |
| Bar 21, beat 1 | Stop | 60 |
| Bar 24, beat 4 | Cue again | 62 |
| Bar 28, beat 1 | Stop | 60 |

The imported pattern creates a MIDI Out channel on port 0, channel 1. Use the same port on the VST3 wrapper input when available. Record the wrapper port, actual incoming notes and offsets, and behavior after a seek or offline render. The port setup follows the host documentation and requires verification for this effect. [Image-Line MIDI Out manual](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/plugins/MIDI%20Out.htm)

## Initial setup evidence

- 2026-11-10, branch `codex/127-fl-studio-loop`, integration revision `7f770c2`. The installed FL Studio executable reports version `26.1.7.5654`.
- The Windows MSVC Release VST3 build passed through the test installer. The installer stopped at the system VST3 directory lock with an ACL denial. Windows reported the elevated retry as canceled before the installer ran. The prior system bundle remained in place. Build logs are under `out/build-logs/Release/20261110-091705-cea16890/`. CTest was outside this setup run.
- Regenerating the WAV and MIDI inputs produced byte-identical files. The disposable project was saved and FL Studio was closed. The project is at `out/issue-127/fixture/ReverbArise Loop Host Probe.flp`, with the WAV on Playlist Track 1 and the command pattern on Track 2, both at bar 1.
- The Audio settings panel showed FL Studio ASIO, 48 kHz, a 512-sample device buffer, Smart disable enabled, Play truncated notes on transport enabled, and Reset plugins on transport disabled. Actual plug-in callback sizes, live phase and VST3 MIDI delivery still require measurement.

## Installed test build

- The retry through Windows PowerShell completed the Release VST3 build and install. The saved fixture now has ReverbArise on Master slot 1. FL Studio loaded the x64 binary from the system VST3 directory with SHA-256 `72ADEE305E10CD0EC85BC198EF4AEE15642824DFB0085370D5467D6518370062`, matching the installer result.
- FL Studio wrapper `2.7.0.5654` reports VST 3.8.0, 64-bit native, `Fx|Stereo`. Input port is set to 0 on the MIDI Input event bus, matching MIDI Out channel 1, port 0. Send note release velocity and Send All Notes Off when playback stops are enabled. These are routing settings. Actual MIDI delivery still requires a callback trace.
- Installer logs are under `out/build-logs/Release/20261110-113657-4c332dea/`. The prior bundle is retained at `out/vst3-install-backups/20261110-113706-8b058afa01ed437a84c8794b6fc5cabc/ReverbArise.vst3`. CTest was outside the install run.

## Callback tracing

Set `PREVERB_FL_STUDIO_TRACE=1` in the test installer's PowerShell process to build the local diagnostic variant. Ordinary builds omit the recorder. Tagged and candidate configurations reject it. Create an empty `ENABLE` file in `out/build/windows-msvc/fl-studio-trace/` before loading the disposable fixture. Each enabled instance writes its own CSV there. The audio callback enqueues bounded observations. A worker writes the file, and the final line reports any lost rows. The trace contains timing, onset offsets, and MIDI bytes.

Play the fixture through bar 29, seek forward past a Cue, restart, and export the song to a disposable WAV using FL Studio's render dialog. Close the traced plug-in instance to finalize the CSV, then run:

```powershell
python -B docs/validation/issue-127-fl-studio-fixture/summarize-trace.py <path-to-trace.csv>
```

Remove the enable file before opening other projects. Reinstall with the trace build variable unset to return to an ordinary build. Keep the prior installer backup until the ordinary build is verified.

## Measured callback and routing results

The complete trace and its summary are retained under `out/issue-127/`. The first offline pass overflowed the diagnostic queue. The repeated live and offline runs with the larger queue recorded zero dropped rows and zero MIDI overflows. The traced Release binary hash was `2EA8299C971DB49F4EFBF6FDE7A136D72AE5EDC065FDC32C383BC57B13ACAF4D`.

- At 48 kHz and 120 BPM, 22,277 playing realtime callbacks had 2 to 250 frames. The full-song bounce had 11,136 playing callbacks of 250 frames, marked non-realtime. All playing callbacks carried sample position, PPQ, tempo and 4/4 meter. Sample position and PPQ agreed within one sample, and all 58 test transient onsets matched their intended beat sample in both paths.
- The timeline wrapped from sample 2,783,750 to 0 after the final 250-frame callback. A playing seek to bar 19 jumped to sample 1,728,000. The first Cue's note-on at 1,704,000 was absent after the seek. Prepare, reset and release counters also changed over the transport and bounce lifecycle.
- On port 0, channel 1, the VST3 received note-on 62 at samples 1,704,000 and 2,280,000, and note-on 60 at 1,920,000 and 2,592,000. Each event arrived at callback offset 0 with velocity 100. The releases arrived 3,000 samples later. The live pass and the bounce delivered the same four pairs.
- A second coincident copy of the MIDI pattern still delivered one positive and one release event at each of the two tested positions. No identical same-callback events appeared. The duplicate pattern was removed after the probe.
- Both probe renders were stereo 48 kHz with 2,784,250 frames and identical PCM. The fixture used the default Learn state. The integration pass below tested offline output with prepared Loop swells. Position-based automation was not qualified.

The second project, `out/issue-127/fixture/ReverbArise Loop Phrase and Commands.flp`, replaces the note pattern with **Loop probe phrase and capture roles.mid** using the same MIDI Out channel. The corrected pass in `out/issue-127/phrase-and-command-callbacks.csv` starts near block 47,000. An earlier pass in that file used an incorrectly ordered MIDI fixture. The generator now sorts events by timestamp.

| Role | MIDI number | Measured Playlist positions |
| --- | --- | --- |
| Live Phrase | 12 | Bars 18, 20, 22, 24, 26 and 28 |
| Learn Start/Arm | 24 | Bar 18 beat 1, in the same callback as Phrase |
| Learn Stop | 25 | Bar 19 beat 3 |
| Perform Cue | 62 | Bars 22 and 26, beat 3 |
| Perform Stop | 60 | Bars 25 and 29, beat 1 |

The native playing sample/PPQ grid is the candidate for capture without a MIDI clock. The six explicit Phrase notes were exactly 192,000 samples apart and establish a separate candidate for live phase. FL Studio's Browse parameters showed bypass and generic MIDI CC controls. ReverbArise exposes no dedicated Loop automation parameters in this build. The verified note route supplies external commands. UI behavior with the command route absent is covered by the subsequent implementation and integration tests.

## Integration checks in FL Studio

The ordinary Release build was installed with SHA-256 `29D5C2F1FB574DC82E20EA881578059A7C9D12158E26CC4339CC6A1FD3653A4E`. The FL Studio process loaded that exact binary from the system VST3 folder. The integration build logs are under `out/build-logs/Release/20261110-120713-1a37ce01/`.

- With the MIDI track muted, Arm was switched on with transport stopped at bar 16. Playback entered Recording Loop and accepted the two-bar Duration take before transport stopped. The saved manifest confirmed 120 BPM, 4/4, eight beats, and 192,000 frames. Its four audio ranges start at 24,000, 72,000, 120,000 and 168,000 in the recorded loop, consistent with starting on the next beat.
- With End signal selected, Recapture audio on and Arm off, playback from bar 16 entered Recording Loop on the Start/Arm note at bar 18. The Stop note at bar 19 beat 3 changed the status to **Finishing at the end of the bar**. The take was accepted at bar 20 while transport kept running. The manifest advanced from revision 3 to revision 4 with eight beats and 192,000 frames. The first transient appeared at the new waveform's left edge, consistent with starting on the sequenced downbeat.
- A repeat reached **Finishing at the end of the bar**. Stopping FL Studio before bar 20 showed **Recording cancelled. Keep DAW playback running through the end of the loop.** The same accepted waveform remained visible and revision 4 stayed the latest manifest.
- Four swells rendered from that take. Perform showed **Connect phrase reference** and disabled Cue while the MIDI track was muted. After the track was restored, the Cue at bar 22 beat 3 queued the Loop. Perform showed **Playing loop** at bars 24 and 28 and returned to **Ready** after the Stop notes at bars 25 and 29.
- The full-song offline bounce from Perform contained 2,784,250 stereo frames at 48 kHz. Its PCM SHA-256 `b60871a283727cfcbc7ac4941a2004d31cd03def416c7950da3b9c1523b4ab43` matched the dry baseline. This verifies the dry-only offline behavior for the tested setup.
- The initial FL Studio pass also captured and accepted Song Mode audio before Loop Mode was enabled. The saved Song source was revision 2 and had no Loop timing metadata. The native Loop playback regression compares every rendered sample against dry input plus the expected wet output across Cue, Stop and phase changes.
- The prepared `out/issue-127/fixture/ReverbArise Loop Ready to Cue.flp` project uses the accepted source and renders in the current profile's asset store. Open Perform and start playback before bar 18 to hear both cycles. Start from the capture fixture to recreate the source in another profile.
- The focused native Release fixture passed the two-bar Duration, verified End Signal, interrupted take retention, separate Learn and Perform notes, local Cue, and seek re-lock checks.

## Release verification

On 2026-11-10, `scripts/build.ps1 -Configuration Release -Test` passed configure and build. The CTest pass recorded 100 of 102 passing. `preverb_perform_knob_reset_test` and `preverb_perform_readability_test` timed out waiting for native Windows focus. Both passed on `ctest --preset windows-msvc-release --rerun-failed` with each preview activated. The original and retry logs are `out/build-logs/Release/20261110-122114-8fad79e0/test.log` and `out/issue-127/release-focus-rerun.log`. UI standards, offline guide currency, strict OpenSpec validation, and deterministic fixture regeneration also passed. Rack and other DAWs were not requalified in this FL Studio pass.

## Playlist alignment follow-up

On the same day, the updated Release VST3 was built and installed with SHA-256 `7AF1C326D74E28DA6914DAD444C5957EC3F8736F328B73C5E8AEB9178ED0C3A2`. The installed binary and local build matched. Windows test install retained the previous bundle under `out/vst3-install-backups/20261110-124427-52def3d63d00495ab237dbe3d516fefe/`.

- In `ReverbArise Playlist Alignment Proof.flp`, the armed two-bar Duration take started as FL Studio played from bar 1 and accepted at bar 3. From bar 1 beat 3, Learn showed **Waiting for the next bar** until bar 2 and accepted at bar 4. Both manifests contained 192,000 frames and began with audio at sample zero. The fixture's louder downbeat appears first for the bar 2 take and third for the bar 1 take.
- With Arm off, the MIDI Start/Arm note started End Signal at bar 18. The Learn Stop note in bar 19 changed the status to **Finishing at the end of the bar**. The plug-in accepted revision 6 at bar 20 with 192,000 frames. The Playlist MIDI pattern is the same as the earlier trace.
- With the same Release binary in headless VCV Rack Free 2.6.6 and Host-FX 2.1.0, the existing 4-, 8-, and 16-step End Signal patches each produced a fresh revision 1 at 48 kHz. Their respective lengths were 84,088, 168,175, and 336,350 frames. Accepted manifests and process logs are under `out/issue-127/rack-regression/`.
- The Release workflow processor test passed both its focused FL Studio run and its complete run, including the existing Rack capture, playback, and phase recovery cases. The FL Studio fixture rejects a take when the callback immediately after provisional start jumps to a different position. It also exercises a MIDI Start/Arm note delivered on the first playing block.
- The full `ctest --preset windows-msvc-release --no-tests=error` run passed all 102 tests, including both Rack playback replays at 137 and 280 BPM. The two native Perform previews were brought to the foreground during the run. The log is `out/issue-127/playlist-alignment-release-ctest.log`.

## Cue and Stop follow-up

On 2026-11-10, the installed Release bundle had SHA-256 `20FE74EB3265CA84103B10A7A075DDCD7FE87B2111BCAE6711911306652FA3D4`.

- FL Studio opened the saved two-bar loop with four prepared swells. Cue showed **Start Playlist playback** while stopped and **Ready** after starting. The manual Cue launched before the first MIDI events. Stop Loop entered **Finishing loop** and returned to **Ready** at the pass boundary. The MIDI Cue at bar 22 beat 3 started wet playback at bar 24, and MIDI Stop at bar 25 ended it. The next MIDI Cue showed **Waiting for bar** at bar 26.
- VCV Rack Free 2.6.6 and Host-FX 2.1.0 loaded the same Release build from the worktree bundle. The [Clock-only fixture instructions](../issue-133-rack-fixture/README.md) use a single timing cable into Host-FX. At 137 BPM the patch captured four bars and generated four swells. Cue entered **Playing lead-in** and **Playing loop**. The first Stop click showed **Finishing loop** and removed the next lead-in display. Stop Now returned to **Ready**. Rack MIDI Cue timing is covered by the native grid fixture. This desktop patch exercises the button.
- The native grid fixture checks both routes at sample level with no Phrase input. It checks the first wet sample, beat one, late MIDI Cue, the absence of the next lead-in after Stop Loop, and the immediate fade on Stop Now. The FL Studio processor fixture checks that a seek clears the old Cue and permits a new one.
- The full Release CTest run passed 101 of 102 tests. The Perform readability preview timed out waiting for Windows keyboard focus and passed on `ctest --preset windows-msvc-release --rerun-failed` with its window in the foreground. Both legacy Rack replays passed. The logs are `out/issue-127/cue-stop-release-ctest.log` and `out/issue-127/cue-stop-release-focus-rerun.log`.

## Rack Duration alignment follow-up

On 2026-11-10, Rack Free 2.6.6 with Host-FX reopened the Clock-only patch at 137 BPM. The existing four-bar waveform had nearly one beat of silence before its first snare transient. With Clock left running and Recapture audio enabled, the updated build accepted a four-bar retake with the first transient at the left edge and three more at the following bar boundaries. The installed and worktree VST3 binaries matched SHA-256 `B1088A554E80BA9C4F3FBA5A4100AC38E627E309348A1C424494F0335C0982DC`.

All six focused Release checks passed, including the FL Studio and Rack processor cases, both Rack playback replays, and the Learn and Perform UI tests. The new processor case verifies the exact start pulse before Clock starts, after an idle stretch in Perform, and after Clock loss. The log is `out/issue-127/rack-duration-ctest.log`.

## Shared MIDI and Loop readout follow-up

- Native Release processor and scheduler tests verified the shared note numbers,
  Playlist Cue with and without a Phrase offset, offset reset on seek, graceful
  Stop and Stop Now, and the legacy Rack replays. The native Perform fixture
  verified captured BPM and meter, Bar and Beat during playback, wrap to Bar 1,
  and unavailable position before beat one and after stopping.
- The full Release run passed 100 of 102 tests. The footer test initially
  checked the wrap after editing a swell, which invalidated the playing render.
  The corrected test passed on rerun. The existing knob reset test required
  foreground focus and passed on its focused rerun. Logs: `out/build-logs/Release/20261110-140430-4c2dbf77/test.log`,
  `out/issue-127/shared-midi-footer-ctest-rerun.log` and
  `out/issue-127/shared-midi-footer-focus-rerun.log`.
- The same Release VST3, SHA-256 `60370536AA9778B62B4CFE31B1B54D72E6EFB808F40C94249D246C53D4E78BB5`,
  recorded an eight-beat End Signal take using a single Clocked module and
  three timing cables in Rack. See the [Clocked wiring and proof](../issue-133-rack-fixture/README.md#use-a-clock-to-automate-end-signal).
- The new shared MIDI pattern was generated for the FL Studio fixture. It has
  native processor coverage; the updated pattern has yet to be replayed in the
  FL Studio desktop host with this build.

### Rack Clock and partial host transport

The Clocked patch accepted two bars at 44.1 kHz and rendered two swells, but
the owner observed Cue waiting with Clock running. Rack reported playing
transport fields without a meter. Perform now checks transport position for
seeks only on the Playlist route.

The Release processor regression passed with a 44.1 kHz End signal take,
ready wet and successful Cue while the simulated Rack host reported a
stationary sample position. The Playlist seek cases also passed.
See `out/issue-127/rack-partial-transport-ctest.log`.

The rebuilt VST3 still showed the original waiting message when the saved
patch reopened. A MIDI probe using the patch's saved Audio 2 settings
measured steady Clock pulses at 48 kHz. The accepted Loop was recorded at
44.1 kHz. Changing only the requested audio rate in a patch copy restored
Cue, and Rack showed Playing loop and an advancing playhead on the
original two-bar take. Perform now displays the required rate for a
sample-rate mismatch, and Diagnostics reports the host transport values.

### Live swell edits and Audition hover help

The first full Windows MSVC Release run passed 101 of 102 tests. The
scheduler fixture still expected Cue before the legacy reference had
relocked after replacing the source. After correcting that expectation,
the updated Release VST3 build and all 11 focused tests passed. The tests
verify the old audio during a pending render, the replacement audio after
the crossfade at the same Loop position, continued phase when all wet is
muted, and safe invalidation when the source changes. The native editor
checks disabled Audition and Cue hover help at a mismatched rate and
Audition recovery at the original rate.

Logs: `out/build-logs/Release/20261110-151143-7086b2cf/test.log` and
`out/issue-127/perform-live-edit-ctest.log`. The focused run includes both
Rack timing replays.

The Lead-In tint initially dimmed the entire cached Wet waveform.
`preverb_waveform_paint_test` reproduced the brightness drop before the fix
and passed afterward. It checks the selected waveform against its idle
brightness and verifies that the next-pass waveform remains visible and faint.
`preverb_loop_perform_ui_test` and the worktree Release VST3 build also passed.
The owner confirmed the visual fix in the prepared 44.1 kHz Rack patch.

### Pattern playback capture follow-up

The owner's `out/issue-127/fixture/ReverbArise Loop Ready to Cue_2.flp` test
copy routes Pattern 2's Fruit Kick into ReverbArise on Insert 1. With Pattern
playback selected, Learn armed for one bar, and transport left running,
the diagnostic build reproduced cancellation at the first wrap. The
48 kHz, 120 BPM, 4/4 trace declared the same cycle `[0, 96000)` on both
sides of the wrap. The last 250-frame callback began at 95,750. The next
began at sample and PPQ zero, with Play still active and unchanged reset
and activation counters. The trace recorded zero dropped rows. See
`out/issue-127/pattern-2-before-fix.csv`.

The ordinary Release VST3
(`C3A729341ADBD6128639809EAAEA1496DCDCEB07369A99F6BA24D1C68BEA78C4`)
accepted the take while Pattern 2 continued playing. The accepted manifest
records four beats and exactly 96,000 frames at 48 kHz, starting at the
first beat. Native Release processor coverage also checks two-bar Duration
across two repeats, a MIDI End Signal closing at the wrap, and cancellation
of a backward seek before the declared endpoint. Both Rack timing replays
and the Playlist capture and playback regressions passed. Pattern End
Signal and two-repeat Duration were exercised in native tests.

The full Release run following this fix passed all 102 tests
(`out/build-logs/Release/20261110-155536-f4064557/test.log`).
This was the baseline before the opt-in End on repeat method.

### End on repeat verification

Native Release processor tests exercised arming at the start of a declared
FL Studio repeat and partway through a pass, closing at the next admitted
boundary, and preserving the old Loop if the range changes. The Rack
fixture checked one complete pass between coincident Clock and Phrase
notes, ignoring an off-beat Phrase and cancelling on Reset. The method
also round-tripped through plug-in state. The native Learn UI tests passed
after updating the empty waveform copy to describe the workflow in both
hosts. The owner approved the revised Learn preview, including the
**End on Signal** label and the Arm and MIDI Stop help.

On 10 November 2026, both desktop hosts loaded the Release VST3 with
SHA-256 `922594A2836C8F63460FE3024E085981E6022EA99EE30C754310A696CD519477`.
With End on repeat selected, Arm was clicked with transport stopped.
Both hosts committed audio before their transports were stopped.

| Host and fixture | Closing boundary | Accepted beats | Frames at 48 kHz |
| --- | --- | ---: | ---: |
| FL Studio 2026, Pattern 2 at 120 BPM | Declared Pattern wrap to beat 1 | 4 | 96,000 |
| Rack Free 2.6.6 with Host-FX 2.1.0, Clocked at 137 BPM | Next Clock-aligned Phrase pulse | 8 | 168,176 |

The FL Studio manifest starts at sample zero. Rack displayed two snare
transients, one at the start of each bar. Both manifests record one
contiguous audio range at revision 1. Local copies are
`out/issue-127/end-on-repeat-fl-studio-manifest.json` and
`out/issue-127/end-on-repeat-rack-manifest.json`. The desktop repeat
check covered Pattern mode in FL Studio. The Playlist repeat path uses the
same declared-cycle fields, which are exercised in the native processor tests.

The approved repeat-capture build passed all 102 Windows MSVC Release tests
on the full rerun (`out/build-logs/Release/20261110-164549-7d2861ee/test.log`).
The first run hit a Windows foreground-focus failure in the existing native
knob reset test. This run predates the Pattern Cue correction below.

### Pattern Cue across a repeat

The owner reported Cue cancelling in Perform as the one-bar pattern wrapped.
Perform now checks the same exact declared boundary as Learn against its own
live transport observations. The Release processor regression queues Cue
partway through a Pattern pass, verifies launch and playhead position over
successive wraps, and verifies that a seek to the same start before the
declared end still stops playback. The processor, Loop playback, and both
Rack replays passed (`out/issue-127/pattern-perform-focused-ctest.log`).

The desktop proof used FL Studio 2026 in PAT mode with the owner's Pattern 2
at 120 BPM and the installed Release VST3 (SHA-256
`A52F7460E2F8C3CF01326010AC0D3A43CBA12F891D69FEC8BACC74C22DDA6368`).
Duration accepted one two-second bar with four detected kick transients.
After generating four swells, Cue showed **Waiting for bar**, then
**Playing loop**. The playhead continued across subsequent Pattern wraps and
returned to Bar 1 in phase with the pattern. **Stop Loop** returned to
**Ready** while the pattern was still playing. The proof project with the
accepted Loop and prepared swells is `out/issue-127/fixture/Pattern Cue Release proof.flp`.
The idle Cue prompt still reads **Start Playlist playback** when PAT is selected.

The first complete Release run of the Pattern Cue fix passed 101 of 102
tests. The existing native knob reset test failed a keyboard-focus assertion
and passed on its focused rerun after its window received foreground focus.
See `out/build-logs/Release/20261110-165908-7565ef15/test.log` and
`out/issue-127/pattern-perform-knob-reset-rerun.log`.

The final full `scripts/build.ps1 -Configuration Release -Test` rerun passed
all 102 tests, including both foreground-sensitive native checks and both
Rack playback replays. See `out/build-logs/Release/20261110-171051-21b8e9f0/test.log`.

### Loop Perform gain compared with Audition

The owner reported weaker swells after Cue than in Dry + wet Audition.
They reported matching live and Audition balance in Song Mode on the
same pre-fix build.
A read-only analysis of the saved FL Studio proof project applied the
saved shaping, calibration and 2 ms wet edges to the rendered stereo
assets. At 0 dB Live Wet with a −1 dBFS wet ceiling, the one-bar four-swell
loop's original bound applied 16.64 dB of attenuation. Summing the samples
at their actual fixed-grid offsets, including partial overlaps at Cue and
Stop, required 4.99 dB. The two-bar fixture needed 2.81 dB instead of
7.05 dB. These figures measure the wet branch and do not include dry audio.

The full Windows MSVC Release suite passed all 102 tests
(`out/build-logs/Release/20261110-205242-68e973d3/test.log`). The Loop
playback test compares the prepared peak to a direct sample scan, checks
peaks that coincide and peaks that cancel only during steady playback, and
checks gain selection in the fixed-grid and legacy routes.

The installed Release VST3
(`21E8988363D8BEA6DE11522F659D6C0844676DE85B76FE3436E6E8E4DA55F1A6`)
restored the Pattern 2 proof project, cued across the Pattern wrap, played
through subsequent passes and returned to standby on transport stop.
The host smoke check covered scheduling and restoration. The owner's
listening comparison in their project is still pending.

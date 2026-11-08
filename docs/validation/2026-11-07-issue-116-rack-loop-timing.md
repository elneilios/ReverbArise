# Issue #116 live Rack timing spike

Tested on Windows x64 with VCV Rack Free 2.6.6, VCV Host 2.1.0 and Fundamental 2.6.4 at 48 kHz. The licensed isolated Rack profile loaded two standalone MSVC Release VST3 probes. The Host-XL fixture used one integer sample counter for a repeating stereo phrase, an alternating Capture Gate, Phrase Start, Cue and Stop. The capture gate's high and low spans each equal one 49,153-sample phrase. A Push trigger resets the fixture clock. Every Host-FX observer receives D0/14, C0/12, E0/16 and F0/17 via Note Gate.

| Host-FX setting | Actual callback | Complete periods | Capture / Phrase reference edges | Cue / Stop reference edges | Maximum edge error |
| --- | ---: | ---: | ---: | ---: | ---: |
| 256 | 128 frames | 32 | 96 | 384 | 0 samples |
| 512 | 256 frames | 32 | 96 | 384 | 0 samples |

The Capture/Phrase observer compared MIDI edges with the gate voltage and the downbeat in the synthetic dry audio. The Cue/Stop observer compared MIDI against those two gate voltages. Each size included 32 distinct callback offsets for Phrase Start, 32 one-sample cues, 32 one-sample stops and 32 simultaneous Cue/Stop rises. No expected edges were missing or displaced in the 32-cycle windows. These are offsets relative to the audio in the same callback. Rack and Host buffering still add end-to-end latency.

## Interruption checks

- Starting with an already-high D0 input emitted note-on in the first callback at sample 1, matching the high dry reference.
- Wiring one source to two E0 mappings produced one MIDI edge per voltage edge for all 46 edges before the reset.
- The patch reset shortened one interval to 43,836 samples before returning to the regular 49,153-sample period.
- Unplugging the held D0 control emitted note-off at sample 1,076,800 while its independently wired dry reference stayed high. The message alone cannot distinguish cable removal from an intended gate fall.
- Switching Host-FX power off and back on produced a 21.673-second trace heartbeat gap with only 12,032 processed samples between adjacent heartbeats and no new prepare, release, reset or bypass callback. The phrase interval spanning resume measured 56,495 samples. This demonstrates detection of an observed phase break. A silent suspension spanning an exact number of cycles could escape phase-only detection.

## ReverbArise and offline replay

The existing ReverbArise Learn Release at `b55c493` captured the same fixture's full 49,153-frame stereo phrase. Its first 48 samples carry the synthetic downbeat. All decoded values agreed with the generated reference within `2.39e-8` in float amplitude.

Offline replay used timestamps from all four measured traces and synthetic prepared wet. It checked an intact 2,048-sample lead-in, a zero-sample downbeat error and exact overlap sums across three wraps. A late cue deferred by one complete phrase. Repeated Cue kept the first launch, simultaneous Stop won, a queued Stop emitted no wet, and an active Stop faded within 240 frames. Missing and shifted markers exercised rejection and fade.

A separate native offline harness linked the existing production DSP libraries and used the first 2,048 samples of the saved Rack take with a short test IR. The existing renderer and shape preparation produced 4,096 stereo frames starting at −2,048, with a lead-in peak of 0.294. A 32-sample placement edit moved the output without changing any shaped samples or performing a second convolution. The source WAV SHA-256 was unchanged.

## Live playback of the proposed policy

The separate `loop-policy.vcv` fixture played the prepared swell through a local Release VST3 probe in Host-FX at 48 kHz. The fixture clock supplied the stereo dry phrase and independent Phrase Start, Cue and Stop signals. It deliberately omitted one marker and shifted a later one by one sample. Two probe instances recorded their dry inputs and actual mixed callback outputs at 128 and 256 frames.

- On-time Cue played the complete 2,048-frame lead-in and reached Bar 1 with zero sample error. The next repeats reused the lead-in before the following downbeat.
- Late Cue deferred by exactly 49,153 samples, one complete phrase.
- Stop faded wet over 240 samples (5 ms). When Cue and Stop arrived on the same sample, Stop won.
- The missing and shifted markers each cleared sync and faded wet. Cue during reacquisition was ignored. Reacquisition alone emitted dry only, and a subsequent explicit Cue launched a new lead-in.
- Each instance checked 933,907 frames. The 776,272 frames outside the expected wet windows matched the dry input exactly. In the wet windows, the maximum amplitude error against an independent dry-plus-wet reference was `3.37e-8`.

The fixture establishes behavior with a controlled fixed-period clock in Rack. Its recordings measure the VST3 callback output. The production integration check below uses that same reference. A free-running musician's clock and physical output latency remain separate checks.

Reproduce with the two standalone targets `rack_policy_clock_VST3` and `rack_policy_VST3`, plus `loop_wet_spike` to write `prepared-wet.f32`. Generate the patch with `make-policy-fixture.ps1`, open it in the isolated Rack profile, then run `check_live_policy.py` against the two completed `policy-*.f32` traces. The results are in `out/loop-mvp-spike/rack/live-policy-results.json`.

- Policy clock SHA-256 `782231cfebe84411284c74de7d9f28dec157d1db799114d4e69ca3289b80588f`
- Policy probe SHA-256 `50b00df6ae313fc3a2cf09085738580fb368bcb106d56974180a518e7c9d66a9`
- Prepared wet SHA-256 `343bac556e34bbd5a38f1fad694659c69f68fe008fa37d7ce5cc4184a1eeff83`

## Proposed production limit

Admit the local MVP when a verified take and its recurring reference have the same stable integer-sample period at the current rate. Establish lock from two consecutive exact intervals. Invalidate it on a missing or shifted marker, a known processing reset or a rate change. Require fresh sync and an explicit Cue before resuming. The patch operator should use Stop before changing cables or suspending Host-FX. This spike supports that narrow timing contract. The broader musical clock policy needs separate evidence.

## Reproduction

Local fixture source, builds and traces are under `out/loop-mvp-spike/rack/` in the issue worktree. `loop-timing.vcv` is the four-observer test and `loop-lifecycle.vcv` holds D0 high on load. `make-fixtures.ps1` generates both from the two local probe bundles. `check_timing.py`, `check_lifecycle.py` and `replay_timing.py` write the measured summaries. `loop-learn-check.vcv` uses the existing ReverbArise binary, and `check_learn.py` checks its saved take against the fixture output. `loop_wet_spike` runs the native offline render and placement check.

- Clock binary SHA-256 `2b665876d3fc916cde88c77492a85345d5ae87a0e967f4b2043073fb2de528f4`
- Observer binary SHA-256 `c947d80799da12a2da21b24665aa673981954e0eca310641358fb6bbafd1aefa`
- ReverbArise Learn binary SHA-256 `1ca0605f2db7ddbccbcac3a017ff76175126fcf46ed4e9150eb2c7fcdd095973`
- Captured WAV SHA-256 `de05c49ce9de01a6112f6f706c5c7dd7a4c54feac2ccfd5332a7a5950afd8e85`

## Local production Loop Perform

The Windows Release build was made in the #116 worktree on top of `b55c493` with local changes. Rack Free 2.6.6 and Host 2.1.0 loaded an isolated copy of ReverbArise. The test used 48 kHz, 128-frame ReverbArise callbacks and the exact 49,153-frame repeating phrase. The runtime audio comparison used ReverbArise binary SHA-256 `bb0d04838daccef52285f0e62ec9b1ca4022e570c9e9282aa16a6f4e0c0f6fa1`.

- Both Host-FX lanes received the same stereo patch audio and the same D0, C0, E0 and F0 signals. One ReverbArise instance prepared and played the swell. The other stayed in Learn as the dry reference. The trace probes recorded their inputs downstream of both instances. Aligning the lanes by the dry reference edge accounted for a 256-sample measurement-path delay relative to the direct marker.
- Across 928,594 aligned samples per channel, the production mix played a 24,000-frame lead-in and reused it at the end of the repeat with zero difference. All 683,151 dry-only frames matched exactly. Stop, a missing marker and a marker shifted by one sample followed the expected 240-frame wet fade within `1.01e-7` amplitude. The late Cue deferred a full phrase. Sync recovery alone produced no wet.
- The second instance subsequently captured its own 1.024-second take. The editor prepared, displayed and invoked Audition on the swell. Moving Placement from 0 to +243.81 ms moved the wet waveform. The test saved the Rack patch during playback and reloaded it. Learn restored the accepted take, and Perform restored the edit and returned to Standby after reacquiring the reference. Audible monitoring on a physical output was outside the trace setup.
- The native Release tests separately cover exact interval matching, overlap, edit replacement while a mix is queued, callback allocation checks, native-rate and checksum rejection, cancelled and failed capture, saved edit restore, and Audition output. The editor test exercises pointer Stop and keyboard Cue at 900×600, at 100% and 125% plugin scale. Automated Song readiness and export checks passed. The live FL Studio Song and rollback smoke test is pending.

The live production trace is in `out/loop-mvp-spike/rack/traces/` as `policy-718c4f44414942f6851e06992b563123.f32` and `policy-54bcd6a0551d4d5485e053ac34a875e1.f32`. Run `out/loop-mvp-spike/rack/mvp-review/check_output.py` with those two paths to compare the recorded inputs. This Rack check used an isolated test patch and VST3 bundle.

The final local Release added Song-only Settings isolation in the editor. Its isolated VST3 binary SHA-256 is `3081bf80baf6bf43dcf3209073dd1b52483cdcfeb329f74424af0eb95c5915a1`. The four focused tests passed, covering Loop capture and playback, native editor bindings, workflow state and Song export. The full suite passed 97 of 99. The existing knob-reset and readability previews both failed to acquire Windows foreground focus, including on the final rerun. They passed on the preceding build in a five-test focused run. Full-run and rerun logs are in `out/build-logs/Release/20261107-192446-d0621070/`. The final test patch is `out/loop-mvp-spike/rack/mvp-final/loop-perform-local.vcv`.

## Revised Perform layout and listening patch

The 900×600 accepted-loop mockup guided the Swells list, Dry and Wet timeline, Timing/Envelope/Sound inspector, shared Audition and Loop Playback row, and Overview. The MVP displays one swell and uses time relative to Loop start. The reference supplies the phrase boundary and period.

Release build `20261107-203224-18581117` passed. The focused Timeline Controls, Swell List, Loop Capture and Learn Generation UI tests all passed. The UI test exercises section changes, zoom and Fit, and Cue and Stop at both 100% and 125%. The UI standards check passed.

The isolated VST3 in `out/loop-mvp-spike/rack/listen/` has SHA-256 `46545fe37c1e6c4818393c0ceb89a371e8a9fd5a2072da6ea3455b459f404dc1`. `ReverbArise-Loop-Listening-Test.vcv` uses a steady phrase clock, one Host-FX with a saved 1.024-second take, and an Audio-2 module wired to the stereo output. Cue and Stop are manual. In live Rack the editor restored the saved take, reached Ready to cue, moved to Repeating wet on Cue and returned to Ready to cue on Stop. The Audio-2 device remains unselected for local listening. The earlier measured audio is also available in the same folder as `Rack-dry-cue-repeat-stop.wav`.

## Shared controls and direct timeline editing

The next local review brings the Song Perform rotary and exact-value controls into the Loop inspector. A source gesture updates the captured interval used for rendering, a Wet gesture moves the swell, and both update the inspector. Overview navigates the zoomed timeline. The playback cursor and footer display the phrase position. The Rack reference defines the phrase boundary and length, so the ruler displays seconds. Reset restores the wet shape and placement. Reset All also restores the source interval and IR.

Windows Release build `20261107-204748-8b7b891a` and all five focused tests passed: Timeline Controls, Swell List, Loop Capture, Workflow State and Learn Generation UI. The native binding test covers source drag commit on release, exact numeric entry and malformed entry rejection, both reset scopes, Cue/Stop and cursor state, and the layout at 900×600 and both user scales. The UI standards check passed.

The review patch `out/loop-mvp-spike/rack/integrated-controls/ReverbArise-Loop-Perform-Review.vcv` uses the verified Release copy with SHA-256 `e451e70309b97e09702d5028a09e598cdb117019550479ca65206e3d73e70e54`. In live Rack, dragging Source End changed 250 ms to 352.58 ms, then moving Wet changed placement to 102.58 ms while Source End stayed fixed. Cue displayed a moving playhead and phrase position. Stop and Reset All returned the fixture to Ready to cue. Overview navigation and Fit were exercised on the same take. The 900×600 editor was left open for owner review with the Audio-2 device unselected.

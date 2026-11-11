# Tasks

## 1. Integrated candidate and fixtures

- [x] 1.1 Record the candidate commit and installed Release VST3 hash. Use the existing test installer and its rollback workflow. Prepare clean Rack and FL Studio fixtures from the tracked generators and make disposable project copies for destructive test cases. Record host, OS, sample rate, device, buffer and routing details.

## 2. VCV Rack

- [x] 2.1 In Rack Free 2.6.6 / Host-FX 2.1.0 / CLKD 2.5.0 at 48 kHz, revalidate Clock-only UI Arm and Duration, Step 1 Start/Arm with Step 4 Stop in End on Signal, and Clock plus aligned Phrase in End on repeat. Verify the observed MIDI 23/24/25 routes and exclusive take lengths. Include stopped-to-running and already-running clock starts.
- [x] 2.2 Verify End on Signal closes at the next downbeat, Duration Stop cancels, and clock loss before completion preserves the prior take. Run repeated sequencer cycles with Recapture Audio off, then enable it and prove a successful new take replaces the previous one.
- [x] 2.3 In Perform, test Clock-only UI Cue and optional aligned Phrase and MIDI Cue/Stop/Stop Now. Check lead-in, repeats, Stop finish, immediate fade, reset, sync loss and fresh Cue. Demonstrate dry Standby without valid timing and with a mismatched legacy Phrase-only route. Verify Song reports the missing absolute timeline on this Rack setup.

## 3. FL Studio

- [x] 3.1 Verify Playlist Duration from a bar line and mid-bar, End on Signal using UI and MIDI, and Pattern/Playlist End on repeat. Include multi-pass Duration across a declared wrap and an arbitrary seek that cannot close a take. Compare accepted boundaries and the capture-local grid.
- [x] 3.2 Verify Playlist and Pattern Cue, the default host bar grid, optional MIDI Phrase origin and Cue/Stop/Stop Now, lead-in and stop behavior. Exercise seeks, declared wraps, transport stop and fresh Cue. Verify current Song snapshots, live playback and bounce on the supported timeline.

## 4. Audio, delivery and recovery

- [ ] 4.1 Physically monitor both hosts with known transients and a long multi-swell session. Compare Audition and live balance, dry transients and lead-ins. Verify generated wet headroom at -1 dBFS, unchanged dry level when summing, and measured overload feedback after a deliberate gain increase. Record timing, peaks, resource use and any dropout or unsupported configuration, including rate changes and host suspension.
- [x] 4.2 Verify finite Loop WAV Preview, Save and Drag placement, repeat spacing, initial lead-in and final tail, including Rack capture with no live timing and FL Studio import. Compare Wet Layer and Dry + Wet with their prepared mix. Measure FL Studio bounce separately and document when WAV is the dependable choice.
- [ ] 4.3 In both hosts, save and reopen a current-build edited Loop, then exercise Locate and verified Restore on disposable copies. Compare accepted identity, frozen edit and dependency checksums, save and reopen again, and verify disarmed Learn, dry Standby and fresh timing and Cue.

## 5. Physical DPI transferred from #126

- [x] 5.1 At actual Windows 100% and 125% display scaling, verify Song and Loop Learn and Perform at the approved 900 × 600 size in the supported desktop hosts. Capture screenshots and record OS and plug-in scales, host and build, clipping, alignment, pointer and keyboard input, and editor reopen behavior. Record gaps explicitly if a host cannot complete a case.

## 6. Documentation and readiness

- [x] 6.1 Publish dated validation evidence in the repository with a host capability matrix separating capture method, local grid, live timing, commands, export/offline, recovery and physical DPI. State unsupported and unverified conditions explicitly. Update the offline guide and Rack/FL Studio appendices with tested wiring, Playlist/Pattern workflows, timing and recovery instructions.
- [ ] 6.2 At the approved readiness gate, run focused native Loop, Song, UI and recovery checks and the standard Windows Release PR regression set. Check UI standards, strict OpenSpec validation and guide currency. Use the full registered suite if preparing a release package. Record revisions, commands, results and any rerun rationale.
- [ ] 6.3 Present the integrated evidence for one consolidated owner acceptance, including the listening check and any open gaps. Record the decision and prepare the #130 review PR against feature/loop-mode. Leave feature release and integration into main for separate approval.

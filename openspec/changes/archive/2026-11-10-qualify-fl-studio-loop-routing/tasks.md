# Tasks

## 1. Establish the FL Studio test route

- [x] 1.1 Install a local Release VST3 with the test-install workflow. Create a minimal FL Studio Playlist with a bright transient, explicit phrase origin, and repeatable test positions. Record host, wrapper, sample rate, and buffer settings.
- [x] 1.2 Trace native timing, callbacks, and continuity during live transport and offline render. Probe MIDI Out Piano Roll → VST3 FX with candidate D5/C5, recording wrapper ports, channel, actual MIDI numbers, offsets, note-on/release, duplicates, and delivery after repositioning.
- [x] 1.3 Record which routes qualify for capture timing, live phase, and each external command. Verify the UI fallback where routing is unavailable. Probe host automation separately before claiming support. Retain the project, settings, and trace in dated validation evidence.

## 2. Implement only qualified routes

- [x] 2.1 Reuse existing capture admission and settlement to support measured Duration and safe End Signal behavior. Preserve accepted audio on an interrupted take and explain unavailable End Signal. Add focused capture regressions and document the verified capture route or fallback.
- [x] 2.2 Connect measured live timing to explicit phrase origin and matching recurrence. Keep Cue unavailable when only capture timing qualifies. Verify capture-only and matching live phase in native fixtures and document the origin setup.
- [x] 2.3 Connect verified external commands to existing Learn and Perform actions with distinct roles. Add focused tests for edge handling, duplicated events, phase loss, and fresh Cue after reset. Update the offline guide with supported command mappings and UI fallback. Reuse existing status and controls, with native owner review for any visual change.

## 3. Verify and document

- [x] 3.1 Demonstrate Duration while transport continues, a completed and interrupted End Signal or the documented fallback, and capture-only versus live Cue readiness.
- [x] 3.2 Demonstrate Cue before a chorus, Stop at a verse, and fresh Cue at a later chorus. Check late cue, seek past a Cue, Playlist wrap, reset, and offline render. Document arbitrary-start automation only if reproduced.
- [x] 3.3 Confirm dry continuity and the Song route, then run required Release checks. Record the end-to-end results and observed limits with the reproducible FL Studio fixture.

## 4. Verify Playlist capture alignment after owner testing

- [x] 4.1 Capture Duration on the first playing bar line or the next bar if starting mid-bar. Verify first-block continuity and MIDI-driven End Signal with native regressions.
- [x] 4.2 Verify the updated capture paths in FL Studio and update Learn feedback and the guide. Keep live playback sync separate.

## 5. Simplify Cue and Stop after owner testing

- [x] 5.1 Queue FL Studio and Rack playback from UI or MIDI at the first bar with room for the full lead-in, without requiring Phrase notes. Use the first Rack Clock pulse as beat one. Verify repeats, both Stop button stages, and fresh Cue after interruption.
- [x] 5.2 Update the existing Perform feedback and guide. Verify both hosts and run the Release checks.

## 6. Align Rack Duration to the running clock bar

- [x] 6.1 Share Rack downbeat observations between Learn and Perform. Verify Duration armed mid-bar, after returning from Perform, and before Clock starts. Keep End Signal and FL Studio coverage.
- [x] 6.2 Update Learn feedback and the guide, and verify a running-clock retake in Rack.

## 7. Share MIDI playback controls and allow a Playlist Phrase origin

- [x] 7.1 Apply Phrase to the running FL Studio playback grid in standby while preserving default Playlist timing, full lead-in, capture timing, and Rack behavior. Verify shifted and default starts and reset handling.
- [x] 7.2 Share the channel 1 MIDI note map across hosts. Verify Stop finishes once, cancels pending launches, suppresses the next lead-in, and yields to the dedicated immediate Stop Now note.
- [x] 7.3 Update the guide and host fixture instructions and record the Release validation and any remaining host checks.
- [x] 7.4 Bind the existing Perform footer to the Loop playhead and captured tempo and meter. Verify it with a native Rack-style processor fixture, preserve the Song binding, and show the native preview for owner review.
- [x] 7.5 Verify that partial host transport fields cannot reset Rack Clock authority in Perform. Exercise the 44.1 kHz End signal capture and Cue path and retain Playlist seek coverage.
- [x] 7.6 Report the required capture rate when it blocks Loop Cue. Verify the disabled control and message in the native editor and verify Cue in Rack with the original capture rate.

## 8. Keep Perform responsive to edits

- [x] 8.1 Retain and crossfade compatible Loop mixes while playing, with processor and callback regressions for material edits, phase, silence and safe invalidation.
- [x] 8.2 Disable unavailable Audition and Cue commands with hover help, verify sample-rate mismatch and recovery in the native editor, and document the behavior in the user guide.

## 9. Capture across a declared FL Studio repeat

- [x] 9.1 Trace the reported Pattern wrap and allow the declared boundary in Learn. Cover Duration, End Signal and interrupted-take retention with native regressions, and replay the reported Duration take in the saved FL Studio project.

## 10. Offer opt-in End on repeat

- [x] 10.1 Reuse the existing capture method selector and show a native Learn preview for owner approval.
- [x] 10.2 Verify whole-pass capture on FL Studio's declared repeat and Rack Clock plus Phrase. Cover mid-pass Arm, early seek or reset, and settings restore with native regressions. Record desktop evidence and update the guide.

## 11. Keep Perform running across a declared FL Studio repeat

- [x] 11.1 Keep queued Cue and active playback through the declared Pattern boundary. Verify bar alignment, cancellation on an early seek, the FL Studio Pattern fixture, and the Rack playback regressions.

## 12. Protect the measured wet level in Loop Perform

- [x] 12.1 Measure fixed-grid wet peaks across overlapping cycles, including Cue and Stop boundaries, so protection reduces only genuinely loud sums. Preserve the conservative guard for variable phase, verify the playback regression and record host fixture evidence.

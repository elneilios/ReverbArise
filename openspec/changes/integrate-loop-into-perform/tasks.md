# Tasks

## 1. Prove Rack and DAW timing inputs

- [ ] 1.1 Identify Stuart's Clock module and settings. Build a local patch with that module, the audible sequence and shared reset, plus the intended Host-FX trigger routing. Capture time-aligned traces of Start, capture end request, verified bar/phrase, Cue and Stop against audio. Test several periods, callback offsets, reset, missed pulses and fractional-sample tempos. Document the measured clock conditions and failure limits.
- [ ] 1.2 Verify the FL Studio host transport/meter and Piano Roll routes, including transport Stop behavior and a separate capture end request while running. Record trigger and bar authority per host. Review the evidence and confirm the supported timing policy before production timing work.

## 2. Learn tempo-synced capture and take ownership

- [ ] 2.1 Add the Loop choice in Learn with shared controls, Fixed length in bars and beats, Follow capture gate and Arm feedback for each. Preserve Song capture. Review a native preview of both Learn options.
- [x] 2.2 Use the proved Start, bar and end-request inputs. Test exact lengths, same-sample and off-boundary Stop, waiting for the next bar, invalid start and reference interruption before the boundary. Keep the last accepted take on cancel, interruption or failure.
- [x] 2.3 Persist accepted take type, source identity, native rate, musical context and exclusive end. Test old Song state, prototype Loop restore, switching take types, failed recapture, stale and missing sources, and cache compatibility.

## 3. Share the Perform editor and marker workflow

- [ ] 3.1 Bind the existing workspace, waveform, Swells list, timeline, Overview, inspector, Settings and Audition to the accepted take. Verify independent multiple-swell edits, source/target/placement, exact entry, envelopes, IR, level, activity, Freeze and both reset scopes. Test Song mapping, Loop lead-in and grid availability, audition mix and stale edit protection.
- [ ] 3.2 Reuse Automatic Swells when enabled in Learn. Test phrase-relative markers, capture edge and coverage handling, disabled generation, protected edits and rejection of stale jobs.
- [ ] 3.3 Replace the prototype editor and import compatible saved prototype edits after the shared path passes. Run UI standards and present native previews of both take types at 900×600 and 100%/125%.

## 4. Cue and stop the prepared Loop

- [ ] 4.1 Add shared Cue Loop, Stop Loop and sync status beside Audition. Wire the verified Rack and FL Studio routes. Verify proposed D5 Cue and C5 Stop with live Piano Roll notes and document the routing.
- [ ] 4.2 Prepare the complete bounded swell mix. Test first lead-in, late Cue, repeats with overlapping wet, Stop precedence, lock loss, fresh Cue after recovery, safe edit replacement, and exact dry passthrough. Measure callback allocation behavior and compare wet against independent sample sums.
- [ ] 4.3 Run the complete Learn-to-Perform sequence with the module identified in Stage 1, checking that the audible downbeat and prepared wet stay aligned across repeats and save/reopen.

## 5. Export a grid-aligned Loop WAV

- [ ] 5.1 Extend the existing Export view and preparation path with Loop lead-in grid, repeat count, and explicit Playlist placement offset. Review the native layout using shared controls.
- [ ] 5.2 Verify the full lead-in, first downbeat offset, internal wraps, final tails, mix/preview parity and the identical current revision from Save and drag. Test zero lead-in, 2.4 bars rounded to a four-bar lead-in, and over-budget or missing-source outcomes.
- [ ] 5.3 Drop the WAV into a disposable FL Studio Playlist and verify the displayed offset places Bar 1 on the intended grid line. Check existing Song export in the same host.

## 6. Validate the integrated local workflow

- [ ] 6.1 Run a native Windows Release build, focused capture, UI, render and persistence tests, and repository validation. Review the shared control layouts and run UI standards. Update the offline guide and record test versions, supported timing, skipped checks and known limitations.
- [ ] 6.2 Smoke-test the complete Rack workflow and an earlier disposable FL Studio Song project for capture, editing, playback, export, saved assets and rollback. Verify DAW Loop capture and cueing with the supported inputs. Keep testing local until the owner is ready to consider a Stuart candidate.

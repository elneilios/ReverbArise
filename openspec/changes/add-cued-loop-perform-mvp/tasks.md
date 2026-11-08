# Tasks

## 1. Prove the patch reference

- [x] 1.1 Build a disposable Rack fixture driven by one clock and reset. Trace a single-cycle Capture Gate, recurring Phrase Start and independent Cue and Stop through Host-FX at both tested callback sizes. Compare the actual samples with dry reference pulses for 32 cycles per size. Record duplicate, simultaneous and short pulses, gate-high startup, reset, disconnect, suspension and the numerical acceptance limit in dated evidence.
- [x] 1.2 Replay the trace against a rough pre-roll and wrap mixer. Verify full lead-in, late-cue deferral, Bar 1 coincidence, overlapping contributions, Stop and mismatch rejection. Review the live measurements with the owner before choosing the production clock policy.

## 2. Prepare one swell from the verified Loop

- [x] 2.1 Add the phrase-local verified source reader and dispatch Perform from the accepted take type. Check native rate, frame length, checksum, failed recapture, reload, Song default and unchanged Song readiness. Document the source contract.
- [x] 2.2 Wire a single manual swell to the existing preparation and Audition path. Verify actual leading wet from a real capture, shape and placement edits, unchanged dry samples, bounded preparation, stale work rejection and persistence isolated from Song edits.

## 3. Synchronize and cue prepared wet

- [x] 3.1 Add bounded phase and cue processing using the reviewed reference. Test exact sample alignment, duplicate and simultaneous commands, late and queued Cue, Stop precedence, invalid period, missing marker, rate changes and lifecycle interruption without callback allocations.
- [x] 3.2 Prepare the initial pre-roll and repeating wet mix off-thread. Compare exact sample sums to an independent reference, including wet overlap near the end, exclusive wrap, pinned edit revisions, dry passthrough, bounded Stop fade and resource-limit rejection.
- [ ] 3.3 Add Cue Loop, Stop Loop and status in the existing Audition row using approved shared components. Verify pointer, keyboard, context help and accurate state transitions at 900×600 and both supported UI scales. Run the UI standards check and present the native editor for owner review. If a new shared control is required, review that component and update its production standard and tests before adoption.

## 4. Verify the local workflow

- [x] 4.1 Run a native Windows Release build and test Learn, edit, Audition, Standby, Cue, full lead-in, repeated wet and Stop against one live repeating Rack part. Check save/reopen and a second instance. Record fixture versions and actual host results separately from synthetic tests, and document the patch wiring.
- [ ] 4.2 Smoke-test the existing Song workflow and rollback in FL Studio using a disposable earlier project. Verify capture, Stop settlement, preserved edits and accepted source, and current playback behavior.

## Workflow follow-up

- Keep distribution local until the owner accepts an end-to-end MVP. Follow the existing issue branch, review and release workflows before preparing a build for Stuart.
- Retain fixed-length capture, DAW Loop MIDI and offline validation, and grid-aligned Loop Export in #116.

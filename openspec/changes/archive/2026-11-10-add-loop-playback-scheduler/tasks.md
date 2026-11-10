# Tasks

## 1. Live phase and scheduling

- [x] 1.1 Add independent live reference admission for an accepted Loop, with explicit phrase length, origin and matching rate, tempo and meter. Test a valid Clock-only capture without recurrence, a three-bar take with a four-bar recurrence, matching shorter and four-bar references, the measured capture tempo estimate, valid adjacent periods, drift, and a Cue at the missing-marker deadline. Document the capture and playback readiness split.
- [x] 1.2 Implement dry Standby, queued entry, lead-in, repeat, and immediate bounded wet Stop. Test earliest and late Cue, lead-in longer than a phrase, marker and Cue ordering, simultaneous Stop and Cue, isolation from Capture Start/Arm and Stop, and cancellation plus fresh Cue after reset, seek, timing loss or reactivation.

## 2. Shared prepared wet and periodic composition

- [x] 2.1 Add a reusable periodic wet compositor using the existing render representation. Check an independent sample-level oracle for multiple swells, negative offsets, overlapping cycles, long tails, exclusive boundaries, startup and bounded correction around adjacent fractional periods at different block partitions.
- [x] 2.2 Publish and pin coherent accepted-source and render revisions through shared preparation and bounded retirement. Test mute and solo eligibility, placement edits, source replacement, a delayed worker, compatible retained IR audio and obsolete publications. Verify no callback allocation, deallocation, I/O, convolution or blocking. Record shared memory and overlap limits with tests that reject an over-budget plan before it plays.

## 3. Processor integration and delivery

- [x] 3.1 Route accepted Loop live wet through the headless adapter, with the existing Song path and original-input capture ordering preserved. Exercise an accepted Loop with shared prepared wet, independent local and external commands, dry passthrough, mode exit and restore. Confirm the next capture preference cannot select a live route or grant phase.
- [x] 3.2 Run the production scheduler against measured 48 kHz CLKD marker, dry-reference and command offsets at 137 and 280 BPM. Check downbeat alignment, complete lead-in, wet overlap sum, unchanged dry, lost marker, reset and fresh Cue after re-lock. Record fixture provenance, results and supported limits. Keep FL Studio host coverage under #127.
- [x] 3.3 Run the Windows Release suite, verify the capture and Song regressions, and validate this OpenSpec change. Update the technical guidance and review offline guide currency against the delivered headless scope. Open the review-ready PR for #125 against `feature/loop-mode` with measured and synthetic evidence.

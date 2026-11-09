# Tasks

## 1. Host inputs and clock

- [x] 1.1 Select distinct sample-timed Clock, Start/Arm and Stop roles. Verify positive Host-FX trigger delivery, simultaneous offsets, audio alignment, held or disconnected inputs and independent legacy notes with an isolated Rack probe. Document the measured mapping and any unsupported connection behavior.
- [x] 1.2 Add bounded capture-local beat verification with explicit 4/4 and exact observed boundaries. Test fractional periods, changed buffer sizes, cold clock and short takes, missing or extra pulses, resets, cadence drift, limits and coexistence with the marker route. Update the Loop timing guidance.

## 2. Capture lifecycle

- [x] 2.1 Share attempt-scoped arm and stop handling across UI intent and sample-timed host commands. Test arm before clock, coincident and mid-beat Start, duplicate and stale commands, Stop while waiting, command priority, and restore or mode and storage changes during an arm. Verify callback processing stays bounded and free of allocations and locks.
- [x] 2.2 Wire exact Duration and End Signal boundaries into the existing capture publication fence. Test Duration Stop including on its configured boundary, final-step and downbeat Stop in End Signal, repeated Stop, missing closing beat, reset, and callback suspension. Verify rejected or obsolete attempts preserve the prior source, edits and revision, and an accepted take reopens disarmed with identical timing and audio. Update the capture state guidance.

## 3. Learn and musician patch

- [x] 3.1 Bind the existing Learn Arm latch and status to waiting, recording, finish pending, cancellation and settlement. Reuse Recapture Audio in Loop and test first capture and replacement permission for UI and MIDI, disarmed restore, and Song independence. Verify method-specific instructions and native component behavior, run `scripts/check-ui-standards.ps1`, and present the native interaction preview for owner approval at the existing Learn review sizes and scales.
- [x] 3.2 Deliver audible Rack patches with Clock and Learn Arm in one and sequencer steps 1 and 4 wired to Start/Arm and Stop in the other. Include audio device instructions. Demonstrate accepted End Signal in both, sequenced replacement with Recapture Audio on and protection with it off, clock-loss cancellation, and a Duration take and cancellation. Record exact accepted extents and host configuration. Update the offline guide and verify its currency.

## 4. Integrated verification

- [x] 4.1 Run the Windows Release wrapper with tests and verify the Song, legacy Loop, UI, and restore regressions. Validate the OpenSpec change and prepare a review-ready PR into `feature/loop-mode` with the measured Rack results and remaining host limits.

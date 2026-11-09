# Proposal

## Why

The verified Rack Duration patch requires separate beat, bar, phrase, and reset paths. Issue #133 under epic #116 simplifies capture wiring and closes the gap left by #124, where End Signal could only be exercised in an automated fixture.

## What Changes

- Admit a capture route with one Clock pulse per beat and optional external Start/Arm and Stop triggers. Use an explicit 4/4 declaration and count bars from the first recorded beat.
- Let Learn Arm and an external Start/Arm request the same single take, including while the clock is stopped. The first beat at or after the request starts capture.
- Make Learn Arm off and external Stop equivalent. Before recording, withdraw the arm. In Duration, cancel the take. In End Signal, finish just before the next bar's downbeat, including the whole bar if Stop lands on its first beat.
- Automatically discard incomplete takes after clock loss, reset, or interruption. Preserve the previous accepted source and dependent edits.
- Show Recapture Audio in Loop Mode. With it off, both UI Arm and external Start/Arm protect an accepted Loop while allowing the first Loop capture.
- Deliver two audible Rack examples. Use Clock and the Learn latch in one, then use sequencer steps 1 and 4 as Start/Arm and Stop in the other. Demonstrate End Signal settlement, Recapture protection and a lost clock preserving the previous take.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `loop-host-admission` — qualify capture from sample-timed pulses and positive commands while keeping synced playback admission separate.
- `loop-phrase-capture` — share arm and stop intent across UI and host inputs and define the complete-bar boundary and cancellation policy.

## Impact

Loop capture timing and command admission, Learn status and Arm binding, native workflow tests, Rack validation patch and evidence, and the offline guide. Preserve the existing marker route and saved Song and Loop projects. Perform scheduling, Export, a native Rack module, and broader DAW certification are outside this issue.

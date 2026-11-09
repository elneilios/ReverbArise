# Proposal

## Why

Knobs and sliders still use several focus, hover and gesture policies after the P4 field migration. The approved P5 native preview gives issue #107 a common look and a predictable way to fine-adjust a value or find the neutral Level by feel.

## What Changes

- Apply the approved dial and track states to the P5 inventory across Learn, Perform, Export, Settings and the timeline. Keep aqua value indicators, quiet pointer feedback and independent outer aqua brackets for keyboard and accessibility focus.
- Add Ctrl+wheel fine adjustment to eligible dials and slider tracks. Preserve the existing Stepper, Reset, keyboard and timing Snap paths, including Ctrl-drag Snap bypass where timing controls already support it.
- Give knobs an optional home detent for pointer drag. Perform Level uses 0 dB at 12 o'clock with a ±0.3 dB attraction band. Nearby values remain reachable through exact entry and fine wheel adjustment.
- Size the companion dial fields consistently, with space for wrapped feedback beneath them.
- Verify the approved design in the real control components before broad adoption and account for every P5 consumer. Keep hidden compatibility controls recorded in the inventory.

## Capabilities

### New Capabilities

### Modified Capabilities

- `ui-design-system` — define shared knob and slider states, control-specific fine wheel behavior, and the optional home detent.

## Impact

- P5 under #107 covers the eleven Perform dials, Learn Event sensitivity, Export Dry and Wet, six Settings sliders, horizontal timeline zoom, and timeline Height. The approved P4 fields and stepper behaviors remain the companion controls.
- This change builds on merged #108 Level and P4 behavior. Engine and recovery work under #8, state formats, gain policy, waveform interaction and other control families are outside its scope.

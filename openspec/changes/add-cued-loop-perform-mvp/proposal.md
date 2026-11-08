# Proposal

## Why

Learn now captures and verifies a stereo Loop through VCV Rack Host-FX. The next local milestone is to turn that take into an editable swell that can be cued before a live repeating phrase and stay aligned as it repeats.

## What Changes

- First prove that a Rack fixture can use the same clock for one complete captured phrase and its recurring phrase-start markers. Measure separate Cue and Stop inputs and the scheduling error of a full negative lead-in.
- Let a verified Loop take open Perform for one manually placed and shaped swell. Use phrase-relative coordinates and the existing reverse-reverb renderer and Audition experience.
- Add Standby, Cue Loop and Stop Loop to the existing Audition row. Launch at an eligible phrase boundary with the complete lead-in, repeat prepared wet alongside the patch and reuse the negative render at each wrap.
- Keep the local milestone focused on a fixed, repeating Rack patch. Fixed-length capture, DAW Loop MIDI and bounce, grid-aligned Loop Export, automatic detection and broad host coverage stay in [#116](https://github.com/elneilios/ReverbArise/issues/116) for later slices.

## Capabilities

### New Capabilities

- `loop-performance` covers preparation, local phrase synchronization and cued repeating wet playback from an accepted Loop take.

### Modified Capabilities

- `workflow-modes` adds explicit Perform readiness for the last accepted Loop take and retains the existing Song and Export rules.

## Impact

Build on `add-learn-follow-capture-gate`. The Loop asset reader, swell render job, bounded wet playback and Perform bindings gain an explicit phrase-local path. Song transport, existing projects, stored Song assets and VST3 identity retain their current contracts. Production UI uses approved shared controls and a native 900×600 review. This milestone is for local testing.

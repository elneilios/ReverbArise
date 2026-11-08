# Proposal

## Why

A musician should be able to capture a repeating patch or a section of a DAW project, shape swells in the existing Perform editor, then cue them live or place a rendered WAV on the Playlist grid. The [#116](https://github.com/elneilios/ReverbArise/issues/116) Rack spike established exact routing and cued playback with a controlled fixture. Stuart's Clock module and musical bar timing are the first feasibility gate.

## What Changes

- Keep Song capture tied to the DAW playhead. Offer Loop only in Learn, with a Start trigger and either a fixed length in bars and beats or Follow capture gate. In the latter, an end request completes capture on the next verified bar boundary.
- Make the last accepted take select the timing and source for Perform and Export. Keep the existing Swells list, timeline, Overview, inspector, Settings and Audition for both take kinds. Reuse automatic marker generation when enabled and the full manual swell workflow.
- Show Cue Loop, Stop Loop and sync status beside Audition for Loop takes. Support verified Rack and DAW trigger routes. Play the full negative wet lead-in before the cued downbeat and reuse it across repeats.
- Extend the shared Export workflow for Loop WAVs with a grid-aligned lead-in, repeat count and placement instructions. Preserve existing Song export behavior.
- Stage the work from host timing evidence through Learn, shared Perform, cueing, Export and local host acceptance. Preserve saved sources, edits and host compatibility throughout.

## Capabilities

### Modified Capabilities

- `workflow-modes` adds tempo-synced Loop capture and routes Perform and Export from the last accepted take.
- `automatic-swell-processing` extends the existing automatic marker workflow to verified Loop takes.
- `swell-workspace` gives accepted Song and Loop takes the same Perform surface and editing tools.
- `transport-swell-playback` adds cue-driven playback for the full Loop swell set.
- `song-export` extends the shared Export surface for grid-aligned Loop WAVs.

## Impact

The change extends the production editor, swell engine and exporter with explicit phrase-local source and timing. The separate prototype editor and single-swell binding are superseded after the shared path passes. The prototype change and dated validation remain the evidence for the controlled fixture. Development and host testing stay local until the owner is ready to consider an external candidate.

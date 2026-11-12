# Proposal

## Why

In FL Studio, Cue Loop can launch on an internal bar of a repeating Pattern. A Playlist arrangement also needs a way to schedule swells before a known phrase start so the first lead-in is audible.

## What Changes

- Start prepared Loop swells automatically with Live playback and a compatible FL Studio repeat range of the same musical length. A matching Playlist repeat can supply the same timing as a Pattern.
- Add **Set Loop Start** in Perform to save the Loop's beat one at the Playlist playhead while stopped or playing. Schedule the complete first lead-in and repeat until stopped. Offer manual entry and clearing in its context menu.
- Keep manual Cue available when there is no active FL Studio placement or compatible repeat. Keep the VCV Rack Clock, Phrase and Cue workflow.
- Add a reusable **Action Button + State Field** to the plugin control library. In Perform, pair Audition Play/Stop with Audition status, Set Loop Start with its accepted position, and Cue/Cancel/Stop with Loop status.
- Keep **Set Loop Start** visible and disabled on unsupported hosts, with contextual hover help explaining why.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `loop-host-admission` defines how host repeat ranges and song positions qualify for Loop playback.
- `transport-swell-playback` defines automatic repeat playback, saved Playlist placement and how they interact with manual Cue.
- `ui-design-system` defines the reusable joined command and read-only state field.
- `swell-workspace` uses the same Audition controls in Song and Loop Perform.

## Impact

Perform command layout, host transport observation, Loop playback scheduling, session serialization, validation fixtures and the offline user guide. This change targets realtime playback. Playlist WAV export remains available for arranging and rendering swells.

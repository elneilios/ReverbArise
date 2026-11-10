# Proposal

## Why

Issue #127 needs a reproducible way to capture a phrase and play its prepared wet swells at multiple points in an FL Studio Playlist. The approved Rack routing does not establish what FL Studio delivers to a VST3 effect.

## What Changes

- Measure the capture grid and live transport independently in FL Studio. Demonstrate Duration and a deliberate End Signal while the timing route continues, or document the verified fallback if End Signal is unavailable.
- Probe MIDI Out Piano Roll routing to the VST3 effect. If the measured route supports command delivery, integrate the verified Cue and Stop route. Keep Learn Start/Arm and Stop distinct from Perform Cue and Stop. If the route is unavailable, document the verified UI workflow and any automation that passes the same checks.
- Demonstrate button and MIDI Cue using host bar lines in Playlist and Pattern playback. Allow a MIDI Phrase note to set a different bar origin. Allow time for the full lead-in and repeat at the accepted length. Keep the launch and playback through a verified repeat boundary, require fresh Cue after a seek or reset, and record offline behavior.
- Offer the same workflow and MIDI note numbers in Rack with a running Clock, using the first pulse as beat one. Let Stop finish the current pass and offer Stop Now for an immediate fade.
- Offer an opt-in End on repeat capture method. Use the host-declared repeat range in FL Studio and Clock plus Phrase in Rack.
- Keep a playing Loop running through compatible swell edits. Explain disabled Audition and Cue actions through the existing Perform hover help.
- Protect fixed-grid Loop wet level using measured swell overlap, including the Cue and Stop boundaries.
- Keep Song Mode behavior and the approved Learn and Perform design. Loop export remains in #128.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `loop-host-admission` — Qualify FL Studio timing and command routes from observed host evidence before enabling or describing them as supported.
- `transport-swell-playback` — Admit the Playlist or Rack Clock grid for Cue, support finishing the current pass on Stop, and crossfade compatible swell edits during playback.
- `swell-workspace` — Show the queued, playing, finishing, and fading states on the existing Loop command and explain why Cue or Audition is disabled.

## Impact

FL Studio VST3 wrapper routing, Loop capture and playback admission, existing Loop command surfaces, native regression tests, a reproducible host project or fixture, and the offline user guide. The supported configuration depends on measured callback and routing behavior.

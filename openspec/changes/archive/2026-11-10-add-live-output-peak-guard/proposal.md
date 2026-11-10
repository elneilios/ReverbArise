# Proposal

## Why

GitHub issue #144 follows the Loop gain correction in PR #143. Song and Loop protect their wet contributions before mixing, but a live dry transient and a swell can still produce an excessive combined peak.

## What Changes

- Protect the summed stereo output during live Dry + Wet playback in Song and Loop. Use one gain for both channels and keep normal dry playback at unity when the guard is idle.
- Preserve the approved source-matched +6 dB wet calibration target, Live Wet defaults, and existing wet overlap protection.
- Qualify a zero-added-latency guard against transient and overlap fixtures, with an FL Studio and Rack listening check before delivery.
- Keep the first implementation scoped to realtime host playback. Audition, Wet Only, capture and offline render paths retain their existing gain behavior.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `transport-swell-playback` adds linked protection to the realtime Dry + Wet sum and defines when incoming dry passes through unchanged.

## Impact

Song and Loop realtime mixing, audio callback verification, and the user guide. No new control, persisted setting, host latency, or dependency is proposed.

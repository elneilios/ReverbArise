# Proposal

## Why

The experimental Stereo Ping Pong Crescendo already has the desired sound. Tempo Sync should keep its alternating echoes on the timeline's musical grid while the user moves and shapes the swell.

GitHub issue: [#192](https://github.com/elneilios/ReverbArise/issues/192). Parent epic: [#189](https://github.com/elneilios/ReverbArise/issues/189).

## What Changes

- Start with a listening spike against the supplied Creative IR Crescendo v2 WAV. Compare the original spacing with eighth and dotted-eighth notes at 150 BPM using intact source bursts. Compare arrival-relative and ruler-aligned versions across on-grid and off-grid edits, then require listening approval before exposing the preset.
- Add a versioned Stereo Ping Pong Crescendo definition with Tempo Sync, stepped Echo Rate, Stereo Spread, Build Intensity, Gate Amount, Damping and existing Level in the shared Learn and Perform Sound editor.
- Anchor synced hits to the plugin ruler's Beat 1 and selected subdivision using compatible fixed Song timing or committed Loop timing. Preserve the user's exact arrival and independent Envelope. Prepare changed responses off the audio callback and reuse the verified replacement and wet-shape handover from #183.
- Keep existing bundled and Custom IR assignments, host automation identities and saved audio compatible. Tempo maps, ramps across a lead-in, other creative presets and envelope changes are outside this change.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `sound-presets` covers the new definition, its rhythm and tempo authority, and preservation of the accepted generated response.
- `swell-workspace` covers the preset's shared controls and timing feedback.

## Impact

Sound catalogue and persistence, bounded response preparation, Learn defaults, selected-swell replacement, Sound picker waveform previews, native controls and FL Studio live and offline verification. #190 and #191 provide the foundation and shared editor on the epic branch.

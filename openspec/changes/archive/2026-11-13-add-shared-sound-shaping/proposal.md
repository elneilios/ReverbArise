# Proposal

## Why

Musicians need the same basic sound-shaping controls on any preset or imported response. The hall, plate and Custom IR listening spike established a useful damping range from a full swell to a short accent, and demonstrated the value of wide source cuts.

## What Changes

- Expose Level, Damping, High Pass and Low Pass in the shared Learn and Perform Sound editor for every available preset, including Custom IR.
- Use the approved wider damping behavior on full responses. Offer explicit Off states and a logarithmic 20 Hz–20 kHz range for both source cuts.
- Use neutral defaults of 0 dB Level, 0% Damping and both cuts Off. Preserve accepted saved sounds and their settings. Learn edits apply to newly created swells.
- Keep Level on its existing gain path. Apply the cuts to the source feeding the wet response and apply Damping to the response decay. Retain the independent Envelope controls.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `sound-presets` covers shared values, render behavior and compatibility.
- `swell-workspace` covers shared controls and their Learn and Perform scope.

## Impact

Shared Sound editor and bindings, session encoding, creation defaults, response preparation, render dependency keys and accepted playback handover. Coordinate with `add-stereo-ping-pong-crescendo` under [Epic #189](https://github.com/elneilios/ReverbArise/issues/189) so Ping Pong exposes one Damping control and retains its accepted behavior. The native review and host listening checks cover the compact layout, neutral compatibility and usable control range.

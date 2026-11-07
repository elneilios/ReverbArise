# Proposal

## Why

Stuart needs to capture a live phrase from his VCV patch. Host-FX Note Gate delivered sample-positioned MIDI to a standalone VST3, and the follow-up probe captured a 48 kHz synth WAV and matched every sample of a separate boundary fixture.

## What Changes

- Deliver the Learn portion of GitHub issue #116 as a private Windows capture candidate. Put **Enable Loop**, **Follow capture gate**, **Arm capture**, and the capture status in Learn.
- Accept one take between a fresh Note Gate rise and the corresponding fall. Use the captured stereo frames and the prepared sample rate to describe the phrase.
- Show the latest verified Loop take and provide access to its WAV for Stuart's test. A failed or cancelled attempt preserves the last accepted take.
- Preserve Song capture, stored Song source and edits, and the existing VST3 identity. In this capture candidate, an accepted Loop take stays in Learn with its WAV available. Fixed length, Loop Perform and grid-aligned Loop Export are subsequent work under #116.

## Capabilities

### New Capabilities

- `loop-phrase-capture` covers explicit gate-based stereo phrase capture and verified access to an accepted take.

### Modified Capabilities

Existing Song capabilities retain their contracts.

## Impact

The VST3 exposes a MIDI input for the capture gate. Learn gains contextual controls and a capture result. A separate phrase buffer and publication path use local sample coordinates. Native component, audio and persistence checks, a real Rack test, a targeted FL Studio Song check and the existing Windows candidate workflow provide the delivery evidence.

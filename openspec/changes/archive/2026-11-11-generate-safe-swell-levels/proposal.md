# Proposal

## Why

A single boosted swell can set the wet peak for an entire Song or Loop. The current shared playback attenuation can cancel the boost and make other swells quieter, even at times when they do not overlap.

## What Changes

- Keep source-relative calibration and reserve headroom when generated wet audio first becomes ready. At default settings the generated wet mix peaks at or below −1 dBFS, including across Loop repeats.
- Save the generated starting gains. Level applies its displayed dB adjustment independently, and Live Wet sets the gain of the wet mix.
- **BREAKING** Replace wet and Dry + Wet playback attenuation with overload reporting. Deliberate user boosts may exceed full scale in the float signal. Apply the same starting gains in Song, Loop, Audition and export.
- **BREAKING** Adopt the new behavior for pre-release development without a project migration or a compatibility mode for the old sound.

## Capabilities

### New Capabilities

### Modified Capabilities

- `swell-gain-staging` defines the generated wet ceiling, durable starting gain and independent Level edits.
- `transport-swell-playback` defines unattenuated float playback and overload reporting in Song and Loop.
- `audition-monitoring` uses the saved gains and reports overload in Wet Only and Dry + Wet.
- `reverse-convolution` aligns manual monitoring gain with the new policy.
- `song-export` carries the saved gains through preview and WAV at the existing independent fader settings.
- `loop-export` uses the same saved gains and Live Wet as Perform without a second wet gain calculation.

## Impact

Generation and render acceptance, saved swell state, playback preparation, audition, export and existing diagnostics. The ceiling applies to generated wet at default settings. Incoming dry and downstream DAW gain can take the mixed output above full scale.

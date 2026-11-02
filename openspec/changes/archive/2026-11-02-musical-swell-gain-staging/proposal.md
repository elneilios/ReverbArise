# Proposal

## Why

The library works functionally, but automatically generated swells still sit too quietly in music. The present post-Level -6 dBFS wet protection can cancel increases to Level, its conservative overlap bound can over-attenuate, and live wet gain cannot boost above 0 dB.

## What Changes

- Introduce a versioned musical gain policy for new instances, with a stronger calibrated creation baseline and 0 dB selected-swell Level meaning that baseline. Preserve adjustment below it and at least +12 dB above it, subject to explicitly indicated protection.
- Calibrate the default swell against the local dry attack using short-window energy, so arbitrary factory/imported IR recording levels receive a consistent source-relative starting balance. Compare this energy candidate against the fixed +6 dB listening reference and the existing peak-matched prototype. Hold each calculated baseline fixed while editing Level, fades, tension and placement, and preserve it with frozen audio.
- Measure the actual aligned wet signal when preparing protection instead of treating the sum of every overlapping member's independent maximum as its actual peak. Keep preparation bounded and outside the callback, with stable gain rather than block-dependent pumping.
- Prototype a -1 dBFS wet safety ceiling and bounded source-energy matching, then obtain user listening approval before locking the method, target offset and numerical default. Avoid amplifying silence/noise and disclose capped calibration and overlap protection. Level remains -60 to +12 dB around the calibrated 0 dB starting point, with the requested amplitude change below the actual ceiling.
- Share musical wet gain/protection behavior across audition, Song playback and offline export, preserving their explicit routing, blend and trim differences.
- Preserve older projects under their original gain policy and saved values. Offer explicit adoption of musical staging in Settings; never silently make existing projects louder.
- Preserve approved IR assets, immutable raw convolution PCM, live dry passthrough and the approved main-screen layout. Add concise protection feedback in existing status/Diagnostics.

## Capabilities

### New Capabilities

- `swell-gain-staging`: Fixed/source-referenced default candidates, stable per-swell calibration, effective level adjustment, shared wet preparation, versioned compatibility and listening acceptance.

### Modified Capabilities

- `transport-swell-playback`: Policy-dependent wet protection and boost-capable live wet gain, shared by realtime and offline paths.
- `audition-monitoring`: Policy-dependent wet/sum protection, preserving blend/trim semantics and exposing effective gain.

## Impact

Expected areas: `SwellSession` creation/state validation, `SwellShape`, engine/group preparation, `SwellTransport`, `TransportPlayback`, `AuditionMix`, inspector/live controls and Diagnostics; numerical, state, processor and listening-fixture tests. This builds on the current `expand-bundled-ir-library` implementation without rewriting its sound assets or claiming its remaining host acceptance is complete.

No new IR designs, dry ducking/crossfade, master output processing, latency, host parameters, transport behavior, automatic marker changes or UI redesign. No runtime loudness riding, continual envelope compensation or arbitrary per-swell normalization to a universal full-scale target. Source-referenced energy is a practical intensity estimate, not a guarantee of equal perceived loudness for every response. It intentionally reduces IR recording-level differences while preserving PCM, timbre, decay and stereo character. The revised energy algorithm requires implementation and a new listening checkpoint before adoption as the release default.

# Proposal

## Why

ReverbArise currently ships one 1.8-second hall response, limiting the musical range available without external files. GitHub issue [#7](https://github.com/elneilios/ReverbArise/issues/7) calls for a small, contrasting, redistributable library designed and auditioned specifically for reverse swells.

## What Changes

- Add five original deterministic static responses: Monumental (~10 s), Intimate Plate (~1 s), Vintage Dark Hall (~4 s), Shimmer Cloud (~7 s), and Whisper (~0.5 s), alongside unchanged Preverb Hall.
- Publish versioned catalogue identities, checked embedded WAV assets, reproducible generation parameters, provenance/licensing, and measured duration, peak, energy and stereo characteristics.
- Populate the existing per-swell dropdown from the real catalogue; preserve imported WAV/WavPack selection, freeze, exact edits, future defaults and legacy managed assignments.
- Retain a verified last-good live contribution during an IR-only replacement; publish the complete replacement at its current Song position with a short safe handover. Clearly distinguish requested and currently audible versions.
- Verify long/overlapping tails, channel/rate handling, loudness, render/cache costs, persistence, real-time safety and FL Studio listening. Describe Shimmer Cloud as static coloration, not dynamic pitch shifting.

## Capabilities

### New Capabilities

- `impulse-response-library`: Curated bundled static responses, deterministic content/provenance, catalogue integrity, compatibility, preparation and listening acceptance.

### Modified Capabilities

- `swell-workspace`: Catalogue-backed per-swell selection and truthful pending/audible IR status without changing the approved layout.
- `transport-swell-playback`: Narrow last-good playback exception for an explicitly requested IR-only replacement, with safe atomic handover and unchanged invalidation for other dependencies.

## Impact

Affected areas: resources/generation tooling and build embedding, DefaultImpulse and immutable asset installation, SwellEngine import/publication ownership, transport prepared-page handover, existing inspector IR binding, tests and user documentation. Preserve plugin/parameter identities, existing managed IR/render files and Preverb Hall's bytes and default behavior. Prefer current asset references and version-7 session fields; do not introduce a schema migration merely to list presets.

This change uses the current workflow. Capture/Live/Finalise (#3), standalone Finalise export/mix (#6), new DSP effects/controls and the experimental Chamber/Glass/Lo-fi palette are outside scope. Implementation assets and code are not created during this planning exercise.

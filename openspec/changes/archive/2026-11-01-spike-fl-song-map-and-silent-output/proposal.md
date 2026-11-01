# Proposal

## Why

Preverb's FX design depends on capturing audio at accurate FL Studio Song positions and playing prepared audio at those positions, including when the mixer input is silent. FL Studio's transport reporting and processing behavior in silent sections are host-specific risks, so they must be measured before investing in persistent song mapping, convolution, or the editor.

## What Changes

- Add a minimal Windows x64 C++20/JUCE/CMake VST3 audio effect with transparent stereo passthrough and diagnostic host-transport reporting.
- Capture input against absolute Song positions into a sparse map that distinguishes unmapped ranges from captured silence and preserves non-contiguous sections.
- Add a deterministic scheduled-tone probe and verify its output during silent input, seeking, looping, and replay.
- Limit the spike to FL Studio 2026 Song mode. Record observed host behavior and blockers; do not build convolution, onset detection, the full waveform editor, production asset management, or Pattern mode support.
- For an explicit recapture, replace only the overlapping captured range and preserve all other mapped intervals.

## Capabilities

### New Capabilities
- `song-timeline-audio`: Capture input into a sparse Song-position map and schedule generated audio against absolute host positions.

### Modified Capabilities

None.

## Impact

- Greenfield project setup: C++20, JUCE, CMake, MSVC, and a Windows x64 VST3 effect target; the current repository contains OpenSpec/package scaffolding but no C++ or CMake source.
- FL Studio 2026 Song-mode host playhead APIs, mixer routing and silent-processing behavior.
- Real-time-safe audio capture staging, sparse timeline bookkeeping, diagnostic UI/state, and automated or reproducible tests for timeline math and scheduling.
- No production convolution, persistent capture asset format, or user-facing waveform editor is introduced by this spike.
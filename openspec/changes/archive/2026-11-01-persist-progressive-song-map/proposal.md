# Proposal

## Why

The M0 spike proved that FL Studio 2026 can provide usable Song positions and that Preverb can capture separate regions while producing scheduled audio on silent input. The current capture map exists only in memory, so closing the project loses the source material needed for later editing and regeneration; M1 makes progressive capture useful across same-machine project reopen.

## What Changes

- Persist captured stereo source regions and explicit coverage metadata in a managed, disk-backed local asset store, keeping file I/O off the audio callback.
- Save compact session/instance identifiers and asset references with the FL Studio project state, and restore the sparse map when the same project is reopened on the same PC.
- Preserve the existing distinction between captured audio, captured silence, and uncaptured gaps, including overlap-only recapture behavior.
- Provide a minimal timeline overview that makes captured regions and uncaptured gaps visible; this is not the full marker/waveform editor.
- Publish asset and manifest updates atomically and retain the last valid data if a write or reopen validation fails.
- Defer Save As/duplicate-project semantics, Pack Project, Relink, and cross-PC portability to M5. Host transport requests remain a separate optional probe; manual transport remains the supported workflow.

## Capabilities

### New Capabilities

None.

### Modified Capabilities
- `song-timeline-audio`: Extend sparse Song-position capture with durable assets, project-state references, same-machine restore, and a coverage overview.

## Impact

- `Source/Capture`: sparse-map representation, background capture worker, managed segment storage, versioned manifests, and asset validation/recovery.
- `Source/Plugin`: compact persisted session/instance identifiers and asynchronous restore/status reporting through JUCE plugin state.
- `Source/UI`: a minimal coverage/waveform overview for captured, captured-silence, and uncaptured ranges.
- Tests for atomic writes, coverage and sample alignment after reopen, corrupt/missing asset handling, and recapture preserving unaffected regions.
- Local storage under the user's application data directory is the proposed M1 working-cache location. This location is not a public compatibility contract; portable packaging and relocation are deferred.
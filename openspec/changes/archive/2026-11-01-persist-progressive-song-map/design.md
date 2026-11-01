# Design

## Context

See proposal.md for motivation and boundaries. M0 currently stores `SparseSongMap` segments as interleaved vectors owned by the processor; `CaptureWorker` drains a bounded queue and updates that in-memory map. Plugin state schema version 1 contains only test-tone settings. The existing map already distinguishes absent samples from covered zero-valued samples and replaces only recaptured overlap.

The behavior contract is in specs/song-timeline-audio/spec.md. M1 guarantees restore for the same saved FL Studio project on the same PC and Windows user profile. Save As/duplicate-project semantics and cross-PC relocation remain deferred.

## Goals / Non-Goals

**Goals:**
- Preserve committed captured audio and explicit coverage across plugin/process restart.
- Keep disk access and manifest processing off the real-time audio callback.
- Restore a project's own capture state without embedding PCM in plugin state.
- Expose a compact overview that distinguishes captured audio, captured silence, and uncaptured gaps.
- Retain the last valid committed map after interrupted writes or invalid new capture data.

**Non-Goals:**
- Pack Project, Relink, Save As/duplicate-project isolation, cross-PC transfer, or automatic cache cleanup.
- Convolution, swell renders, onset markers, full waveform editing, or a marker/region editor.
- Host transport requests. Manual transport remains the supported capture workflow.

## Decisions

### Use a per-instance local managed cache

Store assets under `%LOCALAPPDATA%/K7MA/Preverb/Projects/<project-id>/<instance-id>/`. Generate stable identifiers on first use and serialize them in the plugin state. In this M1 model, the identifiers scope the capture cache for that plugin instance; they are not derived from or presented as the FL Studio `.flp` path because a standard VST3 cannot reliably discover that path. The path is an internal working-cache convention, not a public compatibility API.

An adjacent sidecar was considered, but a standard plugin cannot reliably locate the project file, and sidecars complicate FL Studio Save As. A user-local cache makes same-PC reopen deterministic; project packing and relocation are separate later work.

### Store immutable audio chunks and a versioned manifest

Persist captured stereo PCM as lossless 32-bit-float WAV chunks, split at capture discontinuities and a bounded chunk duration. Record each chunk's absolute start sample, frame count, sample rate, channel count, revision, and checksum in a versioned JSON manifest. Store coverage intervals separately so captured silence can be represented as covered without storing a large zero-filled WAV, while uncaptured gaps have no coverage entry.

Build manifest serialization/parsing as a headless, testable component. Validate paths as relative to the instance cache and reject traversal or malformed records. Keep any waveform peak data in the manifest or a rebuildable cache; it must not be the authority for capture coverage.

### Commit updates by publishing new data before replacing the manifest

The capture worker groups contiguous queued blocks into bounded chunks. It writes each new chunk to a temporary file, closes and validates it, then atomically renames it to an immutable revisioned filename. It writes the replacement manifest to a temporary file and atomically publishes the manifest only after every referenced chunk is ready. The prior manifest and its assets remain valid until publication succeeds. Startup ignores incomplete temporary files; M1 does not purge orphaned assets automatically.

This favors immutable revisions plus one manifest swap over in-place mutation: a crash can leave unused files, but cannot make the last committed map point at a partial replacement. Disk-full or I/O errors leave prior data intact and report the new range as uncommitted.

### Keep persistence and restore off the audio callback

The callback continues to copy dry blocks into the bounded SPSC queue only. A worker handles chunk assembly, hashing, WAV writes, manifest publication, and restore validation. Queue overflow remains uncovered and is reported; it must not become captured silence. Clearing capture requests a worker-side manifest update and does not synchronously delete files on the callback.

On project load, plugin state supplies the cache identifiers and manifest revision. The worker validates and loads the sparse index asynchronously. Until restore completes, the UI reports Loading; missing or corrupt referenced chunks report an unavailable/stale state and are never substituted with captured silence. Valid, unaffected regions remain available.

### Version plugin and manifest state independently

Advance the plugin-state schema from version 1 to version 2, retaining the existing tone settings and adding cache identifiers plus the committed manifest revision/reference. A version 1 state restores tone settings and initializes an empty capture map. Keep a separate manifest schema version so asset-format evolution does not depend on host project-state migrations. Do not serialize PCM or large coverage arrays into the plugin state.

### Make the overview a coverage-first view

Render the overall captured timeline and compact min/max waveform peaks from committed chunks. Use distinct visual states for captured audio, captured silence, and uncaptured gaps. The overview shares the absolute Song position scale; it does not add marker editing, paging, or zoom/navigation beyond the minimum needed to inspect progressive coverage. Peak data can be regenerated from valid WAV chunks and never implies coverage by itself.

### Treat sample-rate changes as incompatible map revisions

The manifest records the sample rate used to capture its map. If FL Studio opens the project at a different rate, preserve the existing assets and mark the map stale; pause further mapping rather than merging samples whose absolute sample indices now refer to a different timebase. M1 does not resample or mix multiple sample rates in one map. The user can explicitly clear the map and recapture at the new rate. Dry passthrough and independent scheduled-tone behavior remain available.

## Risks / Trade-offs

- [The user-local cache is unavailable under another Windows account or PC] -> Keep M1's guarantee explicitly to the same PC/user profile; defer Pack Project and Relink to M5.
- [Two copied projects restore identical cache identifiers] -> Treat duplicate-project isolation as out of scope, detect a cache/manifest conflict when possible, and never overwrite committed chunks in place.
- [Disk fills or a chunk/manifest write fails] -> Keep the previous manifest and assets; report the range uncommitted and preserve the map's prior coverage.
- [A cache asset is missing or fails checksum validation] -> Show an unavailable/stale state for the affected range; do not silently mark it as silence.
- [The project sample rate changes] -> Preserve old assets, mark the map stale, and pause mapping until the user explicitly clears and recaptures at the new rate; do not silently resample or reinterpret sample indices.
- [Large maps exceed memory if all PCM is restored eagerly] -> Keep PCM disk-backed and load only the index, overview peaks, and bounded active buffers.
- [Manifest and plugin state refer to different revisions after an unsaved project close] -> Treat plugin state as the authority for which manifest revision belongs to the project; leave newer unreferenced revisions recoverable rather than guessing or silently adopting them.

## Migration Plan

Read version 1 plugin state using the existing tone fields and default to no capture references. New states write version 2 identifiers and references. No existing captured maps need migration because M0 maps were process-local and not persisted. On first M1 capture, create the managed cache and publish an initial manifest; on failure, leave the plugin in its existing empty-map state and display the error.
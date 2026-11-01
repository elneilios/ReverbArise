# Tasks

## 1. Managed Asset Format

- [x] 1.1 Define a versioned manifest for instance IDs, sample format, coverage intervals, capture revisions, chunk references, and checksums; verify round-trip, malformed-manifest, and unsafe-relative-path tests pass.
- [x] 1.2 Implement lossless stereo 32-bit-float WAV chunk read/write and validation; verify synthetic sample values, absolute start positions, sample rate, and channel metadata round-trip exactly.
- [x] 1.3 Publish chunks through temporary files and atomically replace the manifest only after validation; verify simulated interruption or write failure leaves the prior committed manifest and its assets readable, and document the cache layout.

## 2. Persistent Capture Pipeline

- [x] 2.1 Extend the background capture worker to assemble bounded contiguous chunks and persist them without file I/O on the audio callback; verify tests cover queue overflow, discontinuity boundaries, and captured silence versus uncovered gaps.
- [x] 2.2 Persist overlap-only recaptures as new revisions while preserving unaffected ranges; verify tests reopen the resulting manifest and compare samples before, inside, and after the recaptured interval.
- [x] 2.3 Retain committed data on clear/write failure and avoid deleting referenced assets; verify failure-path tests preserve the last valid map and document recovery status.
- [x] 2.4 Detect a host sample-rate mismatch against the committed manifest, preserve existing assets, mark the map stale, and pause mapping until explicit clear/recapture; verify mismatch and recovery tests.

## 3. Plugin State and Restore

- [x] 3.1 Extend plugin-state serialization with stable cache/instance identifiers and manifest references while preserving tone settings; verify version-1 state migrates to an empty capture map and version-2 state round-trips.
- [x] 3.2 Restore the referenced sparse index and assets asynchronously after project load; verify tests cover loading/ready states, missing or corrupt chunks, unaffected valid ranges, and never interpreting missing data as captured silence.

## 4. Coverage Overview

- [x] 4.1 Generate or rebuild compact waveform peaks from committed chunks and display the absolute Song timeline with distinct captured-audio, captured-silence, and uncaptured states; verify peak/coverage tests and inspect a non-contiguous map in FL Studio.
  - Closed by explicit user acceptance on 1 November 2026. Implementation, automated checks, and supplied screenshots support acceptance; no additional host inspection was performed by the agent.
- [x] 4.2 Update development documentation with same-PC/same-user reopen behavior, cache retention, and unsupported relocation cases; verify the documented validation workflow matches the implementation.

## 5. FL Studio Integration Gate

- [x] 5.1 In FL Studio 2026 Song mode, capture bars 86-90 then bars 1-4, save and reopen the same project on the same PC, and verify both ranges and the unmapped gap are restored at the same sample positions.
  - Closed by explicit user acceptance on 1 November 2026, following confirmation of non-contiguous capture save/reopen. The exact bars 86–90 / 1–4 sequence was not independently verified by the agent.
- [x] 5.2 Recapture an overlap, close/reopen the project, and verify only that overlap changes while other committed ranges remain intact; record the host configuration and result in the M1 validation report.

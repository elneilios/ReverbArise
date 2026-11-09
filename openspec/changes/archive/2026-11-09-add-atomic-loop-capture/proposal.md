# Proposal

## Why

[Issue #124](https://github.com/elneilios/ReverbArise/issues/124) turns the timing contract from [#123](https://github.com/elneilios/ReverbArise/issues/123) into an explicit one-take Learn workflow. A compatible host can supply tempo, 4/4 meter and the required sample-offset MIDI markers. VCV Rack remains the initial integration test. A musician must be able to try a replacement while the last accepted source and arrangement remain safe.

## What Changes

- Add **Loop Mode** in Learn → Source Capture. The toggle selects the next capture type independently of the accepted Song or Loop source.
- Add an **Arm** toggle for one Loop take. Switching it off cancels a pending take. Duration starts at a fresh verified phrase origin and stops at the configured musical boundary. End signal shares settlement rules and requires a separately verified end request. On the approved Rack wiring, raw gate release leaves End signal unavailable.
- Publish one validated phrase source, timing record and revision together. Cancelled, interrupted, failed and superseded attempts preserve the prior accepted source and creative state.
- Persist the next capture preference and accepted source independently. Restore old Song projects using their existing identities and keep any in-flight arm or provisional audio out of saved state.

## Capabilities

### New Capabilities

- `loop-phrase-capture` governs Loop Learn admission, sample boundaries, atomic source replacement and the minimum state needed to restore an accepted take.

### Modified Capabilities

None. The existing Song capture and workflow contracts continue to govern Song.

## Impact

The capture worker and managed asset store gain a phrase-relative transaction. Processor state and Learn bindings distinguish pending intent from accepted source. Port the verified mechanisms and tests from the spike onto `feature/loop-mode`. Live Loop playback, shared creative editing, and broader recovery and portability remain in [#125](https://github.com/elneilios/ReverbArise/issues/125), [#126](https://github.com/elneilios/ReverbArise/issues/126) and [#129](https://github.com/elneilios/ReverbArise/issues/129).

# Tasks

## 1. Investigation and diagnostics

- [ ] 1.1 Add the developer diagnostics ring (host position, sample/PPQ, playing/loop/discontinuity, eligibility, skip/invalidate reasons, coverage/revision, render transitions); verify with unit tests that the callback path performs no allocation/locking.
- [ ] 1.2 In FL Studio, log Song vs Pattern playback (start/stop/seek/loop) and record the actual `PositionInfo`/VST3 context; confirm the revision-invalidation path that removes swells. Document whether any supported discriminator exists.

## 2. Eligibility policy

- [ ] 2.1 Implement `TransportEligibility` with separate capture/wet decisions and reason codes; unit-test Song, Pattern, Unknown, no-position, stopped, offline, seek, loop, bar-1 and short-loop cases (none may imply Pattern).
- [ ] 2.2 Add the saved per-instance Song-mode confirmation (default unconfirmed, state migration) and minimal UI/status; test roundtrip and older-state migration.

## 3. Capture integrity

- [ ] 3.1 Gate `processBlock` capture admission and wet scheduling on the policy; verify dry pass-through is bit-identical in every mode.
- [ ] 3.2 Make `CaptureWorker`/`CaptureSession` replacement transactional (policy epoch per block; partial, interrupted, ineligible and silent-uncaptured spans never replace or invalidate); test Song→Pattern→Song, stop/restart, ambiguous mode, silent input, seek/loop, non-contiguous coverage, leading silence.
- [ ] 3.3 Retain last-good wet until replacement render succeeds, including failed/interrupted captures; test capture/render transitions.
- [ ] 3.4 Test multiple simultaneous instances with independent transports and assets.

## 4. Documentation and acceptance

- [ ] 4.1 Document the fallback, reason codes and diagnostics; add an FL Studio acceptance checklist (Song → Pattern → Song, restart, reopen project).

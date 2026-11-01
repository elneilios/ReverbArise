## Why

In FL Studio, starting **Pattern** playback while enabled ReverbArise mixer instances hold Song-mode captures and prepared swells can (a) replay the Song-locked wet swells and (b) subsequently make them disappear (issue #1). Nothing in the code distinguishes Song from Pattern transport, so pattern-relative positions are treated as absolute Song positions. This risks loss of project-critical captured material, so it is a capture-integrity bug, not a UI bug.

## Investigation findings (code trace, no code changed)

- `PreverbAudioProcessor::updateHostPositionDiagnostics` reads only JUCE `AudioPlayHead::PositionInfo` (time in samples, PPQ, BPM, signature, playing, looping, loop points). JUCE exposes no Song/Pattern flag. The existing docs (`docs/automatic-swell-workspace.md`, product vision) already state the standard VST3 API does not identify FL Studio Pattern/Song mode.
- `processBlock` admits capture on `mappingEnabled && canMap() && isPlaying && samplePositionAvailable`. Any host that reports `playing` with a valid sample position is accepted; Pattern playback reports pattern-relative positions starting near 0.
- `CaptureWorker::handleBlock` treats the position jump as a `continuity` discontinuity, flushes, and `flushPending` calls `CaptureSession::commit(pendingStart, ...)`, which replaces whatever is stored at those absolute positions. Pattern audio (often different, or silent/zero when the pattern lacks the source) therefore overwrites Song bars 1..N. When the content differs beyond change tolerance the manifest revision increments and `commitHandler` -> `SwellEngine::captureCommitted` invalidates dependent swells (source fingerprints no longer match, renders go stale/re-render). That is the most likely disappearance path: **source replacement and revision invalidation, not source deletion**. Silent blocks are committed as silent ranges (`pendingSilence`), so zero signal is also treated as permission to replace prior data. This path still needs confirmation with diagnostics (task 1).
- `SwellEngine::processTransportAudio` -> `TransportPlayback` mixes wet whenever `liveEnabled && time.playing` and mapping is valid. Pattern position 0..N falls inside Song-locked swell intervals, which explains the unexpected replay.
- Capture and playback gating are not separate, and there is no central eligibility decision or reason reporting.

## What Changes

- Add a central, pure, unit-testable **transport eligibility policy** (`TransportEligibility`) producing independent `captureAllowed` / `wetAllowed` decisions plus a reason code, from a host-mode input of `Song`, `Pattern` or `Unknown`.
- Host mode: only a **supported** source may yield `Song`/`Pattern` (e.g. an FL Studio VST3/host-specific extension, if the spike below finds one). Never infer Pattern from bar 1, short loops, seeks or repeated positions. Default and fallback is `Unknown`.
- **Unknown fallback (safe default):** automatic capture does not replace or invalidate existing committed source (only extends coverage into uncovered ranges, or stays idle once any source exists) unless the user opts in via a minimal per-instance **Song mode confirmed / Arm mapping** control. Wet playback in `Unknown` follows a saved, documented setting (default: preserve current behaviour of playing prepared wet only while the user has confirmed Song mode; otherwise silent wet with dry pass-through and a visible status). Automatic-by-default remains the long-term workflow once reliable detection exists or the user has confirmed Song mode.
- **Reliable Pattern:** suspend capture and wet scheduling; dry passes through unchanged.
- Make capture commits **transactional**: a replacement of already-committed ranges is applied only for blocks admitted by the policy for the whole pending span; partial/interrupted/ineligible/zero-uncaptured spans never replace valid ranges; revision invalidation happens only after a verified eligible commit. Render cache keeps last-good wet until a replacement renders successfully.
- Eligibility is evaluated per processor instance (per-instance state; no shared globals), and decisions are made on audio thread with atomics only; the worker re-validates by tagging blocks with the admitting policy epoch.
- **Developer diagnostics:** bounded lock-free ring of structured events (host position, sample/PPQ, playing/loop/discontinuity, eligibility, skip/invalidate reasons, coverage and revision changes, render transitions), surfaced in the existing diagnostics panel. No logging or allocation in the callback.

## Capabilities

### Modified Capabilities
- `song-timeline-audio`: capture admission and replacement are gated by transport eligibility; transactional commit and explicit invalidation reasons.
- `transport-swell-playback`: wet scheduling is gated independently by transport eligibility; last-good render retention.

## Impact

Touches `PluginProcessor` (`processBlock` admission), `CaptureWorker`/`AudioCaptureQueue` (policy epoch on blocks), `CaptureSession::commit` (replace-vs-extend), `SwellEngine`/`TransportPlayback` (wet gate), a new `Source/Core/TransportEligibility.h`, session/state persistence for the Song-mode confirmation, and diagnostics UI. Adds unit tests and an FL Studio acceptance checklist.

Out of scope: Pattern-mode mapping, Playlist clip/Pattern ID identification, manual capture as the permanent default, UI/layout refinement, tempo maps.

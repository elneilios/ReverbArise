## Context

ReverbArise maps source audio to absolute host Song positions (Song-mode-only MVP, non-contiguous sparse coverage, automatic-by-default). The standard JUCE/VST3 playhead cannot say whether FL Studio is in Song or Pattern mode, and in Pattern mode reports plausible pattern-relative positions. See proposal for the traced capture-replacement and wet-scheduling paths.

## Decisions

1. **Central policy.** `TransportEligibility::evaluate(HostMode, hostObservation, instanceState) -> {captureAllowed, wetAllowed, reason}`. Pure function, no I/O or allocation, used from the audio thread; reasons are an enum (`SongConfirmed`, `PatternDetected`, `ModeUnknownProtected`, `NoPosition`, `NotPlaying`, `Offline`, `RateStale`, ...). Capture and wet are decided separately.
2. **Host-mode source (spike first).** Task 1 verifies, in FL Studio, what `PositionInfo` and JUCE's VST3 wrapper (`IVst3 ProcessContext` flags, and any `IPlugView`/`IHostApplication`/FL-specific extension) report in Song vs Pattern. Only a documented, reliable discriminator is adopted. If none exists the mode is always `Unknown`; heuristics are explicitly prohibited.
3. **Unknown-mode safe fallback.** A minimal per-instance saved control "Song mode confirmed" (arm). Unconfirmed: dry pass-through; capture may only *extend* coverage into uncovered ranges when no committed source exists yet (first capture), never replace committed ranges or bump revisions of existing swells; wet playback is not scheduled and status states why. Confirmed: current Song behaviour. This keeps automatic operation after one confirmation and never destroys data silently. Alternative considered: manual capture as default – rejected (contradicts automatic-by-default).
4. **Transactional replacement.** `CaptureWorker` carries the admitting policy epoch/permission per block; `flushPending` commits replacement only if every block in the pending span was admitted with replace permission and the span is not an interrupted partial (discontinuity or policy change flushes without replace). Zero/silent input counts only when eligible; it never erases ranges when ineligible. Revision invalidation callbacks fire only from a verified eligible commit.
5. **Render retention.** On stale or ineligible conditions keep the last-good wet asset audible-capable (but wet-gated by policy); replace only after successful render with matching fingerprint.
6. **Per-instance isolation.** All state lives in the processor/session; no static mutable state; tests instantiate several processors/engines with interleaved transports.
7. **Diagnostics.** Fixed-capacity SPSC ring of POD events written from the audio thread and drained by the UI/dev panel; skipped/invalidated actions always carry a reason code. Overflow counts are reported.

## Risks / Trade-offs

- Without a reliable discriminator users must confirm Song mode once per instance; documented and surfaced in status.
- Extra gating must not regress seeks, loops, non-contiguous coverage, or leading silence: covered by regression tests.
- Persisted confirmation applies only to the saved instance and defaults to unconfirmed for older state versions (migration test).

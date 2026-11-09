# Design

## Context

The current processor uses JUCE's playhead observation for Song, with the existing capture and live playback checks for absolute samples, PPQ, rate, tempo and meter. The production VST3 does not request MIDI input. Song's one-sample origin tolerance is a separate guard from the adjacent phrase intervals observed with CLKD.

The [spike handoff](https://github.com/elneilios/ReverbArise/blob/codex/116-loop-mode-spike/docs/development/loop-mode-production-handoff.md) links the isolated probe sources and evidence. The Note Gate comparison found 1,476/1,476 short-pulse edges at each tested actual callback size of 128 and 256 frames at 48 kHz. CLKD produced 336,350/336,351 frames per four-bar phrase at 137 BPM and 164,571/164,572 at 280 BPM. The 15 aligned wet downbeats used one test swell and a 2,048-sample lead-in. The lifecycle trace from a separate fixture observed a cable disconnect as note-off while dry remained high, and a processing pause without a processor lifecycle event.

## Goals / Non-Goals

**Goals:** Produce a reproducible support decision and an admission contract that downstream capture, scheduler and UI changes can exercise against the same observations. Prove the contract with focused trace and policy cases.

**Non-Goals:** Production MIDI bus changes, a new user-facing configuration surface, capture publication, wet playback, session migration and FL Studio Loop integration. This change establishes the policy and its reference verification for those consumers.

## Decisions

1. **Use the measured Rack Note Gate route.** Start with Rack Free 2.6.6, Host-FX 2.1.0 and Impromptu CLKD 2.5.0 at 48 kHz. Record the Rack buffer setting and actual block sizes separately. Drive the dry phrase and references from the same clock and reset. Use equivalent signal paths where possible, then measure signed audio-to-event offsets. A previous capture fixture introduced a one-sample audio delay through its VCA. Measure that relationship on the intended wiring before assuming zero offset or adding compensation. Ordinary parameter automation lost short pulses and delivered held edges at block granularity, so it cannot substitute for sample-positioned Note Gate evidence.

   | Input | Candidate MIDI note | Role |
   | --- | ---: | --- |
   | Capture gate | 14 | Verified low-to-high start and separately qualified release |
   | Phrase | 12 | Recurring phrase origin marker |
   | Bar | 19 | Verified bar boundary |
   | Beat | 21 | Beat timing |
   | Reset | 22 | Revoke the current reference when this input is configured and verified |
   | Cue | 16 | External performance request |
   | Stop | 17 | External Perform Stop |

   Verify channel, edge interpretation, duplicate behavior and offsets against the hosted trace. Keep equal-offset roles available together so a bar can qualify a same-sample end request. Keep reset, capture completion and Perform Stop distinct. Bound event work per callback in the probe and reference contract. Collect bounded observations in processing and write traces outside the callback. An overflowing trace or invalid event cannot serve as positive support evidence.

2. **Report authority by operation.** Use a coherent per-instance snapshot of each capability, its state, the evidence epoch and a reason. Unsupported means the route or condition is outside supported evidence. Not configured means required setup is missing. Not verified means configured inputs have yet to establish authority. Sync lost means previously verified authority was revoked. Ready means the current epoch meets that capability's rules. For example, “Declare the meter to use Bars and Beats,” “Connect a phrase marker that repeats at the captured length,” and “Reference lost. Restore the clock, then Cue again” identify different next steps. Carry the existing Song admission result through this boundary without changing it. Only the relevant capability can admit an action.

3. **Anchor phase to observed markers with bounded validation.** Verify a declared meter and source origin against coherent beat, bar and phrase observations where the route uses them. Acquire lock over repeated complete intervals and require a cadence compatible with the intended phrase. Start evaluation with the adjacent integer lengths supported by the measured stable tempo, validated against a fixed reference for the epoch. Avoid a rolling tolerance that can walk with drift. Check the multi-interval trend and missing-marker deadline against the verified cadence. An expected marker can bound scheduling, but the observed marker owns the actual boundary. Use long-run and injected drift cases to establish and publish the lock window, loss threshold and reacquisition criteria. Leave wet correction and cross-cycle composition to #125.

4. **Require a trustworthy capture end.** Re-arm requires a fresh low-to-high gate. An attributable end request under the Rack musical policy ends on the first verified bar at or after the request, with an exclusive end. The prior probe shows that a cable unplug can generate the same note-off as a deliberate release. Continued clock alone does not distinguish them. Verify a wiring constraint or observable signal that qualifies release, or require a separately verified end request and withhold the ambiguous route. Capture start, bar, phrase and Perform Stop remain separate. If callbacks pause, process only evidence actually received. On observed interruption or reactivation, revoke stale authority and require fresh observations. Record any pause the host does not make observable as a support limitation.

5. **Treat support as a decision from evidence.** Reuse the existing generators and analyzers, extending only the missing cases on the intended patch or a reproducible equivalent. Archive fixture results by configuration and distinguish fixture replay, native Windows regression results and real-host measurements. Record owner approval and restrictions for each passing route. A no-go or an untested configuration remains unavailable. FL Studio Loop MIDI/automation evidence belongs to #127.

## Risks / Trade-offs

- A perfectly periodic marker can be consistently early or late relative to dry. Record signed offsets and verify the dry path through reset, restart and reload.
- The earlier short trace establishes rounding behavior but cannot establish long-run phase stability. Measure a longer fractional-tempo run and include missed, extra and drifting marker cases.
- Startup-high and disconnect can resemble user input. Verify lifecycle transitions in the host and fail closed where the route offers insufficient evidence.

## Migration Plan

Use isolated probes and existing Song regression checks against the current production baseline. There is no plugin or saved-state migration in this slice. Pass the approved contract and evidence to the dependent issues through their OpenSpec changes.

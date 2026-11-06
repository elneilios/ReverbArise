# Design

## Context

See proposal.md for motivation and the transport-swell-playback delta for the recovery contract. Inspection at dbfdb18 found:

- `PreverbAudioProcessor::prepareToPlay` and `releaseResources` already request a live-clock reset. `processBlock` fetches fresh host timing before calling the engine. There is no processor-specific `processBlockBypassed` override.
- `LiveTransportClock` retains a playing-run reference until a valid Stop, session replacement or explicit reset. Invalid timing does not create a run. Existing #73 tests intentionally forbid rebasing on seeks, loops and internal Live toggles.
- `TransportPlayback::process` reads PCM using absolute Song sample positions, detects discontinuities and applies bounded joins. Worker preparation has separate clock/session validity and retry gates.
- `tests/transport_playback_test.cpp`, `tests/LiveClockEngineCases.h` and the processor harness in `tests/swell_ui_test.cpp` offer existing clock, managed-asset, fake-playhead and allocation checks.

The reported failure has not been independently reproduced. Loop recovery alone does not establish which gate is stale or whether FL Studio calls lifecycle hooks. The design therefore fixes the reproduction/decision procedure rather than prescribing an unproven clock reset.

## Goals / Non-Goals

**Goals:** recover current-position wet through existing scheduling and preparation mechanisms; distinguish host lifecycle events from musical discontinuities; preserve #73 clock protections and durable state.

**Non-Goals:** new controls, blanket timing-policy relaxation, wall-clock heuristics, host transport manipulation, source recapture, rerendering solely for activation, offline timing redesign or asset/schema migration.

## Decisions

1. **Reproduce at the processor boundary before editing behavior.** Use a fake playhead and managed prepared assets, process audible blocks, omit callbacks while advancing the playhead, then resume both with and without release/prepare. Separately exercise the framework host-bypass path. Record actual callback/lifecycle evidence, build identity and FL Studio control when available. Identify the first failing admission/preparation gate using existing observations. A synthetic sequence is not evidence of FL Studio's exact behavior. If these sequences pass, obtain a concrete host trace before inventing a repair.

2. **Repair the demonstrated stale state at its owning boundary.** Reuse existing clock reset requests for confirmed lifecycle invalidation, absolute-offset mixing for resumed callbacks, and bounded worker retry for stale coverage/publication. Change only the mechanism shown to fail, keeping retained PCM reusable when its dependencies remain valid. Do not add a generic reset on sample jumps: those are also normal seeks and loops. Any new activation authority must come from a demonstrated host/framework lifecycle signal; if the host exposes no distinguishing signal and recovery needs a changed mapping policy, stop for owner review and revise this design/spec before implementation.

3. **Preserve publication and audio ownership rules.** Reactivation must invalidate obsolete runtime authority without clearing the saved model or deleting assets. Worker work crossing the boundary must validate current session/run/dependencies before publication and remain retryable when superseded. Audio thread changes must be bounded and avoid allocation/deallocation, locks, disk access and logging. Reuse the current <=5 ms transition; never replay missed lead-in.

4. **Verify with an independent sample oracle.** Use nonconstant identifiable two-channel PCM so a restarted cursor cannot pass. Cover an interior join, later swells before wrap, repeated cycles, warm and cold positions, irregular block sizes, seeks/loops, existing incompatible timing and internal disable controls. Snapshot persistent definitions and verify source/render identities/assets before and after. Extend existing tests instead of creating a separate parallel engine.

## Risks / Trade-offs

- [Host enable controls may skip callbacks, bypass, suspend or rebuild the instance differently] -> distinguish each supported path in automated coverage; record actual host evidence and unresolved coverage honestly.
- [An apparent recovery fix could silently rebase genuine timing drift] -> retain #73 negative tests and require explicit owner review for any timing-policy exception.
- [Worker preparation may race reactivation or restore] -> reuse existing run/session/dependency guards and held-publication test hooks; reject obsolete work without certifying stale readiness.
- [Cold PCM cannot be immediately audible] -> keep the existing bounded preparation/missing-coverage contract and test current-time recovery rather than demanding replay.

## Migration Plan

No persistent migration is expected. Implement and validate on the issue branch, update living transport guidance and guide currency, then open one PR for owner review. Rollback is a code revert retaining all saved projects and assets. A local FL Studio build, if required for the final smoke, must use the install-test-vst3 skill; release packaging is outside this change.

### Authorized validation follow-up

On 2026-11-06 the owner authorized investigation of a baseline native UI crash
blocking final validation. The MSVC broad UI test has a 1 MiB executable stack
reserve and a measured Debug main fixture frame of `0xBA888` bytes, before nested
native UI/graphics calls. A copied executable changed only to an 8 MiB stack
reserve progresses beyond the original `0xc00000fd` failure without replacing
the renderer or driver. Apply that reserve only to `preverb_swell_ui_test`,
document the measured test-harness constraint and require its normal CTest pass.
No production UI, plugin stack or audio scheduling behavior is changed by this
adjacent test-infrastructure repair.

The subsequent native-popup validation failure is addressed only in its test
setup: require the native peer and owning process to have focus before invoking
the real asynchronous reset menu. JUCE component keyboard focus alone does not
satisfy the menu's foreground-process guard. Keep every existing menu/reset
assertion and retain separate full-suite versus focused-rerun evidence.
On Windows the fixture explicitly requests foreground activation for its own
peer because JUCE's peer helpers only bring the window to the top and set
thread focus; they do not call `SetForegroundWindow`.

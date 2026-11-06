# Design

## Context

See proposal.md and the host capture in `docs/validation/2026-11-05-issue-73-clock-investigation.md`, final section. Baseline is `e803323` on the existing #73 branch, with the accepted offline repair already integrated. The installed `02a3a17` reports source origin approximately -0.262295 samples, stable live-run origin +15.737705 samples, audible wet and no retained run-2 timing rejection. This establishes the grid defect; it does not reproduce the older live rejection or certify every callback/map position.

`HostGrid::make` currently requires `transportMappingMatches(accepted, now)`. Timeline availability, drag setup, viewport insertion and inspector knob/wheel paths call it separately. Inspector exact entry and stale invocation checks also use their captured timing. `SwellTimeline::setData` cancels obsolete gestures using both accepted and host mapping. These consumers must share a decision; changing only the warning or availability flag would leave edits rejecting.

`LiveTransportClock` already publishes a bounded value snapshot, separately from telemetry. It pins a valid playing run, checks later observations with the existing one-sample guard, and carries session/reset/run lifecycle. Worker preparation uses this authority; `EditorSnapshot` currently exposes diagnostic observations but no dedicated grid clock authority. The diagnostic stream is deliberately observational and must not become admission authority.

## Goals / Non-Goals

**Goals:** recognize validated fixed-tempo run offsets for grid availability; retain one captured editing grid; keep every affected interaction consistent and lifecycle guarded; avoid render readiness dependencies and changes to audio admission.

**Non-Goals:** re-anchor the captured grid to each live observation, silently move a saved target, change live-run recovery, repair the historical unobserved rejection, support tempo maps, or revise UI geometry/control styling.

## Decisions

### 1. Keep grid coordinates and validation reference distinct

Extend grid construction with an explicit optional current run context. The grid's quantization origin/step remain derived from accepted capture timing. A playing context authorizes availability only when its session/reset/run ownership is current, its own clock gate is admitted, accepted/run/observed rate, tempo and meter agree, and the current host tuple matches the run reference under the unchanged mapping helper. A rejected playing context has precedence: never fall back to source-origin matching to hide a within-run failure.

Without a playing context, preserve strict stopped/source-origin validation and the existing empty-session bootstrap. An unavailable/incoherent playing context is unavailable, rather than a fabricated valid run. Do not add a "16 sample" special case or choose a maximum permitted source/run offset: compatibility is established by independent authority, not offset magnitude. Retain overflow, missing sample position, invalid division and conversion checks. Existing strict callers remain strict by default.

An alternative of using the latest host tuple as the grid anchor would move grid points across Play/Stop and disagree with accepted position conversions. An arbitrary tolerance would admit drift without establishing clock identity. Both are rejected.

### 2. Expose control authority without using diagnostics as control

Expose a value-only optional clock snapshot through the engine's editor/control boundary using the existing dedicated clock publication. Validate current session and reset with the same ownership used by its worker consumer. Do not use current/causal diagnostic samples, audible `liveActive`, worker preparation text or a successful render as proof. Stale or bounded-read failure remains unavailable.

No callback logging, locks, strings, new dynamic state, new tracing or extra clock publication is required. Control/UI processing may carry a small copied context. Avoid a Core-to-Playback include cycle: adapt the validated clock at the engine/UI boundary to the core grid's value input, rather than making core grid math depend on engine/UI or diagnostic types.

### 3. One decision across all existing consumers

Route timeline availability/evidence, drag setup, snapped keyboard actions and target insertion, viewport helpers and inspector knob/wheel invocations through the same authority/result. Pin the accepted grid and current ownership in an edit invocation. Revalidate on publication against current source/selection/session/mode and current clock; a real run/timing change cancels obsolete gestures. Ordinary progress through the same run must not cancel a gesture. Play/Stop and new-run boundaries can require restarting an already active gesture without changing accepted grid points or saved values.

Retain direct exact numeric/sample editing, reset, Ctrl bypass, Shift fine behavior, canonical bounds and unrelated parameter protections. Audit inspector musical conversion and ruler inputs so captured musical labels and snapped anchors use the accepted grid; keep the footer's actual host position informational. Do not re-anchor unrelated Learn/Export consumers or implement new musical conversions.

### 4. Explain accepted offsets in existing Diagnostics

Grid evidence must identify the accepted editing anchor and the separate validation reference, including authority/run identity and source/run/host deltas. A legitimate source-to-run offset is informational; it must not produce the Snap-unavailable warning when the common decision admits. Actual missing prerequisites or rejected drift retain actionable feedback. Update the current core/binding tests that intentionally assert old rejection for stable offsets; retain strict-default API regression cases.

Reuse Snap toggle, grid selector, timeline, KnobValue, typed fields, accessible context help, diagnostic text and existing Copy diagnostics ActionButton. No new control family, token, typography, layout or visual variant is proposed. If implementation requires one, stop for scoped owner review with a directly launched native preview; do not modify standards/debt to bypass the gate.

### 5. Fit the tempo-map successor without implementing it

Issue #71's existing plan is available in the original checkout at `C:/git/k7ma-preverb/openspec/changes/support-existing-tempo-maps/`; read it in place, do not copy or change that other checkout. Its design separates accepted absolute audio, per-run clock validation and coverage-aware musical conversion. This repair uses the existing fixed-tempo implementation of that separation. It adds no persisted evidence or map schema, no constant-BPM interpolation over steps/ramps and no assertion that instantaneous tempo changes are safe under today's audio policy. Future #71 can supply a verified local mapping behind the same distinction.

## Risks / Trade-offs

- Independently read UI/clock tuples can cross a transition -> require a current coherent run and matching host tuple; fail transiently unavailable rather than claim synchronization or authorize stale edits.
- A permissive fallback can mask real drift -> rejected playing authority wins even when a host observation matches the stored origin; test that exact historical pattern.
- Grid edits could appear shifted relative to the host by the source/run offset -> explicitly retain captured-grid semantics and use independent integer/decimal oracles for coordinates; never silently relabel existing assets or promise new absolute alignment evidence.
- Shared inspector paths may bypass the fix or stale guards -> inventory all factory/invocation consumers and exercise canvas/knob/wheel/keyboard/insertion equivalence and lifecycle cases natively.
- Grid access could accidentally require prepared wet -> test missing IR/render, no active wet and disabled output with valid timing.
- Concurrent #58 UI work shares editor files -> keep edits to binding/context paths, preserve its pending geometry and coordinate integration using existing owner authorization where necessary.

## Migration Plan

No state migration or asset rewrite. After a fresh Apply request, implement on the current local #73 branch, preserving the installed build and its rollback bundle until verification completes. Use focused native core/UI/processor tests during implementation and the supported Debug/Release wrappers for final regression gates. Keep fixture checks distinct, record any intermittent failures/recovery, run UI standards and maintain-user-guide currency, and update relevant living guidance/evidence.

Keep source/sample audio, legacy save/reopen and offline offset/drift regressions passing. Reuse valid caches and build locking. A final owner check should be one brief same-project Snap availability plus one snapped edit during Play, after the automated cases pass; do not repeat accepted bounce/audio collection. Prepare an updated test bundle with exact build identity; install through the installation skill only within owner-authorized delivery and with hosts closed. No push, PR, canonical-spec sync, archive or issue closure. Rollback is code/plugin replacement, not project recapture.

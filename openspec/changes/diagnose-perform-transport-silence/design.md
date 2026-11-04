# Design

## Context

See proposal.md for motivation and issue #53 for owner observations. At the inspected main baseline, `TransportPlayback::process` rejects a publication when `transportMappingMatches` or callback range validation fails. `badMapping` is play-gated, so clearing on Stop does not prove a new publication. `PluginEditor.cpp` replaces the underlying status with a generic awaiting message when `liveIncompatible` is true.

`TransportTime.h` compares exact rate/tempo/meter and inferred PPQ-zero origins within one sample. Existing transport-time tests cover this boundary and unsupported values. Processor host observations already use bounded sequence-checked atomic snapshots; mixed reads are not an established cause. `prepareTransport` validates session mapping before preparing and again before publication, uses revision/window fast paths and retries on later worker service. These are focused inspection points, not diagnosed defects.

Owner reports 61 swells, 48 kHz, 122 BPM, 4/4, enabled Dry + Wet at 0 dB. Screenshots round PPQ to three decimals and cannot establish a one-sample origin difference. Audition and Export success demonstrate distinct working paths, not live mapping compatibility.

## Goals / Non-Goals

**Goals:** Make the failed gate measurable without disturbing the instance; reproduce it deterministically; repair only the proven state transition and validate recovery and dependency integrity.

**Non-Goals:** Timing automation support, sample-origin migration, new mapping tolerance, scheduling architecture replacement, Perform composition changes, Learn capture-pass investigation or Generate UX. An unexpected migration or broader requirement returns to owner review.

## Decisions

### 1. Observe the actual admission decision

Extend fixed-size callback telemetry associated with the active publication, including callback observation sequence, validity, mapping reason, sample/PPQ/rate/tempo/meter, inferred origins/delta, callback count/range validity, publication revision/bounds, running/missing flags and workflow/live gates. Keep one bounded last rejection across Stop, identified as historical. Capture all correlated values from the same callback/publication; unavailable telemetry must remain explicitly unavailable.

Use bounded lock-free atomic publication or an existing safe fixed-capacity mechanism. A reader must never dereference callback-owned publication data after retirement. No strings, allocation, deallocation, locks, file operations, logging, waits or unbounded retry in the callback. Reader retries are bounded and may report unavailable evidence. Format sufficient numeric precision only on the worker/UI side.

Worker telemetry separately identifies source/session mapping, current/prepared revision, attempted window/loop hints, eligibility rejection, preparation/publication attempt and result sequence plus monotonic attempt time. Do not claim worker and callback snapshots are simultaneous. Prefer one last attempt/result and counters over a growing log.

Alternative: only display existing flags. Rejected because these cannot discriminate origin/rate/tempo/meter, invalid range and worker failure. Per-block text logs are rejected for real-time cost and unbounded storage.

### 2. Require causal evidence before modifying admission

First add diagnostics and test their coherence/read-only behavior. Use the preserved instance if available; a separate FLP copy is acceptable. Collect playing and stopped evidence across a known wet interval, and a working instance only if it adds discrimination. Never Clear, Recapture, reset renders, delete swells or toggle settings as a diagnostic repair.

Record the exact failed predicate, numeric/state values, expected behavior and deterministic reproduction before selecting the fix. Build a fixture with the observed or explicitly injected state and distinguish a demonstrated mechanism from unverified equivalence to the owner's instance. If evidence is still insufficient, stop behavioral edits and report the specific observation missing. Do not assume preparation throughput or loosen mapping tolerance.

### 3. Extend existing retry and validation mechanisms

Once the cause is established, repair its narrow owner in mapping/admission or preparation/publication state. Retain worker service, immutable publications, generation cancellation and bounded callback handover. Transient rejection must not mark a revision permanently prepared or suppress later service. Every retry rechecks current session/source/render dependencies and compatible mapping; complete data joins current Song time without replaying elapsed lead-in. Incompatible mappings stay unavailable, and ready status is never synthesized.

Alternative: periodically force all revisions or reset the session. Rejected because it hides failure, churns valid work and risks durable state. New thread/queue architecture is outside this plan.

### 4. Reuse the existing Diagnostics surface

Extend existing Settings/Recovery Diagnostics text and snapshot binding with existing theme tokens and text roles. No new library control family, token, visual variant or one-off exception is planned. Read the production style guide and UI standards before implementation. Include component/binding tests for current versus historical observations, reason formatting and lifecycle. Run the UI standards guard without changing its debt inventory. If a new/revised family or exception proves necessary, prepare and launch one native preview and obtain scoped owner approval before wider rollout. Preserve #46's composition and worktree.

### 5. Verify lifecycle and output, not just flags

Extend existing `transport_time_test`, `transport_playback_test`, `swell_engine_test` and processor/workflow coverage as appropriate. Verify stopped-to-playing with 61 eligible members, first and later windows, seeks/loops, transient rejection/retry, genuine incompatibility, current-position audio, generation/source/render changes, restore and two-instance isolation. Assert actual samples and persistent identities as well as diagnostic reasons; preserve allocation/deallocation guards. Compare Audition and Export samples under their existing ramp/gain semantics and assert no Perform capture.

Use `scripts/build.ps1 -Configuration Debug -Test` and Release `-Test` for this audio/concurrency change, respecting the shared build lock and warm tree. Record native MSVC/CTest separately from fixture checks. Add dated evidence under docs/validation and update the relevant living technical guide under docs/development. After automation, use one brief owner FL Studio acceptance on the previously failing instance/copy; extra host requests require a concrete unresolved behavior.

## Risks / Trade-offs

- Telemetry races or reader stalls: use coherent bounded snapshots and explicit unavailable results; test concurrent observation and callback allocation safety.
- A fixture proves a mechanism but not the owner's failure: label that distinction and retain final preserved-instance acceptance.
- Origin drift is genuinely unsupported: preserve suppression and report evidence; do not reinterpret source or weaken guards to force output.
- Retry churn or stale publication: reuse bounded worker cadence, revalidate dependencies and reject superseded work.
- Diagnostic text becomes crowded: reuse the existing developer surface and test native readability without redesigning Perform.

## Migration Plan

No persistent schema, asset, VST3 ID or session-identity migration is expected. Apply only after a fresh implementation request. Preserve projects/assets and use the issue branch for a dedicated reviewed PR. Rollback reverts implementation commits and restores the prior plugin build without rewriting project data. Do not auto-archive this change, merge the PR or close the issue.

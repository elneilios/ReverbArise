# Design

## Context

See proposal.md for motivation and issue #53 for owner observations. At the inspected main baseline, `TransportPlayback::process` rejects a publication when `transportMappingMatches` or callback range validation fails. `badMapping` is play-gated, so clearing on Stop does not prove a new publication. `PluginEditor.cpp` replaces the underlying status with a generic awaiting message when `liveIncompatible` is true.

`TransportTime.h` compares exact rate/tempo/meter and inferred PPQ-zero origins within one sample. Existing transport-time tests cover this boundary and unsupported values. Processor host observations already use bounded sequence-checked atomic snapshots; mixed reads are not an established cause. `prepareTransport` validates session mapping before preparing and again before publication, uses revision/window fast paths and retries on later worker service. These are focused inspection points, not diagnosed defects.

The original report concerns 61 swells at 48 kHz, 122 BPM, 4/4, enabled Dry + Wet at 0 dB. The later owner recording shows 7 ready swells with the same failure: stopped/source origin approximately -0.262295 samples, playing origin approximately 15.737705 samples, constant revision 30 and attempt 16, and both consumers rejecting the approximately 16-sample difference. At Stop the origin matches again without a new publication. The exact host tuples reproduce dry-only output in a native fixture; the opt-in expected-wet assertion fails before the fix. See `docs/validation/2026-11-04-issue-53-live-diagnostics.md`. The host mechanism creating the offset is not established.

On 4 November 2026 the owner approved revising proposal/design/specs/tasks to separate playing-clock validation from parked/source timing while preserving saved sample positions. That decision permits the live mapping-contract change below; no behavioral implementation is part of this artifact update.

## Goals / Non-Goals

**Goals:** Preserve the established evidence and diagnostic safety; repair live admission and worker preparation for valid host transport with a different parked/playing origin; keep absolute audio placement, dependency integrity and within-run compatibility checks authoritative.

**Non-Goals:** Tempo/meter automation support, persisted sample-origin migration, a global tolerance increase or hard-coded 16-sample correction, scheduling/thread architecture replacement, Perform composition changes, Learn capture-pass investigation or Generate UX. Capture, Audition, internal Export and host offline contracts are unchanged. A broader migration returns to owner review.

## Decisions

### 1. Observe the actual admission decision

Extend fixed-size callback telemetry associated with the active publication, including callback observation sequence, validity, mapping reason, sample/PPQ/rate/tempo/meter, inferred origins/delta, callback count/range validity, publication revision/bounds, running/missing flags and workflow/live gates. Keep one bounded last rejection across Stop, identified as historical. Capture all correlated values from the same callback/publication; unavailable telemetry must remain explicitly unavailable.

Use bounded lock-free atomic publication or an existing safe fixed-capacity mechanism. A reader must never dereference callback-owned publication data after retirement. No strings, allocation, deallocation, locks, file operations, logging, waits or unbounded retry in the callback. Reader retries are bounded and may report unavailable evidence. Format sufficient numeric precision only on the worker/UI side.

Worker telemetry separately identifies source/session mapping, current/prepared revision, attempted window/loop hints, eligibility rejection, preparation/publication attempt and result sequence plus monotonic attempt time. Do not claim worker and callback snapshots are simultaneous. Prefer one last attempt/result and counters over a growing log.

Alternative: only display existing flags. Rejected because these cannot discriminate origin/rate/tempo/meter, invalid range and worker failure. Per-block text logs are rejected for real-time cost and unbounded storage.

### 2. Separate asset coordinates from the live playing clock

The evidence gate is complete. Retain absolute sample coordinates as the authority for source, render, placement, shape and live mixing. A parked/source `SongTime` records an asset context; it cannot also be the required PPQ-zero origin of a later playing host observation. Source rate, tempo, meter, session, content identities, render generations and current/frozen/retained eligibility remain mandatory.

Add one ephemeral per-instance live clock state owned by the audio callback, using the coherent `TransportTime` already delivered to `processTransportAudio`. Publish a bounded coherent worker view containing its run identity, established mapping, current observation and admission validity. This is runtime control state, independent of the read-only developer telemetry. Reuse existing sequence/atomic publication conventions; no new worker thread, queue architecture or persisted field.

A fresh valid playing run establishes its mapping from its first coherent playing observation. Require valid finite absolute timing/origin and callback range; source/render musical metadata must match before any wet is admitted. There is no wait for multiple callback blocks before an otherwise valid resident window can play. Subsequent observations must match that run's fixed mapping within the existing one-sample origin allowance. A difference from parked/source origin is informational, not a live incompatibility by itself; 16 samples is an observed example, not a correction constant or an enlarged tolerance.

This deliberately changes the live contract: origin equality is checked within the playing run, not against the saved source origin. No origin delta is added to or subtracted from source/render samples, scheduling positions, capture coordinates or saved state. Normal zero-offset hosts use the same absolute-position output as before.

Lifecycle rules:

- A valid stopped observation ends the playing run; the next valid Play establishes a new runtime mapping. Session replacement/restore/audio-session reset discards old runtime ownership and requires fresh validation.
- Continue observing host run state independently of output enable/workflow. Entering Perform or enabling live requires current clock and asset validation; toggling a control cannot erase an already known incompatible mapping during continuous Play.
- Missing/invalid timing and invalid callback range suppress admission but do not masquerade as a confirmed Stop or create a replacement baseline. A later valid observation is checked against the retained run mapping before resumption.
- Seeks, loops, cache misses, revision edits and worker retries do not start a new run or redefine its origin. A changed origin within continuous Play remains rejected; returning compatible timing requires a fresh valid check. Preserve the existing bounded wet transitions.
- Rate/tempo/meter incompatibility with accepted assets still rejects output, including at the start of a new run. A valid runtime clock alone never means assets or resident coverage are ready.

Keep the shared strict `transportMappingMatches` helper and its one-sample tests unchanged for consumers that require it. Apply the new separation only to live admission/preparation, including live eligibility of retained/frozen contributions. Do not weaken shared retention/export validation or apply a live exception to Capture, Audition, internal Export or offline scheduling. Test the shared `TransportPlayback` use in offline paths explicitly so live-specific policy cannot leak there.

Alternatives considered: a 16-sample tolerance bump, a global offset subtraction, removing all origin checks, or rewriting `model.timing` on Play. Rejected because these respectively admit unvalidated drift, miscorrect working hosts, hide mapping changes within a run, or mutate persisted/asset timing and other workflows.

### 3. Validate preparation against the current live clock and dependencies

Use the same callback-owned clock authority for the worker's live eligibility and publication checks. While playing, validate fixed source/render metadata and current run mapping rather than comparing incoming origin to the parked/source origin. While genuinely stopped, preparation may retain/prepare absolute-position PCM without certifying playing admission. Keep stopped preparation and actual playing admission distinct.

Before publishing, obtain fresh coherent clock and source/session observations. Recheck current dependencies, callback clock ownership/run identity, supported mapping and window coverage. If the run/session changes during preparation, explicitly revalidate under the new valid context or reject and retry; never stamp an unchecked old clock as current. An unavailable control snapshot is a transient retry condition, not a permanent prepared revision or success. A genuine unsupported playing mapping remains rejected until compatible validation is available.

`preparedLiveRevision` and window/loop fast paths may reuse verified PCM but cannot skip required clock validation. Existing resident data with matching session and source/render dependencies stays eligible for warm admission at its unchanged absolute positions; no full rerender or forced page churn is required merely because Play supplies a different origin. Source/session replacement still revokes old ownership even when numerical rate/tempo/meter or window bounds happen to match.

Retain the worker service cadence, immutable publications, generation cancellation and boundary handover. Transient publication/eligibility rejection must remain retryable after its condition clears. A late result joins the current position without replaying missed wet. Use existing invalidation and dependency mechanisms rather than adding a reset command or forcing revisions periodically.

Extend developer evidence as part of the repair: expose run identity, established live reference and validity independently of source/publication origin delta, and derive the displayed gate from the actual admission decision. A supported source-to-playing delta must not still display an origin rejection. Preserve historical rejection labels and separate callback/worker observations.

### 4. Reuse the existing Diagnostics surface

Extend existing Settings/Recovery Diagnostics text and snapshot binding with existing theme tokens and text roles. No new library control family, token, visual variant or one-off exception is planned. Read the production style guide and UI standards before implementation. Include component/binding tests for current versus historical observations, reason formatting and lifecycle. Run the UI standards guard without changing its debt inventory. If a new/revised family or exception proves necessary, prepare and launch one native preview and obtain scoped owner approval before wider rollout. Preserve #46's composition and worktree.

### 5. Verify lifecycle and output, not just flags

Extend existing `transport_time_test`, `transport_playback_test`, `swell_engine_test` and processor/workflow coverage as appropriate. Turn the recorded-value expected-wet replay into a passing regression. Verify zero-offset and nonzero-offset live starts, warm first-block admission, stable advancement, origin drift within a run, invalid timing without false Stop/rebaseline, confirmed Stop/Play, Perform entry/enable gating, and rate/tempo/meter incompatibility. Extend engine/processor coverage to the recorded 7-member case and 61 eligible members, first/later windows, seeks/loops, worker transition/retry, source/render changes, restore and two-instance isolation. Assert actual samples and persistent identities as well as diagnostic reasons; preserve allocation/deallocation guards. Compare Audition and Export samples under their existing ramp/gain semantics and assert no Perform capture.

Use `scripts/build.ps1 -Configuration Debug -Test` and Release `-Test` for this audio/concurrency change, respecting the shared build lock and warm tree. Record native MSVC/CTest separately from fixture checks. Add dated evidence under docs/validation and update the relevant living technical guide under docs/development. After automation, use one brief owner FL Studio acceptance on the previously failing instance/copy; extra host requests require a concrete unresolved behavior.

## Risks / Trade-offs

- Telemetry races or reader stalls: use coherent bounded snapshots and explicit unavailable results; test concurrent observation and callback allocation safety.
- A fixture proves a mechanism but not the owner's failure: label that distinction and retain final preserved-instance acceptance.
- A source-to-host origin difference could be confused with drift: accept the independently validated run reference but reject changes within that run; display both references and preserve absolute source positions.
- A missing observation could look like Stop and silently rebase: distinguish invalid timing from a confirmed valid stopped observation and test recovery against the retained baseline.
- Sharing playback machinery could change offline/capture behavior: keep strict helpers/policies scoped and verify those existing workflows remain unchanged.
- Retry churn or stale publication: reuse bounded worker cadence, revalidate dependencies and reject superseded work.
- Diagnostic text becomes crowded: reuse the existing developer surface and test native readability without redesigning Perform.

## Migration Plan

No persistent schema, asset, VST3 ID or session-identity migration is expected. The owner-approved planning revision is ready for resumed Apply; its runtime clock remains entirely ephemeral. Preserve projects/assets and use the issue branch for a dedicated reviewed PR. Rollback reverts implementation commits and restores the prior plugin build without rewriting project data. Do not auto-archive this change, merge the PR or close the issue.

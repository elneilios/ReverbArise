# Design

## Context

See proposal.md for motivation and the issue boundary. `PluginProcessor::processBlock` currently admits dry input when mapping or scoped recapture is enabled, then runs transport/offline mixing independently. CaptureWorker batches at up to five seconds and publishes on chunk boundaries, discontinuities or a 250 ms idle timeout. CaptureSession already publishes immutable revision manifests and keeps old assets, but a whole multi-chunk pass is not one transaction. ScopedRecapture tracks a bounded source interval rather than the workflow's pass-wide replacement permission.

State currently has a version-7 processor envelope and version-8 swell JSON; these are separate version domains. Old metadata contains mapping/automatic/live preferences but no workflow mode. Existing gain policies, library assignments, freeze, exact envelopes, sparse coverage and retained-IR playback must survive the migration.

## Goals / Non-Goals

**Goals:** one authoritative mode/capability snapshot, pass-atomic source publication, no implicit overwrite, asynchronous Stop validation and deterministic cancellation/restore. Keep PCM streaming bounded rather than holding a song in RAM.

**Non-Goals:** dedicated Capture/Finalise screens, full-timeline preview/export, source portability/cleanup, Character DSP or unrelated editor changes. Provide a narrow transition/status control and integration seams; do not represent unimplemented exports as usable. Preserve existing audition scope rather than implement #5 here.

## Decisions

### 1. Separate the selected mode from operation phase

Use one processor-owned workflow authority with persisted `Capture`, `Live`, `Finalise` and transient phases such as idle, recording, draining, validating and failed. Publish a coherent bounded capability snapshot for callback admission and UI commands: source capture, scheduled wet, creative editing, manual preview, mode-change availability and reason. Do not derive modes from combinations of mapping/live/automation toggles.

```text
Capture idle -- DAW Play --> recording -- DAW Stop --> draining/validating
     ^                            |                         |
     |                            | discontinuity           | valid useful commit
     |                            v                         v
     +--- empty / failure --- same pass, new span           Live
                                                            |
                                   ready enabled wet        v
                                                        Finalise
                                                            |
                                                       Return to Live
```

Manual requests from any entry point use the same guards. Recording Capture and its Stop settlement lock mode changes. A loop/backward seek closes a span, not the pass and not the mode. A new Play before settlement completes cannot join the closing pass or publish through its completion; finish the immutable closed pass, permit the automatic Live transition when valid, and let new Capture recording require a new admitted pass. Explain settlement rather than block the callback. Mode changes while Live/Finalise is playing apply at processing boundaries with existing short transitions.

Live entry needs verified compatible captured audio, or the user-approved compatibility exception: an enabled, unmuted, solo-eligible verified frozen render at compatible rate/tempo/meter. This preserves existing frozen-wet recovery when dry is cleared/missing; it never grants source replacement or fabricates dry for audition. Mapped silence alone is not useful musical source, with no other loudness threshold. Finalise entry still requires useful source and an eligible ready swell. Re-check on the command, not just drawing. Later missing dependencies gate affected actions/output without deleting state; unaffected verified frozen wet remains usable.

Rejected: retaining mapping/live booleans as competing mode authorities, allowing UI-only locks, or requiring every render to finish before leaving Capture.

### 2. Make capture a bounded pass transaction over the committed map

Keep the existing source store/root, per-chunk float fidelity, hashes and immutable manifest references. Add a worker-owned transaction with base manifest identity, project/instance epoch, pass ID, replacement permission and staged span metadata. Write chunks to unique provisional assets; retain only bounded PCM and a sparse interval index. Verify each chunk and build/validate a complete candidate manifest before one durable publication on Stop. Readers, save state and swell dependencies continue using the last committed manifest until acceptance; the Capture progress snapshot can separately expose staged coverage as provisional.

For ordinary Capture, subtract committed audio AND silence coverage before staging incoming spans. Also subtract earlier provisional spans, so loops cannot overwrite first-recorded positions inside the same pass. Split partial overlaps and copy only uncovered samples with exact offsets. Never use performance-tolerance comparison as permission to replace mapped data.

With Recapture explicitly armed before a pass, the worker stages encountered mapped and unmapped spans. Within repeated positions in that pass, the last completely received span wins deterministically; until Stop the committed source remains unchanged. Publish all admitted replacements and additions as one candidate revision. Successful replacement consumption disarms Recapture even when decoded PCM is identical; failed/aborted attempts preserve committed data and expose retry status, with arming cancelled on restore and explicit cancellation. No full-song RAM buffer or in-place source rewrite.

Queue overflow, nonfinite samples, invalid/incompatible rate/position, disk-full, asset/hash verification failure or explicit cancellation abort the pass transaction rather than publish an incomplete replacement. Preserve previously committed source and old snapshots. Provisional orphan assets are not valid source and remain separate from sole source copies; this slice adds no destructive storage cleanup. Capture cancellation can be a recovery/Settings command, not a redesign of the main editor.

Rejected: using per-chunk atomic writes as proof of pass atomicity, accumulating complete PCM until Stop, or choosing a global storage migration/garbage collector from #8.

### 3. Stop is an asynchronous ordered fence, not an idle timer

Observe host transport independently of editor lifetime. Assign a monotonic pass token and operation/session epoch. Use bounded preallocated control/data sequencing so the worker knows the final admitted queue boundary even if a queue is full; a Stop signal cannot be lost or processed before earlier audio. Duplicate stopped callbacks cannot enqueue another commit. Never wait for CaptureWorker or perform storage/validation on the realtime callback or UI thread.

The worker drains only the closing pass, verifies its assets/manifest, publishes once and returns a result containing epoch, pass ID, accepted revision, affected intervals, usefulness and error. Under the same publication/epoch fence as restore/Clear/new sessions, obsolete jobs cannot replace source or trigger a mode change. Clear/restore must supersede or serialize publication before changing ownership; checking a token only after disk publication is insufficient. Existing immutable publication and snapshot-validation primitives should be extended, not bypassed.

Successful useful completion sends source changes to dependency analysis once and automatically enters Live. Automatic Swells off does not block Live: manual creation remains available with committed source. No-op/empty capture stays Capture; an explicit successful identical recapture of useful audio can enter Live and disarm without forcing needless asset/render changes. A mapped-only ordinary pass has no useful newly captured data and stays Capture even if an older valid map exists; the user may select Live while stopped.

Missing timing never means sample zero or an inferred Stop. Stop can be recognized from an explicit valid host transport report without usable musical mapping; without a trustworthy stop event keep pending data private until an explicit safe recovery/cancel. Do not invent host transport calls or treat releaseResources/destruction as permission to publish a partial pass. Test FL Studio's post-Stop callbacks and state-saving behavior.

The existing blocking `flush()` on entry to host non-real-time processing is distinct from realtime Stop: synchronous bounded offline preparation remains permitted by the current export contract, but must not silently commit an unfinished Capture transaction or cause a mode transition. Offline export reads only the accepted committed snapshot and never captures input.

### 4. Mode gates wrap existing audio paths and command admission

Capture: record original input only during valid realtime DAW playback; output dry unchanged. No scheduled wet, diagnostic test tone or manual preview is admitted in production Capture. UI navigation/status remains usable; creative commands reject with a reason. Automatic background detection/rendering can run against accepted source, but completion cannot start voices or unlock modes early.

Live: no capture, regardless of legacy Settings mapping values, automation toggles, Pattern/Song, realtime/offline host flags or selected swell. Existing live enable/routing/wet level, prepared pages, calibrated gain, IR-only retained versions and audition priority retain their meanings inside this gate.

Finalise: no capture and no host-scheduled wet, including host WAV bounce; dry passes through even if saved Live routing is Wet Only. Preserve all creative/accepted asset state; reject edits, queued UI callbacks and legacy recovery actions that would change it. A separate explicit internal-preview path remains allowable by the mode contract; this slice does not implement export preview. Existing preview is usable only where its current validated scope applies; clearly identify it and do not imply song-aligned export is present. Capture disallows it. Entering a mode stops any now-disallowed manual voice; returning to Live restores the saved live preferences without autoplay.

Keep mode gating distinct from changing saved live preferences or deleting prepared audio. Integrate it below UI bindings in engine/processor command admission so keyboard shortcuts, Settings, chooser completions, Diagnostics and async completions cannot circumvent it. Dependency restoration/preparation may proceed while creative editing is locked; preservation is not a freeze of all workers. Retire pages off-callback, use existing <=5 ms wet/preview stop transitions, and ensure finalized dry passthrough has no sustained wet/old routing.

### 5. Recapture and Automatic Swells stay independent

Recapture controls source replacement only. Automatic Swells controls new automatic marker acceptance/generation through existing automatic-processing policy, with saved marker sensitivity and future IR defaults. OFF still allows capture and subsequent manual Live swells; existing definitions/edits/frozen results are never erased. Keep existing automatic refresh and diagnostic recovery semantics only where compatible with the selected mode. Recapture completion queues only changed dependencies; identical accepted PCM reuses existing renders/calibration.

Ordinary playback can no longer use Regeneration sensitivity to authorize source updates. Preserve its saved numeric value for compatibility but retire/disable the misleading source-overwrite control in this foundation's existing global/Settings binding with a short explanation; do not silently replace it with another function. Full Capture settings redistribution belongs to #11. A useful source commit starts render work asynchronously; job completion guards include accepted source revisions and workflow/session epochs. Reconcile the old automatic-first-use/manual-silent spec language explicitly.

### 6. Version workflow state without moving source assets

Give the processor envelope a new version for workflow state, with strict finite enum/size validation and explicit compatibility with old envelopes and all supported swell JSON gain revisions. Store the requested mode and normal creative/preferences state, not pass tokens, pending manifests, active voices or Recapture permission. Keep the independent swell JSON schema unchanged unless a genuine required field cannot live in the workflow envelope; never confuse its current version 8 with processor envelope version 7.

Confirmed user decision: every legacy project lacking workflow mode enters Capture, even when valid captured source exists or old mapping/live flags were on. Preserve source/IR/render references, gain policy/calibration, swells/freeze, all numerical edits and independent preferences. Recapture off protects those source ranges; after stopped restore the user may choose Live once prerequisites verify. New-format sessions restore their saved mode, pending capability validation; no transport request, pass resume, Recapture arm or audition starts. Saved Capture may capture uncovered positions only on subsequent valid Play.

Save while recording/committing references the last accepted revision, not provisional data. Restore or duplication gets fresh operation tokens; clones may reference the same immutable prior assets but subsequent writes publish independent branch manifests, preventing one clone from changing the other's committed pointer. Test current project/instance ownership before deciding whether extra branch identity metadata is needed. No mutable manifest pointer or shared pending job may act as cross-instance state.

### 7. Keep visual review narrow and evidence explicit

Add one restrained mode selector/subtitles/status to the existing editor header with truthful availability reasons and an independently labelled Recapture control in existing settings/recovery space. Resolve conflicting mapping switches through workflow authority instead of leaving two ways to enable capture. Keep the working Live editor, new shared Wave zoom and all exact/envelope controls unchanged. Capture/Finalise can show the existing content read-only with clear mode messaging for this foundation; this is temporary integration, not delivery of #11/#6 mockups.

Take native snapshots at 1100x850 and the existing larger sizes. Ask the user to review selector/status/disabled controls before broad host validation; this preserves the user's visual checkpoints without approving a new workspace design. Tests must distinguish visible UI disabling from model-side rejection, use faults/delays for Stop settlement, and retain independent source/sample/export oracles rather than merely testing the implementation's enum values.

## Risks / Trade-offs

Validation sign-off agreed on 2026-11-02: core FL Studio Song/Pattern capture,
protection, explicit Recapture/cancellation, routing, loops/seeks, exports,
save/reopen, legacy migration and FX-copy isolation have user-reported passes.
The user accepts the existing automated coverage for silent-only capture,
delayed validation/save/restart races, pending-job export and frozen recovery
instead of requiring further FL Studio reproduction of those edge cases.
Retain a requirement-to-evidence crosswalk in
`docs/validation/workflow-foundation-evidence.md`; identify these as automated
acceptance, not host passes. Additional host/DPI scaling remains unperformed
and explicitly documented. No runtime requirement or workspace scope changes.

- [Stop may not arrive as expected in all host callback patterns] -> Test real FL Studio Song/Pattern, editor closed, stop/restart and suspended processing; retain provisional data privately and provide honest recovery/cancel rather than infer successful commit from an idle timeout.
- [Long passes or rapid seeks exhaust queue/disk resources] -> Bounded streaming and sparse span indexing, overflow abort, explicit diagnostics and old revision preservation; no higher PCM budget.
- [Late commits race restore/duplication] -> Fence actual publication by session epoch and independent manifest branch, not just completion notifications; fault-inject both sides of publication.
- [New mode behavior changes legacy projects] -> Explicit Capture migration confirmed by user, Recapture off, visible instructions to enter Live, unchanged gain/assets/edits and pre-upgrade project backup for rollback.
- [Finalise appears to promise export prematurely] -> Mode/status only, dry-output guidance and no fake export tile; keep #5/#6 integration seams separate.
- [Stop validation is mistaken for wet readiness] -> Transition after source validation, show remaining render counts in Live; never wait for all wet jobs.

## Migration Plan

Delivery naming addition requested during apply: change the JUCE product/build
name to ReverbArise while retaining `com.k7ma.preverb`, `K7Ma`/`Prv1` and stable
VST3 component/controller identifiers. Verify generated IDs before/after and
legacy state restore. Host scan/display-name effects require a FL Studio check;
do not silently delete a previously installed Preverb Spike bundle. Internal
processor/session/asset identifiers are not renamed by this delivery change.

Capture baseline state/audio fixtures first. Introduce the mode authority and transaction API behind processor integration, then replace legacy capture admission and wire guarded controls. Re-run current legacy/gain/IR/freeze/transport/export oracles with explicit mode setup reflecting the new contract; do not weaken output comparisons. Validate old envelopes migrate to Capture and new envelope roundtrips all three modes. Record native and FL Studio checkpoints before marking acceptance complete. Rollback uses a pre-upgrade FLP and immutable old asset references; older binaries must reject unsupported workflow metadata rather than misread it. No automatic cache removal or storage migration is part of deployment.

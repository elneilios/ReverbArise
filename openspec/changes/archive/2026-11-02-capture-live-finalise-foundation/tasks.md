# Tasks

Planning does not complete implementation. Keep these boxes unchecked until the behavior and each task's verification pass. Issue #10 is the foundation slice of #3; the dedicated layouts, full-timeline audition and export remain separate work.

## 1. Baseline and workflow authority

- [x] 1.1 Record current processor-envelope and swell-JSON versions, plugin/parameter identity, source/IR/render hashes and legacy/current state fixtures including musical calibration, frozen/missing assets and mapped silence; verify baseline focused capture/state/gain/IR/transport tests before altering contracts and record the exact source/build revision.
- [x] 1.2 Add one authoritative Capture/Live/Finalise model with transient recording/draining/validating/result phases and coherent capability snapshots; verify an independent transition-table oracle covers fresh Capture, Live/Finalise prerequisites, silence-only source, invalid timing, no-useful pass and unavailable-after-restore states.
- [x] 1.3 Add guarded command admission and truthful lock reasons for recording/settling Capture; verify direct model requests cannot bypass locks, stopped valid Capture permits manual Live entry, and delayed background wet work is not a prerequisite for Live.
- [x] 1.4 Document the mode/capability table and provisional-versus-committed semantics in current development guidance; verify every documented state/transition maps to a test and no export/layout capability is claimed.

## 2. Protected transactional source capture

- [x] 2.1 Extend source storage with a worker-owned staged pass keyed by base manifest/session epoch/pass ID, using immutable assets and bounded PCM; verify multi-chunk staging leaves committed readers/save references unchanged and preserves exact float samples, sparse gaps and silence without full-song allocation.
- [x] 2.2 Implement additive interval subtraction against both committed and earlier staged coverage; verify independent sample oracles for partial blocks, audio/silence overlaps, noncontiguous seeks, repeated loops, negative/overflow coordinates and unchanged mapped asset identities with Recapture off.
- [x] 2.3 Implement independent pass-wide Recapture permission with deterministic last-received replacement inside an armed pass; verify Automatic Swells on/off cannot grant overwrite, ordinary playback never replaces mapped source, successful identical/different recapture disarms correctly and explicit cancellation preserves prior data.
- [x] 2.4 Add pass drain, full candidate validation and one durable Stop publication using existing immutable manifest mechanisms; verify all replacements/additions become visible together, only affected intervals change, successful no-op identity is reusable and old saved snapshots remain readable.
- [x] 2.5 Add fault-injection tests for disk-full/write/rename failure, corrupt/nonfinite data, queue overflow, invalid rate and interrupted validation across multiple staged chunks; verify transaction rollback/no partial source, actionable errors, unchanged prior references and no deletion of sole source/export assets.
- [x] 2.6 Fence publication ownership against Clear/restore/new session and duplicate instances, not just completion callbacks; verify races before/during/after publication cannot overwrite another epoch's source, clones can branch manifests independently and stale operations cannot revive data.
- [x] 2.7 Update capture/storage guidance with atomic-pass behavior, mapped-silence protection, explicit retry/cancel and current storage locations; verify instructions against capture tests and retain #8 portability/cleanup as separate scope.

## 3. Host transport and audio-path integration

- [x] 3.1 Integrate bounded preallocated pass/control sequencing with processor host observation so Stop cannot outrun queued audio or be lost on a full queue; verify duplicate Stop is idempotent and realtime callbacks neither wait nor acquire locks/allocate/free/read files for capture settlement.
- [x] 3.2 Integrate explicit Stop -> drain/validate -> automatic Live on useful success; verify delayed validation, pending wet jobs, all-silence/empty/mapped-only passes, identical armed replacement, failure, missing timing and no host transport requests using deterministic processor fixtures.
- [x] 3.3 Handle loops, seeks, backward passes, editor-closed operation and restart during settlement; verify absolute sample oracles, no loop-triggered transaction publication, no reopened closing pass, obsolete completion rejection and no lost/duplicated source intervals.
- [x] 3.4 Gate realtime scheduled wet and capture by mode below the UI; verify Capture dry/no wet, Live existing routing/no capture and Finalise dry/no capture/no scheduled wet, including saved Wet Only routing, per-instance isolation and original-input-only capture.
- [x] 3.5 Integrate bounded mode/voice transitions and separate explicit preview admission; verify <=5 ms transitions, current-position Live reentry, no replay/ghost/duplicate output, Capture preview rejection, Finalise creative lock and no auto-start on render/IR completion.
- [x] 3.6 Integrate mode-aware host offline snapshots without implicitly flushing/accepting provisional Capture; verify no offline capture, accepted Live member/export oracles, Finalise/Capture dry-only bounce, pending capture/IR/gain jobs and independent mixed-mode two-instance callbacks.
- [x] 3.7 Bind successful source acceptance to bounded automatic analysis/refresh once per accepted revision; verify Automatic Swells off still permits manual Live creation, no accepted targets from rejected passes, preserved manual edits/suppression/freeze, identical PCM reuse and only changed dependency regeneration/calibration.
- [x] 3.8 Reconcile legacy mapping/scoped-recapture/tolerance entry points with workflow authority and update transport/automatic guidance; verify saved tolerance values survive without authorizing implicit replacement, marker sensitivity remains independent and no secondary command can capture in Live/Finalise.

- [x] 3.9 Preserve the user-approved verified frozen-wet exception for Live entry after dry source is missing/cleared; verify incompatible/muted/solo-excluded frozen audio does not qualify, unfrozen dependencies stay blocked, dry is never fabricated and frozen playback/export regressions still pass.

## 4. State migration and recovery


- [x] 4.1 Version workflow metadata in the processor state independently of supported swell-JSON gain revisions; verify strict bounds/enum/unknown-version rejection transactionally, all three new-mode roundtrips and unchanged source/IR/render/gain identities without embedding PCM.
- [x] 4.2 Implement confirmed legacy migration to Capture for every supported state without workflow mode, irrespective of old mapping/live flags; verify valid/missing/silent source, empty v1/v2 swells, default/saved trim, musical/legacy gain and frozen edits survive with Recapture off and no implicit Live selection.
- [x] 4.3 Implement committed-only save during recording/settlement and fresh-epoch restore/duplication; verify no pass/Recapture/voice resumes, saved Live/Finalise waits for validation without losing state, subsequent Capture Play is additive and old snapshots remain readable after later recapture.
- [x] 4.4 Document old-project Capture entry, manual Live selection, missing-source recovery and rollback using pre-upgrade project copies; verify the documented actions in state/processor tests and record unsupported older-binary behavior without migrating the asset root.

## 5. Minimal mode controls and visual checkpoints

- [x] 5.1 Bind a restrained Capture/Live/Finalise selector/subtitles/status and independent Recapture control into the existing editor, with prerequisite/recording/settlement explanations and no dedicated workspace/export redesign; verify native empty, recording, settling, failed, Live and Finalise snapshots at 1100x850 and supported larger sizes.
- [x] 5.2 Gate canvas/knob/exact/keyboard/Settings/Diagnostics/file-chooser commands through the same capabilities, preserving IR/envelope/gain/Wave zoom and numeric edit ownership; verify delayed chooser callbacks, active edit then mode change, direct recovery bypass attempts and return-to-Live bindings with native UI/model tests.
- [x] 5.3 Preserve saved tolerance metadata while disabling/retiring misleading implicit-regeneration source controls, and reconcile mapping controls with mode authority; verify no secondary toggle creates a contradictory mode or erases saved marker/automation/live/audition preferences, and document the disposition of each affected control.
- [x] 5.4 Obtain user review of actual native mode/status/guarded-control visuals before broad host validation; record approvals or adjustments, retain existing brand/layout and keep untested host/DPI checks explicitly open.

## 6. Integrated acceptance

- [x] 6.1 Run configured full Debug build/CTest and focused Release capture/state/processor/gain/library/handover/streaming/transport/UI tests; preserve independent output oracles while updating explicit mode setup, record commands, pass counts, callback guards, working-memory evidence, binary/asset identities and strict OpenSpec/diff validation.
- [x] 6.2 Record FL Studio Song/Pattern acceptance for fresh/legacy Capture, additive audio/silence protection, explicit independent Recapture, cancelled-pass preservation, recording mode locks and automatic Live after useful Stop; supplement with user-accepted automated coverage for silent-only input, failure rollback and settlement guards. Record binary identity, available host/rate/buffer details and unreconfirmed metadata explicitly, without claiming unperformed host checks passed.
- [x] 6.3 Record FL Studio acceptance with editor open/closed, loops/seeks, mode-specific routing/no Live capture, Finalise dry-only bounce, Live two-instance/mixed-mode exports, saved modes/legacy migration, FX-copy isolation and save/reopen during recording. Use the user-approved automated coverage for delayed validation/save/restart, pending-job exports and frozen recovery; document unperformed FL Studio/DPI checks and verify no source loss, duplicated wet or stale completion in the accepted evidence.
- [x] 6.4 Review every issue #10 acceptance item against requirement/test/host evidence, confirm #11/#5/#6/#8 scope boundaries and no IR/gain regressions; run final strict all-spec validation and diff checks and leave any unresolved acceptance unchecked.

- [x] 6.5 Rename the built/displayed plugin to ReverbArise and deliver ReverbArise.vst3; verify unchanged VST3 component/controller IDs, manufacturer/plugin codes, bundle ID, parameter inventory and old-session restore, and record the new output path without silently deleting an installed old bundle.

## Workflow follow-up

- Update each checkbox only after its implementation and stated verification pass, using current CLI task locations across usage resets.
- Review native visuals before proceeding to host acceptance; do not count generated snapshots or code compilation as user approval.
- No implementation, commit/push, GitHub comment or issue closure is authorized by creation of these planning artifacts.
- Apply via `$openspec-apply-change capture-live-finalise-foundation` after planning review. Sync/archive only after required implementation and real host evidence are complete.

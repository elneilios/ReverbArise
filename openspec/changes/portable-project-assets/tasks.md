# Tasks

Phase 0 and phase 1 were authorized on 2026-11-06; the owner approved the compatibility/backup matrix. Checked tasks record verified work only. The owner approved the automatic-scan native preview and task 2.4 production Settings integration is verified; phase 1 is complete and later phases remain gated. See [measured results](../../../docs/validation/2026-11-06-issue-8-storage-audit.md) and [contract/rollback matrix](../../../docs/development/asset-storage-contracts.md). Later phases require separate explicit approval. Phase 1 permits read-only inventory/adapters and the approved Settings integration, plus disposable fixture writes. It does not authorize relocation, deletion, packing, association or changes to capture/DSP/storage writers. See design.md for phase gates and rollback.

## 1. Contract and measurement gate

- [x] 1.1 Complete the owner-reviewed version/readiness matrix and handoff against main df765d95170caaaab10a646e43088ea3be66ce53 (or explicitly record a newer baseline): processor envelope v8, creative v1–11, capture schemas 1/2, Export v2/unversioned legacy. Use the design reconciliation as static input only; verify current source/context and all Export-history ownership contracts without editing UI/state machine/capture semantics. This planning revision does not complete the task.
- [x] 1.2 Create the dated deterministic disposable-root growth fixture described in the evidence report; verify 3–5 minute dense/sparse input, silence/gaps, forty swells, repeated IR writes/generations and shape-only edits through per-step bytes/counts/hash reports rather than theoretical totals.
- [x] 1.3 Exercise reopen, Clear, duplicate, old snapshots and concurrent processes in disposable roots; verify unchanged prior source/render hashes and distinguish shared directory IDs from independently accepted maps in the report.
- [x] 1.4 Perform the owner-authorized diagnostic FL Studio/JUCE project-folder discovery spike without guessing paths; record exact versions, raw host-context/state-stream support and unsaved/first-save/reopen/Save As/moved-project observations, with preset-path ambiguity checks, no asset relocation and explicit-folder/user-data fallback. Unknown capability is a valid result, not permission to infer a folder.
- [x] 1.5 Document the approved version/readiness, backup and binary rollback matrix beside the audit; verify every supported old format has a fixture and unknown dependency versions fail safely before phase 2 is authorized.

## 2. Read-only inventory and dependency adapters

- [x] 2.1 Add bounded legacy/new-format read adapters and a dependency visitor covering source, silence/gaps, every schema-2 context IR/span/origin, default IR, each swell's IR/render/calibration, creative-v11 generationSource/autoGenerateOnStop and every saved Export history WAV/descriptor. Verify zero-swell historical-context IRs distinct from current defaults, explicit empty IRs, schema-1 unknown history, frozen/disabled/stale state, v2 and legacy Export gains/laws, and unknown-version closure tests. Do not replay contexts or generate on restore.
- [x] 2.2 Add read-only selected-root inventory by category/revision and exact-file versus decoded-equivalence duplicate reporting; verify fixture totals, count shared physical objects once, unknown/inaccessible scope and no user writes.
- [x] 2.3 Add rebuildable observed-snapshot/alias/pin accounting with conservative legacy protection; verify missing index, interrupted catalog update, offline roots and unenumerated FLPs never make durable data eligible.
- [x] 2.4 Expose location/count/byte/uncertainty summaries in the existing settings/diagnostics area after preparing a native preview; obtain owner look/interaction approval for revised control families before broader rollout and verify component/binding tests without altering #11 Learn controls.
- [x] 2.5 Update inventory/backup guidance and dated results; verify documented categories/limits against fixture reports and keep native MSVC/CTest separate from fixture-only checks.

## 3. Exact-byte Pack and Relink

- [x] 3.1 Add immutable portable descriptor/snapshot/receipt versions and bounded parsing with legacy aliases; verify exact source/context spans, coverage semantics, session edits/defaults/auto-generation policy/fingerprints and all Export-history paths/checksums/origins/frames/gain laws. Preserve separate version domains and schema-1 unknown provenance; verify malformed/oversized-state rejection and receipts distinguish managed dependency closure from unenumerated external Playlist files.
- [x] 3.2 Add coherent accepted-state pinning and explicit per-instance collection scope; verify capture/import/render races never mix epochs and missing instances/history are excluded from completeness claims.
- [x] 3.3 Add copy-to-destination staging, operation journal, closure verification and completion publication; verify destination-only hashes and fault injection at every WAV/descriptor/snapshot/marker boundary without changing original refs.
- [x] 3.4 Add cancellation, interrupted-pack inspection/resume and storage diagnostics; verify disk-full, read-only, locked/offline roots and cross-volume copies preserve originals and previous successful packs.
- [x] 3.5 Add handle-aware containment and Windows path policy; verify traversal, reserved/device/UNC/drive-relative paths, alternate streams, Unicode/long paths, case collisions and symlink/junction replacement races cannot escape the owned operation.
- [x] 3.6 Add explicit Relink with store/snapshot/hash validation and ambiguous-candidate handling; verify moved folders, changed absolute hints, conflicting store copies and unknown versions without silently adopting newer heads.
- [x] 3.7 Add Storage Backup and Restore/Locate controls backed by verified Pack/Relink after native preview approval; verify bindings show included instance scope, pending-work exclusions, incomplete/cancel states and separate DAW/external-Playlist backup responsibility without changing creative/capture workflow.
- [ ] 3.8 Run destination-only restore in an isolated Windows profile with original cache/IRs unavailable; verify exact source/wet/export-history hashes, historical context IRs, positions, peaks, edits/defaults/auto-generation policy and zero convolution/resampling/recapture counters for compatible complete snapshots. Include zero-swell context-only IR dependencies and non-current/legacy-law exports; missing history must fail complete-pack validation without changing accepted state.
- [x] 3.9 Document the transfer and relink procedure, multi-instance receipts and failure recovery; verify it against one consolidated short FL Studio Save As/duplicate/reopen/transfer smoke test and record actual host/DPI/PC coverage without repeating automated edge cases.

- [ ] 3.10 Add missing-asset diagnostics and Locate/Restore/Re-learn recovery after native preview and separate implementation approval. Verify decodable swell IDs/positions/shapes/edits survive missing source, custom IRs, corrupt backups, partial/cancelled re-learning and failed regeneration; retain unaffected assets, explicit frozen-result consent and saved auto-generation policy. Require complete fresh source and verified IRs before offered regeneration, and disclose changed-sound/export limits. No Clear or automatic history replay.

## 4. Optional association and independent future writes

- [ ] 4.1 Add the Storage asset working-folder picker and explicit Use This Folder only after copy/pack verification, with current user-data fallback and existing host-change notification. Use validated host project location as a proposed default only after separate runtime-evidence/rollout review; verify unsaved/unsupported hosts, cancellation, read-only destinations and Save As never silently move existing assets and successful switch requests a host save without claiming it happened.
- [ ] 4.2 Add independent writer branch/lease ownership while preserving legacy UUID provenance; verify reopened/copied/duplicated snapshots across two processes cannot overwrite history or publish into one another.
- [ ] 4.3 Add explicit physical Fork via verified copy, never automatic Save As migration; verify old FLPs still resolve exact legacy dependencies and both store copies remain independently usable.
- [ ] 4.4 Publish association/version/rollback guidance and run supported old-state fixtures; verify rollback with the retained pre-upgrade FLP, original binary and untouched legacy store, documenting that upgraded states need not open in old binaries.

## 5. Separately approved scratch administration

- [ ] 5.1 Obtain a separate reviewed scratch-only implementation scope; deliver the eligible-class matrix showing source, capture staging, IRs, durable renders/history, exports and unknown legacy temporary files are excluded.
- [ ] 5.2 Add opt-in preview restricted initially to journaled cancelled pack-copy scratch; verify exact counts/bytes/reasons and zero eligibility for sole recordings, exports and possibly referenced old-FLP assets.
- [ ] 5.3 Add cross-process maintenance coordination and immediate handle/path/hash/reference/lease revalidation; verify active readers/jobs, new references after preview, locked files and path races skip candidates without callback blocking.
- [ ] 5.4 Add journaled same-volume quarantine and restart-safe hash-verified undo; verify crash at each journal/move boundary, destination conflicts and preserved ambiguous data.
- [ ] 5.5 Add at least 30-day grace plus separate explicit permanent-removal review; verify time expiry alone never erases files and fresh uncertainty or leases veto final removal.
- [ ] 5.6 Add recovery reporting for abandoned source/snapshots without automatic adoption; verify explicit validated recovery preserves both previous accepted refs and originals on failure.
- [ ] 5.7 Update retention/recovery documentation and dated fault-injection evidence; verify every cleanup eligibility condition and rollback operation has a passing focused fixture before enabling this optional phase.

## 6. Separately reviewed efficiency experiments

- [ ] 6.1 Measure store-local exact-byte dedup candidates separately from normalized decoded identities; verify any approved object reuse retains distinct dependency descriptors, legacy aliases and signed-zero-distinct data without deleting legacy files.
- [ ] 6.2 Benchmark independent lossless WavPack and byte-compression prototypes outside production storage using dense/sparse/low-level/>unity/subnormal/signed-zero fixtures at 44.1/48/96 kHz; deliver ratio, CPU/wall, peak memory, seek/open latency and exact bitwise round-trip results.
- [ ] 6.3 Verify corrupt/truncated/hybrid-with-missing-correction inputs fail without lossy fallback and resource limits hold off callback; retain WAV as the default unless a later codec adoption review authorizes a versioned reader/writer change.
- [ ] 6.4 Document measured trade-offs and adoption/rollback recommendation; verify no claim of compression benefit rests on arithmetic estimates, integer conversion or content hashes that normalize sample bits.

- [ ] 6.5 Propose measured duplicate-write reduction and a bounded rebuildable-cache eligibility/reconstruction matrix under separate review; verify no legacy saved render, sole source, imported IR, frozen output, export or unknown old-FLP dependency becomes evictable merely through deletion/regeneration/current-reference absence. No collector or eviction rollout without later approval.

## Workflow follow-up

- Owner approved this decision capture and starting diagnostic task 1.4 on 2026-11-06. Phase 1 remains delivered; phase-2 backend implementation follows the subsequent authorization below. Keep PR #31 open; do not sync canonical specs, archive, merge or expand spike authorization into storage/recovery/cleanup implementation.
- Future implementation requires a new request and phase-specific review; a checked planning artifact is not evidence any implementation task has run.

## Current authorized work

The subsequent owner request authorizes phase 2 using explicit folder selection and the user-data fallback. Tasks 3.1-3.6 supply the backend. The owner approved the native preview ("looks fine, carry on") and task 3.7 production integration is now verified. Tasks 3.8/3.9 retain isolated-profile and consolidated host verification gates; task 3.10 retains its separate recovery implementation approval. Phase-4 association/writer branches and later cleanup/compression remain gated. Existing completed phase-0/1 tasks remain verified; later copy/association/recovery/control-family/cache/cleanup safety gates remain unchanged. Each completed task still requires exact-line guarding and refreshed Apply verification.

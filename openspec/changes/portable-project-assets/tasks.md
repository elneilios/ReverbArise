# Tasks

Phase 0 was authorized on 2026-11-06. Checked tasks record verified phase-0 evidence only; owner-review tasks and all later phases remain gated. See [measured results](../../../docs/validation/2026-11-06-issue-8-storage-audit.md) and [contract/rollback matrix](../../../docs/development/asset-storage-contracts.md). Later phases require separate explicit approval. Phase 0 permits disposable fixture writes only; it does not authorize relocation, deletion or other changes to user storage, DSP or production storage behavior. See design.md for phase gates and rollback.

## 1. Contract and measurement gate

- [ ] 1.1 Complete the owner-reviewed version/readiness matrix and handoff against main df765d95170caaaab10a646e43088ea3be66ce53 (or explicitly record a newer baseline): processor envelope v8, creative v1–11, capture schemas 1/2, Export v2/unversioned legacy. Use the design reconciliation as static input only; verify current source/context and all Export-history ownership contracts without editing UI/state machine/capture semantics. This planning revision does not complete the task.
- [x] 1.2 Create the dated deterministic disposable-root growth fixture described in the evidence report; verify 3–5 minute dense/sparse input, silence/gaps, forty swells, repeated IR writes/generations and shape-only edits through per-step bytes/counts/hash reports rather than theoretical totals.
- [x] 1.3 Exercise reopen, Clear, duplicate, old snapshots and concurrent processes in disposable roots; verify unchanged prior source/render hashes and distinguish shared directory IDs from independently accepted maps in the report.
- [ ] 1.4 Perform the optional FL Studio/JUCE state-context/path spike without guessing project paths; document supported versions, observations and explicit-folder fallback, with no dependency on the spike finding path support.
- [ ] 1.5 Document the approved version/readiness, backup and binary rollback matrix beside the audit; verify every supported old format has a fixture and unknown dependency versions fail safely before phase 2 is authorized.

## 2. Read-only inventory and dependency adapters

- [ ] 2.1 Add bounded legacy/new-format read adapters and a dependency visitor covering source, silence/gaps, every schema-2 context IR/span/origin, default IR, each swell's IR/render/calibration, creative-v11 generationSource/autoGenerateOnStop and every saved Export history WAV/descriptor. Verify zero-swell historical-context IRs distinct from current defaults, explicit empty IRs, schema-1 unknown history, frozen/disabled/stale state, v2 and legacy Export gains/laws, and unknown-version closure tests. Do not replay contexts or generate on restore.
- [ ] 2.2 Add read-only selected-root inventory by category/revision and exact-file versus decoded-equivalence duplicate reporting; verify fixture totals, count shared physical objects once, unknown/inaccessible scope and no user writes.
- [ ] 2.3 Add rebuildable observed-snapshot/alias/pin accounting with conservative legacy protection; verify missing index, interrupted catalog update, offline roots and unenumerated FLPs never make durable data eligible.
- [ ] 2.4 Expose location/count/byte/uncertainty summaries in the existing settings/diagnostics area after preparing a native preview; obtain owner look/interaction approval for revised control families before broader rollout and verify component/binding tests without altering #11 Learn controls.
- [ ] 2.5 Update inventory/backup guidance and dated results; verify documented categories/limits against fixture reports and keep native MSVC/CTest separate from fixture-only checks.

## 3. Exact-byte Pack and Relink

- [ ] 3.1 Add immutable portable descriptor/snapshot/receipt versions and bounded parsing with legacy aliases; verify exact source/context spans, coverage semantics, session edits/defaults/auto-generation policy/fingerprints and all Export-history paths/checksums/origins/frames/gain laws. Preserve separate version domains and schema-1 unknown provenance; verify malformed/oversized-state rejection and receipts distinguish managed dependency closure from unenumerated external Playlist files.
- [ ] 3.2 Add coherent accepted-state pinning and explicit per-instance collection scope; verify capture/import/render races never mix epochs and missing instances/history are excluded from completeness claims.
- [ ] 3.3 Add copy-to-destination staging, operation journal, closure verification and completion publication; verify destination-only hashes and fault injection at every WAV/descriptor/snapshot/marker boundary without changing original refs.
- [ ] 3.4 Add cancellation, interrupted-pack inspection/resume and storage diagnostics; verify disk-full, read-only, locked/offline roots and cross-volume copies preserve originals and previous successful packs.
- [ ] 3.5 Add handle-aware containment and Windows path policy; verify traversal, reserved/device/UNC/drive-relative paths, alternate streams, Unicode/long paths, case collisions and symlink/junction replacement races cannot escape the owned operation.
- [ ] 3.6 Add explicit Relink with store/snapshot/hash validation and ambiguous-candidate handling; verify moved folders, changed absolute hints, conflicting store copies and unknown versions without silently adopting newer heads.
- [ ] 3.7 Add Pack/Relink settings controls after native preview approval; verify bindings show scope, excluded pending work, incomplete status and backup responsibility without changing creative/capture workflow.
- [ ] 3.8 Run destination-only restore in an isolated Windows profile with original cache/IRs unavailable; verify exact source/wet/export-history hashes, historical context IRs, positions, peaks, edits/defaults/auto-generation policy and zero convolution/resampling/recapture counters for compatible complete snapshots. Include zero-swell context-only IR dependencies and non-current/legacy-law exports; missing history must fail complete-pack validation without changing accepted state.
- [ ] 3.9 Document the transfer and relink procedure, multi-instance receipts and failure recovery; verify it against one consolidated short FL Studio Save As/duplicate/reopen/transfer smoke test and record actual host/DPI/PC coverage without repeating automated edge cases.

## 4. Optional association and independent future writes

- [ ] 4.1 Add explicit Use This Folder only after pack verification with legacy fallback and existing host-change notification; verify failed/cancelled switch retains old association and successful switch requests a host save without claiming it happened.
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

## Workflow follow-up

- Keep this proposal open for owner review. Do not apply, sync canonical specs, archive or merge from this planning request.
- Future implementation requires a new request and phase-specific review; a checked planning artifact is not evidence any implementation task has run.

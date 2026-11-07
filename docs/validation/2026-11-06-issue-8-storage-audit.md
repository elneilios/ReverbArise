# Issue #8 phase-0 storage audit — 2026-11-06

This report measures the current production storage APIs in disposable synthetic roots. It accompanies the [contract/version and rollback matrix](../development/asset-storage-contracts.md) and [OpenSpec change](../../openspec/changes/archive/2026-11-07-portable-project-assets/design.md). The [2026-11-03 investigation](2026-11-03-asset-storage-investigation.md) remains unchanged historical evidence; its capacity figures were estimates, not these measurements.

## Scope and reproduction

Production source baseline: main `df765d95170caaaab10a646e43088ea3be66ce53`. Existing PR branch was integrated by merge, preserving its original planning commits. The fixture/report revision and SHA-256 of the exact harness/binary are recorded in [machine-readable results](2026-11-06-issue-8-storage-audit.json). Working-tree changes during measurement were the phase-0 harness, target registration, runner and documentation; production `Source/` remained identical to the baseline. Fixture seed 17, Windows x64 MSVC Release, JUCE 9.0.4, 48 kHz stereo float32.

The native harness uses `CaptureSession`, `SwellAssetStore`, `readSourceInterval`, `renderReverseConvolution`, `prepareShape`, `SongMix` and `publishSongExport`. It does not generate stand-in WAVs/JSON in place of the storage APIs. Capture is staged in five-second chunks and committed once per pass, with accepted schema-2 context metadata matching current workflow publication. The pinned contexts explicitly lack an IR, exercising supported dry-only capture; IRs are imported later. Dense source is five minutes of deterministic channel-distinct finite pseudo-noise; it is a duration/storage stress fixture, not a representative musical or codec benchmark. Sparse source has 150 seconds of audio, 75 seconds of recorded zero and 75 seconds of gaps. Forty overlapping four-second source windows use six four-second IRs, rotated over four full convolution generations. IR import repeats ten identical sets. Name/gain/trim edits run the production shape operation without another convolution/store write. This is API-level measurement, not a claim that the UI/worker always schedules these exact writes.

Run the opt-in fixture after building `preverb_storage_audit`:

```powershell
& {
    ./scripts/measure-storage.ps1
    if ($LASTEXITCODE -ne 0) { throw 'Native audit failed' }
}
```

The runner chooses a new `Pv8/Issue8Audit-<uuid>` child of the Windows temporary directory, retains all synthetic audio and prints the result path. `-OutputParent` can select another disposable parent. No production cache is scanned, moved, linked, repaired or cleaned. Two hidden child processes load the same exact saved snapshot, meet a ready/go barrier, independently replace different one-second spans and save separate locators. A verifier restores both maps and the original, then rehashes every pre-existing fixture file. Full inventories identify file paths, category, logical/allocated bytes and SHA-256; fixture input IRs and synthetic host-state JSON are distinguished from managed storage. NTFS allocation is queried with `GetFileInformationByHandleEx(FileStandardInfo)`; it is not inferred from logical size. No hardlinks are introduced.

## Measurements

| Step | Logical bytes | Files |
| --- | ---: | ---: |
| dense_300s_first | 115220571 | 61 |
| identical_dense_pass | 115220571 | 61 |
| changed_dense_pass_2 | 230441142 | 122 |
| changed_dense_pass_3 | 345661713 | 183 |
| sparse_300s_150audio_75silence_75gap | 403275721 | 214 |
| six_4s_ir_imports | 421711699 | 232 |
| ten_equal_sets_of_six_ir_imports | 504685885 | 340 |
| forty_swell_generation_1 | 627595159 | 420 |
| forty_swell_generation_2 | 750504433 | 500 |
| forty_swell_generation_3 | 873413707 | 580 |
| forty_swell_generation_4 | 996322981 | 660 |
| saved_session | 996399807 | 661 |
| forty_name_gain_trim_shape_edits | 996399807 | 661 |
| old_snapshot_and_reopen | 996399807 | 661 |
| clear_then_restore | 996400051 | 662 |
| failed_snapshot_publication | 998320259 | 664 |
| aborted_pass_retained_source | 1000240467 | 666 |
| managed_export | 1115440571 | 667 |
| final_before_concurrent_writers | 1115518591 | 668 |

| Final category | Logical bytes | Files |
| --- | ---: | ---: |
| capture_wav | 407062256 | 214 |
| exports | 115200104 | 1 |
| fixture_external_ir | 9216624 | 6 |
| fixture_host_state | 154846 | 2 |
| ir_json | 27300 | 60 |
| ir_wav | 92166240 | 60 |
| manifests | 54125 | 5 |
| render_json | 101736 | 160 |
| render_wav | 491535360 | 160 |

Per-step counts/bytes, category totals, final file inventories, exact duplicate bytes, CPU/wall time and peak working set are in the JSON. Timing includes scan/hash overhead and the parent process's work, not child CPU; the separate post-writer scan reports its own process counters. The native build tree was cold with reused pinned dependency sources; runtime OS/file caches were uncontrolled. No comparable baseline speedup is claimed.

Before concurrent writers: **1115518591 logical bytes** in 668 files; **1117195224 allocated bytes**. Exact-byte redundancy across all inventoried categories is 151689568 bytes. Selected current dependencies total 420163311 bytes; 695355280 bytes lie outside that selected set. Final parent-process measurement: 49.299 seconds wall, 42.766 seconds CPU, 51023872 bytes peak working set. After the two synchronized writers: 1119388769 bytes across 676 files. The retained successful root is `%TEMP%\Pv8\Issue8Audit-fc0fa24c`.

The selected-current-dependency set contains the latest dense/sparse capture records, their audio, the session's six retained IR/library entries, forty latest renders and the managed export. Bytes outside that set include old source/render versions, repeated IR imports, failed/aborted source files, fixture input audio and synthetic host-state files. This is a **fixture-specific selection**, not a full old-FLP reference graph, an orphan count or reclaimable storage. Every durable file remains protected. Exact-byte duplicates do not establish deletion eligibility; decoded IR content equality was also checked independently of filename reuse.

## Observations and safety findings

- One dense five-minute pass produces 60 bounded capture WAVs and one immutable manifest. An identical full pass preserves its reference and adds no files. Two changed passes retain all previous chunks/manifests. Source storage is proportional to retained changes, not bounded by the 256 MiB resident working allowance.
- The sparse case proves recorded silence has readable coverage with zero samples while gaps remain unreadable. Audio files store only its 150 nonzero seconds; silence/gaps are not interchangeable. Schema-2 context ownership is retained in both dense and sparse captures; separate small compatibility fixtures exercise schema 1.
- Repeated identical IR imports have equal decoded content identities but different managed paths and additional WAV/descriptor pairs. There is no content-addressed store lookup in this path. Keep immutable aliases and old references if reuse is later designed; do not delete the measured duplicates.
- Source inspection distinguishes import from reuse: `SwellEngine.cpp::importJob` reads an already managed IR when supplied; external import calls the writing path before deduplicating the remembered library list by decoded content. Selecting an existing managed choice need not write another IR. The ten-import fixture measures repeated external imports, not every UI selection or engine scheduling path.
- Four actual convolution generations create 160 full wet WAVs and descriptors. Forty name/gain/trim/shape operations add **zero files and zero bytes** and preserve prior hashes. UI/host scheduling is not inferred solely from this direct shape-path test.
- Old saved capture restore recovers the original samples after newer passes; the saved creative JSON restores all forty render references, and their IR/render files verify. Clear and reopen do not discard prior files. Failed manifest publication and aborted capture leave previous accepted references valid and can retain extra source files. Such files may be sole recordings and cannot enter scratch cleanup.
- The concurrent-process fixture demonstrates shared legacy project/instance IDs and directories with distinct immutable snapshots and independently accepted maps. Each clone sees its own changed span and the original data in the other writer's span. Every file present before the writers remains byte-identical. This does not establish host-specific Save As notification behavior, new writer leases or a complete general concurrency proof.
- Managed export is created and verified under the instance's `exports/` directory; the saved synthetic session retains its history descriptor. It remains protected with older renders/source and external Playlist copies. The audit never deletes an export.

## Compatibility, path investigation and validation

The repository wrapper `scripts/build.ps1 -Configuration Release -Test` configured successfully, but its broad cold build was deliberately stopped while compiling unrelated tests; its full build/CTest result is **not passing/not claimed**. Child processes were verified stopped and the per-tree lock released before targeted commands. Cached source/build artifacts were retained.

Focused native Release build used the existing `windows-msvc-release` preset under the same `Enter-BuildLock` handle, targeting `preverb_storage_audit`, `preverb_persistent_capture_test`, `preverb_capture_transaction_test`, `preverb_swell_asset_test`, `preverb_swell_session_test`, `preverb_song_export_test`, `preverb_workflow_processor_test`, `preverb_processor_test` and `preverb_generation_configuration_test`. The already built `preverb_capture_context_test` was included in the matching focused CTest selection. CTest results are recorded below; these are native MSVC tests, distinct from the opt-in measured API fixture. No full-suite, Debug or FL Studio pass is claimed.

Focused CTest: **10/10 passed**. Local logs: `out/audit-focused-build.log`, `out/audit-focused-ctest.log`; allocation-corrected audit rebuild log: `out/audit-final-build.log`. The measured native fixture and synchronized-writer verifier also passed. OpenSpec strict validation, relative-link checks and Git whitespace/scope checks are recorded with the PR update.

Initial audit iterations corrected fixture setup (the export mix must be measured before publication; schema-2 recapture/clone writes must carry accepted context) and allocation reporting (compressed/logical size is not allocation size). Dense measurements were upgraded from the contextless compatibility adapter to current schema-2 workflow metadata; the committed result comes from a complete rerun of every scenario with that contract and its exact source/binary hashes. Failed synthetic runs were retained, never used as passing measurements. Reused JUCE source was verified at tag 9.0.4 / `3fb5cbc0340758596dac39f7c8de47d562b4896e`; WavPack at pinned `5803634a030e2a11dba602ba057b89cc34486c67`.

The [matrix](../development/asset-storage-contracts.md) records all current version domains, fixture locations, dependencies, readiness and rollback limits. Generated creative fixtures remove later-introduced fields for versions 1–11, retain a stable swell and validate compatibility defaults. Unknown creative/capture versions leave output unchanged. The additional engine fixture proves structural decoding can accept engine 2 while `SwellAssetStore::verify` rejects it and still verifies the retained engine-1 file. Existing native tests cover processor envelopes 1–8 and legacy/v2 Export metadata. These are synthetic codec/native tests, not FLPs produced by every old binary.

A separate disposable long-path probe under the issue worktree failed at second 0 with `Uncommitted: Could not create the WAV chunk output stream`; no accepted capture was published. Short temporary-root captures succeeded. This is an observed path-length/environment limitation, not proof of universal long-path support or its exact threshold. The report does not silently fix production path handling. Future Pack/Relink still requires measured Unicode/long-path/containment/error behavior before acceptance; the optional host metadata investigation cannot remove that gate. Probe log: `out/audit-long-path-probe.log`.

The optional FL Studio runtime path spike was not performed. The matrix records inspected JUCE/VST3 versions, official host documentation and the actual byte-buffer state handoff; no FL Studio version is claimed to supply a usable FLP path. Explicit folder selection remains the design fallback. No host path was guessed.

## Phase boundary and remaining coverage

Tasks 1.2 and 1.3 are tracked individually after their fixture checks. The contract matrix and backup/rollback guidance are delivered for owner review; tasks 1.1 and 1.5 retain their explicit owner-review requirements. Task 1.4 remains open because its optional runtime spike was not run. No later-phase task is complete or authorized by this report.

No actual FL Studio project was recorded, saved, duplicated or transferred; no second Windows profile/PC, historical binary, cloud-sync/offline drive, real disk-full/power-loss, compression, allocation-pressure or DPI matrix was tested. Manifest publication obstruction is fault injection, not a disk-full test. The five-minute synthetic fixture does not establish real-song deduplication/compression ratios or universal growth limits. The runtime path limitation and external-reference uncertainty remain concrete input to later gates.

Phase 1/2 stay closed pending the prescribed review. No default-root change, portable reader/writer, relink, user-visible inventory, deduplication, codec adoption, quarantine or cleanup was implemented. Protect pre-upgrade FLPs, binaries and complete legacy stores; age, duplicate bytes, Clear and absent current definitions never establish safe reclamation.

# Portable asset backup backend — issue #8

Phase 2 supplies a native exact-byte backup backend and the owner-approved Storage Backup/Restore/Locate composition. The owner accepted the native preview on 2026-11-06 ("looks fine, carry on"); it is now integrated into production Settings with the existing comfortable focus-revealing viewport. The [scoped FL Studio smoke](../validation/2026-11-06-issue-8-fl-studio-storage-smoke.md) now passes; isolated-profile restoration remains a gate. Existing user-data storage remains authoritative. No automatic DAW-folder discovery or live working-folder switch is implemented.

## Scope and formats

`Storage/PortableAssets.h` is control/worker-only. `PreverbAudioProcessor::pinPortableAssets` obtains a coherent accepted source/creative pair without restoring state or generating audio; `prepare` pins the required immutable source files and checks metadata and SHA-256. Pending capture/render/import work is excluded. A changing or loading accepted pair fails and can be retried.

Store, snapshot, descriptor, receipt and operation-journal domains each start at version 1, with independent constants and reader version 1. Unknown dependency-bearing versions/keys, unsafe aliases, excessive nesting/size and missing closure fail without replacing the caller's previous output. The snapshot retains processor XML, capture JSON and exact legacy dependency aliases, including every historical context IR and saved Export-history file. Schema-1 context provenance stays explicitly unknown. Production legacy codecs and saved DAW-state formats are unchanged.

Raw WAV bytes use `objects/sha256/<prefix>/<sha>.wav`; raw source/IR/render metadata uses `descriptors/<sha>.json`. Only identical bytes are reused inside the selected backup store. Decoded equivalence, including normalized signed zero, never authorizes reuse or deletion. A receipt identifies one exact store UUID/snapshot hash and states that pending work, other instances, older FLPs and external Playlist copies are excluded. Its path/byte counts are logical dependency totals, not physical store size or whole-project coverage.

Verification means exact file hashes, supported metadata and complete declared dependencies. It does not independently certify playable audio or generation readiness for arbitrary forged files. Existing capture/IR/render readers retain their format/sample validation and working-memory admission. No source resampling, convolution or historical-context replay occurs during pack/locate/restore copying.

## Transaction and recovery

1. Explicitly choose an existing empty folder or a supported existing backup store, separate from the source tree. Never claim an arbitrary populated folder as a store.
2. `backup` writes an immutable operation plan under `transactions/<operation>/plan.json`, stages copies locally and flushes/verifies each publication. Destination hashes and metadata must match before publishing the snapshot and completion receipt, last.
3. `locate` requires the exact expected store UUID and snapshot digest. It verifies the receipt and full closure from the selected folder; it never chooses a latest head. A moved folder can resolve; a conflicting/corrupt copy fails independently.
4. After reopening without process-local hints, `locateReceipt` takes an explicitly selected completion receipt inside `receipts/`, validates its filename and locator through pinned handles, then performs the same complete verification. It never enumerates or chooses the newest receipt. The UI requires matching project/instance IDs.
5. `restoreCopy` explicitly recreates legacy `<project>/<instance>/<alias>` files under a selected existing recovery projects root. Existing identical files are verified; conflicting bytes are preserved and rejected. This backend does not switch the live plugin root, adopt state or request a DAW save.

Cancellation/failure retains originals, previous complete snapshots and incomplete transaction evidence. `inspectInterrupted` reads a known operation plan; `resume` requires an explicitly supplied source root and revalidates already copied objects. Journal path hints grant no discovery authority. If every object was already copied, completion can resume without the originals. Disk-full/read-only/locked/offline diagnostics remain failures, never partial success. There is no cleanup, quarantine or delete API.

Back up each instance and retain its exact locator/receipt. Save the FLP and DAW/external Playlist audio separately. See the operator procedure below; automatic saved-locator integration is not delivered. Keep original files, the pre-upgrade FLP and original plugin binary for rollback; do not overwrite the sole rollback FLP.

## Windows policy and bounds

The backend pins local drive-root ancestors and opens one path component at a time using rooted native handles with reparse traversal disabled. Publication uses a pinned target-directory handle, never an untrusted reconstructed absolute path. Volume-GUID resolved identities enforce nonoverlap. Reparse points/junctions, UNC/device/mapped-network roots, alternate streams, reserved names, drive-relative paths, case-conflicting aliases and traversal are rejected. Unicode selected parent folders are supported. The initial policy rejects paths over 240 UTF-16 units and aliases over 192 characters; unsupported paths produce an explicit failure rather than a fallback traversal.

Limits include 20,000 files, 16 GiB per dependency, 64 GiB total dependency bytes, 64 MiB snapshot/journal documents and 8 MiB individual dependency metadata; small store/receipt documents are bounded separately. JSON/XML structural preflight bounds nesting and node/member counts before ordinary decoding. Free-space admission is conservative and cannot guarantee later OS writes. Cancellation is cooperative between bounded copies; synchronous OS calls can still block the worker. No filesystem work or new locking is placed in the audio callback.

## Operator procedure and remaining recovery boundary

Open Settings → Storage; the inventory scans on page opening. Scroll to Backup and recovery. Back up assets selects an existing empty folder or valid backup store. Wait for Backup complete, then copy that entire folder together with the separately saved FLP/external audio. Repeat for every participating instance and retain its receipt; no instance can discover all other instances or older FLPs.

Open the matching DAW project before Locate/Restore. With no in-memory backup locator, explicitly select `receipts/<snapshot-hash>.json` inside the copied folder. Subsequent actions in the same page may select the known backup folder instead. Locate validates only; Restore copies exact aliases into the displayed current projects root. It never changes the accepted arrangement, source reference, workflow mode or generation policy. Existing conflicting bytes fail without overwrite. Stop operation cancels cooperatively and incomplete transactions remain available to backend inspection/resume; no UI resume/cleanup command is delivered.

The controls currently require a coherent available accepted source to establish their session scope. They do not provide task 3.10's missing-source admission, automatic asset reload, arrangement recovery or Re-learn flow. Consequently this integration alone does not establish fresh-profile end-to-end portability. Do not describe restore-copy completion as live recovery. Keep this limitation and tasks 3.8/3.10 explicit until verified recovery is delivered; task 3.9 records only the scoped same-profile host smoke.

## Native review

`preverb_portable_storage_preview --review` opens the candidate using disposable fixtures. The three buttons perform real backup, exact-backup location and restore-copy operations, with native folder selection and explicit receipt-file selection after reopening. Restore goes to the displayed disposable recovery destination. The composition reuses shared controls/tokens and guards picker/results by session identity/epoch. Hiding/closing cancels work. It deliberately does not exercise live association or missing-asset Re-learn recovery.

See [dated phase-2 evidence](../validation/2026-11-06-issue-8-portable-backend.md), [version/rollback matrix](asset-storage-contracts.md) and the active `portable-project-assets` tasks for remaining approval gates.

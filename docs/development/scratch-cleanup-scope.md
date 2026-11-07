# Phase 5 scratch cleanup scope

The owner approved this scope under task 5.1 of `portable-project-assets` on 7 November 2026. The first phase 5 increment offers an opt-in, read-only native review preview for proven leftovers from a cancelled Backup in one explicitly selected managed backup store. Quarantine, undo, permanent removal and production enablement have later gates. The current Storage inventory continues to report zero reclaimable bytes. [Issue #8](https://github.com/elneilios/ReverbArise/issues/8)

## What current backups can prove

Backup writes an operation plan at `transactions/<operation>/plan.json` with a version, operation ID, store ID and the intended snapshot. It stages `part-<uuid>` files under the transaction while copying, then verifies and publishes immutable objects, a snapshot and a completion receipt. The same staging helper also writes transaction metadata. An interrupted plan can be inspected and resumed using verified published objects and, when needed, the original source. The plan alone supplies neither a terminal cancellation record nor the identity and hash of each unpublished part. A missing receipt can also mean interruption between snapshot and receipt publication. No existing plan or filename qualifies a leftover for preview. [Pack publication and resume](../../Source/Storage/PortableAssets.cpp) · [Backup behavior and Windows path policy](portable-asset-backups.md)

## Read-only preview candidate

Backup can write `transactions/<operation>/scratch-cancelled.json` after a verified payload copy is cancelled before its first publication attempt. The producer closes the staged file before committing the separate ownership record. It binds the record to the existing plan and to both physical roots. A fault, unsealed partial file, Fork, Restore or Resume produces no eligible record. The original plan, snapshots and receipts keep their existing formats.

The preview opens existing handles only. It scans the chosen store's transaction namespace and verifies the original instance folder supplied by its caller. A supported legacy Resume holds the operation plan open, which blocks the preview's exclusive read of that plan. The staging file must also permit an exclusive read. The preview remains advisory. A snapshot or receipt for the exact Backup plan, or an inability to inspect those paths, excludes that transaction. The initial reader uses the exact original instance as its surviving-copy proof. A missing original excludes the candidate, even if another backup might exist. No folder is inferred from a journal. At most 256 transaction entries and 64 GiB of candidate and counterpart bytes are inspected per preview. Cancelling or reaching a bound clears the eligible list.

The disposable native review uses existing text roles, `ActionButton` controls and a read-only scrollable report. The review executable binds its own synthetic original instance. Its picker opens on the synthetic backup store. Production Settings keeps its existing inventory and Backup/Restore controls. Preview rollout remains separate from this review build.

## Maintenance admission

Task 5.3 adds a worker-only `withRevalidatedScratch` scope for one exact path selected from a complete preview. The displayed report carries a digest binding the physical roots, ownership record, operation plan, staged file and verified counterpart. The worker repeats path, identity, byte and reference checks against that evidence. A byte-identical replacement with a new file identity needs a fresh preview. The evidence identifies what was reviewed. Action approval still belongs to the later UI and operation gates.

Backup, Resume, Locate and associated-store handles already pin the existing `store.json` with shared read access. Maintenance opens that same ordinary file exclusively and validates its store ID. It creates no lock file. A busy or unsupported store refuses the scope. Holding the exclusive handle prevents a cooperating reader or writer from entering, including a supported older build that uses the existing shared open. The operation plan and staged file must also allow exclusive access. The surviving original stays pinned against writes and removal while ordinary readers of that intact copy can continue.

Pack readers address published objects and descriptors through their fixed paths. Working and recovery aliases resolve beneath their instance roots. The private staging path is outside those namespaces. Maintenance also checks the exact plan's snapshot and receipt paths before and after verifying bytes. Either artifact can indicate new recovery state, even when the other is absent. Unsupported, inaccessible or changed metadata keeps the candidate protected.

The synchronous callback receives an opaque scope while the store, pinned directories, plan, ownership record, candidate and counterpart remain open. Quarantine keeps that scope through publication and uses the same held candidate handle and pinned directory. An earlier report or a reopened pathname cannot authorize a move. Coordination and filesystem work stay on the worker path.

## Quarantine and undo

Task 5.4 adds worker APIs for an explicitly approved preview entry and an explicitly selected undo operation. Quarantine currently requires a local NTFS store. It uses the existing exclusive store reservation and the already revalidated staging handle. The move stays inside the pinned store on the same volume. It uses a handle-relative rename with replacement disabled. [Windows rename contract](https://learn.microsoft.com/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_file_rename_information)

Each cancelled Backup operation owns one quarantine directory at `quarantine/<operation>/`. `move.json` binds the exact ownership proof, store, source staging path, target path, both physical parent directories and the file identity, byte count and SHA-256. The same physical file is renamed to `payload` after the intent is durable. `moved.json` records verified completion. `undo.json` records explicit intent to return the payload to its original staged path. `restored.json` records verified undo. Each phase record binds the exact move intent. Journal publication flushes the staged bytes, renames without overwrite on a write-through handle, and flushes the published file. [Windows file caching](https://learn.microsoft.com/en-us/windows/win32/fileio/file-caching) · [NTFS write-through rename behavior](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/ns-fileapi-createfile2_extended_parameters)

Inspection opens the selected operation under the same store reservation. It verifies the journal against the original Backup proof, then reconciles the two pinned locations by identity and hash.

| Observed evidence | Result |
| --- | --- |
| No committed move intent and no quarantine payload or phase marker | No committed quarantine operation. Unpublished journal bytes stay protected. |
| Verified move intent with the file still in staging and no completion or undo | Staged. A fresh explicit quarantine retry can reuse the exact intent. |
| Only the verified quarantine payload exists | Quarantined. This also covers interruption after the move and before its completion record. An undo intent in this state means undo can be retried explicitly. |
| Valid undo intent and only the verified staging file exists | Restored. A repeated undo can complete the interrupted journal record without moving the file again. |
| Both locations exist, neither exists, a directory changed, or proof is unreadable or contradictory | Ambiguous. Preserve all files and require inspection. |

Inspection never moves files. Undo reopens and verifies the payload, commits its intent and refuses any occupied original destination. It restores the staged copy even if the separate original audio later becomes unavailable, because undo preserves the copy rather than removing it. Completed or interrupted undo history retires the prior quarantine review. This increment supports one reversible quarantine cycle per cancelled Backup copy. Any failure can occur after a rename, so callers must inspect the operation again before reporting its state. Later work owns grace accounting, permanent removal and production controls.

## Eligibility matrix

“Conditional” means eligible for **preview** only when every proof below succeeds. All other rows are excluded from phase 5 scratch actions, including when the current project has no reference to them.

| File or state | Initial eligibility | Reason and handling |
| --- | --- | --- |
| Private, unpublished Backup payload copy from a durably cancelled operation | Conditional | A supported journal must identify the exact staged file and a separately verified surviving copy. It must also show that publication never adopted that file. |
| Pack plan, journal, staging metadata, completion receipt, snapshot or published object | Excluded | Retain operation history, resume inputs and accepted backup dependencies. A published object stays protected even when the operation has no receipt. |
| Active, resumable or interrupted pack with no verified terminal cancellation | Excluded | Retain files when a process or recovery operation may need them. An abandoned or crashed operation, timeout, disk-full error or missing receipt cannot stand in for cancellation. |
| Capture chunks, coverage manifests, context/provenance and all capture staging | Excluded | An abandoned pass can contain the only surviving recording. Report it separately for explicit recovery. |
| Imported, bundled, default, context and per-swell IR assets and descriptors | Excluded | Retain exact bytes used by current or historical state and capture contexts. |
| Current or historical wet renders, frozen results, descriptors and retained revisions | Excluded | Earlier FLPs can still use them after Clear, deletion, regeneration or Save As. Exact hashes and apparent rebuildability cannot establish ownership or disposal rights. |
| Export history, DAW Playlist WAVs, user Save destinations and export staging | Excluded | Treat exports as user-owned even inside an asset store. Temporary export names are outside the approved producer scope. |
| Restore, Locate, Fork, recovery-workspace and other writer leftovers | Excluded | They have separate ownership and rollback contracts. Only the Backup payload-copy producer is in scope. |
| Legacy `.tmp`, `.part`, `part-*`, `.unpublished-*` and any unknown or unjournaled file | Excluded | Names, extensions, location, age and duplicate hashes are insufficient. Preserve files that cannot be classified. |
| Inaccessible or ambiguous files, linked files, unknown journal versions and changed roots | Excluded | Show why the affected scope is incomplete. Unknown or unmeasured entries do not contribute eligible counts or bytes. |

## Proof required for a candidate

All checks must pass for the same file. A missing or stale proof excludes it. The later implementation must keep the evidence bounded by the existing Windows path, metadata and file budgets.

1. **Producer and location.** Select one local managed backup store explicitly. Verify its store ID and pinned volume and directory identities. Accept only a normal file in the exact private staging directory for an operation that a supported, versioned ownership journal identifies as a Backup payload copy. Open paths by pinned directory handles. Exclude reparse points, hard links, alternate streams, overlaps with source or quarantine, and unsupported paths. Store relative candidate paths and never use absolute paths from metadata to open files.
2. **Identity and terminal state.** The future producer records the operation ID, candidate path, file identity, size and SHA-256 of the sealed staged bytes. Record terminal cancellation durably after the writer has stopped and closed the file. Require exact identity and byte verification against the record. A partially written or unsealed file, ambiguous journal transition, or publish or resume in flight blocks the candidate. A versioned ownership record can coexist with the existing plan format so older backups remain readable. Existing transactions receive no inferred ownership or backfill.
3. **Protection and another copy.** Prove that the staging file was never published as an object or alias and that no snapshot, accepted reference, pack, reader, export, job or recovery operation depends on it. Verify a readable byte-identical copy with a distinct physical file identity through the exact original accepted dependency or a verified completed backup. A digest alone cannot prove that another copy still exists. Exclude the candidate if no counterpart can be verified, metadata is incompatible or reference coverage is uncertain. The private staging namespace and publication records must establish that an older FLP cannot address the staging path through supported readers. Unenumerated FLPs continue to protect all durable and possibly referenced assets.
4. **Exclusive action.** Treat the preview as advisory. Before any move or removal, take worker-side cross-process maintenance coordination and recheck store, journal, leases, open handles, identity, bytes, counterpart and references against the exact reviewed candidates. A new job, lost counterpart, changed file, busy lock, unsupported writer or inaccessible metadata skips that file or blocks the affected scope. Protect the evidence through the operation so a path swap between verification and action cannot change the target. Keep scans, waits and filesystem work off the audio callback.

## Actions and review gates

| Step | Review and observable behavior |
| --- | --- |
| 5.1 Scope | Owner reviews this matrix and the Backup-only boundary before implementation. |
| 5.2 Preview | User requests a read-only preview for the selected store. List exact eligible relative paths, logical file count and bytes, store and operation scope and reason. Separate protected and unknown entries. A partial or cancelled scan grants no action authority. Reuse approved Storage controls and obtain native review for any new or revised control family before rollout. |
| 5.3 Revalidation | Prove cross-process ownership and fail-closed behavior before a preview can authorize a file move. Production maintenance remains off by default. |
| 5.4 Quarantine and undo | After explicit approval of the exact candidates, write a durable move journal and move each revalidated file on the same volume with no overwrite. Record the original and quarantine identities and verify bytes. An interrupted move reconciles the two locations after restart. Conflicts or ambiguous state preserve both sides for inspection. Undo requires hash verification and refuses overwrite. A move to quarantine retains the bytes on disk. |
| 5.5 Permanent removal | Retain quarantined files for at least 30 days from verified quarantine. An elapsed deadline alone leaves files in place. A second, explicit review lists the exact files after grace and requires fresh proof, including the surviving copy. A missing journal or uncertain deadline blocks removal. |
| 5.6 Recovery | Report abandoned source or available snapshots separately. Require explicit, verified recovery and preserve current accepted references and original files on failure. Recovery cannot confer scratch eligibility. |
| 5.7 Enablement | Review dated fault-injection results and user guidance before enabling the optional workflow. Keep every destructive operation opt-in. |

The preview reports logical bytes in eligible scratch. Physical or allocated bytes, where measured, stay separate. Quarantine does not free disk space. Excluded or unknown bytes and quarantined files are never presented as immediately reclaimable.

## Acceptance fixtures for later implementation

Run against disposable roots, with exact before and after path, hash and accepted-state checks. Use focused native tests for each acceptance case and separate any later UI review from backend verification.

| Fixture | Expected result |
| --- | --- |
| One sealed, cancelled Backup copy and a separate verified counterpart, mixed with every protected matrix class | Preview lists exactly one file and its exact bytes and reason. Protected bytes and accepted references remain unchanged. |
| Same names and hashes with a v1-only plan, missing or corrupt journal, unsupported version, absent receipt as the sole cancellation evidence, unsealed copy, ordinary I/O failure or interrupted producer | Zero eligible files for those operations. Plans, objects and available recovery inputs stay intact. |
| Candidate loses its counterpart, gains a recovery reference or lease, changes contents or path, is replaced with a link, or is locked after preview | Revalidation skips the changed candidate. Exercise independent processes and pathname replacement without blocking the callback. |
| Crash before and after each journal and move boundary, interrupted undo, and a conflicting original destination | Restart preserves recoverable files, verifies identity before undo and refuses overwrite or ambiguous adoption. |
| Grace not elapsed, grace elapsed without review, explicit review with newly missing evidence, and explicit valid review | The first three retain bytes. Only the last removes the specifically approved quarantined copy after fresh checks. |
| Abandoned capture and prior accepted snapshot, including interrupted recovery | Both remain protected. Only an explicit, verified recovery may change the accepted reference. |

Phase 5 excludes durable asset collection, automatic eviction, deduplication, compression, retention changes and any default-folder cutover. Issue #8 remains open for later work. [OpenSpec tasks](../../openspec/changes/portable-project-assets/tasks.md) · [Retention requirements](../../openspec/changes/portable-project-assets/specs/asset-retention/spec.md)

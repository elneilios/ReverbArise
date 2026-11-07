# Issue #8 phase 5 maintenance coordination

Task 5.3 adds worker-only admission for one reviewed scratch path. It uses the
existing shared store metadata open to coordinate across processes. It repeats
root, journal, identity, hash and surviving-copy checks with the handles held.
Exact-plan snapshots and receipts veto admission, including a snapshot that
arrives during the checks. See the [approved scope and maintenance contract](../development/scratch-cleanup-scope.md).

MSVC Release wrapper `20261107-115143-e8959187` passed for the implementation
based on `81267e3`. Focused CTest passed **12/12 (74.86 s)**. The selection
included the scratch preview, bindings, maintenance, callback, portable storage,
working-folder, snapshot, recovery, inventory, workflow and Export regressions.

- Independent native processes held a verified reader, a live Resume job and
  an exclusive maintenance scope. Maintenance refused the first two. While the
  exclusive scope was held, another maintainer, Backup, Resume and Locate were
  refused. A separate store could still be admitted. An external rename of the
  pinned candidate failed. Files remained available after lease release.
- The tests rejected an unreviewed path, incomplete report, busy staging file,
  locked plan and exclusive lock on the counterpart. A shared reader of the
  intact original remained compatible. Replacing that original with a new file
  of identical bytes invalidated the evidence. A staging junction introduced
  after lease acquisition was refused and the outside fixture hash matched.
- A successful Resume published a new same-plan recovery reference after
  preview. Admission rejected that stale review. A second fixture wrote a
  valid snapshot between the first reference check and final admission. The
  worker refused it and retained both the staged file and snapshot. Cancellation
  also refused admission.
- With the maintenance scope deliberately held on a worker, 32 real processor
  audio callbacks completed before the lease was released. The allocation guard
  observed zero allocations and deallocations. Tests used disposable roots and
  did not open FL Studio or a desktop review window.

Strict OpenSpec validation, UI standards and offline guide currency passed.
Guarded Apply refresh confirmed task 5.3 and 27/36 overall. The admission scope
exposes no mutation operation. Quarantine, undo, permanent removal, recovery
reporting and production enablement remain separate. Issue #8 stays open.

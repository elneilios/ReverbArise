# Issue #8 phase 5 read-only scratch preview

The owner approved task 5.1's scope and eligibility matrix on 7 November 2026 and
authorized the read-only preview in draft PR #118. See the [approved scope and
eligibility matrix](../development/scratch-cleanup-scope.md).

## Native verification

MSVC Release build wrapper `20261107-113637-936d25bc` passed on the phase 5
implementation based on `7b6659b`. Focused CTest passed **7/7 (11.31 s)**.
The selection covered the scratch reader and component bindings, portable Backup
and Resume, working-folder association, portable snapshots and recovery, and
inventory. The component fixture rendered an offscreen image. It did not open an
interactive review window or a DAW.

- A real cancelled Backup left one sealed, unpublished 120-byte copy. The preview
  reported its exact store, operation, relative path, byte count and eligibility
  reason. The mixed store also contained a successful backup, an interrupted
  legacy-style plan with identical staged bytes, an unsealed cancellation, and
  protected capture, IR, render, export and unknown file examples. The preview
  returned only the one proven copy. Before and after file path and SHA-256
  inventories matched across both selected fixture roots.
- A pinned operation plan, unsupported ownership version, wrong original root,
  missing original, hard link and byte-identical replacement all excluded the
  staged file. Cancelled scanning and the transaction-entry bound produced an
  incomplete result with zero eligible bytes. Protected content stayed intact.
- The disposable UI checked opt-in folder selection, cancelled picker, worker
  result binding and exact displayed scope. It uses the existing command buttons
  and text roles. UI standards and user guide currency passed.

The review entry point is `preverb_portable_storage_preview.exe --scratch-review`.
It supplies its own disposable original and Backup store. Preview reads the
selected store and verifies the original instance supplied by the caller. A
completed backup elsewhere does not supply counterpart proof in this increment.
The producer writes a separate record only for a sealed payload copy cancelled
before publication. Older plans remain readable and their leftovers protected.

OpenSpec Apply was refreshed after each completed task. Tasks 5.1 and 5.2 bring
the active change to 26/36 complete. Production Settings integration, maintenance
coordination, quarantine, undo, removal, recovery reporting and enablement remain
behind their existing review and implementation gates. Issue #8 stays open.

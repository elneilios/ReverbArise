# Issue #8 phase 5 quarantine and undo

Task 5.4 implements the worker-side move and explicit undo under the existing
maintenance reservation. A committed move intent binds the private staged
file, the Backup ownership proof and both pinned parent identities. Local NTFS
renames use the existing no-replace native handle mechanism with write-through.
Inspection reconciles the immutable journal and the two possible locations by
identity and hash. See the [quarantine and undo contract](../development/scratch-cleanup-scope.md#quarantine-and-undo).

MSVC Release wrapper `20261107-120903-67eb808e` passed for the implementation
based on `2b421f6`. Focused CTest passed **12/12 (28.15 s)**, including the
quarantine, preview, maintenance, portable Backup and Resume, recovery, callback
and working-folder checks. Tests ran on disposable local NTFS fixtures. The
interruption tests terminate a process and reopen its on-disk state. Physical
power loss and other filesystems were outside this run.

- A child process exited at 21 boundaries. Cases cover before directory or
  journal work, after sealing each journal record, after publishing it and
  before or after either rename. Inspection stayed read-only. The original
  staged file or the quarantine payload retained the same verified hash and
  the separately owned source fixture hashes matched. Explicit retry or undo
  completed the appropriate interrupted state, including a move with a missing
  completion record and a restored file with interrupted journal completion.
- Occupied paths introduced immediately before each rename were preserved
  alongside the original bytes. Neither path was overwritten. Both-path and
  neither-path states, changed staging directory, corrupt payload, mismatched
  intent and a late conflicting undo record were refused without adopting the
  ambiguous data. A repeat undo after verified restoration was idempotent.
- Injected disk-full errors during intent and completion writes preserved the
  pre-move file or the verified quarantine payload respectively. A fresh
  inspection identified the appropriate state and explicit undo restored the
  exact file. An incomplete preview was refused before creating a quarantine
  directory.

Strict OpenSpec validation, UI standards and user-guide currency passed. Apply
confirmed task 5.4 and 28/36 overall. This increment supports one reversible
quarantine cycle per cancelled Backup copy. Production controls, minimum grace
accounting, permanent removal and recovery reporting retain their separate
gates. Issue #8 stays open.

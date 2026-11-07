# Issue #8 scratch and capture-recovery integration gate

Windows x64 MSVC Release wrapper 20261107-122836-64dbd9d7 passed at 86956cc plus the task 5.5 and 5.6 changes. The combined native CTest selection passed **22/22 in 55.51 seconds**. It covered Backup, working folders, source and history recovery, scratch workers, storage bindings, capture, exports, the processor, the common swell engine and WavPack IR import. The final test-only target rebuild used the same build lock and passed the two changed tests in 14.05 seconds.

| Gate | Native evidence |
| --- | --- |
| Exact scratch eligibility and protection | Preview finds the sealed cancelled Backup copy in a mixed store. The wrong root, legacy or incomplete ownership, corrupt or unsupported record, unknown scope, hard link, alternate stream, missing counterpart, replaced identity, pinned plan and partial scan all fail closed. |
| Revalidation and audio safety | Independent-process reader, writer and maintenance lease fixtures veto new work. Stale evidence, locks, junction/path races, new receipts or snapshots and cancellation preserve candidate bytes. The callback fixture runs with the worker reservation held. |
| Reversible quarantine | 21 process exits at journal and rename boundaries, disk-full faults, corrupt payloads, and occupied paths establish restart reconciliation and hash-verified undo. |
| Separate final removal | Expiry alone leaves the file in place. Incomplete grace evidence, changed deadline, active handles, missing original and late references veto final removal. A verified review removes only the pinned quarantine payload. An interrupted completion stays ambiguous. |
| Explicit historical capture recovery | An unindexed audio file and a newer immutable snapshot appear separately. Missing source and injected disk-full block publication. A successful separate copy verifies exact source bytes, historical IR and manifest. Previous accepted reference and original files remain intact. |
| Portable byte sharing | The existing Backup format reuses one exact wet object for two aliases with distinct descriptors, keeps signed-zero-distinct bytes separate and restores all aliases and descriptors without changing originals. |

See the prior [preview](2026-11-07-issue-8-scratch-preview.md), [maintenance](2026-11-07-issue-8-scratch-maintenance.md), [quarantine](2026-11-07-issue-8-scratch-quarantine.md) and [removal](2026-11-07-issue-8-scratch-removal.md) evidence. [Retention and recovery guide](../development/scratch-cleanup-scope.md)

All file operations used disposable roots on local NTFS. This pass used the supported Release build and focused native tests. It did not exercise FL Studio, a fresh Windows profile, physical power loss or the full CTest suite. The dated evidence and guide are ready for separate owner review before optional production cleanup is enabled.

# Issue #8 — phase-4 production Settings integration

Source: integration diff based on `0c80b93`, branch
`codex/8-working-folder-association`, draft PR #112. Native Windows/MSVC Release,
JUCE 9.0.4, reused warm build tree. No cold-build efficiency claim is made.

## Implemented and focused native evidence

- Production Storage uses the approved working-folder family with two pinned
  action rows and scrolling details. Choose verifies without switching; Use
  performs worker-side validation and conditional processor publication. Existing
  scan-retention and Backup/Restore/Locate component checks remain in place.
- Processor checks cover unchanged Learn state/source, memory-only v9 saves,
  saves during restore, independent reopen leases, physical copies and legacy
  reopen. Whole-store loss retains metadata and blocks writes until explicit
  workspace adoption; its incomplete association reopens with original origins.
- Exact Locate and receipt Restore into a selected new store are validated with
  unchanged source references, zero convolution and original hashes retained.
  The move fixture stays within the existing 240-unit native path boundary;
  exceeding it is an explicit unsupported-path error, not a missing-file bypass.
- Two native processor processes hold distinct leases simultaneously, then
  actually capture, import IRs, render swells and export different audio. Each
  saved state reopens with its exact source and a new lease. Existing capture,
  IR, render, manifest and Export-history hashes remain unchanged. This is not
  a two-FL-Studio-process or second-Windows-profile claim.
- Focused native runs passed production processor recovery and Storage Settings
  together (2/2, 5.48 s), then actual writer publication/recovery (5.52 s) and
  production Settings with snapshots (3.66 s). Earlier fixture and binding
  failures were corrected, without weakening the assertions or path policy.
- UI standards passed: 66 files, 59 bounded occurrences; no inventory regeneration
  or new styling token. Native component screenshots are supporting evidence,
  not owner acceptance of untested host/DPI behavior.

## Remaining gates

Full Release-wrapper result is recorded below after completion. Associated
destination-only fresh-profile proof, retained original-binary/pre-upgrade FLP
rollback and the consolidated scoped FL Studio smoke test remain pending.
Task checkboxes are changed only through guarded Apply completion/refresh.
Issue #8 stays open; no cleanup, eviction, compression, release, merge or archive
is authorized by these results.

## Supported wrapper and guarded writer completion

Full supported Release wrapper `20261107-005305-6088d5da` built successfully and
passed 86/87 CTests in 398.97 s. The sole failure was the knob-reset fixture's
Windows foreground prerequisite. Its later direct native `--test-knob-resets`
run passed (`out/phase4-native-reset-direct.stdout.log`). Computer Use activation
was attempted after selecting the returned fixture window; the helper reported
"foreground window did not report a process id". No successful external focus
activation is claimed and no assertion was removed.

After adding cancellation to the production handoff and the associated-profile
fixture cases, wrapper `20261107-011723-f0b241fa` built successfully and passed
86/87 in 337.12 s. Knob reset passed; readability failed a native wait. The
focused readability retry passed 1/1, 2.14 s
(`out/phase4-production-readability-retry.log`). All capture, storage, working-folder,
ordinary/associated recovery and two-process publication cases passed in that
wrapper. No uninterrupted 87/87 pass is claimed; that gate remains open.

Task 4.2 was completed with an exact source-path/line/description guard and a fresh
Apply query confirming 21/36 complete. Tasks 4.1, 4.3 and 4.4 remain open pending
their remaining acceptance, especially profile/host/rollback validation. Native
preparation produced all six legacy/associated profile cases and proved their
original source trees were removed; this is fixture preparation, not a successful
different-SID profile run.
## Final production hardening checkpoint

Warm Release build `20261107-015149-32150b32` completed. The current native
portable-recovery executable passed, including simultaneous process publication,
production folder handoff, normal recovery, exact Locate/Restore and preservation.
Host saves pair source and creative state under the existing control locks, retain
accepted metadata during pending restore, and perform no filesystem lookup.
Raw host XML/creative JSON complexity and relative registry hints are rejected
before parser/path adoption. The authoritative embedded capture manifest wins
over a locally altered same-ID/revision map. Focused affected coverage previously
passed 14/14 (48.62 s); these are native checks, not fresh-profile or FL acceptance.

## Full-run fault and capture checksum lifetime correction

At `841f2a3`, supported wrapper `20261107-020143-a579c352` built successfully
and passed 86/87 in 305.28 s. All native foreground fixtures and Storage/recovery
checks passed. Automatic-coordinator instead crashed; Windows Application event
1000 recorded access violation `0xc0000005` in `bcrypt.dll` at offset `0x6a08`.
Its retained dump is `preverb_automatic_coordinator_test.exe.71052.dmp` in the
current user's CrashDumps directory. Five immediate focused repetitions passed
(24.61 s), so repetition alone did not establish the cause or waive the gate.

Inspection identified a concrete existing capture-checksum lifetime defect:
`calculateSha256` declared the caller-owned BCrypt hash buffer after its handle
owner, freeing the buffer before `BCryptDestroyHash` on every return. The buffer
now belongs to the handle owner and outlives its destructor body. This follows
the [BCrypt lifetime contract](https://learn.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptcreatehash).
It is consistent with the crash, without claiming a symbolized dump proved the
specific failing call. No hashing format, durable identity or callback changes.
The capture test additionally runs 128 concurrent owned-directory write/read
round trips with exact PCM/checksum and deliberate-corruption rejection checks.

Warm wrapper `20261107-020950-3692976e` builds the correction. The production
Storage snapshot check passes; screenshots show the working-folder section at
the top and both pinned action rows, with details scrolling above them.

The actual different-profile launcher was attempted. `Start-Process -Verb RunAs`
reported cancellation before the administrator script started. No different-SID
success or test-account creation is claimed. The profile gate remains open,
alongside the original-binary/pre-upgrade FLP rollback and scoped host smoke.
The installed system bundle was observed at a separate experimental revision
`9a8b9d073e33ea03dac059dc2cdcfa510c3ecd8f` (SHA256
`DED49001B0395EAA96B08810FA771119724E2B50B41EEBCA32D2D29BFA0DCB31`);
it was not replaced and no FLP was modified during this checkpoint.

## Final supported native regression

At clean source revision `c701c5b`, supported Release wrapper `20261107-021227-4842de91`
passed configure/build and **87/87 CTests (276.36 s)** in one uninterrupted run.
The corrected coordinator, native foreground fixtures, production Storage,
capture, association, independent processes, ordinary recovery and Export passed.
Before this run, the focused capture/coordinator/portable-recovery group passed
ten consecutive repetitions each (85.00 s total); the capture case performs
128 concurrent exact round trips per repetition. Earlier failures remain above.

The native gate is satisfied. Different-SID profile validation still did not
start because Windows elevation was canceled. Host installation/smoke and actual
original-binary/pre-upgrade-FLP rollback have not been performed for this revision.
No phase-4 acceptance is inferred from native success; tasks 4.1/4.3/4.4 remain
open, task 4.2 remains complete (21/36 overall), and PR #112 stays draft.
Issue #8 remains open and no original project, store or installed bundle changed.

## Fresh profile, FL Studio and rollback acceptance

The owner accepted Windows elevation on retry. `validate-portable-profile.ps1`
ran under a different Windows SID and exited 0. All six destination-only restores
passed: full, context-only and unfrozen for both legacy and associated states.
Their closures contained 14, 5 and 14 files respectively in each family. Asset
hashes and metadata matched exactly. Each case recorded zero convolution,
resampling and capture staging. The launcher verified removal of the temporary
account and profile. Test EXE SHA-256:
`1FC6CBCF20E1D9AAFB144D8814381E26514F77AC18692E675F768FD0BD1628CC`.

The supported plugin-only Release installer built `ecee561` (the native-tested
source at `c701c5b` plus evidence). A system-folder access denial was resolved with
Windows elevation. FL Studio 26.1.7.5654 on the same Windows PC, with Windows
desktop DPI configured at 96, loaded the system VST3 module directly. Candidate
module SHA-256: `F9AD21C712DC46F93047952BE78E46F709F685304FDB5E66CB52D66C642611A5`.

- Opened a hash-identical copy of the retained phase-2 v8 FLP. Storage used the
  legacy user-data folder. Choosing Assets-A verified the two IR dependency files
  while retaining the active root. Use This Folder switched the root and showed
  the host-save reminder. A separate FLP saved with envelope v9.
- Save As into another directory retained Assets-A in the serialized state. Its
  byte-identical duplicate (SHA-256 `01EEFBEF2D73762C6831D27E6A562A630C72BACC98E0D217E0A5E565454027CC`
  before the fork) reopened with Assets-A. Explicit verification and adoption of
  Assets-B produced a distinct store ID. After saving the fork, the two FLPs
  reopened independently with their respective stores. The original and both
  copies matched the historical SHA-256 values for the WAV and descriptor.
  `fsutil file queryfileid` reported three distinct WAV identities.
- Restored the exact bundle installed before the smoke (`9a8b9d0`, the private
  compatibility test build with a v8 reader) using the installer transaction
  helper with FL Studio closed. Verified complete bundle hashes and the loaded
  system module SHA-256 `DED49001B0395EAA96B08810FA771119724E2B50B41EEBCA32D2D29BFA0DCB31`.
  FL Studio opened the retained v8 FLP and displayed its saved Preverb Hall IR.
  A separate v8 verification save retained the exact default/imported IR metadata
  and project/instance IDs. Both pre-upgrade FLP files retained SHA-256
  `41A37BFD990CF760CEB9CFE8C753BB0BCB1AA8F72C5D43EA1DC38F998E1CBE93`.
  The legacy store matched the historical two-file hashes. The original installed
  bundle is back in place and the phase 4 candidate is retained in the installer backups.

The FLP fixture contains an IR and its descriptor. Captured audio, render and
Export dependency coverage comes from the native and fresh-profile fixtures.
This host check covers one FL Studio version, one PC and the configured DPI above.
Rollback uses the pre-upgrade FLP with the original binary and legacy store.
Associated v9 FLPs require a reader that supports v9.

Tasks 4.1, 4.3 and 4.4 were completed with guarded Apply refreshes, bringing the
active change to 24/36 complete. Issue #8 stays open for the separately gated
phases 5/6.

## Regression after syncing main

Merged main at `89da1e7` and regenerated the user guide review record. The
combined Release build at `4b54774` passed. Wrapper `20261107-084355-2085df8d`
passed 86/87 tests (392.31 s), including all phase 4 storage, compatibility and
recovery tests. The knob-reset fixture timed out waiting for native foreground
focus (`peer=1 foreground=0`). Its focused CTest retry passed 1/1 (11.96 s) with
the original focus assertion. No automated foreground intervention was used.
UI standards passed (67 files, 59 bounded occurrences). The earlier 87/87 run
and the host and profile checks apply to the builds recorded above. The owner
approved skipping the additional uninterrupted rerun on the combined branch.
The full run and focused retry together have passing results for all 87 tests.

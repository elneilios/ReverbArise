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

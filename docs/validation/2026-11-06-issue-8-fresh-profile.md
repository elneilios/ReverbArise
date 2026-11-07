# Issue #8 fresh Windows-profile validation — 2026-11-06

Task 3.8, `portable-project-assets`, PR #31. Tested commit `d39e14e`, MSVC Release / JUCE 9.0.4 on the existing Windows machine. This supersedes only the pending profile result in the [earlier recovery checkpoint](2026-11-06-issue-8-storage-polish-recovery.md); that dated account of setup failures remains preserved.

The owner authorized another attempt. An elevated `scripts/validate-portable-profile.ps1 -Executable out/build/windows-msvc/Release/preverb_portable_recovery_test.exe` run completed successfully. The script prepared exact backups, removed their original fixture roots through fixture teardown, and launched the validator under a newly created standard Windows account with its own loaded profile/environment. Token SID and the default Projects root were checked against that account, not the owner or a redirected same-user folder. No DAW-state adoption or live association switch occurred.

| Case | Exact restored paths | Result |
| --- | ---: | --- |
| Full arrangement, retained imported/default/assigned/historical IRs, frozen disabled result and current/non-current legacy-law Export history | 14 | PASS |
| Zero swells, explicit empty default/library, historical-context-only IR and Export history | 5 | PASS |

Both cases verified every source/wet/export/context dependency hash, exact encoded creative metadata (IDs, positions, shapes/edits, default and auto-generation policy), capture context spans, sample values, silence/gap semantics and saved wet origin/peak. Counters confirmed **zero convolution, zero resampling, zero capture stages and zero rendered results**. The full frozen result reached Ready under compatible original host timing without regeneration. Existing `preverb_portable_assets_test` additionally rejects missing saved Export history before complete-pack publication; its 19-test boundary run passed immediately before this unchanged-binary profile retry.

Executable SHA-256: `4E50D375578D1ED353DCA2E6AD4882B3916AC3FF162C497BD5C5CDAD1CFFD12A`. Child exit **0**. Newly created account and exact SID-owned profile were both removed successfully. Random credentials stayed in memory; no persistent security-policy changes. Structured details: [launcher/profile results](2026-11-06-issue-8-fresh-profile.json). Redundant copied runner executable can be removed while preserving these reports and tiny fixtures.

This is automated native validation on a separate Windows profile on the same PC, not an FL Studio/second-PC/DPI test. The earlier same-profile FL Studio smoke remains separate. Task 3.10 native recovery approval/production integration and all phase-4/later association, ownership, cleanup and compression gates remain unchanged.

## Shared-engine revalidation

The subsequent shared-engine implementation repeats the isolated standard-account proof and adds a genuinely generated unfrozen render. All full/context/unfrozen snapshots restore with exact hashes/metadata and zero capture/convolution/resampling; its distinct native binary, cleanup and results are recorded in [shared-engine evidence](2026-11-06-shared-swell-engine.md) and [structured result](2026-11-06-shared-swell-engine-profile.json). The earlier result above is retained unchanged.

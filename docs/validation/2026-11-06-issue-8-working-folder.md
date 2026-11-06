# Issue #8 — phase-4 working-folder preparation

Date: 2026-11-06. Baseline: merged main `f98157e`; branch
`codex/8-working-folder-association`. Windows native MSVC Release, JUCE 9.0.4,
warm `out/build/windows-msvc` in the retained issue worktree. No cache clearing,
downloads, plugin installation, release or FLP edits.

## Scope and acceptance

The [implementation boundary](../development/working-folder-association.md)
describes the backend and disposable preview. Production capture/render/export
roots and host serialization are unchanged. The owner authorized phase 4, while
the existing native control review remains required before integration.

| Task | Result at this checkpoint |
| --- | --- |
| 4.1 | Verified preparation and revised control composition with binding tests; live processor-root handoff, host-save notification integration and native approval remain pending. |
| 4.2 | New exclusive branch lease on each backend reopen; two real processes hold different leases simultaneously. Actual capture/render/export publication across processes remains pending. |
| 4.3 | Backend physical copy into another store preserves original dependency hashes. Production Fork command and independent future-write validation remain pending. |
| 4.4 | Candidate version/rollback guidance published. Original-binary/pre-upgrade-FLP rollback exercise remains pending. |

All are high-risk persistence/concurrency tasks; 4.1 also includes UI interaction.
No checkbox is completed by partial fixture evidence. Refreshed Apply reports
**20/36 complete, 16 remaining**; the active change and issue stay open.

## Native results

- `scripts/build.ps1 -Configuration Release` passed after adding the preview.
  Log: `out/build-logs/Release/20261106-202612-e4ef0682/`.
- Focused CTest selection `working_folder|portable|storage|shared_swell_engine`:
  **10/10 passed, 18.81 s**. It includes storage settings, backup/recovery,
  exact snapshots, both new working-folder cases and shared render maintenance.
- `preverb_asset_inventory_test`: **passed, 0.21 s** separately.
- Final focused `preverb_portable_assets_test --working-folder` passed after
  adding bounded/hostile candidate-envelope decoding checks. These are compiled
  native tests, not source-only fixtures or FL Studio tests.
- Initial complete wrapper run: **83/86 passed, 367.67 s**, exit 8.
  Log: `out/build-logs/Release/20261106-201902-ccb79e69/`.
  Three failures reproduced in a focused retry, without changing assertions:
  `preverb_perform_knob_reset_test` lacks OS foreground focus;
  `preverb_perform_readability_test` times out at its focus predicate;
  `preverb_swell_engine_test` times out at line 114 waiting for transient stale
  phase after a tempo change. These failures prevent a green integration baseline.
  The swell-engine executable was not rebuilt by phase 4 (18:08:59 timestamp,
  SHA-256 `9AEA8876B19DB4DA0DF0217F8A1A0624F1117403A642B3620230B71F867FEED7`).
  No production engine or existing test assertions changed.

- Final complete wrapper run, including both new tests and final codec guards: **84/87 passed, 366.24 s**. Log: `out/build-logs/Release/20261106-203004-a01623e7/`. The same three failures in existing tests remain unresolved; no required assertion was bypassed. Production integration and merge remain blocked.

The new backend fixture verifies exact dependency bytes, metadata/descriptor
closure, no switch during preparation, stale/unavailable inputs, cancellation,
read-only destinations, injected completion/revalidation faults, wrong-store
rejection, explicit location without local hints, complete physical copy and
two simultaneous child processes with distinct exclusive leases. Candidate
metadata tests cover exact round-trip, mismatched manifest/snapshot, future and
noncanonical versions, XML entities, excessive nesting and unknown payload keys.

Native panel tests verify cancelled/stale picker results, preparation retaining
the original root, later edits invalidating candidates, binding rejection,
explicit Use/save-request wording and pinned action bounds at 420×240/680×420.
The binding is disposable; it is not a production processor or host-save proof.

## Other checks and remaining gates

- UI standards: **passed, 66 files / 59 bounded occurrences**, revision 10;
  no new token, visual exception or regenerated debt inventory.
- `openspec validate portable-project-assets --strict`: passed.
- Semantic user-guide review: existing production instructions remain accurate;
  candidate controls are excluded. Explicit review record refreshed and currency
  check passed. Canonical specs were not synced or archived.
- Final whitespace and changed-document relative-link checks are recorded with
  the PR verification. No associated-store fresh-profile test or new host/DPI
  coverage is claimed. Prior recovery acceptance is preserved.

Native review entry point:
`preverb_portable_storage_preview --working-folder-review`. A copied executable
is launched directly for owner review, using disposable captured/source/IR/wet/
export fixtures and native folder selection. Only one phase-4 review window is
opened and it remains open for the owner. The existing design requires native
approval before production rollout; no approval is inferred from test success.

Future integration must retain independent writer/root pins for active jobs,
verify coherent host-state saves without callback disk I/O, preserve exact old
FLP aliases and test failure-safe root handoff, missing-store recovery, physical
Fork and rollback. No cleanup, eviction, quarantine, compression, dedup adoption,
release or merge approval is granted by this checkpoint.

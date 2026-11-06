# Issue #8 phase-1 inventory evidence — 2026-11-06

Branch: `codex/8-portable-asset-design`, continuing [PR #31](https://github.com/elneilios/ReverbArise/pull/31). Start revision `21230d2`; production persistence/capture/Export baseline remains `df765d9`. Owner approved compatibility and backup rules and authorized phase 1 on 2026-11-06. Tasks 1.1/1.5 closed individually with Apply line guards and post-update instruction/progress verification. The optional host-path runtime spike remains unperformed; no host path is inferred.

## Delivered scope

- Bounded read-only adapters/dependency visitor for current independently versioned legacy formats, including historical context IRs, unused imported-library entries, retained frozen/disabled work and all Export history. Unknown versions/engines refuse replacement. Future portable formats remain unsupported, not invented by this phase.
- Read-only selected-root inventory with category/file/revision observations, logical versus unique physical/allocation counts, exact hashes and separately labelled decoded equivalence.
- Rebuildable in-memory snapshot/alias/pin observations with unconditional durable/unknown-history protection. Missing index, interrupted/unknown metadata and offline scope cannot confer deletion authority. Reclaimable bytes remain zero; there is no collector or index writer.
- Native Storage composition and provider binding staged for owner review. The preview is not yet wired into the production Settings menu. Task 2.4 remains unchecked pending native look/readability/interaction approval and subsequent integration checks.

See [API/limits and backup guidance](../development/asset-inventory.md) and the [approved matrix](../development/asset-storage-contracts.md). No original measurement or historical evidence was relabelled.

## Native verification

Windows x64, MSVC Release, JUCE 9.0.4 and pinned WavPack sources; reused the existing phase-0 build/cache. Configuration/build/test operations used the existing presets and repository `Enter-BuildLock` handle, with no overlapping wrapper operation or cache reset. Targeted commands were chosen to avoid recompiling unrelated cold targets; no full-suite/Debug/host pass is claimed.

Native targets: `preverb_asset_inventory_test` and `preverb_storage_inventory_preview`. CTest names: `preverb_asset_inventory_test` / `preverb_storage_inventory_panel_test`.

- Dependency fixtures exercise actual capture manifests/contexts, distinct historical/default/unused-library/swell IRs, disabled/frozen retained renders, export history, zero-swell/default-empty state, schema-1 uncertainty, envelopes 1–8 and unsupported processor/capture/Export/engine refusal. No restore/generation service is called by the readers.
- Inventory fixtures prove exact totals, unchanged prior hashes, hardlink/overlapping-root physical counts, exact versus signed-zero decoded identity, a real NTFS junction exclusion, missing and exclusively locked roots/files, read-only input, entry/hash/metadata bounds, entity XML refusal, Unicode paths, cancellation and unknown portable marker refusal. A junction points only into another disposable fixture directory and is removed before fixture teardown; no user data is used.
- Catalog fixtures rebuild without a prior index, retain active export pins and keep arbitrary unknown assets protected. Service fixtures supersede requests and clear publication on cancellation.
- Component fixtures verify automatic page-open/reopen scan requests, hidden-page inactivity, no polling refresh loop, stopping without automatic retry, contained geometry, populated summaries and rejection of stale results after scope rebinding. Native screenshots preserve existing fonts/roles and the helper notice uses explicit lines rather than shrinking to fit.

Native Release: **12/12 affected CTest checks passed**; after final inventory/composition changes the **2/2 new targets passed again**, and added reader-boundary/Unicode cases passed in a final reader-only run. No native failure remains. Initial fixture compilation needed the explicit capture-session include; the corrected tests ran against the production APIs.

Local logs: `out/phase1-final-ctest.log` (12 affected checks), final changed inventory/component target build and retest under `out/phase1-final-build.log` / `out/phase1-final-tests.log`. Existing capture, session, processor, asset and Export code was unchanged; unchanged focused results were reused after changes confined to the new inventory/composition and retested affected new targets.

UI standards check passes with no token/debt baseline changes. OpenSpec strict validation, task progress, link/whitespace checks and guide currency were checked before PR update. The offline guide's existing backup/cache/portability cautions remain correct; no released-menu instructions or prototype screenshots are added for the staged preview. Its explicit currency review records that rationale.

## Owner review and limits

The prepared native window is launched directly for review and retained. Final review executable SHA-256: `9380b3236e36033b53a4968fb330b74530379f0fa925c335e0386ea4df40cf94`; it runs from an ignored review copy so the native build executable stays unlocked. [Supporting component screenshot](2026-11-06-issue-8-storage-preview.png) is a synthetic preview, not a production guide screenshot. Owner-directed review revision: opening Storage automatically scans; the vague Inspect button and manual refresh were removed. Stop scan appears only during work. Native lifecycle tests cover open/close/reopen, hidden rebinding and cancellation. Review scope: location/value readability, automatic page-open flow, scope/uncertainty wording and the protected-history notice. Existing shared primitives/tokens are reused; this does not seek approval for new visual tokens or later storage operations.

No user assets were scanned by these fixtures. No production capture/DSP/state identity, root default, asset writer or existing Settings/creative controls changed. No on-disk catalog, migration, packing, relink, compression, quarantine or deletion was added. No FL Studio, second-profile/PC, real offline drive, power-loss/disk-full or complete DPI matrix result is claimed. Cooperative scan budgets do not bound a blocking OS storage call. Offline FLP/export references and cross-process leases remain unknown and protected.

Phase 1 is not fully complete until task 2.4's native review and integration are complete. Later phases remain separately gated. Do not sync/archive/merge this change as part of this checkpoint.

Automatic-scan review revision: both affected native targets passed (2/2); lifecycle evidence is retained in `out/phase1-auto-scan-test.log`. Prior component-button invocation evidence is superseded by page-open lifecycle checks. No additional backend/DSP scope changed.

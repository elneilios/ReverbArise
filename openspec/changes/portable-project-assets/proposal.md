# Proposal

## Why

ReverbArise stores irreplaceable captures and saved render dependencies outside the FLP. Issue #8 needs an auditable portability and retention contract before any migration or cleanup can risk older projects.

## What Changes

- Deliver a source-backed audit and explicitly separate verified code, arithmetic estimates, host unknowns and proposals. Preserve the original 2026-11-03 audit; the design records a 2026-11-06 contract reconciliation at main df765d95170caaaab10a646e43088ea3be66ce53, not a new runtime audit.
- Prefer a validated project-local asset folder where host discovery proves reliable, retain the current user-data fallback elsewhere, and expose a configurable working folder in Storage. Use immutable relative references, explicit association and verified copy-before-switch migration. Provide Backup and Restore/Locate actions with exact completed wet renders for regeneration-free compatible reopen.
- Propose content-addressed immutable objects with distinct dependency descriptors and independent writer branches; preserve existing project/instance identities and legacy paths.
- Extend the delivered read-only inventory with planned missing-asset diagnostics and explicit Locate, Restore backup or Re-learn recovery. Preserve decoded swell identities, positions and edits; regenerate only after fresh source and required IRs are available, with separate consent and honest frozen/exact-sound limits. Improve efficiency through measured duplicate avoidance and separately reviewed disposable-cache classification; retain protected history and opt-in journaled scratch quarantine. Unknown old FLP references prohibit inferred durable-asset reclamation.
- Evaluate exact float32 compression separately, retaining uncompressed WAV as the initial storage and rollback baseline.
- Phase 0 and the read-only phase 1 are delivered. The owner approved this plan refinement and starting the FL Studio project-folder discovery spike on 2026-11-06. Folder association, Backup/Restore, disaster recovery and storage-management implementation remain separately gated; no relocation, purge, DSP or #11 Learn changes are authorized by this revision.

## Capabilities

### New Capabilities

- `project-asset-portability`: explicit folder association, immutable dependency closure, packing/relink, copy/fork ownership and nondestructive compatibility.
- `asset-retention`: storage accounting, conservative reference classification, transient quarantine and recovery.

### Modified Capabilities

None. Existing capture source/context, Learn generation, song export, `song-timeline-audio`, `swell-session`, `swell-regeneration`, `impulse-response-library` and workflow contracts remain authoritative. Adapters must distinguish processor envelope v8, creative state v11, capture schemas 1/2 and Export metadata v2/unversioned legacy; accepted context IRs and every saved export-history dependency belong to pack closure. The new capabilities extend resolution/administration without changing capture, musical rendering or editing rules.

## Impact

Future implementation would extend CaptureAssetStore, SwellAssetStore and state/dependency resolution with worker-side services and versioned adapters. This plan refinement changes no production behavior or canonical specs; the separately authorized discovery spike is diagnostic only. Existing `PreverbState`, internal modes, UUIDs, source format and legacy K7MA/Preverb paths remain readable. Older binaries may reject future state versions: retain pre-upgrade FLPs and the untouched legacy store for rollback.

Audit: [current architecture](../../../docs/development/asset-storage-audit.md). Evidence/limits: [dated investigation](../../../docs/validation/2026-11-03-asset-storage-investigation.md). Design and tasks here define phase gates; approval is required before implementing later phases beyond the explicitly authorized discovery spike.


## Phase-4 explicit recovery-workspace approval — 2026-11-06

The owner approved an explicit recovery workspace when the entire original asset
folder is irrecoverable or offline. This is a distinct recovery operation, not
a successful pack, verified migration or complete backup. It requires deliberate
folder selection/confirmation, preserves decodable swell metadata, original
locators and surviving assets, and exposes missing/unknown dependencies. It must
never silently fall back, overwrite originals or claim all assets were copied.
After adoption, use normal Learn with detection off and existing complete-source,
IR, automatic unfrozen regeneration, frozen-consent and export-loss protections.
No Learn/Generate controls are added to Storage. Ordinary association and Fork
retain complete verified-copy gates, host-save requests and rollback. Phase-4
implementation remains authorized; cleanup, compression, default cutover and
release/merge gates are unchanged. This approval completes no task checkbox.

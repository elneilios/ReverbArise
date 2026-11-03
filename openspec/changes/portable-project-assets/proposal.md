# Proposal

## Why

ReverbArise stores irreplaceable captures and saved render dependencies outside the FLP. Issue #8 needs an auditable portability and retention contract before any migration or cleanup can risk older projects.

## What Changes

- Deliver a source-backed audit and explicitly separate verified code, arithmetic estimates, host unknowns and proposals.
- Propose an explicitly associated project asset folder, immutable portable snapshots, Pack/Relink and verified copy-before-switch migration; include completed wet renders for regeneration-free compatible reopen.
- Propose content-addressed immutable objects with distinct dependency descriptors and independent writer branches; preserve existing project/instance identities and legacy paths.
- Propose read-only storage inventory, protected historical dependencies, opt-in quarantine for proven scratch, and recovery journals. Unknown old FLP references prohibit inferred durable-asset reclamation.
- Evaluate exact float32 compression separately, retaining uncompressed WAV as the initial storage and rollback baseline.
- This submission is **planning only**. Tasks describe future gated work, not authorization to apply, migrate, delete, change DSP or alter #11's Learn surface, state machine or capture semantics.

## Capabilities

### New Capabilities

- `project-asset-portability`: explicit folder association, immutable dependency closure, packing/relink, copy/fork ownership and nondestructive compatibility.
- `asset-retention`: storage accounting, conservative reference classification, transient quarantine and recovery.

### Modified Capabilities

None. Existing `song-timeline-audio`, `swell-session`, `swell-regeneration`, `impulse-response-library` and workflow contracts remain authoritative. The new capabilities extend resolution/administration without changing capture, musical rendering or editing rules.

## Impact

Future implementation would extend CaptureAssetStore, SwellAssetStore and state/dependency resolution with worker-side services and versioned adapters. No production source or canonical specs change in this proposal. Existing `PreverbState`, internal modes, UUIDs, source format and legacy K7MA/Preverb paths remain readable. Older binaries may reject future state versions: retain pre-upgrade FLPs and the untouched legacy store for rollback.

Audit: [current architecture](../../../docs/development/asset-storage-audit.md). Evidence/limits: [dated investigation](../../../docs/validation/2026-11-03-asset-storage-investigation.md). Design and tasks here define phase gates; approval is required in a later request before implementation.

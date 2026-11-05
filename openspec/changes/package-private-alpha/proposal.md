# Proposal

## Why

GitHub issue: #59. Perform #48 and Learn generation #56 are now merged, making a private musician-facing Windows alpha useful. The existing local release pipeline supplies traceable bundles, but recipients need an illustrated offline guide and evidence for the exact combined packaged binary.

## What Changes

- Extend the existing release payload with a concise quick start and one static, self-contained HTML guide covering installation, Learn capture/markers/generation, Perform editing/audition, Export WAV, persistence, troubleshooting and feedback.
- Reuse native production-editor screenshot evidence with deterministic functioning states; embed the images and build identity in the guide and include it in the existing manifest inventory.
- Prepare a clean-main `v0.1.0-alpha` Windows x64 candidate through `scripts/release.ps1`, preserving numeric version 0.1.0 and all host/storage identities.
- Record exact-source Release regression, standards, identity, OpenSpec, archive/offline rendering and final packaged-binary host evidence. Record clean-machine/second-host coverage honestly.
- Prepare the third-party redistribution review for explicit owner approval before any external sharing. Distribution remains private/manual; no tags or public release.
- Add a concise guide-maintenance skill integrated with issue delivery, and a read-only release gate rejecting stale guide review inputs without rewriting content.
- Add Settings About with generated version, candidate/build identifier and full Git SHA, plus Copy build information. Metadata uses CMake version, explicit release inputs and Git, with no clock-derived identifier.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `local-release-packaging`: require a recipient-ready offline illustrated guide tied to the packaged source and authentic native evidence, with private-alpha acceptance and redistribution records.
- `product-release-identity`: expose authoritative build information in Settings and copied reports, and verify its release candidate/source identity before packaging.

## Impact

Affected areas include release/build metadata, Settings About, guide maintenance and validation. Existing shared ActionButton, text roles, spacing and native screenshot mechanisms are reused. No DSP, new control family/style token, installer, signing, state migration or screenshot system is proposed. Final clean-main packaging follows owner-reviewed merge; earlier branch bundles are internal rehearsals. Canonical specs remain unchanged until the normal sync/archive workflow.

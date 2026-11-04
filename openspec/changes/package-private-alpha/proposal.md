# Proposal

## Why

GitHub issue: #59. Perform #48 and Learn generation #56 are now merged, making a private musician-facing Windows alpha useful. The existing local release pipeline supplies traceable bundles, but recipients need an illustrated offline guide and evidence for the exact combined packaged binary.

## What Changes

- Extend the existing release payload with a concise quick start and one static, self-contained HTML guide covering installation, Learn capture/markers/generation, Perform editing/audition, Export WAV, persistence, troubleshooting and feedback.
- Reuse native production-editor screenshot evidence with deterministic functioning states; embed the images and build identity in the guide and include it in the existing manifest inventory.
- Prepare a clean-main `v0.1.0-alpha` Windows x64 candidate through `scripts/release.ps1`, preserving numeric version 0.1.0 and all host/storage identities.
- Record exact-source Release regression, standards, identity, OpenSpec, archive/offline rendering and final packaged-binary host evidence. Record clean-machine/second-host coverage honestly.
- Prepare the third-party redistribution review for explicit owner approval before any external sharing. Distribution remains private/manual; no tags or public release.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `local-release-packaging`: require a recipient-ready offline illustrated guide tied to the packaged source and authentic native evidence, with private-alpha acceptance and redistribution records.

## Impact

Affected areas are `scripts/release-support.ps1` and release fixtures, existing native UI evidence tests, guide source/assets under `docs/`, release guidance and dated validation evidence. No DSP, production control family, style token, installer, signing, state migration or new screenshot system is proposed. Final clean-main packaging follows owner-reviewed merge of any payload/tooling changes; earlier branch bundles are internal rehearsals. Existing canonical specs remain unchanged until the normal sync/archive workflow.

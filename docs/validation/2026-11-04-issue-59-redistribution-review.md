# Private alpha redistribution review — owner decision pending

This is the concrete review packet for #59, not a licence grant or approval. Local package generation is internal evaluation only. No binary has been authorised for external sharing by this record. Keep confidential entitlement/revenue/contract evidence private; record only a non-confidential decision summary here.

| Material | Pinned evidence and package location | Review status |
| --- | --- | --- |
| ReverbArise source and binary | Repository has no root general LICENSE grant. Stable plugin identity and original code are unchanged. | Owner must establish rights to distribute the combined product and choose the recipient's private evaluation grant/conditions. A public repository is not itself a licence. |
| JUCE 9.0.4 | Actual dependency `LICENSE.md` and `JUCE.spdx.json` become `licenses/JUCE-LICENSE.md` and `licenses/JUCE.spdx.json`. Pinned licence offers AGPLv3 or JUCE commercial terms. | Owner must select the applicable path, establish eligibility/entitlement and satisfy the selected path's obligations, including any required corresponding source/materials. No entitlement is inferred. |
| VST3 SDK bundled with JUCE | Actual SDK `LICENSE.txt` copied as `licenses/VST3-SDK-LICENSE.txt`; MIT, Steinberg 2025. | Notice included by existing pipeline; retain copyright/permission/disclaimer text. |
| WavPack 5.9.0 | CMake pins `5803634a030e2a11dba602ba057b89cc34486c67`; BSD 3-clause notice copied into `ReverbArise.vst3/Contents/Resources/WavPack-LICENSE.txt`. | Preserve binary-distribution notice and endorsement restrictions. |
| Inter font | `ReverbArise.vst3/Contents/Resources/Inter-OFL.txt`. | Preserve OFL notice; no modified font or reserved-name claim is introduced. |
| Original Hall audio | `resources/README.md` copied as `licenses/Original-Hall-provenance.md`. Deterministically generated original audio, CC0-1.0 dedication. | Preserve provenance; project code rights remain a separate decision. |
| Five bundled IRs | `resources/irs/LICENSE.md` and manifest copied into `Contents/Resources/IR-LICENSE.md` and `IR-manifest.json`. Original procedural audio, CC0-1.0. | Verify full bundle includes notices/manifest; no external sample library or third-party effect capture is claimed. |
| JUCE transitive dependencies | Existing release notice inventory copies zlib, HarfBuzz, libpng, JPEG, Ogg, Vorbis, Opus, Opusfile, Opusenc, and complete FLAC header notice into `licenses/`. SPDX inventory includes modules not necessarily used by this Windows build. | Preserve actual pinned notices. Inventory inclusion does not assert every component is linked; owner reviews applicable obligations and any additional required materials. |

The [JUCE 9 EULA](https://juce.com/legal/juce-9-licence/) was checked on 2026-11-04 alongside the pinned licence. Its distribution/seat conditions require an owner-specific decision; this preparation does not select a tier, infer entitlement, or establish compliance. Relevant sections include licence types, product ownership, framework users, and distribution (1.2, 1.5–1.11). Any AGPL path must be reviewed separately against its full terms and the combined product/source delivery.

## Decision to record before external sharing

Owner direction received **2026-11-05**: plan to use the free tier for now. Recorded intended path: **JUCE Starter (commercial)**. Eligibility/entitlement and applicable conditions under the JUCE 9 EULA still require owner confirmation; this statement is a path preference, not approval of compliance or external sharing. Intended scope from #59 is private musician DAW evaluation. Final product distribution grant, exact approved candidate and any required additional materials remain pending.

- Named owner/licensing path and non-confidential evidence reference; applicable entitlement and obligations satisfied.
- Project distribution authority and intended private recipient/use/redistribution conditions.
- JUCE, IR, font and third-party notice/material review completed; any extra required source/material supplied.
- Exact final source, package and module identity plus final FL Studio acceptance reference.
- Explicit owner approval, date and scope. **Current decision: pending.**

No public release, tag push, upload, private transmission or self-merge is authorised by this packet. Local copying to a recipient also requires the review approval.

## Subsequent owner sign-off — 2026-11-05

In direct response to the prepared-review sign-off question covering Starter eligibility/licensing, project distribution rights, bundled notices/materials and exact alpha.1 private-evaluation approval, the owner stated: **“Starter is correct - currently I am the only user. I intend to share alpha builds with a friend. At a later date I will decide if I am distributing this plugin to the public but haven't decided that yet”.**

Record this owner confirmation for **JUCE Starter**, one current framework user and **one friend for private DAW evaluation** of alpha.1, source `b26eef73fefb878c22f0bd0128c0fc3353e8d438`, ZIP SHA-256 `5e9483ad0f5af806a3de720365148eda6606727a3d3ea3249bc89f689b5e4374`, module SHA-256 `fc69341777d91b6f5d13a6999a1077e8b88de2a6a071082beb04919652a3d3a1`. The owner separately accepted their earlier ZIP-installed FL Studio trial and waived another run; exact host version/rate/buffer and a new alpha.1 run are not asserted. Offline alpha.1 guide inspection received explicit **pass**.

This records the owner's decision in the stated private scope, not an independent legal audit or an entitlement-document inspection. No confidential evidence was requested or retained. **Public distribution remains undecided and unapproved.** New revisions, broader recipients/use or public distribution need their own decision; this does not grant a general project licence or authorize a release/tag/upload. No binary is transmitted by this sign-off record. Earlier pending statements above describe the state before this subsequent decision.

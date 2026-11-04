# Design

## Context

See proposal.md for motivation and the local-release-packaging delta for the acceptance contract. `release.ps1` delegates to `release-support.ps1`, invokes the Release wrapper with tests, copies the complete VST3 and actual dependency notices, writes README.txt, inventories the payload and produces an immutable ZIP/checksum/sidecar manifest. It already preserves explicit candidates and prevents dirty-source packaging. Its README is presently developer-oriented.

`tests/learn_generation_ui_test.cpp` captures the real production editor with `createComponentSnapshot`, gated by `PREVERB_UI_M3_OUTPUT`, using `GenerationFixture`. `perform_composition_preview.cpp` also has existing screenshot output. These provide the screenshot mechanism; focused fixture coverage can be extended for the four required guide states. Existing screenshots alone do not establish exact-candidate provenance. The current canonical specs intentionally lag the merged Learn/Perform changes; use those changes and current production bindings for guide details without independently archiving them.

## Goals / Non-Goals

**Goals:** Extend payload documentation through the existing inventory and validation transaction; make provenance and manual gates reviewable; protect installed plugins and saved projects.

**Non-Goals:** Production UI changes, new controls/tokens, a separate screenshot or packaging system, an installer, public publication, license interpretation on the owner's behalf, or claiming bit-identical rebuilds.

## Decisions

1. **Extend the current payload assembly.** Keep the build lock, native test invocation, source checks, notice copying, immutable output and archive revalidation. Add guide assembly before inventory/hashing. Store maintainable guide source in `docs/development/` and dated capture/validation evidence in `docs/validation/`; update the existing release guide and index as needed. A post-ZIP insertion would invalidate the existing manifest and is rejected.

2. **Generate images from the same clean source being packaged.** Reuse the production editor and existing evidence output with deterministic captured/generated data and ready Export state. If a missing state needs a fixture extension, assert real readiness and bindings rather than painting mock data over screenshots. Record native test commands, dimensions, source SHA and image hashes. Guide assembly embeds PNG data URIs and source identity at package time. Reject missing/invalid images or mismatched provenance. Generation happens in ignored output; do not change tracked files during packaging. A source-SHA stamp belongs in generated guide text/provenance, avoiding a tracked file containing its own commit hash.

3. **Keep the delivered guide static.** Use HTML/CSS with internal anchors and embedded images; no JavaScript, remote fonts, external links or external media. Include the short quick start separately. Feedback instructions ask recipients to reply to the private sender with candidate/source identity, DAW/version, steps, expected/actual behavior and relevant screenshots/logs. Do not require an account or online dependency to read the guide. Guide language describes only existing production behavior, including capture requiring DAW playback, Generate on Stop's actual default, finite selected-swell audition and Export's independent mix.

4. **Preserve release identity conventions.** Use `-ReleaseTag v0.1.0 -Candidate alpha` for the initial final package; numeric plugin identity stays 0.1.0. Never overwrite a used candidate directory; use a distinct explicit `alpha.N` identity for subsequent attempts and record why. Internal branch rehearsals use distinct candidate labels. Do not create a tag. Record the full main SHA, MSVC/CMake and build cache state, validation scope, omissions and hashes. Existing tests are evidence only for the source they actually exercised.

5. **Stage review before final main packaging.** Implement and validate payload changes on the issue branch, prepare an internal rehearsal and review-ready PR with `Refs #59`. After the owner merges, select a clean identified main revision containing both prerequisites and these changes; run the final release pipeline and exact-source checks, extract into a new directory, verify inventory, and inspect the guide offline. Record final package identity in dated evidence without repackaging merely to include its own ZIP hash. Do not self-merge to satisfy the main requirement. Keep final acceptance tasks pending until this step occurs.

6. **Separate technical preparation from owner decisions.** Prepare a concrete redistribution inventory using pinned JUCE, WavPack, Inter, VST3 SDK, JUCE dependencies and all IR provenance; identify missing project distribution rights and required materials. The owner chooses and approves the licensing path and private-sharing rights. Launch prepared native previews directly for any necessary component checkpoint; none is planned for production controls. Prepare one concise final FL Studio smoke checklist covering scan/load, capture, generation, selected Perform audition/Stop and project-copy save/reopen with retained audio/assets. Run against the extracted final bundle without overwriting existing installations; owner-operated host steps remain explicit if automation cannot establish them. Record DAW version, sample rate, buffer and binary hash. Clean-machine/second-host testing can remain explicitly unverified.

## Risks / Trade-offs

- [Main advances or production UI changes] → Recheck prerequisites and guide bindings, regenerate images and rerun matching gates for the selected final SHA; do not reuse stale screenshots.
- [Fixtures show pending rather than playable results] → Wait on real render/export readiness with bounded assertions and caption deterministic fixtures honestly.
- [Large embedded images] → Choose useful readable native sizes and compress losslessly where needed; verify legibility and every image after extraction.
- [Offline HTML appears complete but fetches resources] → Check resource references and disallowed active content automatically, then inspect the extracted file locally with network disabled and images decoded.
- [Rights or host acceptance unresolved] → Preserve internal artifacts and report the exact remaining gate; never label the alpha shareable or issue complete prematurely.
- [Package tests simulate builds] → Extend release fixtures for missing/mismatched guide assets and archive integrity, while reporting fixture checks separately from real MSVC/CTest and host results.

## Migration Plan

No plugin/state migration. Existing installed bundles and DAW projects remain protected. Review and merge tooling/docs first, package clean main, then complete final binary host acceptance and owner redistribution approval. Keep the package internal until both pass. Installation guidance preserves the whole VST3 bundle and backs up any existing installation with the DAW closed; rollback restores that preserved bundle and uses copies of projects. No distribution action is part of this change.

## Open Questions

- Which JUCE licensing path and project distribution grant will the owner approve? This is an external-sharing gate; technical preparation can proceed independently.
- Which recipient host or clean-machine environment is available? Record exact tested coverage; neither is presumed available or required to claim universal compatibility.

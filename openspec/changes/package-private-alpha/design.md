# Design

## Context

See proposal.md for motivation and the local-release-packaging delta for the acceptance contract. `release.ps1` delegates to `release-support.ps1`, invokes the Release wrapper with tests, copies the complete VST3 and actual dependency notices, writes README.txt, inventories the payload and produces an immutable ZIP/checksum/sidecar manifest. It already preserves explicit candidates and prevents dirty-source packaging. Its README is presently developer-oriented.

`tests/learn_generation_ui_test.cpp` captures the real production editor with `createComponentSnapshot`, gated by `PREVERB_UI_M3_OUTPUT`, using `GenerationFixture`. `perform_composition_preview.cpp` also has existing screenshot output. These provide the screenshot mechanism; focused fixture coverage can be extended for the four required guide states. Existing screenshots alone do not establish exact-candidate provenance. The current canonical specs intentionally lag the merged Learn/Perform changes; use those changes and current production bindings for guide details without independently archiving them.

## Goals / Non-Goals

**Goals:** Extend payload documentation through the existing inventory and validation transaction; make provenance and manual gates reviewable; protect installed plugins and saved projects.

**Non-Goals:** UI changes outside Settings About, new control families/tokens, a separate screenshot or packaging system, an installer, public publication, licence interpretation on the owner's behalf, or claiming bit-identical rebuilds.

## Decisions

1. **Extend the current payload assembly.** Keep the build lock, native test invocation, source checks, notice copying, immutable output and archive revalidation. Add guide assembly before inventory/hashing. Store maintainable guide source in `docs/development/` and dated capture/validation evidence in `docs/validation/`; update the existing release guide and index as needed. A post-ZIP insertion would invalidate the existing manifest and is rejected.

2. **Generate images from the same clean source being packaged.** Reuse the production editor and existing evidence output with deterministic captured/generated data and ready Export state. If a missing state needs a fixture extension, assert real readiness and bindings rather than painting mock data over screenshots. Record native test commands, dimensions, source SHA and image hashes. Guide assembly embeds PNG data URIs and source identity at package time. Reject missing/invalid images or mismatched provenance. Generation happens in ignored output; do not change tracked files during packaging. A source-SHA stamp belongs in generated guide text/provenance, avoiding a tracked file containing its own commit hash.

3. **Keep the delivered guide static.** Use HTML/CSS with internal anchors and embedded images; no JavaScript, remote fonts, external links or external media. Include the short quick start separately. Feedback instructions ask recipients to reply to the private sender with candidate/source identity, DAW/version, steps, expected/actual behavior and relevant screenshots/logs. Do not require an account or online dependency to read the guide. Guide language describes only existing production behavior, including capture requiring DAW playback, Generate on Stop's actual default, finite selected-swell audition and Export's independent mix.

4. **Preserve release identity conventions.** Use `-ReleaseTag v0.1.0 -Candidate alpha` for the initial final package; numeric plugin identity stays 0.1.0. Never overwrite a used candidate directory; use a distinct explicit `alpha.N` identity for subsequent attempts and record why. Internal branch rehearsals use distinct candidate labels. Do not create a tag. Record the full main SHA, MSVC/CMake and build cache state, validation scope, omissions and hashes. Existing tests are evidence only for the source they actually exercised.

5. **Sign off the clean PR candidate, then verify main.** Owner-approved order on 2026-11-05: package a clean committed PR revision containing prerequisites and current main integrations; run exact-source native validation/screenshots, extract into a fresh directory, inspect the guide offline, install the matching bundle and obtain consolidated FL Studio and redistribution sign-offs before merging. After the owner-authorized merge, verify the resulting clean main revision separately through native Release and release identity checks. Keep archive pending until that verification; do not relabel PR evidence as main evidence. Preserve the accepted candidate/source identity when recording later documentation-only commits.

6. **Separate technical preparation from owner decisions.** Prepare a concrete redistribution inventory using pinned JUCE, WavPack, Inter, VST3 SDK, JUCE dependencies and all IR provenance; identify missing project distribution rights and required materials. The owner chooses and approves the licensing path and private-sharing rights. Launch prepared native previews directly for any necessary component checkpoint; none is planned for production controls. Prepare one concise final FL Studio smoke checklist covering scan/load, capture, generation, selected Perform audition/Stop and project-copy save/reopen with retained audio/assets. Run against the extracted final bundle without overwriting existing installations; owner-operated host steps remain explicit if automation cannot establish them. Record DAW version, sample rate, buffer and binary hash. Clean-machine/second-host testing can remain explicitly unverified.

## Release-readiness extension requested by owner

- Guide maintenance is an explicit authoring workflow, invoked by deliver-issue after user-facing implementation. Review delivered code, relevant canonical specs (including intentional differences in active changes) and guide coverage. A committed review inventory hashes Source/, canonical specs, CMake/metadata inputs, release/installation scripts, screenshot fixtures and the HTML template. A separate recording command follows semantic review even when no prose change is needed. Packaging only checks currency and fails before building when inputs differ; it never invokes a skill or updates tracked files.
- CMake generates build-information.json and a matching header from PROJECT_VERSION, validated explicit release tag/candidate and the full Git SHA/dirty status. No build timestamp, random ID or absolute workspace path enters the identity. The supported wrapper passes explicit CLI values on every configure, clearing a cached candidate for ordinary builds. Packaging forwards its identity inputs, verifies generated metadata against expected identity/SHA and fails stale builds. The installation helper accepts the same optional explicit identity to preserve a deliberate alpha identity; ordinary local installs remain local builds. Local/unavailable-source builds are labelled honestly.
- Add an About tab to the existing Settings container without moving/revising the five existing pages. About ReverbArise uses existing section/control/helper text roles, EllipsisLabel, comfortable spacing and the unchanged ActionButton family. Display full source identity through accessible descriptions/tooltips and copy full metadata via the message-thread clipboard action. No new style tokens/variants or persistence/audio behavior; existing family acceptance applies. Native component/binding tests cover copy and state invariance. Extend existing production evidence fixture with a focused About preview/snapshot; directly launch it for owner review when prepared.
- Guide records Settings → About and copied bug-report information. Its four workflow screenshots remain from the existing mechanism. Native/CMake/PowerShell fixtures cover invalid/stale identity, candidate reset, deterministic generation, stale/missing review, no source mutation in packaging and clipboard/UI binding. Keep final main/host/licensing gates independent.

## Risks / Trade-offs

- [Main advances or production UI changes] → Recheck prerequisites and guide bindings, regenerate images and rerun matching gates for the selected final SHA; do not reuse stale screenshots.
- [Fixtures show pending rather than playable results] → Wait on real render/export readiness with bounded assertions and caption deterministic fixtures honestly.
- [Large embedded images] → Choose useful readable native sizes and compress losslessly where needed; verify legibility and every image after extraction.
- [Offline HTML appears complete but fetches resources] → Check resource references and disallowed active content automatically, then inspect the extracted file locally with network disabled and images decoded.
- [Rights or host acceptance unresolved] → Preserve internal artifacts and report the exact remaining gate; never label the alpha shareable or issue complete prematurely.
- [Package tests simulate builds] → Extend release fixtures for missing/mismatched guide assets and archive integrity, while reporting fixture checks separately from real MSVC/CTest and host results.

## Migration Plan

No plugin/state migration. Existing installed bundles and DAW projects remain protected. Package and sign off the clean PR revision first; merge only after final binary host/guide and redistribution sign-offs, then verify clean main before archiving. Keep the package internal until both pass. Installation guidance preserves the whole VST3 bundle and backs up any existing installation with the DAW closed; rollback restores that preserved bundle and uses copies of projects. No distribution action is part of this change.

## Open Questions

- Which JUCE licensing path and project distribution grant will the owner approve? This is an external-sharing gate; technical preparation can proceed independently.
- Which recipient host or clean-machine environment is available? Record exact tested coverage; neither is presumed available or required to claim universal compatibility.

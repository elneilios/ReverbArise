# Tasks

## 1. Authentic production evidence

- [x] 1.1 Extend existing native UI evidence fixtures for empty Learn, captured markers, ready generated Perform with selected swell, and ready Export; verify actual production bindings/readiness with bounded assertions and successful PNG capture through the existing mechanism, documenting exact commands and fixture scope in dated validation evidence.
- [x] 1.2 Produce screenshot provenance with source SHA, commands, dimensions and hashes; verify missing images, failed readiness and mismatched source cannot be accepted for guide assembly, and document how the release pipeline consumes same-source ignored outputs.

## 2. Recipient documentation and payload integration

- [x] 2.1 Author maintainable guide source and concise README covering numbered installation/scanning, routing, Learn capture during DAW playback, marker inspection, generation, Perform audition/editing, Export WAV and save/reopen; verify every instruction against current production bindings and include honest limitations, host coverage, troubleshooting and reply-to-sender feedback instructions.
- [x] 2.2 Integrate static HTML assembly and embedded native images into the existing release transaction before inventory/hashing, stamping candidate/source identity; verify no external URLs/resources, scripts or frameworks and reject incomplete/stale evidence. Update release guidance and relevant docs index links.
- [x] 2.3 Extend existing release-wrapper fixtures to verify guide/README inventory, embedded media, identity matching, missing/stale evidence rejection and ZIP revalidation while retaining dirty-source, lock, notice and immutable-output safeguards; run the fixtures and record their simulated-build scope separately from native results.

## 3. Internal rehearsal and review gates

- [x] 3.1 Prepare a concrete redistribution review inventory from pinned JUCE, project rights, WavPack, Inter, VST3 SDK, JUCE dependencies and IR provenance; verify all required notice files/materials and record unresolved obligations and owner decisions in dated evidence without treating preparation as approval.
- [x] 3.2 Run a clean committed branch rehearsal through scripts/release.ps1 with a distinct explicit alpha rehearsal candidate, plus UI standards and strict applicable OpenSpec checks; verify Release native regression and built product/identity results, ZIP/checksum/manifest inventory and extracted guide offline rendering with every embedded image visible. Record revision, tool versions, cache state, commands, results and omissions.
- [x] 3.3 Prepare the focused tooling/docs PR with Refs #59, change link, rehearsal evidence and remaining main/host/licensing gates; verify issue-scoped diff and leave it open for owner review without merging or closing the issue.

## 4. Final clean-main alpha and acceptance

- [ ] 4.1 After owner-reviewed merge, select a clean identified main revision containing #48, #56 and the payload changes; regenerate same-source screenshots and run scripts/release.ps1 -ReleaseTag v0.1.0 -Candidate alpha (or a recorded fresh alpha.N if identity is already used), UI standards, strict OpenSpec and applicable identity checks. Verify all native Release tests and retain exact source/tool/results evidence; do not mark complete on branch rehearsal results.
- [ ] 4.2 Extract the final immutable ZIP into a fresh directory, verify SHA-256 against checksum/manifest and every inventoried payload file, inspect the full VST3/notices/README and render the standalone guide locally offline; verify all four authentic images are readable and match this packaged revision, and record paths, hashes and visual inspection results.
- [ ] 4.3 Prepare and complete one consolidated FL Studio smoke test against the extracted final binary for scan/load, capture, generation, Perform audition/Stop and project-copy save/reopen without overwriting existing installations/projects/assets; record binary hash, host version, sample rate, buffer, observations and owner acceptance, with clean-machine and second-DAW coverage tested or explicitly unverified.
- [ ] 4.4 Obtain explicit owner approval of the prepared redistribution review and any required additional material; verify the dated approval covers the intended private recipients/use and record the final alpha's sharing status. Keep the bundle internal while approval or final host acceptance is pending; perform no external sharing, tag push or publication.

The following release-readiness work is completed before final acceptance tasks 4.1–4.4.

## 5. Release-readiness extension

- [x] 5.1 Add concise maintain-user-guide skill, deliver-issue integration and explicit review-record/check commands; verify stale/missing/added/deleted code/spec/guide inputs fail and read-only checks preserve files, and document the maintenance workflow.
- [x] 5.2 Generate deterministic version/candidate/build/SHA metadata and wire explicit resettable release inputs through the existing wrapper/packager; verify generated/compiled identity consistency, invalid inputs, candidate reset, repeat generation and stale metadata rejection with focused fixtures.
- [x] 5.3 Add Settings About and Copy build information using approved shared controls/text roles/spacing; verify full clipboard content, metadata display, bounds/accessibility and unchanged session/capture/render state with native component tests; prepare and directly launch the existing native preview mechanism for review.
- [x] 5.4 Review delivered behavior/canonical specs with maintain-user-guide, update the HTML with About/copy instructions and record currency; verify self-contained guide assembly and authentic current About evidence without packaging-time authoring.
- [ ] 5.5 Run relevant wrapper/identity/currency/UI/native regression and internal packaging checks, preserve exact source/artifact evidence and update PR #63; verify PR remains open/unmerged and no publication or external sharing occurs.

## Workflow follow-up

- Keep one issue PR and update its evidence/status as final acceptance becomes available. Use Closes #59 only when it fully resolves the issue under CONTRIBUTING.md; otherwise retain Refs #59.
- Leave owner merge, actual private distribution and any later public publication to separate explicit owner decisions.
- Do not auto-archive this change or the prerequisite changes; use the standard sync/archive workflow only when requested.

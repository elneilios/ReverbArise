---
name: build-release
description: Prepare a validated ReverbArise Windows release ZIP with build-specific release notes and the offline user guide. Use when asked to build or package an alpha, candidate or release; publishing and external sharing remain separate actions.
---

# Build Release

Read `AGENTS.md`, `CONTRIBUTING.md` and `docs/development/releases.md`. Reuse `scripts/release.ps1`; do not create another packager or modify completed ZIPs.

## Establish the release boundary

- Inspect current branches, PRs and source status. Package only the exact accepted, committed revision; never include another agent's unfinished work or infer completion from a closed issue alone.
- Resolve the numeric version and a fresh explicit candidate label against the product-version contract and existing outputs. Preserve existing package identities and outputs.
- Establish the **previous release actually shared with the recipient**, including its full source SHA, from its manifest/acceptance record or owner direction. Do not assume the latest tag, rehearsal, local installation or last commit is that release. Ask only if this baseline cannot be established. For a genuinely first release use null baseline fields.

## Author the notes before packaging

Inspect the baseline-to-target diff, merged PRs, relevant current specs and validation evidence. Create `docs/development/release-notes/<releaseIdentity>.json` using the schema/example in `docs/development/releases.md`.

- Write concise musician-facing changes and fixes since that baseline, grouped by user outcome. Include known limitations, compatibility/default changes and actionable upgrade steps where relevant, such as explicit regeneration for older unfrozen swells when the delivered change requires it.
- Include only behavior present in the packaged source. Distinguish native/fixture evidence from actual host acceptance; do not claim backlog features, unfinished PRs, universal artifact elimination or untested host compatibility.
- Match `releaseIdentity` exactly, including the candidate. Record the previous release label/full SHA. Use plain text in all list items; packaging HTML-escapes them. Use empty arrays for inapplicable sections, not TODOs or invented claims.
- Review the notes against the final source after any integration changes. Do not reuse another candidate's notes without checking them. Commit the notes on the release branch through the normal review workflow; never edit main directly.

## Validate and package

1. Run `tests/release_notes_test.ps1` and the existing release-wrapper fixtures when changing note/packaging tooling. Keep simulated orchestration evidence distinct from native builds.
2. Use `maintain-user-guide` to review the delivered build and refresh its currency record deliberately. Commit intended changes and confirm a clean worktree before packaging. Packaging must not invoke an agent or rewrite tracked notes/guide inputs.
3. Run the existing `scripts/release.ps1 -ReleaseTag <tag> -Candidate <candidate>` (omit Candidate for stable). Preserve its full native Release/CTest, source, identity, guide and immutable-output gates. If providing a multiline PowerShell command, wrap it in `& { ... }` with exit-code handling.
4. Verify the ZIP hash/manifest inventory and freshly extracted `Release-Notes.html`, `README.txt`, complete VST3 bundle and `User-Guide.html`. Open the HTML offline; confirm identity, source, baseline, new features/fixes, upgrade advice and limitations match the exact candidate. Never inject notes by reopening an already completed ZIP.
5. Record the exact source/candidate and tests actually performed, plus any remaining exact-binary FL Studio acceptance. Keep owner host testing consolidated. Leave any requested PR open for review; packaging does not authorize merging, publishing, tags or sending binaries.

Notes are authored for the release being built, not a static generic changelog. The packager validates them before native build, renders the offline HTML with the actual source SHA, and includes it in the same checksummed inventory as the plugin and guide.

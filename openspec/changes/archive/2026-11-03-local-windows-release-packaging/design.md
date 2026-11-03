# Design

## Context

See proposal.md for motivation. build.ps1 owns configure/build/CTest and its binary-directory lock. verify-version.cmake owns numeric version, candidate syntax and built VST3 identities. The bundle already carries WavPack, Inter and IR notices. JUCE use is currently internal evaluation; there is no repository-wide distribution license grant.

## Goals / Non-Goals

**Goals:** fail-closed packaging; inspectable source/artifact provenance; separate manual publication with licensing evidence.

**Non-Goals:** bit-identical compiler/ZIP outputs, license eligibility determination, automatic tagging, installation, signing or binary/session migration.

## Decisions

- Use release.ps1 plus importable release-support.ps1, PowerShell/.NET and existing CMake helpers. No module dependencies. Tests use disposable real Git repositories, native tool doubles and dummy bundles, with explicit simulated labels.
- Require -ReleaseTag vX.Y.Z and optional -Candidate rc.1. Verify clean tracked/untracked state, full HEAD and any existing identity tag before and after build/staging. No dirty-tree or skip-test escape hatch. Missing local tags are permitted for internal packages; existing ones must resolve to HEAD.
- Invoke build.ps1 in a child of the current PowerShell host with Release -Test so exit cannot terminate packaging. Reacquire its shared build lock before built checks and copying. Resolve the unchanged preset binary directory; validate cache source/generator/x64. Verify the actual PE machine and Windows file version in addition to #20 metadata.
- Stage in a unique ignored sibling directory and atomically move to an absent final identity directory. Never use overwrite/force on release outputs. Preserve failed staging for diagnostics. Full bundle copied recursively, rejecting reparse points; compare staged file hashes with source. Add JUCE's pinned license and VST3 SDK license to the packaged notices alongside existing notices. Include internal-use README, manifest sidecar and ZIP hash sidecar; manifest hashes ZIP and bundle files, avoiding a circular embedded ZIP checksum.
- Keep -Publish as a separate mode consuming an existing package. Revalidate clean matching HEAD, tag/version, file inventory and hashes. Require -RedistributionReviewed, a substantive -LicenseReview record and exact -ConfirmPublish identity@SHA before network access. Require the separately manually pushed remote tag to identify that SHA, then gh release create --verify-tag with explicit --prerelease for candidates and --latest=false. No calls to publishing mode during native implementation validation. Tests intercept the gh boundary.

## Risks / Trade-offs

- Unsigned binaries and an inventory are not cryptographic provenance attestation; owner reviews the source/toolchain and licenses before redistribution.
- Build wrapper lock is released when its process exits; release immediately reacquires it and validates unchanged source/cache before copying. Concurrent source edits are detected by repeated Git checks; developers must keep a release checkout idle.
- Output directories are immutable; retry with a distinct explicit candidate or deliberately relocate old outputs while idle. Interrupted staging is retained, never promoted as a completed release.
- Local ZIPs remain internal evaluation artifacts. Copying license texts does not grant a JUCE, project or asset redistribution entitlement.

## Migration Plan

Add scripts and documentation only. Rollback removes these additions; no plugin state or installed files are modified. Leave the change open for owner review; do not sync/archive without approval.

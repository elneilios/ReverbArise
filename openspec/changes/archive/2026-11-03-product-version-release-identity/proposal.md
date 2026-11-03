# Proposal

## Why

Issue #20 needs a stable, manually controlled product version and a verifiable release identity before Issue #21 adds packaging. JUCE currently inherits CMake's version implicitly, with no cheap release-tag check or documented schema separation.

## What Changes

- Keep `project(Preverb VERSION 0.1.0 ...)` as the only authoritative numeric product version and explicitly pass it to JUCE.
- Add a dependency-free CMake script for reading the version and validating a requested exact `vX.Y.Z` tag; provide machine-readable local release metadata for Issue #21.
- Permit an explicit candidate label as metadata only, without changing the numeric version or host identity.
- Verify generated and built JUCE/VST3 metadata; document manual version bumps and compatibility invariants.

## Capabilities

### New Capabilities

- `product-release-identity`: Authoritative product version, release/tag verification, candidate labels and stable host identity.

### Modified Capabilities

None. Local build and persistence contracts remain unchanged.

## Impact

Focused changes to CMake, a small verification script and tests, and living development guidance. No automatic bumping, services, packaging, publication, DSP/UI changes, storage migrations or identifier changes. No archive or merge is authorized.

# Proposal

## Why

Issue #21 needs a local, auditable Windows VST3 package after the completed build (#19) and product identity (#20) foundations. Packaging must not imply redistribution permission or publication.

## What Changes

- Add a fail-closed Release build/test/package command with clean Git revision and exact product/release identity checks.
- Preserve the complete bundle and notices; emit an immutable versioned ZIP, SHA-256 and build manifest under ignored out/releases.
- Add a separate manual publication mode with artifact revalidation, explicit confirmation and a recorded redistribution review gate, particularly for JUCE.
- Document extraction, FL Studio checks and separate native versus fixture evidence.

## Capabilities

### New Capabilities

- `local-release-packaging`: locally validated Windows x64 packages and separately authorized manual GitHub publication.

### Modified Capabilities

None. Consume the existing local-windows-validation and product-release-identity contracts unchanged.

## Impact

PowerShell scripts, focused orchestration fixtures and development/validation documentation only. No DSP, host IDs, storage formats, installers, signing, CI or new package dependencies.

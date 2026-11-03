# product-release-identity Specification

## Purpose

Provide a manually controlled product version and verifiable release identity while preserving host identifiers and independent persisted-data schemas.

## Requirements

### Requirement: One authoritative product version
The numeric product version SHALL come only from the root CMake project declaration, initially 0.1.0. It SHALL use three nonnegative decimal components without leading zeroes, within the supported JUCE numeric representation. Normal commits and merges SHALL NOT automatically bump it.

#### Scenario: Read without building
- **WHEN** a developer requests the product version
- **THEN** a lightweight local check returns the CMake version without downloading dependencies or configuring the plugin

### Requirement: Verifiable release identity
A supplied release tag SHALL exactly match v followed by the configured product version. Invalid tags and mismatches SHALL fail nonzero before release work. An explicitly supplied candidate label SHALL be recorded separately and SHALL NOT change the numeric plugin version. Verification SHALL expose machine-readable metadata for later packaging without creating a package or publishing.

#### Scenario: Matching tag
- **WHEN** v0.1.0 is checked against product version 0.1.0
- **THEN** verification succeeds and records the matching tag and numeric version

#### Scenario: Mismatched or malformed tag
- **WHEN** a different numeric tag or an invalid tag is supplied
- **THEN** verification fails clearly without producing successful release metadata

#### Scenario: Explicit candidate
- **WHEN** rc.1 is supplied as a candidate label for product version 0.1.0
- **THEN** release metadata identifies the candidate while the plugin version remains 0.1.0

### Requirement: Built metadata and stable host identity
The built JUCE plugin and VST3 metadata SHALL report the CMake product version. Manufacturer code K7Ma, plugin code Prv1, bundle ID com.k7ma.preverb and existing VST3 class identities SHALL remain unchanged by product releases or candidate labels.

#### Scenario: Verify a built plugin
- **WHEN** a supported local build is checked
- **THEN** its generated JUCE definitions and built VST3 metadata match the authoritative version and established identity

### Requirement: Independent storage compatibility
Product versions SHALL remain independent of plugin saved-state, swell-session, capture and asset/manifest schema versions. A product bump or candidate label SHALL NOT migrate or invalidate persisted state, source audio, assets or saved DAW sessions. The manual bump and verification procedure SHALL document these boundaries.

#### Scenario: Product version changes
- **WHEN** an owner changes only the CMake product version
- **THEN** storage schema versions and persisted identities remain unchanged and existing state restoration behavior is preserved

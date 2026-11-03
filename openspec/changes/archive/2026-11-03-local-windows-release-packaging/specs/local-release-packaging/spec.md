# Local release packaging delta

## Purpose

Produce locally validated Windows x64 VST3 artifacts with traceable source identity and deliberate owner-controlled redistribution.

## ADDED Requirements

### Requirement: Validated source and release identity
The packager SHALL require a clean Git worktree, the actual HEAD commit and an exact CMake version tag. Existing release tags SHALL identify HEAD. Candidate labels SHALL remain explicit without changing plugin or storage identity. Source changes during validation SHALL fail packaging.

#### Scenario: Invalid source identity
- **WHEN** the worktree is dirty, the version mismatches, or an existing release tag points elsewhere
- **THEN** packaging fails before producing a completed release

#### Scenario: Explicit candidate
- **WHEN** a clean checkout is packaged with candidate rc.1
- **THEN** the directory, ZIP and manifest identify vX.Y.Z-rc.1 and preserve numeric plugin version X.Y.Z

### Requirement: Matching native validation
Packaging SHALL invoke scripts/build.ps1 for Release with CTest validation and verify built version and stable identity before copying artifacts. Missing or empty binary, required notices or provenance resources SHALL fail packaging. Concurrent wrapper mutation SHALL be excluded during bundle inspection and copying.

#### Scenario: Validation failure
- **WHEN** building, testing or built identity validation fails
- **THEN** no completed package is produced and the command exits nonzero

### Requirement: Complete immutable artifacts
Successful packaging SHALL preserve the entire VST3 bundle and required notices and produce a versioned Windows x64 ZIP, SHA-256 checksum and build-manifest.json under ignored out/releases. The manifest SHALL record version, candidate, full Git SHA, UTC time, target, toolchain, artifact name and checksum. Existing outputs SHALL never be silently replaced.

#### Scenario: Repeat invocation
- **WHEN** the release identity already has an output directory
- **THEN** packaging fails and leaves its contents untouched

#### Scenario: Successful package
- **WHEN** native Release validation and bundle checks succeed
- **THEN** the ZIP contains the complete bundle and notices and its checksum matches the manifest and checksum file

### Requirement: Separate manual publishing and licensing gate
Local packaging SHALL work without GitHub access once dependencies are available. GitHub publishing SHALL be a separate manual action requiring authenticated gh, artifact/source revalidation, an exact confirmed identity and revision, and an explicit recorded redistribution review covering JUCE and other included materials. Local test packages SHALL convey no redistribution authorization.

#### Scenario: Packaging alone
- **WHEN** a developer generates an internal local ZIP
- **THEN** no GitHub release or tag is created and redistribution remains unapproved

#### Scenario: Unconfirmed publication
- **WHEN** publication lacks confirmation or the redistribution review
- **THEN** the command fails before mutating GitHub

### Requirement: Honest operational guidance
Documentation SHALL describe prerequisites, extraction/installation, manual tag publication, the redistribution gate and FL Studio compatibility checks. Evidence SHALL distinguish simulated orchestration tests, actual native validation and unperformed manual host checks.

#### Scenario: Recorded validation
- **WHEN** implementation checks are documented
- **THEN** actual commands and results are recorded separately from simulated or unperformed checks

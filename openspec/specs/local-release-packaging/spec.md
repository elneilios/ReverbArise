# local-release-packaging Specification

## Purpose

Produce locally validated Windows x64 VST3 artifacts with traceable source identity and deliberate owner-controlled redistribution.

## Requirements

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

### Requirement: Reviewed guide currency before packaging
Release packaging SHALL check a committed guide review record against the delivered code, canonical specifications, guide and relevant build/release inputs before building. Missing or stale review SHALL fail clearly. Guide authoring SHALL remain a separate explicit maintenance workflow; packaging SHALL NOT rewrite guide content or its review record, or invoke an agent.

#### Scenario: Delivered behavior changes
- **WHEN** a reviewed code/specification/guide input changes or is added/removed
- **THEN** packaging fails until the guide is reviewed and its currency record is deliberately refreshed

#### Scenario: Current guide
- **WHEN** the guide review record matches all current inputs
- **THEN** packaging proceeds without modifying tracked guide content or its review record

### Requirement: Offline illustrated recipient guide
The release ZIP SHALL include a short quick start and a single static HTML user guide with embedded media, no external URLs, scripts, frameworks or network dependencies. The guide SHALL identify the candidate and source revision and explain installation/scanning, routing, Learn capture and markers, Generate Swells, Perform audition/editing, Export WAV, save/reopen, host coverage, troubleshooting, limitations and feedback.

#### Scenario: Extracted offline reading
- **WHEN** a recipient opens the guide locally after extracting the ZIP with network access unavailable
- **THEN** all instructions and embedded images display, with numbered musician-facing tasks and no missing external resources

### Requirement: Authentic build-matched illustrations
Guide screenshots SHALL come from the existing native UI evidence mechanism using the ordinary production editor at the packaged source revision. They SHALL show functioning empty Learn, captured markers, generated Perform with a selected swell, and Export states. Captions SHALL distinguish deterministic fixtures from DAW acceptance and SHALL NOT imply unavailable controls or untested host compatibility.

#### Scenario: Screenshot provenance
- **WHEN** guide images are accepted for a candidate
- **THEN** evidence records their source revision, fixture commands and state coverage and verifies their presentation against the packaged build

### Requirement: Private alpha acceptance record
A private alpha acceptance record SHALL identify the clean PR revision, candidate, tools, native Release/UI/product/OpenSpec results, ZIP/inventory hashes and offline-guide inspection. Exact-binary FL Studio capture, generation, Perform audition and save/reopen SHALL be distinguished from prior host runs. Clean-machine and second-host coverage SHALL be explicit.

#### Scenario: Pre-merge sign-off and main verification
- **WHEN** the owner signs off the clean PR candidate and authorizes its merge
- **THEN** the resulting clean main revision is verified separately after merge, without relabelling PR results as main results

#### Scenario: Earlier integration evidence
- **WHEN** earlier component or host tests exist but the final packaged binary has not been tested in FL Studio
- **THEN** alpha host acceptance remains pending and those earlier results are not reported as final packaged-binary acceptance

### Requirement: Owner-controlled private sharing
External private alpha sharing SHALL require explicit owner approval of JUCE licensing, project distribution rights and bundled IR/font/third-party obligations with required notices included. Package creation SHALL remain local and SHALL NOT create or push tags or publish a public release. Evidence SHALL distinguish an internally prepared candidate from an approved externally shareable bundle.

#### Scenario: Redistribution review pending
- **WHEN** a candidate exists but the owner has not approved the redistribution review
- **THEN** the candidate remains internal evaluation only and external sharing is pending

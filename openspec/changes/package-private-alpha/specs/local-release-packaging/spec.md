# Spec Delta

## ADDED Requirements

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
- **THEN** the resulting clean main revision is verified separately before archiving, without relabelling PR results as main results

#### Scenario: Earlier integration evidence
- **WHEN** earlier component or host tests exist but the final packaged binary has not been tested in FL Studio
- **THEN** alpha host acceptance remains pending and those earlier results are not reported as final packaged-binary acceptance

### Requirement: Owner-controlled private sharing
External private alpha sharing SHALL require explicit owner approval of JUCE licensing, project distribution rights and bundled IR/font/third-party obligations with required notices included. Package creation SHALL remain local and SHALL NOT create or push tags or publish a public release. Evidence SHALL distinguish an internally prepared candidate from an approved externally shareable bundle.

#### Scenario: Redistribution review pending
- **WHEN** a candidate exists but the owner has not approved the redistribution review
- **THEN** the candidate remains internal evaluation only and external sharing is pending

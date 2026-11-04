# Spec Delta

## ADDED Requirements

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
A private alpha acceptance record SHALL identify a clean main revision containing the required integrations, candidate identity, tools, native Release results, UI standards, product identity, OpenSpec checks, ZIP/inventory hashes and extracted offline-guide inspection. Final packaged-binary FL Studio capture, generation, Perform audition and save/reopen SHALL be recorded separately from prior host runs. Clean-machine and second-host coverage SHALL be explicit.

#### Scenario: Earlier integration evidence
- **WHEN** earlier component or host tests exist but the final packaged binary has not been tested in FL Studio
- **THEN** alpha host acceptance remains pending and those earlier results are not reported as final packaged-binary acceptance

### Requirement: Owner-controlled private sharing
External private alpha sharing SHALL require explicit owner approval of JUCE licensing, project distribution rights and bundled IR/font/third-party obligations with required notices included. Package creation SHALL remain local and SHALL NOT create or push tags or publish a public release. Evidence SHALL distinguish an internally prepared candidate from an approved externally shareable bundle.

#### Scenario: Redistribution review pending
- **WHEN** a candidate exists but the owner has not approved the redistribution review
- **THEN** the candidate remains internal evaluation only and external sharing is pending

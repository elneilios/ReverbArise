# Swell regeneration delta

## MODIFIED Requirements

### Requirement: Regeneration operates on the requested set
The plugin SHALL provide Regenerate Selected and Regenerate All using committed capture without host playback. Selected SHALL process only selected confirmed swells. All SHALL force fresh work for eligible confirmed swells regardless of current readiness, including disabled or muted definitions, while skipping frozen or importing-IR targets. Jobs SHALL freeze source/IR inputs, report progress, and publish only complete results matching the latest request for each swell.

#### Scenario: Generate selected targets
- **WHEN** the user regenerates one selected swell while another is already current
- **THEN** only the selected swell is rendered and the other's audio, edits, and identity remain unchanged

#### Scenario: Batch contains an invalid source
- **WHEN** Regenerate All includes one swell with a source gap and another with complete valid source
- **THEN** the first reports its own failure without publishing partial output or preventing the second from completing

#### Scenario: Edit while queued or rendering
- **WHEN** a swell is deleted or its source/IR changes before its job finishes
- **THEN** that completion cannot recreate the deleted swell or replace the newly requested result

#### Scenario: Repeat Regenerate All on current results
- **WHEN** eligible swells are Ready using current processing and the user explicitly invokes Regenerate All again
- **THEN** each eligible target receives a fresh render request through bounded background work, even though its inputs and processing version are unchanged
- **AND** the action does not recapture source, alter edits or override frozen/importing ownership

#### Scenario: Regenerate other swells during an IR replacement
- **WHEN** a valid IR replacement supersedes a render already in progress and the user invokes Regenerate All
- **THEN** the other eligible swells can complete their requested regeneration and the replacement can settle under its own request
- **AND** the replacement retains the previous accepted assignment and render until the verified new assignment, wet shape and render publish together

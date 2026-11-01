# Spec Delta

## ADDED Requirements

### Requirement: Committed capture assets persist beyond the running session
The system SHALL preserve committed captured source samples and their absolute Song positions outside the running plugin process.

#### Scenario: Reopen a saved project on the same PC
- **GIVEN** capture for one or more song ranges has been committed and the FL Studio project has been saved
- **WHEN** the same project is reopened on the same PC and Windows user profile
- **THEN** its committed captured samples remain available at their original Song positions

### Requirement: Project state resolves its capture assets
The system SHALL save compact identifiers and references needed to associate an FL Studio project instance with its managed capture assets, without embedding the captured PCM in plugin state.

#### Scenario: Restore the correct instance assets
- **GIVEN** the project contains a saved Preverb instance with committed capture assets
- **WHEN** FL Studio restores that instance from project state
- **THEN** the instance resolves its own capture data and coverage rather than another instance's data

### Requirement: Persisted coverage retains sparse-map semantics
The system SHALL restore captured audio, captured silence, and uncaptured gaps as distinct states at their original absolute Song positions.

#### Scenario: Restore non-contiguous ranges
- **GIVEN** a project has committed captures in bars 86-90 and bars 1-4 with an uncaptured interval between them
- **WHEN** that project is reopened
- **THEN** both ranges remain aligned and the intervening gap remains uncaptured

#### Scenario: Restore recaptured overlap
- **GIVEN** a committed range was recaptured over only part of existing coverage
- **WHEN** the project is reopened
- **THEN** the replaced overlap contains the new capture and unaffected samples remain unchanged

### Requirement: Capture commits are recoverable
The system SHALL publish capture updates so an interrupted write cannot replace the last valid committed map with a partial capture or manifest.

#### Scenario: Interrupted capture commit
- **GIVEN** a valid capture map exists and a new capture update is being committed
- **WHEN** the application or system stops before the new commit completes
- **THEN** reopening retains the previous valid map and does not present the incomplete range as committed

### Requirement: Missing or invalid capture assets are reported
The system SHALL identify missing or invalid referenced assets on reopen without presenting those ranges as captured silence.

#### Scenario: Referenced asset is missing or corrupt
- **GIVEN** saved project state references a capture asset that cannot be read or verified
- **WHEN** the project is reopened
- **THEN** the affected range is reported unavailable or stale, while unaffected valid ranges remain accessible

### Requirement: Sample-rate changes do not reinterpret persisted captures
The system SHALL preserve captures at their recorded sample rate and prevent incompatible new samples from being merged into that map after the host sample rate changes.

#### Scenario: Reopen at a different sample rate
- **GIVEN** a project has committed captures recorded at one sample rate
- **WHEN** the project is opened with a different host sample rate
- **THEN** the existing map is marked stale, retained at its original positions, and mapping remains paused until the user clears and recaptures the map at the new rate

### Requirement: Song overview communicates capture coverage
The system SHALL provide a compact overview on the Song timeline that distinguishes captured audio, captured silence, and uncaptured gaps.

#### Scenario: Inspect progressive coverage
- **GIVEN** the map contains non-contiguous captured ranges and at least one captured-silence range
- **WHEN** the user inspects the overview
- **THEN** the captured ranges, captured silence, and uncaptured gaps are visually distinguishable at their absolute Song positions
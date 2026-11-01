# song-timeline-audio Specification

## Purpose

This capability captures audio into a sparse map of absolute song positions and schedules generated audio against the host timeline, preserving timing across non-contiguous capture, seeking, and replay.

## Requirements

### Requirement: Host song position is observable
The effect SHALL expose the FL Studio Song transport state and available per-block position, tempo, meter, and block-size information without presenting unavailable host data as valid values.

#### Scenario: Song transport diagnostics
- **GIVEN** the effect is processing FL Studio 2026 Song-mode playback
- **WHEN** a host audio block is processed
- **THEN** diagnostics identify the available absolute sample or PPQ position, transport state, tempo, meter, and block size for that block

### Requirement: Capture coverage is sparse and position-aware
The effect SHALL associate captured input samples with absolute Song positions and distinguish captured audio, captured silence, and uncaptured ranges.

#### Scenario: Capture separate song sections
- **GIVEN** mapping captures bars 86 through 90
- **WHEN** mapping later captures bars 1 through 4
- **THEN** both sections remain aligned to their Song positions and the interval between them remains uncaptured

#### Scenario: Captured silence remains mapped
- **GIVEN** mapping is enabled while the host processes a zero-valued input block
- **WHEN** the block is stored in the song map
- **THEN** its interval is marked as captured silence rather than uncaptured

### Requirement: Recapture replaces only overlapping samples
When mapping captures an interval that overlaps existing coverage, the new pass SHALL replace samples only within that captured interval and preserve all other mapped intervals.

#### Scenario: Recapture an existing section
- **GIVEN** the map contains captured sections before, within, and after the recaptured interval
- **WHEN** mapping captures the same interval again
- **THEN** samples and coverage within the interval reflect the new pass while sections outside it remain unchanged

### Requirement: Scheduled tone is independent of source input
The effect SHALL generate a configurable 440 Hz test tone at -18 dBFS peak over its configured absolute Song interval, with short click-free boundary fades, independently of incoming signal content.

#### Scenario: Tone on a routed silent mixer channel
- **GIVEN** the effect is on a routed mixer channel and its input is zero-valued
- **WHEN** Song playback reaches the configured interval from bar 8 up to the start of bar 10
- **THEN** the effect output contains the scheduled tone at the configured level and interval

### Requirement: Scheduled tone follows absolute song position
The effect SHALL derive scheduled tone phase and event position from the absolute Song timeline so seeks and repeated playback do not restart or duplicate an event incorrectly.

#### Scenario: Seek into an active tone
- **GIVEN** playback seeks to the middle of the scheduled tone interval
- **WHEN** the host processes the first block after the seek
- **THEN** output begins at the tone sample corresponding to that absolute position rather than restarting at phase zero

#### Scenario: Replay or loop over the tone interval
- **GIVEN** the host replays or loops across the scheduled interval
- **WHEN** the same absolute Song positions are processed again
- **THEN** the tone has repeatable phase and boundaries without duplicated or missing event audio

### Requirement: Source passthrough remains available
The effect SHALL pass incoming stereo audio through while mapping is active and outside the scheduled test tone interval.

#### Scenario: Capture without altering source
- **GIVEN** mapping is active and no test tone is scheduled for the current block
- **WHEN** stereo input is processed
- **THEN** the effect outputs the input without replacing it with previously captured audio

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

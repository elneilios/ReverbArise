# Spec Delta

## Purpose

This capability captures audio into a sparse map of absolute song positions and schedules generated audio against the host timeline, preserving timing across non-contiguous capture, seeking, and replay.

## ADDED Requirements

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
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
Only explicitly armed Recapture in Capture mode SHALL permit replacing mapped samples. A successfully validated pass SHALL atomically replace its encountered intervals and preserve all others. Ordinary playback SHALL protect mapped audio and mapped silence, filling only unmapped positions. Automatic Swells and performance tolerance SHALL NOT authorize replacement.

#### Scenario: Recapture an existing section
- **GIVEN** mapped sections exist before, within and after the encountered recapture interval
- **WHEN** an explicitly armed Capture pass records that interval and successfully commits on DAW Stop
- **THEN** only encountered samples/coverage use the new pass and untouched sections retain their committed values

#### Scenario: Playback across partially mapped source
- **WHEN** an unarmed Capture block overlaps mapped audio, mapped silence and an unmapped gap
- **THEN** only the gap is staged, with exact sample alignment, and mapped samples/identities remain protected

#### Scenario: Song and Pattern playback
- **WHEN** FL Studio reports usable absolute positions in either Song or Pattern playback with Recapture off
- **THEN** unmapped positions can be captured but previously mapped positions cannot be replaced
- **AND** Pattern mode is not blanket-suppressed or claimed to have uniquely reliable Song content

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
Capture and Finalise SHALL pass incoming stereo dry unchanged without host-scheduled wet or capture-generated audio. Capture SHALL record original input before any generated output. Live SHALL retain its saved routing contract while never capturing. Separate explicit internal preview SHALL remain distinct from host-scheduled audio.

#### Scenario: Capture without altering source
- **GIVEN** Capture is active and DAW playback supplies stereo input
- **WHEN** input blocks are admitted for mapping
- **THEN** host output remains dry passthrough and previously captured audio, wet renders and diagnostic tone do not replace or enter the recorded source

#### Scenario: Finalise with a saved Wet Only live preference
- **WHEN** Finalise is selected and normal DAW processing runs without internal preview
- **THEN** dry passes unchanged, with neither capture nor scheduled wet, and the saved Live routing preference is preserved

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
Capture passes SHALL stage immutable source assets and publish a complete validated manifest atomically on explicit DAW Stop. Until acceptance, readers and project saves SHALL reference the previous committed map. Abort, failed validation/publication or incomplete capture SHALL NOT partially replace that map. Prior committed revisions SHALL remain recoverable.

#### Scenario: Interrupted capture commit
- **GIVEN** a valid map exists and a multi-chunk pass is being staged or committed
- **WHEN** processing stops unexpectedly before durable publication completes
- **THEN** reopening the earlier committed state retains the previous valid map and no incomplete range is presented as committed

#### Scenario: Failure after multiple staged chunks
- **WHEN** disk-full, corrupt/nonfinite data, queue loss or publication failure occurs after earlier pass chunks were written
- **THEN** the pass is rejected, all prior committed references remain unchanged and a useful error is shown
- **AND** provisional assets are not mistaken for accepted source or deleted along with sole committed copies

#### Scenario: Successful multi-chunk replacement
- **WHEN** Stop closes an admitted recapture pass spanning multiple chunks and seeks
- **THEN** all validated encountered changes appear together as one accepted revision rather than a sequence of partial source replacements

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

### Requirement: Recapture permission is explicit and transient
Recapture existing audio SHALL be off by default and independent of Automatic Swells. Its permission SHALL apply only to an explicitly armed Capture pass, disarm after a successfully committed replacement pass, and never resume from saved state. Failure SHALL report retry/cancel status without implying success.

#### Scenario: Independent toggles
- **WHEN** Automatic Swells is switched off and Recapture is explicitly armed
- **THEN** encountered source can be transactionally replaced without creating automatic swells
- **AND** enabling Automatic Swells with Recapture off cannot replace mapped source

#### Scenario: Successful and failed consumption
- **WHEN** a useful explicitly armed pass succeeds, including identical verified replacement PCM
- **THEN** Recapture becomes off
- **AND WHEN** another armed pass fails or is cancelled
- **THEN** no partial changes commit and the remaining arm/cancel state is truthfully reported

### Requirement: Capture progress distinguishes provisional and accepted source
Capture progress SHALL identify staged coverage separately from the committed map. Loops/seeks SHALL preserve absolute coordinates. Ordinary Capture SHALL not overwrite positions already staged earlier in the same pass; armed Recapture SHALL resolve repeated received positions deterministically without exposing partial replacements.

#### Scenario: Loop within an uncommitted pass
- **WHEN** ordinary Capture revisits positions first recorded earlier in the same pass
- **THEN** those staged samples stay fixed, later uncovered positions can be added and no loop wrap is treated as a completed Stop commit

#### Scenario: Repeated explicit replacement
- **WHEN** an armed pass records different performances at the same positions before Stop
- **THEN** its last completely received samples at those positions determine the validated replacement and the old committed map remains readable until acceptance

### Requirement: Pass cancellation preserves source ownership
Cancelled or obsolete capture work SHALL NOT publish into another restored session, cloned instance or newer transaction. Instances sharing immutable old assets SHALL retain independent publication ownership. Cancellation SHALL not delete previous snapshot dependencies or durable exports.

#### Scenario: Duplicate and record independently
- **WHEN** an instance is duplicated and one copy captures/recaptures while the other retains its saved map
- **THEN** only the recording copy accepts a new source manifest and the other copy retains identical prior samples

#### Scenario: Rate or timing becomes invalid
- **WHEN** rate incompatibility, invalid coordinates or unsupported capture data invalidates an open pass
- **THEN** the pass cannot publish a partial replacement, previous map/rate remain unchanged and recovery status identifies the cause

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
Only a Song take explicitly authorized with Arm and with Recapture Audio enabled SHALL permit replacing mapped samples. A successfully validated pass SHALL atomically replace its encountered intervals and preserve all others. An armed additive take SHALL protect mapped audio and mapped silence, filling only unmapped positions. Automatic Swells and performance tolerance SHALL NOT authorize replacement.

#### Scenario: Recapture an existing section
- **GIVEN** mapped sections exist before, within and after the encountered recapture interval
- **WHEN** an explicitly armed Capture pass records that interval and successfully commits on DAW Stop
- **THEN** only encountered samples/coverage use the new pass and untouched sections retain their committed values

#### Scenario: Playback across partially mapped source
- **WHEN** an armed additive Song capture block overlaps mapped audio, mapped silence and an unmapped gap
- **THEN** only the gap is staged, with exact sample alignment, and mapped samples/identities remain protected

#### Scenario: Song and Pattern playback
- **WHEN** FL Studio reports usable absolute positions in either Song or Pattern playback during an armed Song take with Recapture Audio off
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
Capture passes SHALL stage immutable source and generation-context metadata, then publish one coherent validated source/context record atomically on explicit DAW Stop. Until acceptance, readers and saves SHALL retain the previous accepted record. Abort, validation/publication failure or incomplete capture SHALL NOT partially replace it. Prior committed revisions SHALL remain recoverable.

#### Scenario: Interrupted capture commit
- **GIVEN** a valid accepted map/context record exists and a multi-chunk pass is being staged or committed
- **WHEN** processing stops unexpectedly before durable publication completes
- **THEN** reopening the earlier state retains its prior source/context and no incomplete range or pass default is presented as committed

#### Scenario: Failure after multiple staged chunks
- **WHEN** disk-full, corrupt/nonfinite data, queue loss or publication failure occurs after earlier pass chunks were written
- **THEN** the pass is rejected, all prior accepted references remain unchanged and a useful error is shown
- **AND** provisional assets/context are not mistaken for accepted data or deleted along with sole committed copies

#### Scenario: Successful multi-chunk replacement
- **WHEN** Stop closes an admitted Recapture pass spanning chunks and seeks
- **THEN** all validated encountered source/context changes appear together as one accepted record instead of partial replacements

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
The system SHALL provide a mode-appropriate full-song capture overview: compact in Perform and the sole large dry timeline in Learn. It SHALL distinguish captured audio, captured silence, uncaptured gaps, unavailable data and provisional recording with explicit inspectable states, keeping real absolute positions and accepted/provisional totals distinct.

#### Scenario: Inspect progressive coverage
- **GIVEN** the map contains non-contiguous captured ranges and at least one captured-silence range
- **WHEN** the user inspects the overview
- **THEN** audio, silence and gaps are visually distinguishable and inspectable at their absolute positions without relying solely on waveform height

#### Scenario: Inspect an unfinished pass
- **WHEN** admitted incoming source is displayed before Stop acceptance
- **THEN** it is labelled provisional/Recording, prior accepted coverage remains distinguishable and capture totals do not claim it has already committed

### Requirement: Recapture permission is explicit and transient
Recapture existing audio SHALL be off by default and independent of Automatic Swells. Its permission SHALL be pinned when Arm authorizes a Song pass, disarm after a successfully committed replacement pass, and never resume from saved state. Failure SHALL report retry/cancel status without implying success.

#### Scenario: Independent toggles
- **WHEN** Automatic Swells is switched off and Recapture Audio is enabled before the user presses Arm
- **THEN** encountered source can be transactionally replaced without creating automatic swells
- **AND** enabling Automatic Swells with Recapture off cannot replace mapped source

#### Scenario: Successful and failed consumption
- **WHEN** a useful explicitly armed pass succeeds, including identical verified replacement PCM
- **THEN** Recapture becomes off
- **AND WHEN** another armed pass fails or is cancelled
- **THEN** no partial changes commit and the remaining arm/cancel state is truthfully reported

### Requirement: Capture progress distinguishes provisional and accepted source
Capture progress SHALL expose real admitted provisional spans/peaks separately from accepted source. Loops/seeks SHALL retain absolute coordinates. Ordinary Capture SHALL protect accepted coverage and earlier staged positions; armed Recapture SHALL resolve repeated positions deterministically. Provisional display SHALL NOT grant render/save/playback access or publish partial replacements.

#### Scenario: Loop within an uncommitted pass
- **WHEN** ordinary Capture revisits positions first recorded earlier in the same pass
- **THEN** those staged samples stay fixed, later uncovered positions can be added and no loop wrap is treated as a completed Stop commit
- **AND** incoming differences at protected positions do not falsely appear as accepted replacements in the display

#### Scenario: Repeated explicit replacement
- **WHEN** an armed pass records different performances at the same positions before Stop
- **THEN** its last completely received samples determine the validated replacement, provisional feedback reflects admitted data and the old accepted map remains readable until acceptance

#### Scenario: Continuous recording feedback
- **WHEN** valid audio arrives during a long pass before a storage batch is written
- **THEN** the timeline provides bounded progressive feedback from real admitted input rather than waiting for Stop or drawing fabricated peaks

#### Scenario: Cancel or supersede the displayed pass
- **WHEN** capture is cancelled, fails or is superseded by restore/new-session ownership
- **THEN** obsolete provisional display is discarded and cannot overwrite or relabel the current instance's accepted source

### Requirement: Pass cancellation preserves source ownership
Cancelled or obsolete capture work SHALL NOT publish into another restored session, cloned instance or newer transaction. Instances sharing immutable old assets SHALL retain independent publication ownership. Cancellation SHALL not delete previous snapshot dependencies or durable exports.

#### Scenario: Duplicate and record independently
- **WHEN** an instance is duplicated and one copy captures/recaptures while the other retains its saved map
- **THEN** only the recording copy accepts a new source manifest and the other copy retains identical prior samples

#### Scenario: Rate or timing becomes invalid
- **WHEN** rate incompatibility, invalid coordinates or unsupported capture data invalidates an open pass
- **THEN** the pass cannot publish a partial replacement, previous map/rate remain unchanged and recovery status identifies the cause

### Requirement: Accepted source retains deterministic generation provenance
Accepted source SHALL resolve the immutable generation context assigned to each admitted span. Ordinary additive capture SHALL preserve protected spans' provenance; Recapture SHALL replace only encountered provenance with its source. Saves/consumers SHALL observe one coherent accepted source/context record, including across loops, sparse seeks and instance copies. Explicit or opted-in shared Generate SHALL freeze current accepted source/settings/default intent separately without rewriting this capture history.

#### Scenario: Additive capture crosses old and new coverage
- **WHEN** a pass with a different Default IR crosses old mapped audio/silence and an uncaptured interval
- **THEN** old source/context remains unchanged and only the newly admitted interval receives the new pass context

#### Scenario: Different contexts meet at an attack boundary
- **WHEN** onset confirmation or extraction spans two accepted contexts
- **THEN** the arrival context remains its immutable capture provenance; a new shared Generate deliberately uses the request's current accepted Default IR instead of treating that historical context as an unrequested generation command

#### Scenario: Audio identity is unchanged
- **WHEN** identical PCM is encountered with unchanged context, or an explicit context-only update is accepted
- **THEN** unchanged audio-content identities and existing render/calibration dependencies are reused without unnecessary convolution

#### Scenario: Context metadata is unavailable
- **WHEN** accepted source is readable but required generation provenance or its IR cannot be verified
- **THEN** dry source stays preserved; recovery of a saved assignment reports its missing dependency without substitution, while a new Generate must explicitly validate its own current source/default snapshot and cannot silently repair missing historical data

### Requirement: Progressive display cannot compromise realtime capture safety
Progress collection/publication SHALL use bounded metadata and coalesced updates without full-song PCM duplication, callback allocation, locks, file access or waiting. Display pressure SHALL be handled explicitly without corrupting source admission or inventing coverage. Playback and mode transitions SHALL retain the established audio safety contract.

#### Scenario: Large sparse recording with the editor open or closed
- **WHEN** a long pass visits many separated positions and the editor opens/closes or summaries reach their budget
- **THEN** display work stays bounded, current coverage remains truthful, source transactions retain exact admission semantics and audio callbacks remain nonblocking

#### Scenario: Saving while progress is visible
- **WHEN** a project is saved during a provisional display update
- **THEN** it references only the coherent accepted source/context and cannot restore the provisional overlay as durable audio

### Requirement: Failed capture retains actionable diagnostic evidence
The system SHALL retain bounded first-failure evidence independently of provisional waveform data: reason, pass/epoch, accepted source identity/revision, attempted frame range, queued/dropped block counts and available host timing. Missing timing SHALL remain explicitly unavailable. Later symptoms SHALL NOT overwrite the original failure. Evidence SHALL survive settlement and retry until another failure or explicit session reset supersedes it.

#### Scenario: Queue pressure after visible recording progress
- **GIVEN** a pass has displayed provisional progress and staged multiple chunks
- **WHEN** capture admission fails because its queue is full
- **THEN** diagnostics retain the queue-full reason and failure-time context after the provisional waveform is discarded
- **AND** previously accepted references, source/context and immutable history remain unchanged

#### Scenario: Distinct failure reasons
- **WHEN** invalid host timing, incompatible rate, unsupported block size, invalid samples, storage/validation failure or a recoverable worker exception rejects a pass
- **THEN** evidence identifies that cause and its available context rather than substituting a generic queue failure
- **AND** later drops or settlement errors do not replace the first recorded failure

#### Scenario: Retry and session ownership
- **WHEN** a failed pass is followed by a successful retry
- **THEN** the earlier failure remains inspectable with its original pass identity and is not presented as a current failure
- **AND WHEN** restore or explicit session reset supersedes that session
- **THEN** stale completion cannot attach its evidence to the newly active session

### Requirement: Failed provisional capture feedback identifies durable coverage
After capture failure, user feedback SHALL identify that the in-progress pass was not accepted, distinguish discarded provisional progress from retained committed source, and provide a readable cause and retry guidance. If no source had been committed, feedback SHALL say so rather than imply retained audio. Display and diagnostics SHALL NOT grant access to rejected source as accepted audio.

#### Scenario: First pass fails
- **GIVEN** no accepted source exists and Learn displays provisional audio
- **WHEN** that pass fails
- **THEN** provisional audio disappears and feedback explains that no source from the pass was accepted and no earlier committed source exists

#### Scenario: Failure with prior committed coverage
- **GIVEN** earlier accepted source exists
- **WHEN** an additive or Recapture pass fails
- **THEN** its provisional overlay is discarded, earlier committed coverage remains visible and readable, and feedback identifies the retained source separately from the rejected pass

### Requirement: Failure reporting preserves bounded realtime processing
Capture failure observation SHALL use bounded metadata without callback allocation/deallocation, locks, waiting, file access or per-block logging. Formatting, storage inspection and publication SHALL occur outside realtime processing. Diagnostic capacity exhaustion SHALL NOT corrupt accepted source or publish an incomplete pass.

#### Scenario: Repeated failures under delayed worker pressure
- **WHEN** the capture worker is delayed and callbacks repeatedly encounter failure
- **THEN** callback failure reporting remains bounded and nonblocking, dry passthrough continues and the first failure is retained coherently
- **AND** stopping and retrying after the pressure is removed can accept a new complete pass without changing older immutable snapshots

### Requirement: An armed Song take follows valid transport
An armed Song take SHALL start at the first eligible playing block after Arm. While stopped, Arm SHALL wait for valid Play. An active Song take SHALL settle once on DAW Stop. Disarming during Song recording SHALL cancel provisional work and preserve the accepted source.

#### Scenario: Arm during valid playback
- **WHEN** a user arms Song while the DAW is already playing with compatible position and timing
- **THEN** capture starts at the first eligible block after Arm, with no backfill from earlier playback

#### Scenario: Arm while stopped
- **WHEN** a user arms Song before playback, then plays and stops
- **THEN** valid Play starts the one take and Stop settles it
- **AND** another Play requires a new arm to start a new take

#### Scenario: Disarm during Song recording
- **WHEN** the user disarms during a Song take
- **THEN** provisional source is discarded and a subsequent Stop cannot accept it or start Generate on Stop

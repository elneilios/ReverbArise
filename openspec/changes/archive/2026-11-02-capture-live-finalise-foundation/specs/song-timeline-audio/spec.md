# Spec Delta

## MODIFIED Requirements

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

### Requirement: Source passthrough remains available
Capture and Finalise SHALL pass incoming stereo dry unchanged without host-scheduled wet or capture-generated audio. Capture SHALL record original input before any generated output. Live SHALL retain its saved routing contract while never capturing. Separate explicit internal preview SHALL remain distinct from host-scheduled audio.

#### Scenario: Capture without altering source
- **GIVEN** Capture is active and DAW playback supplies stereo input
- **WHEN** input blocks are admitted for mapping
- **THEN** host output remains dry passthrough and previously captured audio, wet renders and diagnostic tone do not replace or enter the recorded source

#### Scenario: Finalise with a saved Wet Only live preference
- **WHEN** Finalise is selected and normal DAW processing runs without internal preview
- **THEN** dry passes unchanged, with neither capture nor scheduled wet, and the saved Live routing preference is preserved

## ADDED Requirements

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

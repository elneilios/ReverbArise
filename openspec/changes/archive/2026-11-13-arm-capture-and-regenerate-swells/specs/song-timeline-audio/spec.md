# Spec Delta

## MODIFIED Requirements

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

## ADDED Requirements

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

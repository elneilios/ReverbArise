# workflow-modes Specification

## Purpose
Define the mutually exclusive Capture, Live and Finalise workflow, including safe host-driven capture settlement, transition prerequisites and observable action availability without coupling it to a particular workspace layout.

## Requirements

### Requirement: One workflow mode governs instance behavior
Each instance SHALL have exactly one Capture, Live or Finalise mode. Fresh instances SHALL start in Capture with Recapture off. Mode SHALL govern source capture, scheduled wet, creative editing and internal audition independently of saved automation, monitoring and live preferences.

#### Scenario: Fresh instance
- **WHEN** a fresh instance is inserted without useful captured source
- **THEN** Capture is selected, Recapture is off, and Live/Finalise are unavailable with explanations
- **AND** no generated wet or internal audition starts automatically

#### Scenario: Independent instances
- **WHEN** two instances use different modes and receive interleaved host callbacks
- **THEN** their mode, capture-pass ownership and output remain independent

### Requirement: Mode entry prerequisites reflect verified material
Live entry SHALL require useful verified compatible source or an eligible verified frozen wet render at compatible timing. Finalise entry SHALL require useful compatible source plus an enabled, unmuted, solo-eligible ready render compatible with the current rate, tempo and meter. Mapped silence alone SHALL NOT satisfy usefulness. Pending wet renders SHALL NOT prevent source-ready Live entry. Frozen recovery SHALL NOT fabricate dry or permit replacement.

#### Scenario: Manual workflow after dry capture
- **WHEN** useful source has committed with Automatic Swells off and no prepared swell
- **THEN** Live is available for manual editing and Finalise remains unavailable

#### Scenario: Silence or failed verification
- **WHEN** coverage is only silence, unavailable, incompatible or still validating
- **THEN** neither mode becomes available by treating that coverage as useful compatible source
- **AND** valid stored material is retained without reinterpretation

#### Scenario: Finalise readiness
- **WHEN** useful compatible source and an enabled ready swell exist
- **THEN** Finalise can be selected without requiring an immediate export

#### Scenario: Ready render has incompatible timing
- **WHEN** useful source matches the host but a ready render has a different sample rate, tempo or meter
- **THEN** that render cannot qualify for Finalise or frozen-wet recovery; eligibility does not substitute for the later export workspace's full asset/output preparation

#### Scenario: Frozen wet recovery without source
- **WHEN** captured dry is missing/cleared but an enabled unmuted solo-eligible frozen render verifies at compatible timing
- **THEN** Live entry and wet editing/playback are available without recreating source
- **AND** unfrozen source-dependent swells remain unavailable, captured-dry audition still requires real dry, and no Recapture permission is granted

### Requirement: Capture activity locks manual mode switching
Manual mode switching SHALL be disabled during active DAW Capture and while its Stop-triggered drain, commit or validation is settling. Rejection SHALL identify the reason, apply to every command path, and leave current mode/source unchanged. A stopped idle Capture SHALL permit transitions whose prerequisites hold.

#### Scenario: Attempt to leave recording Capture
- **WHEN** a user requests Live or Finalise during recording or delayed Stop validation
- **THEN** the request is rejected with a recording/settling explanation and no partial source is exposed

#### Scenario: Mapped-only playback
- **WHEN** DAW playback in Capture encounters only protected mapped positions
- **THEN** source remains unchanged and stopping leaves Capture idle without claiming new useful capture
- **AND** Live can be selected explicitly while stopped if its prerequisites hold

### Requirement: Validated useful Stop commits automatically enter Live
An explicit DAW Stop SHALL close the pass once and settle asynchronously. Useful new audio, or successful explicit mapped replacement with useful committed material remaining, SHALL automatically select Live without waiting for wet renders. Empty, mapped-only, aborted, incompatible or failed passes, and sessions left without useful material, SHALL remain Capture with truthful results and prior valid data intact.

#### Scenario: Stop while wet work remains pending
- **WHEN** source validation succeeds and related swell jobs are still queued or rendering
- **THEN** Live is selected immediately after source acceptance and reports unfinished wet preparation honestly

#### Scenario: Empty or failed pass
- **WHEN** Stop closes an empty/mapped-only pass, publication fails, or no useful material remains after a silence-only replacement
- **THEN** Capture remains selected, the real reason is shown and earlier committed source remains available

#### Scenario: Identical explicit recapture
- **WHEN** an explicitly armed pass receives useful mapped audio identical to the accepted source and validation succeeds
- **THEN** Live is selected and Recapture disarms without forcing duplicate source or wet assets

#### Scenario: Recapture deliberately replaces a note with silence
- **WHEN** an explicitly armed pass successfully replaces mapped samples with silence and other useful committed source remains
- **THEN** Live is selected and Recapture disarms, including for identical already-mapped silence; unchanged PCM retains its source/render identity

### Requirement: Transport events cannot publish a capture pass twice
Capture settlement SHALL be idempotent and scoped to its originating instance/session/pass. Duplicate Stop reports, loops, seeks, restarts, restoration and superseded work SHALL NOT publish a pass twice, overwrite a newer revision or trigger a stale mode transition. Missing timing SHALL NOT be interpreted as zero position or a successful Stop.

#### Scenario: Repeated Stop and delayed completion
- **WHEN** repeated stopped callbacks arrive while one validation job is delayed
- **THEN** the pass is committed at most once and completion causes at most one accepted mode transition

#### Scenario: Loop and restart
- **WHEN** playing transport loops/seeks or restarts before a prior Stop settles
- **THEN** spans retain their absolute coordinates, loops do not falsely finalize the pass and new audio cannot enter the closing transaction

#### Scenario: Restore during validation
- **WHEN** a different saved session is restored while a pass completion is pending
- **THEN** that completion cannot publish into the restored session or change its mode

### Requirement: Mode changes preserve the working session
Entering Finalise SHALL preserve captured source, swell definitions, accepted IR/render versions and editing/monitoring preferences while locking creative edits. Returning to Live SHALL restore editing and saved live behavior without recapture, data loss, export requirements or implicit audition/host transport requests.

#### Scenario: Return without exporting
- **WHEN** a user enters Finalise and then returns to Live without creating any file
- **THEN** the same source, swells, edits and prepared assets remain and Live can be used indefinitely

#### Scenario: Existing exported assets
- **WHEN** the working session includes durable exported files produced by another capability and modes change
- **THEN** those files are not removed or overwritten by the mode transition

### Requirement: Workflow settlement remains outside realtime processing
Realtime host processing SHALL only perform bounded transport observation, capture enqueue and prepared mixing. Mode changes and Stop handling SHALL NOT introduce file access, asset validation, rendering, allocation/deallocation, mutex acquisition or waiting in that callback. Worker delay SHALL remain explicit without blocking audio.

#### Scenario: Slow storage on Stop
- **WHEN** source validation is delayed or storage fails while the host continues processing
- **THEN** dry audio continues under the mode contract, mode status remains settling/failed as appropriate and realtime processing performs no blocking storage work

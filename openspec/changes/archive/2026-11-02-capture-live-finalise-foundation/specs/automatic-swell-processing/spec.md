# Spec Delta

## MODIFIED Requirements

### Requirement: New instances process captured Song audio automatically
New instances SHALL start in Capture with automatic processing and Auto markers enabled and Recapture off. Capture SHALL admit dry input only during realtime playback with usable absolute position and compatible timing. Only committed source SHALL own accepted targets/renders. After successful useful Stop validation, Live SHALL be selected independently of pending wet work.

#### Scenario: Fresh insert and playback
- **WHEN** a new instance captures musical input during supported DAW playback and the user stops
- **THEN** useful committed coverage causes automatic Live entry and targets/renders prepare without Capture, Suggest or Generate actions
- **AND** Capture playback itself emits no scheduled wet and background completion does not start internal audition

#### Scenario: Sparse capture and unsupported position
- **WHEN** playback visits bars 86-90 then 1-4, or usable absolute position becomes unavailable
- **THEN** admitted spans retain absolute coordinates and unmapped gaps, invalid timing is explained and no fabricated coordinates or partial invalid pass become committed

### Requirement: Processing states and restoration remain truthful
The system SHALL expose Waiting for source, Queued, Rendering, Ready, Stale/updating, Missing IR and Failed as applicable. It SHALL persist provenance, suppression, independent IR references and automation preferences. Restore SHALL validate assets/fresh timing and obey saved or migrated workflow mode. Captured content SHALL distinguish last committed source from provisional capture.

#### Scenario: Reopen edited automatic session
- **WHEN** a session with manual edits, suppressed targets and different per-swell IRs is reopened
- **THEN** identities and edits survive, verified results can be reused, missing assets remain explicit and no wet audition starts
- **AND** Recapture is off and no in-flight capture resumes

#### Scenario: Upstream edits while stopped
- **WHEN** source audio is changed upstream without explicit Capture/Recapture publication
- **THEN** the plugin does not claim to detect or adopt that change and retained source dependencies remain unchanged

#### Scenario: Disable automation
- **WHEN** automatic processing or marking is explicitly disabled and the project is saved/reopened
- **THEN** the preference is retained without deleting source or manual swells, and source admission remains controlled by workflow mode rather than enabling itself through automation

## ADDED Requirements

### Requirement: Automatic Swells does not grant source replacement
Automatic Swells SHALL govern automatic creation independently of Capture/Recapture permission. Off SHALL still permit dry capture and subsequent manual Live swells. Future defaults and manual/edited/frozen definitions SHALL survive toggling. Automation SHALL NOT capture source in Live/Finalise or publish accepted results from rejected provisional source.

#### Scenario: Dry-only learning
- **WHEN** useful dry is captured with Automatic Swells off and Stop validation succeeds
- **THEN** Live becomes usable for manual swell creation without requiring any automatic target or wet render

#### Scenario: Toggle without recapturing
- **WHEN** automatic marking is enabled or sensitivity changes in Live
- **THEN** permitted analysis uses committed source and cannot record upstream changes or overwrite mapped material

### Requirement: Source acceptance invalidates only real changed dependencies
A successful source transaction SHALL notify dependent work using its accepted revision and affected intervals. Identical accepted PCM SHALL reuse valid targets/renders/calibration; real changes SHALL invalidate only affected unfrozen dependencies. Rejected, duplicate or superseded transaction results SHALL NOT create targets or replace accepted renders.

#### Scenario: Atomic recapture with an unaffected neighbour
- **WHEN** one pass replaces source for one swell, preserves another identically and commits successfully
- **THEN** only genuinely changed dependencies refresh, protected edits remain and no provisional revision leaks into either swell

#### Scenario: Rendering continues after Live entry
- **WHEN** source validation completes before all corresponding wet jobs
- **THEN** Live shows truthful queued/rendering/ready states and admits only verified eligible wet while preparation continues

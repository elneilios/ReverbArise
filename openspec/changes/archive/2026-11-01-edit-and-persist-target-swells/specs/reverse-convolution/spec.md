# Spec Delta

## MODIFIED Requirements

### Requirement: Source selection uses absolute Playlist bars
The plugin SHALL provide separate source start/end bar controls, with an exclusive end, distinct from diagnostic tone controls, and sample-accurate source editing for each selected swell. Bar entries SHALL resolve to absolute samples using fixed host tempo/meter; precise edits SHALL not be rounded to whole bars. Invalid ranges, unavailable timing, or incompatible capture/host sample rates SHALL prevent generation with an explanation.

#### Scenario: Select after seeking
- **GIVEN** the host is stopped at a later Song location with valid timing
- **WHEN** the user selects an earlier start/end bar interval
- **THEN** generation uses those absolute Playlist bars rather than offsets from the current playhead or capture-chunk start

#### Scenario: Unavailable timing
- **WHEN** the user requests generation without sufficient host timing or with an incompatible capture sample rate
- **THEN** the plugin reports that the bar interval cannot be resolved and preserves existing capture assets

#### Scenario: Edit a sub-bar source
- **WHEN** a selected swell's source handle is moved between grid lines with snap off
- **THEN** its exact sample interval is rendered without rounding it to the bar-entry controls

### Requirement: Audition is a manual sequential wet preview
Manual Audition and Stop SHALL use the selected shaped swell, defaulting to overlapping wet with optional isolation and Wet Only or contextual Dry + Wet per audition-monitoring. Playback SHALL traverse the complete prepared interval once, independently of Song position. Wet Only SHALL clip neighbours to the positioned anchor window. Disclosed safety ramps SHALL apply; live dry SHALL pass unchanged. Audition SHALL never start automatically or change host transport.

#### Scenario: Audition while transport is stopped
- **GIVEN** a verified render and a host that continues processing the effect
- **WHEN** the user presses Audition with transport stopped
- **THEN** the selected prepared result plays without requiring host seek or Play; Wet Only starts at its audible start and Dry + Wet includes its aligned captured listening context and configured tail

#### Scenario: Host seeks during audition
- **WHEN** the host seeks or loops during manual audition
- **THEN** the preview continues sequentially without retriggering or joining at a Song-relative offset

#### Scenario: Stop and replay
- **WHEN** the user presses Stop and subsequently Audition
- **THEN** the first audition ends and the next begins at the prepared interval's first sample, respecting audible edits and mode without muting live dry input

#### Scenario: Audition is not captured
- **GIVEN** mapping is enabled with zero dry input
- **WHEN** the plugin adds wet or captured-dry preview output
- **THEN** capture contains only zero dry input and never generated or replayed preview audio

#### Scenario: Cached wet with unavailable source
- **WHEN** a previously committed wet asset and its saved identity verify but current dry source is unavailable
- **THEN** the UI retains an explicit source-unavailable state and permits only labeled last-committed Wet Only audition, without claiming the render is current or permitting Dry + Wet

### Requirement: Audition has independent protected level control
The plugin SHALL attenuate the safety-ramped aggregate wet branch above -6 dBFS without quiet boost or raw-sample changes. It SHALL expose raw, shaped, attenuation and predicted monitoring peaks, plus saved -60 to 0 dB trim defaulting to 0 dB when absent. Changes SHALL ramp over 5 ms without callback allocation or blocking. Restore SHALL preserve validated saved trim. Dry + Wet SHALL additionally follow audition-monitoring protection.

#### Scenario: Loud imported IR
- **WHEN** the safety-ramped wet branch peak is +7.2 dBFS and new-instance trim is 0 dB
- **THEN** wet branch attenuation is -13.2 dB and Wet Only peaks at -6 dBFS, while stored render samples and capture remain unchanged

#### Scenario: Quiet render and live adjustment
- **WHEN** the safety-ramped aggregate wet branch peaks below -6 dBFS and the user changes audition level while playing
- **THEN** no automatic boost occurs, the level change ramps smoothly without regeneration, and live dry input is unchanged

#### Scenario: Combined output level
- **WHEN** live dry audio or diagnostic tone accompanies audition
- **THEN** the displayed monitoring peak describes only the prepared audition contribution, and its ceiling does not bound combined plugin output

#### Scenario: Restore audition preference
- **WHEN** a project saved at -24 dB audition trim is reopened
- **THEN** that trim is restored, visibly indicated and resettable, and no audition starts automatically

#### Scenario: Legacy state without trim
- **WHEN** supported legacy state has no saved audition trim
- **THEN** it uses 0 dB; an explicitly stored old -12 dB value remains -12 dB

### Requirement: Preview validity follows its inputs
The system SHALL identify each result by source content dependencies, sample interval, prepared IR, and render rate. Selection/IR changes, overlapping recapture that changes source content, capture Clear, or host sample-rate/tempo/meter changes SHALL invalidate affected work and stop audition. Unrelated or sample-identical recapture SHALL preserve validity. Restored results SHALL be validated against saved dependencies before becoming current.

#### Scenario: Overlapping recapture invalidates a preview
- **GIVEN** a ready result depends on a captured source interval
- **WHEN** committed recapture modifies that interval
- **THEN** the preview is marked stale and must be regenerated before audition

#### Scenario: Unrelated recapture preserves a preview
- **WHEN** committed capture changes only outside the result's source interval
- **THEN** an otherwise unchanged result remains current

#### Scenario: Host rate or musical mapping changes
- **WHEN** the host sample rate, tempo, or meter changes after generation
- **THEN** audition stops and the old result is not reinterpreted at the new rate or silently remapped to new bars

#### Scenario: Unchanged recapture
- **WHEN** recapture changes storage revisions but the selected source samples are identical
- **THEN** a result with otherwise unchanged dependencies stays current

## REMOVED Requirements

### Requirement: M2 preview is session-local
**Reason**: M3 requires basic swell/settings persistence and replaces the temporary M2 preview model with saved editable swell sessions.
**Migration**: Legacy v1/v2 states restore capture/tone and an empty swell collection; M3 states restore definitions and validated assets under the `swell-session` capability without automatic audition.

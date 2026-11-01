# Spec Delta

## MODIFIED Requirements

### Requirement: Audition is a manual sequential wet preview
Manual Audition SHALL play the selected swell's complete prepared interval sequentially, with overlapping wet or Isolate selected and Wet Only or contextual Dry + Wet per audition-monitoring. Wet Only SHALL clip neighbours to the positioned anchor window. Disclosed safety ramps SHALL apply; live dry SHALL pass unchanged. Audition SHALL never start automatically or change host transport. Enabled Song playback SHALL take priority per transport-swell-playback.

#### Scenario: Audition while transport is stopped
- **GIVEN** a verified render and a host that continues processing the effect
- **WHEN** the user presses Audition with transport stopped
- **THEN** the selected prepared result plays without requiring host seek or Play; Wet Only starts at its audible start and Dry + Wet includes its aligned captured listening context and configured tail

#### Scenario: Host seeks during audition
- **WHEN** the host seeks or loops during manual audition with live playback disabled
- **THEN** the preview continues sequentially without retriggering or joining at a Song-relative offset

#### Scenario: Stop and replay
- **WHEN** the user presses Stop and subsequently Audition while Song playback does not have priority
- **THEN** the first audition ends and the next begins at the prepared interval's first sample, respecting audible edits and mode without muting live dry input

#### Scenario: Audition is not captured
- **GIVEN** mapping is enabled with zero dry input
- **WHEN** the plugin adds wet or captured-dry preview output
- **THEN** capture contains only zero dry input and never generated or replayed preview audio

#### Scenario: Cached wet with unavailable source
- **WHEN** a previously committed wet asset and its saved identity verify but current dry source is unavailable and it was not explicitly frozen while ready
- **THEN** the UI retains an explicit source-unavailable state and permits only labeled last-committed Wet Only audition, without claiming the render is current or permitting Dry + Wet

#### Scenario: Song playback takes priority
- **WHEN** live playback is enabled and Song transport starts during manual audition
- **THEN** manual output transitions out within 5 ms, subsequent Audition requests are inhibited while Song is playing, and the plugin mixes Song-positioned wet without sustained duplication

#### Scenario: Explicitly frozen wet with unavailable source
- **WHEN** a swell was explicitly frozen while ready and its managed wet verifies at the original rate/tempo/meter
- **THEN** it remains eligible for live playback and Wet Only audition without current source or IR
- **AND** Dry + Wet audition still requires genuine captured dry context

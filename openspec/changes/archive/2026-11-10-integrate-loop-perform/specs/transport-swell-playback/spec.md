# Spec Delta

## MODIFIED Requirements

### Requirement: Loop Cue requires independent live phase
Loop live playback SHALL require a verified recurring reference with an observed phrase origin and matching accepted rate, tempo, meter and beat length. Capture timing SHALL supply only the source grid and exact recorded extent. Standby SHALL keep checking the reference, pass dry and report the unmet prerequisite for Cue.

#### Scenario: Accepted capture lacks matching recurrence
- **WHEN** a Clock-only take has a verified local grid and the live phrase reference is absent or repeats every four bars for a three-bar take
- **THEN** the accepted loop and its grid remain valid and Cue stays unavailable with the corresponding reason
- **AND** a matching reference can admit Cue automatically after two complete matching intervals and coherent prepared wet

#### Scenario: Verify Phrase while still in Learn
- **WHEN** an accepted Loop receives valid live Phrase pulses in Learn
- **THEN** those observations SHALL count toward verification for the same Loop in Perform, with no wet playback or Cue in Learn
- **AND** reset, timing interruption, incompatible cadence, or a replacement source SHALL require fresh observations
- **AND** entering Perform SHALL still require a fresh Cue before wet playback

#### Scenario: Valid fractional reference
- **WHEN** marker intervals alternate within the verified adjacent integer lengths
- **THEN** live phase stays verified within the epoch drift bound and each observed marker fixes its actual downbeat
- **AND** the accepted audio extent and render sample rate are preserved

#### Scenario: Dry audio is deliberately out of phase
- **WHEN** prepared wet is cued against a verified compatible live phrase reference with dry audio offset from that reference
- **THEN** wet is scheduled against the observed phrase markers without a manual phase confirmation
- **AND** Dry + Wet preserves the incoming dry samples and does not inspect their content to admit phase

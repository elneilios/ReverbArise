# Spec Delta

## MODIFIED Requirements

### Requirement: Monitoring protection and trim remain transparent
Wet Only SHALL apply accepted starting gains, Level and Live Wet to the positioned wet sum. Dry + Wet SHALL add captured dry at unity. Both routes SHALL preserve finite above-unity float samples and disclose measured overload without changing gain.

#### Scenario: Hot correlated blend
- **WHEN** the audition sum of captured dry and wet exceeds full scale
- **THEN** the original float sum is played and overload is identified as a mixed output peak

#### Scenario: Existing low trim
- **WHEN** older pre-release state contains a legacy audition trim
- **THEN** the first audition after reopening uses Live Wet and unity captured dry in Dry + Wet
- **AND** the stored legacy trim and blend do not affect playback

#### Scenario: Level rises below protection onset
- **WHEN** a user raises a swell by 6 dB in Wet Only
- **THEN** its wet contribution increases by the requested ratio even above full scale, with any overload reported

### Requirement: Group monitoring preserves dry and relative wet levels
Overlapping wet SHALL retain each member's saved starting gain, Level, envelope and positioned offset. Captured dry SHALL be mixed once at unity in Dry + Wet. Wet Only SHALL exclude captured dry. Live Wet SHALL set wet gain independently of the shape and listening window.

#### Scenario: Correlated overlapping wet and one dry branch
- **WHEN** two hot wet contributions overlap in Dry + Wet or Wet Only
- **THEN** the wet samples match the aligned sum of both accepted contributions
- **AND** Dry + Wet includes captured dry once and original assets retain their bytes

#### Scenario: Audition and export compare equal membership
- **WHEN** equal membership is auditioned and exported Wet Only at corresponding unity controls
- **THEN** the wet samples agree within tolerance, apart from documented monitoring boundary ramps

#### Scenario: Matching stays fixed across audition scope
- **WHEN** a source-matched swell is auditioned with different isolation, routing or listening tail
- **THEN** its accepted starting gain and Live Wet preference are retained

#### Scenario: Energy calibration does not follow monitoring loudness
- **WHEN** Live Wet changes for energy-calibrated swells
- **THEN** their source-energy measurements, starting gains and Level values remain fixed

### Requirement: Perform audition exposes a Wet only toggle
Perform SHALL provide a latching button naming the selected mode, with Dry + wet when Off and Wet only when On. New instances SHALL default to Off. Existing status roles SHALL disclose readiness, monitoring ramps and overload. Audition routing SHALL remain independent of Export and live routing.

#### Scenario: Toggle live-level listening
- **WHEN** Wet only is toggled with an unchanged valid invocation
- **THEN** accepted pressed state, prepared sound and status agree without autoplay, convolution, asset edits or Export mix changes
- **AND** stale or disabled invocations cannot change a different session or workflow

#### Scenario: Restore old and new preferences
- **WHEN** a session saved under the generated-headroom policy reopens
- **THEN** its accepted Wet only choice restores without starting playback, and malformed metadata is handled transactionally

#### Scenario: Monitoring protection and unavailable source
- **WHEN** the user auditions Dry + Wet with unavailable captured dry
- **THEN** the reason is disclosed and dry is neither padded nor silently replaced with Wet Only

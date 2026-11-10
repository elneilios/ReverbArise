# Spec Delta

## MODIFIED Requirements

### Requirement: Musical admission requires explicit meter and origin
Bars, beats and musical grids SHALL require verified beat timing and explicit meter. Capture SHALL establish an origin shared by the audio and clock. Loop playback SHALL require verified recurrence matching the intended phrase. The routed phrase marker SHALL define the live wet origin automatically. Equal tempo alone SHALL NOT establish phase.

#### Scenario: Beat and bar observations without a declared meter
- **WHEN** periodic inputs arrive with no explicit meter configuration
- **THEN** musical duration and grid admission remain unavailable with a meter explanation

#### Scenario: Capture length differs from the recurring reference
- **WHEN** a one-bar take is verified and the only verified recurrence is four bars
- **THEN** the take remains valid and live Cue reports the missing matching phase reference

#### Scenario: Short sequencer phrase matches the accepted take
- **WHEN** an accepted one-, two- or four-bar take uses the supported Rack timing profile
- **THEN** the plug-in SHALL verify two matching intervals from live Phrase pulses before enabling Cue, including valid intervals observed in Learn for that accepted take
- **AND** an observed cadence that differs from the accepted length SHALL leave Cue unavailable

#### Scenario: Verified take length rounds to an adjacent frame count
- **WHEN** an accepted take matches the reference's verified sample rate, stable tempo, explicit meter, and duration in meter beats
- **AND** its recorded frame count is the floor or ceiling of the nominal period derived from those timing fields
- **THEN** either frame count matches the recurrence without changing the recorded extent
- **AND** an out-of-range frame count or mismatched musical timing leaves Cue unavailable

#### Scenario: Capture uses one clock pulse per beat
- **WHEN** a compatible capture route provides stable sample-timed beat pulses and explicitly declares 4/4
- **THEN** the first recorded beat defines the capture-local downbeat
- **AND** subsequent observed beats establish bar boundaries without separate bar or phrase inputs
- **AND** this local grid alone does not qualify an external recurring phase reference for synced playback

#### Scenario: Clock and Phrase remain connected
- **WHEN** the simplified capture Clock and a live Phrase pulse are present together
- **THEN** Learn derives capture boundaries from Clock, including when Phrase coincides with Clock, Arm or Stop
- **AND** Perform verifies live recurrence independently using Phrase
- **AND** competing legacy beat/bar inputs still invalidate a simplified take

#### Scenario: Producer routes the wet origin separately
- **WHEN** a supported route supplies compatible recurring phrase markers and the dry signal has a different origin
- **THEN** the verified markers define the wet origin without a confirmation step or an audio alignment check
- **AND** capture retains its own independent timing and input requirements

# Spec Delta

## MODIFIED Requirements

### Requirement: Musical admission requires explicit meter and origin
Bars, beats and musical grids SHALL require verified beat timing and explicit meter. Capture SHALL establish an origin shared by the audio and clock. Synced playback SHALL additionally require a verified recurrence matching the intended phrase. Equal tempo alone SHALL NOT establish phase.

#### Scenario: Beat and bar observations without a declared meter
- **WHEN** periodic inputs arrive with no explicit meter configuration
- **THEN** musical duration and grid admission remain unavailable with a meter explanation

#### Scenario: Capture length differs from the recurring reference
- **WHEN** a one-bar take is verified and the only verified recurrence is four bars
- **THEN** the take remains valid and live Cue reports the missing matching phase reference

#### Scenario: Verified take length rounds to an adjacent frame count
- **WHEN** an accepted take matches the reference's verified sample rate, stable tempo, explicit meter, duration in meter beats and phrase origin
- **AND** its recorded frame count is the floor or ceiling of the nominal period derived from those timing fields
- **THEN** either frame count matches the recurrence without changing the recorded extent
- **AND** an out-of-range frame count or mismatched musical timing leaves Cue unavailable

#### Scenario: Capture uses one clock pulse per beat
- **WHEN** a compatible capture route provides stable sample-timed beat pulses and explicitly declares 4/4
- **THEN** the first recorded beat defines the capture-local downbeat
- **AND** subsequent observed beats establish bar boundaries without separate bar or phrase inputs
- **AND** this local grid alone does not qualify an external recurring phase reference for synced playback

### Requirement: Verified inputs retain their sample offsets and roles
The host input contract SHALL preserve valid callback-relative sample offsets and distinguish Capture Start/Arm, Capture Stop, capture gate, phrase, bar, beat, reset, Cue and Perform Stop. Invalid offsets SHALL NOT be clamped into valid commands. End Signal SHALL require an attributable request and the verified musical boundary specified for its capture route.

#### Scenario: Valid events at the same sample
- **WHEN** a qualified end request and a bar marker share a sample on a route whose documented policy allows that coincident marker
- **THEN** that sample is the exclusive capture end
- **AND** a phrase or bar marker by itself is neither a capture end request nor Perform Stop

#### Scenario: Host starts with a high capture gate
- **WHEN** capture is armed or restored with the gate high or of unknown state on a gate-based route
- **THEN** a fresh verified low-to-high transition is required to establish a start

#### Scenario: Gate release cannot be attributed safely
- **WHEN** a disconnect is detected or its release cannot be distinguished from a deliberate request on the admitted route
- **THEN** ambiguous evidence cannot establish a successful capture end
- **AND** a route that cannot provide the necessary distinction requires a separately verified end request or reports that method unavailable

#### Scenario: Positive commands are independent of reset and Perform
- **WHEN** the simplified capture route receives verified Start/Arm or Stop triggers
- **THEN** it processes each trigger at its observed offset as capture intent
- **AND** gate release, clock reset, and Perform commands do not supply that intent

## ADDED Requirements

### Requirement: Capture clock is verified from observed cadence
The simplified capture route SHALL verify stable tempo from consecutive beat offsets before accepting a take. Verification SHALL allow measured integer-sample rounding and SHALL NOT derive tempo from callback size or replace an unobserved boundary with a predicted one.

#### Scenario: Capture arms before the clock starts
- **WHEN** a valid arm precedes the first clock pulse
- **THEN** that pulse can start a candidate at its actual sample
- **AND** settlement waits for enough observed intervals to verify the cadence through the required closing beat

#### Scenario: Rounded intervals cannot share a stable period
- **WHEN** observed intervals can share a period only by assigning exactly one sample of rounding error to a timestamp difference
- **THEN** the candidate cadence is rejected

#### Scenario: Clock is incompatible or lost
- **WHEN** a required beat misses its deadline, drifts beyond the measured cadence, or arrives after a timing invalidation
- **THEN** the incomplete candidate cannot settle as an accepted loop
- **AND** capture requires a fresh arm and valid timing

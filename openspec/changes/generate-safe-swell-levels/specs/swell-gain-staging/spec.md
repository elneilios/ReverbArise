# Spec Delta

## MODIFIED Requirements

### Requirement: Level provides useful adjustment around the baseline
New Level edits SHALL use the existing finite -12 to +12 dB range, 0 dB neutral, and 1/0.1 dB coarse and fine steps. Level SHALL apply the exact requested gain relative to the accepted starting gain and reuse raw convolution. Editing one swell SHALL leave the gains of all other swells unchanged.

#### Scenario: Turn down and boost
- **WHEN** a prepared swell moves from 0 dB to -6 dB and +6 dB
- **THEN** its contribution follows the corresponding amplitude ratios within declared floating-point tolerance, without changing another swell or triggering convolution

#### Scenario: Reach the protection limit
- **WHEN** Swell 1 goes from 0 dB to +12 dB and its peak exceeds full scale
- **THEN** the wet float signal retains the full increase and reports overload
- **AND** a nonoverlapping Swell 2 emits the same samples as before the edit

#### Scenario: Use the focused sweep
- **WHEN** Level is edited using drag, wheel, keys, numeric entry or accessibility
- **THEN** all inputs use the same limits and accepted values, with the existing knob appearance and interaction
- **AND** invalid typed or accessibility input leaves the accepted value unchanged

#### Scenario: Adjust both directions through neutral
- **WHEN** the user drags the native knob below and above 0 dB and uses fine adjustment
- **THEN** gain moves in the corresponding direction and neutral points to 12 o'clock at supported sizes and scales

### Requirement: Protection uses the actual positioned aggregate
At unity wet and 0 dB Level the generated wet composition SHALL peak at or below -1 dBFS. Before accepting generated wet, the system SHALL reserve any needed headroom from verified, positioned wet samples. The resulting reduction SHALL be saved with the generated starting gains and SHALL never boost quiet material.

#### Scenario: Independent peaks do not coincide
- **WHEN** fixed-grid wet intervals overlap but their actual sum fits below the ceiling
- **THEN** the generated mix is not reduced solely because the sum of separate maxima exceeds the ceiling

#### Scenario: Correlated hot overlap
- **WHEN** default generated wet from a batch would exceed the ceiling, including Loop wrap or partial passes
- **THEN** the batch receives a common reduction that brings the measured peak within the ceiling while preserving its starting balance
- **AND** paging, seeking and block partitioning do not change the saved gain

#### Scenario: Add to an accepted mix
- **WHEN** generation adds or replaces a swell while retaining other accepted swells
- **THEN** the headroom check applies to the new material and leaves retained starting gains and Level edits intact
- **AND** if the new material cannot be admitted within the default ceiling, the last coherent accepted wet revision remains available and the failure is reported

#### Scenario: Verify before making wet audible
- **WHEN** the render or headroom measurement is pending, fails or is superseded
- **THEN** the unchecked generated wet does not become audible
- **AND** playback and export may use the last compatible accepted revision under their existing readiness rules

### Requirement: Wet gain semantics agree across playback paths
Manual audition, live Song and Loop playback, and export SHALL use the same accepted starting gains and Level adjustments. Explicit mix controls, listening windows and boundary ramps SHALL remain separate. Wet paths SHALL preserve finite above-unity samples and report overload.

#### Scenario: Compare the same wet signal
- **WHEN** the same accepted swells and interval play Wet Only with unity monitoring controls
- **THEN** interior wet samples match across the applicable paths within numerical tolerance, excluding documented boundary ramps
- **AND** changing isolation selects membership without recalibrating members

## ADDED Requirements

### Requirement: Saved projects restore generated starting gains
Projects saved under the generated-headroom policy SHALL restore the accepted starting gain, calibration revision and Level together without starting playback. Pre-release snapshots have no audible compatibility or migration requirement.

#### Scenario: Restore a generated mix
- **WHEN** a project with adjusted and frozen swells is saved and reopened
- **THEN** each accepted starting gain and Level restore without accumulated attenuation or a new measurement
- **AND** invalid gain data cannot admit unchecked new wet

## REMOVED Requirements

### Requirement: Saved projects preserve their gain policy
**Reason**: The new gain policy replaces the historical policy choices and their audible compatibility guarantees.
**Migration**: Pre-release snapshots require no gain-policy migration.

### Requirement: Older saved Level values remain unchanged until edited
**Reason**: This requirement preserves the sound and input treatment of earlier development snapshots.
**Migration**: No migration is required for pre-release snapshots. New edits retain the existing -12 to +12 dB range.

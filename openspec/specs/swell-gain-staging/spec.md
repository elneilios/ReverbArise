# swell-gain-staging Specification

## Purpose
Give automatic reverse-reverb swells a musically useful starting balance and effective level adjustment, with consistent prepared wet gain, explicit protection and preservation of existing projects.

## Requirements

### Requirement: Source and shaped wet share display amplitude
Source and shaped-wet detail lanes SHALL use the same amplitude-to-relative-height
mapping. A typeable display-only Wave zoom SHALL magnify both equally, default
and reset to 0 dB, and SHALL NOT change audio or calibration. Level changes SHALL
remain visible without inverse display normalisation. Independently scaled raw
grey references SHALL be identified separately from comparable source/wet data.

#### Scenario: Compare source and wet while adjusting Level
- **WHEN** source and shaped-wet samples have equal amplitudes at shared zoom
- **THEN** they have equal relative displayed heights in their lanes
- **AND** changing Level changes wet height while source height stays fixed
- **AND** changing Wave zoom changes both equally without altering audio

### Requirement: Musical defaults require listening calibration
New instances SHALL use a versioned musical gain policy with a documented baseline method for new swells. Local source-energy matching SHALL be compared with the user-preferred fixed +6 dB reference and the peak-matched prototype before the final method, tuning and wet ceiling receive explicit user listening approval. Level at 0 dB SHALL represent the approved baseline. Approved IR assets and raw convolution PCM SHALL remain unchanged.

#### Scenario: Compare default candidates
- **WHEN** the user reviews candidate defaults on transient, chord, sustained and busy musical sources with the six bundled responses
- **THEN** the comparison identifies dry, legacy wet, fixed/matched candidate wet, applied protection and routing at matched dry level
- **AND** the recorded approval identifies the method, exact tuning, ceiling, build and fixtures before the default is finalized

#### Scenario: Relative response characters survive
- **WHEN** quiet and dramatic factory responses or an imported IR are prepared under the musical policy
- **THEN** fixed-gain candidates preserve authored relative levels, while source-energy matching discloses reduced recording-level differences and preserves timbre, decay, stereo character and IR/render assets
- **AND** silence remains silent and near-silence is not automatically raised to a target level

### Requirement: Source matching references the local dry attack
Source-energy calibration SHALL compare strongest short-window RMS in the verified local dry attack and creation-default wet before calibration, Level and protection. One gain SHALL link both channels. Its revision SHALL define attack bounds, RMS window/hop, channel linking, short-interval treatment, offset, floors and gain limits. Sample peaks SHALL remain separate protection measurements; missing coverage SHALL be explicit.

#### Scenario: Match an isolated attack
- **GIVEN** verified dry and default wet RMS amplitudes D and W above the matching floors and a gain request within its bounds
- **WHEN** the energy candidate uses target offset T dB
- **THEN** its baseline is 20*log10(D/W)+T and the resulting strongest wet RMS window has the requested relationship to the dry attack within declared tolerance before aggregate protection, without changing dry or raw wet PCM
- **AND** Level remains at 0 dB relative to that calculated baseline
- **AND** the result is an intensity estimate with no claim of equal perceived loudness; whole-tail averaging and borrowing another event are excluded

#### Scenario: Nearby louder event
- **WHEN** another transient occurs beyond the defined local reference window or at the next confirmed target
- **THEN** that later event does not set the current swell's reference energy

#### Scenario: Arbitrary IR recording levels
- **GIVEN** otherwise identical IRs scaled by a finite factor above floors and within gain limits
- **WHEN** the same source is calibrated with each
- **THEN** compensating baselines produce equivalent wet amplitudes within declared tolerance before protection, while polarity, stereo balance and waveform character remain preserved
- **AND** appending a silent IR tail does not increase calibration merely by diluting a whole-file average

#### Scenario: Quiet and strong versions of an event
- **GIVEN** the same event scaled to quiet and strong levels above floors with the same IR
- **WHEN** each receives source-energy calibration
- **THEN** their calibrated wet levels retain the source amplitude ratio before protection rather than converging on a universal full-scale target

#### Scenario: Short stereo reference and dense wet response
- **WHEN** the dry reference or wet support is shorter than the configured RMS window, or only one channel contains the strongest energy
- **THEN** calibration uses a common available window length without zero-padding missing coverage and applies the same computed gain to both channels
- **AND** sparse-peak and dense-response cases receive numerical and listening validation, including any reduction needed for the actual wet sample peak

#### Scenario: Quiet or unavailable reference
- **WHEN** dry/wet falls below a configured floor, the required gain exceeds its cap, or local source coverage is unavailable
- **THEN** matching does not apply unbounded upward gain; the floor, cap or unavailable result is disclosed without claiming an exact match
- **AND** silence is never divided into an infinite gain or promoted to a full-scale target

### Requirement: Measured baselines remain stable during editing
A measured baseline SHALL persist with its method/revision, effective measurement windows, dry/wet RMS and sample peaks, reference and accepted dependency identities. Level, fade, tension, crop and wet-placement edits SHALL NOT remeasure it. Genuine source, IR, extraction/support, target-reference or policy changes SHALL recalculate it with the corresponding accepted audio. Freeze SHALL preserve the accepted baseline with its render; repeated analysis SHALL not accumulate calibration.

#### Scenario: Shape an already matched swell
- **WHEN** the user changes its Level, fade, tension, crop or placement without changing measurement dependencies
- **THEN** the saved baseline stays unchanged and the edits remain audible without inverse normalization
- **AND** reset with unchanged dependencies returns to the same baseline rather than recalibrating against the edited shape

#### Scenario: Replace an IR and freeze
- **WHEN** an IR replacement is prepared and later the accepted result is frozen, saved and reopened without its source
- **THEN** replacement audio and its recalculated baseline become accepted together, and the frozen result restores the saved baseline without source remeasurement
- **AND** a pending or superseded replacement cannot apply its baseline to the older audible IR

#### Scenario: Adopt matching without reference data
- **WHEN** an old frozen swell has neither measurable reference source nor a saved matching baseline during explicit adoption
- **THEN** it reports the missing calibration prerequisite without silently inventing a baseline or presenting itself as matched

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

#### Scenario: Measure the audible membership
- **WHEN** the generated mix contains disabled, muted or solo-excluded swells
- **THEN** their samples do not affect the starting gains of the swells that will play

#### Scenario: A failed replacement still plays its accepted render
- **WHEN** a new swell is admitted alongside a failed IR replacement whose accepted audio is still compatible
- **THEN** the headroom check includes the fallback audio at its accepted starting gain

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

### Requirement: Reset retains a clear neutral Level
Wet reset SHALL retain the documented timing/fade defaults and reset Level to 0 dB relative to the instance's gain policy. Full response SHALL remain independent of calibration. Neither reset, reanalysis, repeated rendering nor IR replacement SHALL accumulate gain or silently change the policy.

#### Scenario: Reset a musical swell twice
- **WHEN** a gain-edited musical swell receives Reset wet shape twice
- **THEN** both results use exactly the same calibrated baseline, 0 dB Level, 20 ms default overlap and Smooth release subject to genuine support
- **AND** source, IR, name, identity and activity/freeze are preserved

### Requirement: Gain preparation preserves operational guarantees
Gain analysis and preparation SHALL remain within the shared 256 MiB working budget and outside live callbacks. Live mixing SHALL add no allocation, blocking, I/O or latency. Cancelled analyses SHALL not publish. Existing accepted IR handover, freeze, availability, offline snapshot and instance-isolation rules SHALL remain effective.

#### Scenario: Analyze during an IR replacement
- **WHEN** a long replacement is pending while gain preparation, looping and a two-instance export occur
- **THEN** output uses the correct accepted versions and independently snapshotted gain plans, without stale publication, missing neighbours, unbounded memory or callback work

### Requirement: Saved projects restore generated starting gains
Projects saved under the generated-headroom policy SHALL restore the accepted starting gain, calibration revision and Level together without starting playback. Pre-release snapshots have no audible compatibility or migration requirement.

#### Scenario: Restore a generated mix
- **WHEN** a project with adjusted and frozen swells is saved and reopened
- **THEN** each accepted starting gain and Level restore without accumulated attenuation or a new measurement
- **AND** invalid gain data cannot admit unchecked new wet

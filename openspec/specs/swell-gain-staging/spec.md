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
New swell Level edits SHALL use one finite -12 to +12 dB range for drag, wheel, keys, numeric entry and accessibility. The dial SHALL use the standard appearance and drag response, with 0 dB at 12 o'clock. Creation and reset SHALL use 0 dB. Wheel and keys SHALL step by 1/0.1 dB. Below protection onset, edits SHALL produce the corresponding amplitude ratio. Protection SHALL engage only when the prepared aggregate requires it and disclose reduction. Gain edits SHALL reuse raw convolution.

#### Scenario: Turn down and boost
- **GIVEN** a prepared swell with sufficient headroom
- **WHEN** its Level moves from 0 dB to -6 dB and +6 dB
- **THEN** the wet sample amplitudes follow the respective dB ratios within declared floating-point tolerance, without hidden inverse compensation, changed dry, or another convolution

#### Scenario: Reach the protection limit
- **WHEN** a requested boost would exceed the musical wet ceiling
- **THEN** protection keeps the prepared wet within that ceiling and reports requested gain and applied attenuation separately
- **AND** reducing Level restores unprotected adjustment once sufficient headroom exists; the UI does not claim unlimited boost at a fixed ceiling

#### Scenario: Use the focused sweep
- **WHEN** a user adjusts an ordinary in-range Level with the main dial
- **THEN** the dial and input look like the other knobs, 0 dB points to 12 o'clock, and coarse/fine wheel and key steps are 1/0.1 dB
- **AND** dragging, wheel and key movement stop at those same limits; new typed/accessibility values outside them or nonfinite values are rejected without changing the accepted value

#### Scenario: Adjust both directions through neutral
- **GIVEN** an ordinary swell at 0 dB
- **WHEN** the user drags the real native knob down and up, and uses fine adjustment
- **THEN** gain moves in the corresponding direction inside [-12,+12] and neutral points to 12 o'clock at the supported compact size and user scales

### Requirement: Protection uses the actual positioned aggregate
Musical-policy protection SHALL use verified aligned wet samples, including envelopes, placement, user gains and eligible overlaps. Independent member peak sums SHALL NOT cause attenuation when the actual aggregate fits. A fixed gain plan SHALL remain stable across pages, block sizes and seeks for the same accepted schedule. Live dry SHALL not be processed by this protection.

#### Scenario: Independent peaks do not coincide
- **WHEN** overlapping swell intervals have their largest samples at different times and their actual sum remains below the ceiling
- **THEN** their sum is not attenuated solely because the sum of independent maxima exceeds the ceiling

#### Scenario: Correlated hot overlap
- **WHEN** aligned wet samples genuinely exceed the ceiling
- **THEN** one disclosed fixed aggregate gain protects the sum while preserving relative member levels
- **AND** paging, seeking or changing host block partition does not change that gain or introduce pumping

### Requirement: Wet gain semantics agree across playback paths
The musical baseline, selected-swell gain and wet protection calculation SHALL be shared by manual audition, live Song playback and offline export for identical admitted membership and settings. Explicit audition blend/trim, live wet level, listening windows and boundary ramps SHALL remain distinct, documented factors rather than hidden calibration differences.

#### Scenario: Compare the same wet signal
- **WHEN** the same accepted swells and interval are played Wet Only with unity monitoring controls in all three paths
- **THEN** interior wet samples match within declared numerical tolerance, excluding only documented boundary ramps
- **AND** subset isolation or different routing is identified when it legitimately changes the protection context

### Requirement: Saved projects preserve their gain policy
The saved session SHALL identify its gain policy and calibration revision. States predating this change SHALL restore legacy gain and protection with their saved Level, trim and routing intact. New instances SHALL default to musical staging. Adopting musical staging in an existing instance SHALL require an explicit Settings action and SHALL preserve definitions, assets and numerical edits.

#### Scenario: Reopen an old project
- **WHEN** a supported pre-change state reopens with edited, imported and frozen swells
- **THEN** its audible gain behavior and saved control values remain legacy-compatible without automatic adoption or playback

#### Scenario: Adopt and restore musical staging
- **WHEN** the user explicitly adopts musical staging, saves and reopens
- **THEN** the chosen policy/revision and unchanged numerical edits restore deterministically without accumulating baseline gain
- **AND** invalid or unknown policy data is rejected transactionally, not interpreted as permission to boost

#### Scenario: Restore a peak-prototype project
- **WHEN** a supported version-8 peak-matched prototype state reopens after energy matching is added
- **THEN** its revision retains peak-matching semantics and saved output until explicit adoption of the energy revision
- **AND** missing energy-specific metadata is not fabricated or interpreted as a valid energy measurement

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

### Requirement: Older saved Level values remain unchanged until edited
Supported saved Level values below -12 dB SHALL retain their exact value and sound until the user operates Level. The ordinary input field SHALL show the actual saved value. A primary press on the dial SHALL bring an older value to -12 dB. Loading, displaying, focusing, saving, unrelated edits and rejected/cancelled entry SHALL preserve it. New accepted edits SHALL be within [-12,+12]. Reset SHALL set 0 dB.

#### Scenario: Reopen an extended value
- **GIVEN** a Legacy or Musical project with Level -30 dB or -60 dB, including a frozen swell
- **WHEN** it is reopened, inspected, focused and saved without an accepted Level edit
- **THEN** Level, calibration, raw asset identities and sound are preserved without clamping or automatic gain-policy adoption
- **AND** the ordinary input field shows the actual saved value while the dial rests at its lower endpoint

#### Scenario: Replace an older value deliberately
- **GIVEN** a swell at -30 dB
- **WHEN** the user enters a valid new Level of -6 dB, selects -12 dB through absolute accessibility entry, or explicitly resets Level
- **THEN** the chosen values become -6, -12 and 0 dB respectively, without changing other shape fields, calibration or raw assets
- **AND** saving/reopening retains the newly chosen value and ordinary bounded editing applies

#### Scenario: First touch brings an older value into range
- **GIVEN** a saved -30 dB value displayed with the dial at its -12 dB lower endpoint
- **WHEN** the user presses the dial with the primary button, even without dragging, or takes an outward wheel/key step
- **THEN** the value becomes -12 dB
- **AND** inward drag, wheel and key motion starts at the displayed lower limit, so an inward coarse wheel/key step selects -11 dB

#### Scenario: Reject or cancel a new out-of-range value
- **GIVEN** a preserved -30 dB value
- **WHEN** the user attempts a new typed or absolute-accessibility value of -24 dB or cancels an uncommitted edit
- **THEN** the new out-of-range entry is rejected and the accepted saved value/sound remain unchanged
- **AND** hover, text focus, unchanged text and stale selection/session intents preserve the old value

# Spec Delta

## MODIFIED Requirements

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

## ADDED Requirements

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

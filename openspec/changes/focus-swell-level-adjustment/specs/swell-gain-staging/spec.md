# Spec Delta

## MODIFIED Requirements

### Requirement: Level provides useful adjustment around the baseline
Musical-policy swell Level SHALL retain finite, typeable -60 to +12 dB adjustment relative to its calibrated baseline. Its main dial SHALL use a linear-in-dB -12 to +12 dB sweep with 0 dB centered. Below protection onset, gain edits SHALL produce the corresponding amplitude ratio. Protection SHALL visibly explain required reduction. Gain edits SHALL reuse raw convolution.

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
- **THEN** its sweep endpoints are -12 and +12 dB, its midpoint is 0 dB, and coarse/fine steps are 1/0.1 dB
- **AND** typed finite values remain accepted down to -60 dB; out-of-range or nonfinite entry is rejected without changing the accepted value

## ADDED Requirements

### Requirement: Extended Level values remain exact and editable
Supported saved Level values below -12 dB SHALL retain their value and audible behavior through load, display, unrelated edits and save. The UI SHALL display the actual value and explain extended entry while the dial indicator rests at its lower endpoint. Opening or focusing the editor SHALL NOT change gain. Explicit absolute accessibility edits SHALL support the full numeric range.

#### Scenario: Reopen an extended value
- **GIVEN** a Legacy or Musical project with Level -30 dB or -60 dB, including a frozen swell
- **WHEN** it is reopened, inspected and saved without a Level edit
- **THEN** Level, calibration, raw asset identities and sound are preserved without clamping or automatic gain-policy adoption
- **AND** resetting only Level explicitly sets it to 0 dB without changing other shape fields

#### Scenario: Edit an extended value deliberately
- **GIVEN** a swell at -30 dB
- **WHEN** its Level receives a coarse upward step or an absolute accessible edit to -24 dB
- **THEN** the resulting values are -29 dB and -24 dB respectively, with no initial jump to -12 dB
- **AND** cancelling an edit or changing selection before commit leaves the captured target unchanged

# Spec Delta

## MODIFIED Requirements

### Requirement: Level provides useful adjustment around the baseline
New swell Level edits SHALL use one finite -12 to +12 dB range for drag, wheel, keys, numeric entry and accessibility. The linear-in-dB dial SHALL visibly identify both limits and place 0 dB physically at 12 o'clock, with 0 dB creation/reset and 1/0.1 dB coarse/fine steps. Below protection onset, edits SHALL produce the corresponding amplitude ratio. Protection SHALL visibly explain required reduction. Gain edits SHALL reuse raw convolution.

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
- **THEN** the visible low/high labels identify -12 and +12 dB, 0 dB points to 12 o'clock, and coarse/fine steps are 1/0.1 dB
- **AND** dragging, wheel and key movement stop at those same limits; new typed/accessibility values outside them or nonfinite values are rejected without changing the accepted value

#### Scenario: Adjust both directions through neutral
- **GIVEN** an ordinary swell at 0 dB
- **WHEN** the user drags the real native knob down and up, and uses fine adjustment
- **THEN** gain moves in the corresponding direction inside [-12,+12], neutral returns to 12 o'clock, and the endpoint labels remain readable at the supported compact size and user scales

## ADDED Requirements

### Requirement: Older saved Level values remain unchanged until edited
Supported saved Level values below -12 dB SHALL retain their exact value and sound until an accepted Level edit or explicit reset. The UI SHALL identify the actual saved value and the fixed editing limits. Loading, displaying, focusing, saving, unrelated edits and rejected/cancelled entry SHALL NOT clamp it. New accepted edits SHALL be within [-12,+12]; reset SHALL set 0 dB. Compatibility SHALL NOT enable new extended-range editing.

#### Scenario: Reopen an extended value
- **GIVEN** a Legacy or Musical project with Level -30 dB or -60 dB, including a frozen swell
- **WHEN** it is reopened, inspected, focused and saved without an accepted Level edit
- **THEN** Level, calibration, raw asset identities and sound are preserved without clamping or automatic gain-policy adoption
- **AND** the actual saved value remains readable with a saved-value indication while the bounded dial rests at its lower endpoint

#### Scenario: Replace an older value deliberately
- **GIVEN** a swell at -30 dB
- **WHEN** the user enters a valid new Level of -6 dB, selects -12 dB through absolute accessibility entry, or explicitly resets Level
- **THEN** the chosen values become -6, -12 and 0 dB respectively, without changing other shape fields, calibration or raw assets
- **AND** saving/reopening retains the newly chosen value and ordinary bounded editing applies

#### Scenario: Relative input respects the displayed lower limit
- **GIVEN** a saved -30 dB value displayed with the dial at its -12 dB lower endpoint
- **WHEN** the user applies an outward/downward relative input, or instead takes an inward/upward coarse step
- **THEN** the outward input makes no change and the inward coarse step selects -11 dB from the displayed bounded position
- **AND** native drag, wheel and keys obey this same bounded policy and cannot author -31 dB

#### Scenario: Reject or cancel a new out-of-range value
- **GIVEN** a preserved -30 dB value
- **WHEN** the user attempts a new typed or absolute-accessibility value of -24 dB or cancels an uncommitted edit
- **THEN** the new out-of-range entry is rejected and the accepted saved value/sound remain unchanged
- **AND** mouse-down without actual bounded movement, unchanged text and stale selection/session intents do not commit a replacement

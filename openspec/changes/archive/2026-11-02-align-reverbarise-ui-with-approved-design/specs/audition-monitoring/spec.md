# Spec Delta

## MODIFIED Requirements

### Requirement: Audition blend has explicit endpoints
The audition band SHALL expose one continuous Dry-to-Wet slider and exact 0-100% entry. Below 100%, gains SHALL be dry=1-wet and wet=wet; 100% SHALL use genuine Wet Only eligibility/window semantics without requiring dry. Separate mode buttons SHALL be absent. Restore SHALL show 100% for saved Wet Only and the saved blend for mixed mode, without rewriting latent preferences or changing live routing.

#### Scenario: Endpoints and reference blend
- **WHEN** the continuous audition control is set to 0%, 50%, 65% and 100% Wet
- **THEN** its prepared contribution is respectively dry only, the half-gain sum, 35% dry plus 65% wet, and wet only, subject to disclosed monitoring protection and trim

#### Scenario: Mode round trip
- **WHEN** a user sets 35% Wet, moves the control to 100% and then explicitly enters 35%
- **THEN** 100% excludes recorded dry, the previous mixed preference remains recoverable in saved state while Wet Only is active, and the explicit 35% edit restores the mixed contribution

#### Scenario: Restore existing monitoring state
- **WHEN** an old session contains Wet Only plus latent 50% blend, or Dry + Wet at 65%
- **THEN** the slider respectively shows 100% or 65%, opening/closing the editor does not rewrite either session, and playback does not start

#### Scenario: Missing dry at the wet endpoint
- **WHEN** verified committed wet is available but captured dry context is unavailable
- **THEN** 100% remains eligible under the existing labeled wet-recovery rules, while a value requiring missing dry is unavailable with its reason and is not silently padded

## ADDED Requirements

### Requirement: Audition percentage and geometry agree
The slider's thumb centre SHALL use the usable travel after thumb insets, with filled track and exact percentage representing the same value. Pointer and numeric entry SHALL agree at endpoints and intermediate values. This presentation SHALL change audition monitoring only, preserving existing gain protection, ramps and live output preferences.

#### Scenario: Reference percentages
- **WHEN** the control is set to 0%, 50%, 65% and 100% at supported sizes/scales
- **THEN** its thumb centre occupies respectively 0, 0.5, 0.65 and 1 of usable travel, and typed and dragged values match within declared rounding tolerance
- **AND** live routing, live wet level, raw asset identity and convolution count remain unchanged

#### Scenario: Preview and Song playback priority
- **WHEN** audition is requested while enabled live Song playback has priority
- **THEN** existing priority and inhibition rules apply, Play/Stop remain labeled preview actions and neither requests host transport changes

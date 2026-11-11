# Spec Delta

## MODIFIED Requirements

### Requirement: Mode entry prerequisites reflect verified material
Live entry SHALL require useful verified compatible source or an eligible verified frozen wet render at compatible timing. Song Finalise entry SHALL require an enabled, unmuted, solo-eligible verified exportable wet render compatible with current rate, tempo and meter, independently of dry availability or mixdown readiness. Loop Finalise entry SHALL use verified accepted capture-local timing and eligible wet independently of current host timing or live synchronization; its file readiness SHALL follow the Loop export contract. Mapped silence alone SHALL NOT satisfy usefulness. Pending wet renders SHALL NOT prevent source-ready Live entry. Frozen recovery SHALL NOT fabricate dry or permit replacement.

#### Scenario: Manual workflow after dry capture
- **WHEN** useful source has committed with Automatic Swells off and no prepared swell
- **THEN** Live is available for manual editing and Finalise remains unavailable

#### Scenario: Silence or failed verification
- **WHEN** coverage is only silence, unavailable, incompatible or still validating
- **THEN** neither mode becomes available by treating that coverage as useful compatible source; an independently verified eligible wet render retains its explicit recovery eligibility
- **AND** valid stored material is retained without reinterpretation

#### Scenario: Finalise readiness
- **WHEN** useful compatible source and an enabled ready swell exist
- **THEN** Finalise can be selected without requiring an immediate export

#### Scenario: Ready render has incompatible timing
- **WHEN** useful source matches the host but a ready render has a different sample rate, tempo or meter in Song mode, or a Loop render is incompatible with its accepted capture-local timing
- **THEN** that render cannot qualify for Finalise or frozen-wet recovery; eligibility does not substitute for the later export workspace's full asset/output preparation

#### Scenario: Frozen wet recovery without source
- **WHEN** captured dry is missing/cleared but an enabled unmuted solo-eligible frozen render verifies at compatible timing
- **THEN** Live entry and wet editing/playback are available without recreating source
- **AND** unfrozen source-dependent swells remain unavailable, captured-dry audition still requires real dry, and no Recapture permission is granted

#### Scenario: Export with independently recoverable wet
- **WHEN** dry source is missing but an eligible frozen wet render verifies at compatible timing
- **THEN** Finalise entry is available and wet-only preparation requires no invented dry; dry-inclusive mixes use the explicitly warned-silence policy
- **AND** source-dependent unavailable renders do not become exportable merely because the mode changed

#### Scenario: Entry and current file readiness differ
- **WHEN** a verified wet render permits Finalise entry and the user selects 0% Wet or a new mix is still preparing
- **THEN** Finalise remains selected, its current Save/Drag/preview actions reflect actual mixdown readiness and dry-only source alone still cannot unlock initial entry

#### Scenario: An accepted Loop is ready to edit without live sync
- **WHEN** a useful verified Loop is accepted and capture is idle
- **THEN** Perform is available using that phrase's local coordinates even when Song transport, Loop phase or prepared wet is unavailable
- **AND** entering Perform starts in dry Standby without moving host transport
- **AND** Loop export availability follows its separately delivered prerequisites

#### Scenario: Loop Export is independent of Cue readiness
- **WHEN** capture is idle and an accepted Loop has useful verified capture-local material and eligible wet, but the live clock or Phrase reference is unavailable
- **THEN** Export entry remains available under the Loop export prerequisites
- **AND** Save, Drag and preview readiness reflect the current finite composition without enabling Cue or inventing live synchronization

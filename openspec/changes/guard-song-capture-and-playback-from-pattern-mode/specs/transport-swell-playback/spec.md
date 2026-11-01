# Spec Delta

## ADDED Requirements

### Requirement: Wet scheduling is gated by transport eligibility
Song-locked wet playback SHALL be scheduled only when the central policy allows it. When Pattern mode is reliably identified, or Song mode is unconfirmed under an unknown host mode, wet output SHALL be suppressed with a visible reason while dry input passes through unchanged. Last-good renders SHALL be retained until a replacement is prepared, and each instance SHALL be gated independently.

#### Scenario: Pattern mode playback
- **WHEN** Pattern mode is reliably detected during playback
- **THEN** no Song-locked wet is output and dry is unaffected

#### Scenario: Ambiguous host mode
- **WHEN** the host mode is unknown and Song mode is not confirmed
- **THEN** wet is not scheduled, nothing is invalidated, and diagnostics state the reason

#### Scenario: Multiple instances
- **WHEN** several enabled instances receive different transports
- **THEN** each instance's gating, captures and renders are independent

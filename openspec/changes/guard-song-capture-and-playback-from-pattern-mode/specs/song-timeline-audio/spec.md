# Spec Delta

## ADDED Requirements

### Requirement: Capture replacement is gated by transport eligibility
Automatic capture SHALL be admitted by a central policy separate from wet scheduling. Committed Song source SHALL be replaced and dependent renders invalidated only after a verified, uninterrupted commit admitted with replacement permission. When host mode is unknown or Pattern, the system SHALL NOT replace or invalidate existing coverage, and zero or uncaptured input SHALL NOT erase prior data. Host mode SHALL NOT be inferred from bar 1, short loops, seeks or repeated positions.

#### Scenario: Pattern playback after Song capture
- **GIVEN** committed Song captures and rendered swells
- **WHEN** Pattern playback starts and stops, then Song playback resumes
- **THEN** captures, revisions and last-good renders are unchanged and every skip is reported with a reason

#### Scenario: Interrupted capture
- **WHEN** a capture is interrupted by a discontinuity or policy change before verified commit
- **THEN** valid committed source and renders are retained

#### Scenario: Legitimate Song behaviour
- **WHEN** Song mode is confirmed or detected and the host seeks, loops, captures non-contiguous ranges or starts after silence
- **THEN** mapping works as before

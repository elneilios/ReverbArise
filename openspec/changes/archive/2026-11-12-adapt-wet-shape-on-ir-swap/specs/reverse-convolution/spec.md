# Spec Delta

## MODIFIED Requirements

### Requirement: Preview validity follows its inputs

The system SHALL identify each result by source content dependencies, sample interval, prepared IR, and render rate. Selection changes, overlapping recapture that changes source content, capture Clear, or host sample-rate/tempo/meter changes SHALL invalidate affected work and stop audition. Unrelated or sample-identical recapture SHALL preserve validity. Restored results SHALL be validated against saved dependencies before becoming current. Compatible IR replacements SHALL follow the accepted audition handover in swell-editing.

#### Scenario: Overlapping recapture invalidates a preview
- **GIVEN** a ready result depends on a captured source interval
- **WHEN** committed recapture modifies that interval
- **THEN** the preview is marked stale and must be regenerated before audition

#### Scenario: Unrelated recapture preserves a preview
- **WHEN** committed capture changes only outside the result's source interval
- **THEN** an otherwise unchanged result remains current

#### Scenario: Host rate or musical mapping changes
- **WHEN** the host sample rate, tempo, or meter changes after generation
- **THEN** audition stops and the old result is not reinterpreted at the new rate or silently remapped to new bars

#### Scenario: Unchanged recapture
- **WHEN** recapture changes storage revisions but the selected source samples are identical
- **THEN** a result with otherwise unchanged dependencies stays current

#### Scenario: Pending replacement has no accepted preview
- **WHEN** an IR selection is pending and there is no compatible accepted preview
- **THEN** the candidate remains unavailable to Audition until its complete render and monitoring mix are ready

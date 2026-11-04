# Spec Delta

## ADDED Requirements

### Requirement: Live admission diagnostics distinguish failure gates
Developer Diagnostics SHALL distinguish configuration/workflow exclusion, unavailable publication, incompatible timing, invalid callback range and missing prepared coverage. Observations SHALL identify their callback or worker context and expose mapping values, preparation/publication revisions, window bounds and retry outcome without modifying session state or adding unbounded callback work.

#### Scenario: Published mapping is rejected while playing
- **GIVEN** a publication exists and enabled Perform playback rejects incoming host timing
- **WHEN** developer Diagnostics is inspected
- **THEN** it identifies the rejected timing dimension and displays source/published and observed sample, PPQ, rate, tempo, meter, inferred origins and origin difference with enough precision to assess the supported tolerance
- **AND** callback range validity, publication revision and current position are visible without presenting worker observations as the same callback

#### Scenario: Stop clears a play-gated failure
- **WHEN** transport stops after a mapping rejection
- **THEN** stopped readiness is distinguished from successful playing admission and a bounded last-rejection observation remains identifiable as historical evidence
- **AND** no stopped observation claims wet was audible or a window was newly prepared solely because the playing flag cleared

#### Scenario: Disabled output and preparation misses
- **WHEN** workflow, saved live enable, routing, wet level or unavailable coverage explains absent wet
- **THEN** Diagnostics exposes the relevant configuration or preparation reason separately from timing rejection, including current/prepared revision and published coverage when available

#### Scenario: Inspect without disturbing the failing instance
- **WHEN** Diagnostics opens, refreshes or closes during playback or while stopped
- **THEN** project state, source/render identities, assets, transport and output remain unchanged
- **AND** callback observation publication performs no allocation, deallocation, lock, blocking wait, file access or logging

### Requirement: Transient live preparation rejection recovers at current position
Recoverable preparation or publication rejection SHALL remain eligible for bounded worker retry while current dependencies and timing are valid. Recovery SHALL validate the latest session, source/render identity and mapping before admitting complete prepared wet at current Song time. Unsupported timing and invalid assets SHALL remain explicitly unavailable; recovery SHALL NOT reinterpret stored positions or fabricate ready output.

#### Scenario: Stopped preparation followed by moving Song playback
- **GIVEN** an instance with 61 eligible swells has a stopped prepared window
- **WHEN** compatible Song playback starts before a swell and advances through it
- **THEN** available wet follows its absolute position and later windows remain eligible for preparation without an Export roundtrip, reset or recapture

#### Scenario: A transient publication attempt fails
- **WHEN** a preparation/publication attempt is transiently rejected and its blocking condition subsequently clears while playback continues
- **THEN** a later bounded worker attempt can publish a fully revalidated current window and wet joins at the then-current offset with the existing bounded transition
- **AND** elapsed wet is not replayed and retry status remains distinct from permanent timing incompatibility

#### Scenario: Dependencies change during retry
- **WHEN** source, render, session generation or mapping changes before a pending retry publishes
- **THEN** obsolete work is rejected and only complete output validated against the latest compatible dependencies can be admitted

#### Scenario: Genuine incompatibility persists
- **WHEN** rate, tempo, meter or sample-to-PPQ mapping remains unsupported
- **THEN** affected wet stays unavailable with its actual reason and stored positions and accepted assets remain intact
- **AND** retry does not relax mapping guards or report readiness without validation

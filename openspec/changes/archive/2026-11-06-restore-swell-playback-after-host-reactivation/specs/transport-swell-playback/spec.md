# Spec Delta

## ADDED Requirements

### Requirement: Host reactivation rejoins current Song time
After host processing resumes during running transport, eligible prepared wet SHALL rejoin the current absolute Song offset without requiring a loop wrap or restart. Reactivation SHALL preserve accepted source, renders, edits and saved Live preferences. Recovery SHALL retain timing/asset validation, bounded preparation and callback safety.

#### Scenario: Host skips processing while transport advances
- **GIVEN** Live playback is enabled with compatible prepared swells
- **WHEN** the host disables processing, advances transport and re-enables processing before the next loop wrap
- **THEN** resident eligible wet resumes at the current absolute position and subsequent scheduled swells play before that wrap
- **AND** missed audio is not replayed or shifted to compensate for the suspension

#### Scenario: Re-enable inside a swell
- **WHEN** compatible processing resumes inside a resident swell's playback interval
- **THEN** wet joins that interval's current sample offset using the existing transition of at most 5 ms
- **AND** it does not restart the swell or continue its pre-disable cursor

#### Scenario: Repeated cycles preserve output and state
- **WHEN** host disable/re-enable repeats at different valid Song positions
- **THEN** wet remains aligned without duplicate contributions or stale suppression and accepted source, prepared render identities, edits and saved preferences remain unchanged
- **AND** another plugin instance remains independent

#### Scenario: Cold resumed position
- **WHEN** compatible processing resumes outside resident prepared coverage
- **THEN** bounded preparation reports missing coverage and retries for the current position without requiring a loop or restart
- **AND** complete eligible wet joins the then-current offset once ready without replaying elapsed audio

#### Scenario: Reactivation does not bypass admission
- **WHEN** processing resumes with unavailable timing, incompatible rate/tempo/meter, invalid assets, disabled saved Live output or a workflow outside Live
- **THEN** existing exclusion and diagnostic rules remain effective and no stale wet is admitted
- **AND** an ordinary seek, loop, missing callback interval or internal Live toggle alone does not authorize replacement of an established incompatible run mapping

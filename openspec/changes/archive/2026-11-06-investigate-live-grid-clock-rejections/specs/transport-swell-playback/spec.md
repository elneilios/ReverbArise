# Spec Delta

## ADDED Requirements

### Requirement: Live timing rejection retains its causal observations
Diagnostics SHALL retain bounded ephemeral evidence of a live run's initial observation, last admitted callback before its first timing rejection, and that first rejection. Each SHALL identify its run/session/reset, callback range, timing and admission result. Run-reference-to-host origin deltas SHALL be explicit and distinct from source/publication deltas. Missing or incoherent evidence SHALL remain unavailable. Collection SHALL preserve existing timing admission and real-time safety.

#### Scenario: Source offset remains stable during playback
- **GIVEN** valid fixed rate, tempo and meter with a playing origin different from accepted source timing
- **WHEN** callbacks remain compatible with the established run
- **THEN** diagnostics distinguishes the informational source offset from the validated run origin without claiming a live rejection
- **AND** available prepared wet retains its existing absolute-position admission and output

#### Scenario: Recorded run origin changes
- **GIVEN** a run starts at sample 3399344 and PPQ 143.999322222222219 at 48000 Hz, 122 BPM and 4/4
- **WHEN** that same run observes sample 3795967 and PPQ 160.801390972220531
- **THEN** evidence identifies the approximately minus-16-sample run-origin delta, the first rejecting callback and the last preceding admitted observation if one exists
- **AND** a near-zero source/publication-to-host delta cannot be presented as successful run validation or permission to replace the reference

#### Scenario: Later symptoms cannot replace the first cause
- **WHEN** additional rejected callbacks, seeks, loops, window publications or preparation misses follow a run's first timing rejection
- **THEN** the run's first rejection and its preceding admitted observation remain retained separately from the existing latest historical rejection
- **AND** neither evidence collection nor those events silently create a new run baseline

#### Scenario: New run and unavailable preceding evidence
- **WHEN** a confirmed Stop/Play, session replacement or explicit audio reset establishes a new run under the existing lifecycle
- **THEN** evidence is associated with the new identity and prior observations cannot be labelled as its causal sequence
- **AND** if no callback was admitted before its rejection, or a coherent snapshot cannot be read, the preceding evidence is explicitly unavailable

#### Scenario: Inspect evidence without changing the project
- **WHEN** diagnostics is opened, refreshed or closed while playback continues, or the project is saved and reopened
- **THEN** observation does not change admission, routing, wet samples, source/render identities, saved definitions or host transport, and diagnostic history is not persisted in the project
- **AND** callback evidence uses bounded work without allocation, deallocation, locks, blocking, file access, text formatting or logging

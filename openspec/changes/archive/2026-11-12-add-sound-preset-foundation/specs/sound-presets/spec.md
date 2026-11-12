# Spec Delta

## Purpose

Define durable Sound assignments for swell creation and editing, with versioned preset settings and a verified impulse response for playback and project restoration.

## ADDED Requirements

### Requirement: Sound assignments identify the definition and exact response

An accepted Sound assignment SHALL identify a stable preset, its definition version, supported typed values and the exact managed response reference. The initial catalogue SHALL reuse the six shipped bundled responses and managed Custom IR. Catalogue order and labels SHALL have no authority over saved assignments.

#### Scenario: Bundled or custom choice is accepted
- **WHEN** a shipped response or managed import is accepted
- **THEN** the accepted assignment uses the stable bundled definition or Custom IR definition, respectively, with the exact immutable response
- **AND** names, parameter defaults and format validation remain independent of catalogue position

#### Scenario: Previously assigned bytes differ from the installed catalogue
- **WHEN** a stored response is unavailable or differs from a current catalogue revision
- **THEN** restoration retains the saved assignment and reports unavailable dependencies through existing recovery behavior
- **AND** no currently installed response is silently substituted

### Requirement: Accepted Learn defaults affect newly created swells

The instance SHALL retain a versioned future Sound assignment and a Level default of 0 dB for new or migrated projects with no saved Learn Level. New automatic and manual swells SHALL copy the accepted settings at their existing guarded creation boundary. Editing defaults SHALL leave existing definitions and renders intact.

#### Scenario: Create swells after changing a default
- **WHEN** an eligible default change is accepted and a new automatic or manual swell is created
- **THEN** the new swell uses the accepted preset, response and Level from that creation request
- **AND** existing swells keep their assigned settings and gain

#### Scenario: A queued creation request loses authority
- **WHEN** the accepted defaults, session or guarded capture context change before publication
- **THEN** stale work cannot publish a swell assembled from mixed defaults or transfer authority to another session

#### Scenario: Generate matches an existing eligible automatic swell
- **WHEN** a successful Generate applies the current default response to an eligible automatic swell under the existing replacement policy
- **THEN** the matched swell retains its ID and existing Level
- **AND** the new Learn Level default is applied only to newly created swells

### Requirement: Sound replacement publishes one coherent accepted edit

A response-changing edit SHALL reuse bounded background preparation and commit the assignment, effective wet shape and matching render as one accepted edit. Failed, stale and superseded requests SHALL leave the prior accepted state intact. The realtime callback SHALL perform no new preparation, I/O or waits.

#### Scenario: A shorter response needs fitted bounds during active playback
- **WHEN** a valid shorter response is requested on an eligible verified swell
- **THEN** the existing contribution remains eligible until the verified replacement and fitted shape publish together
- **AND** the target, gain and unrelated swells stay intact, and timing intent remains available for a longer response

#### Scenario: A replacement fails, loses authority or targets a frozen swell
- **WHEN** preparation fails, a newer request supersedes it, its session or target authority changes, or Freeze prevents acceptance
- **THEN** the stale or invalid edit cannot publish or change the saved sound, fitted shape or render
- **AND** a failure of the current request remains actionable, while a discarded request cannot overwrite newer status

#### Scenario: Export or host bounce starts with a replacement pending
- **WHEN** a pending request completes after a composition or host offline snapshot is taken
- **THEN** that snapshot uses the coherent accepted assignment and render for its duration
- **AND** the next eligible snapshot can use the new accepted revision

#### Scenario: Level or Envelope changes independently
- **WHEN** an eligible Level or Envelope edit is accepted without a response change
- **THEN** it follows its existing gain or shape update path and does not rebuild the full convolution response

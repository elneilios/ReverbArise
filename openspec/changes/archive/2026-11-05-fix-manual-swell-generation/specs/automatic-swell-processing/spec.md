# Spec Delta

## ADDED Requirements

### Requirement: Explicit manual preparation is independent of automatic policy
Accepted manual creation and render-affecting edits in Perform SHALL request background preparation of the affected unfrozen swell even when automatic processing, markers and legacy auto-refresh are off. Automatic preferences SHALL remain unchanged. Capture commits alone SHALL retain dry-only policy; manual preparation SHALL preserve source, identity, edits, immutable renders and publication guards.

#### Scenario: Manual creation after dry-only capture
- **WHEN** short non-silent captured audio is committed with automatic processing and markers off and the user adds a manual target with complete source and a valid accepted IR
- **THEN** the target progresses through preparation to a verified Ready render and enables user-requested audible Perform Audition without enabling detection or starting playback automatically

#### Scenario: Edit manual dependencies
- **WHEN** the user accepts an arrival, source-interval or IR edit with all automatic policies off
- **THEN** only the affected eligible swell prepares against its latest accepted dependencies, preserves its identity and rejects obsolete completions
- **AND** shape-only, gain and placement changes retain their existing minimum-work behavior

#### Scenario: First snapped timeline insertion
- **WHEN** dry-only capture is accepted, no swell has yet pinned session timing, and the user double-clicks covered dry audio in Perform with Snap enabled
- **THEN** valid current fixed host timing and matching capture rate permit creation at the snapped arrival without a prior '+' insertion
- **AND** missing host position, invalid timing or incompatible established timing still blocks snapped insertion

#### Scenario: Capture alone remains dry-only
- **WHEN** dry-only Learn capture commits or Perform is entered without an explicit manual request
- **THEN** those events do not authorize automatic discovery or disabled-pass convolution, and saved automatic preferences remain off

#### Scenario: Imported WavPack IR
- **WHEN** a valid imported .wv IR is accepted as the manual target's assigned or future-default IR with sufficient source
- **THEN** manual preparation verifies and uses that immutable IR, yielding a usable render without substitution

### Requirement: Manual preparation feedback identifies unavailable dependencies
Manual preparation SHALL expose pending, stale, missing or unsupported inputs, insufficient coverage and rendering failures with an actionable reason. Ready and audition availability SHALL require a verified current render. Derived wet timing SHALL use bounds belonging to the selected swell and valid timing, or show unavailable values rather than fabricated render bounds.

#### Scenario: Missing coverage or invalid IR
- **WHEN** a manual request lacks complete source coverage, usable timing or a supported verified IR
- **THEN** feedback identifies the blocking input or failure and the swell does not falsely become Ready or auditionable

#### Scenario: Late-song unprepared selection
- **WHEN** a short captured clip near bar 40 has a newly added stale target without verified wet bounds
- **THEN** displayed pre-roll and end offset derive from valid selected-swell bounds or remain unavailable, rather than treating song origin as an unprepared wet endpoint

#### Scenario: Selection or session changes during preparation
- **WHEN** selection changes, a target is deleted, or a session is replaced while manual work is pending
- **THEN** obsolete work cannot publish into a different target/session or supply another target's timing or audition readiness

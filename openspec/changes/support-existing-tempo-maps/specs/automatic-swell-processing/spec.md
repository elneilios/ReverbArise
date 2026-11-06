# Spec Delta

## ADDED Requirements

### Requirement: New source windows have an explicit local duration policy
New swells SHALL use a verified mapped quarter-note endpoint when available. Otherwise a valid arrival-local observed tempo SHALL choose a fixed sample duration, disclosed as a duration default rather than exact beat alignment. No tempo observed at a distant playhead SHALL define that default. Missing local tempo SHALL require explicit sample/time bounds. Accepted windows SHALL remain fixed until an authorized edit or generation changes them.

#### Scenario: Default window spans a tempo change
- **WHEN** a new target has real source coverage and valid arrival-local tempo but no exact mapped endpoint through a step or ramp
- **THEN** its default sample duration is derived once from that local tempo, with the disclosed observation provenance and approximation policy
- **AND** existing next-arrival, completed-pass and source-coverage safeguards apply without inventing missing source or claiming an exact one-beat span

#### Scenario: Missing local tempo or protected edits
- **WHEN** no valid arrival-local tempo or verified beat endpoint exists, or an existing window is manually edited/frozen
- **THEN** the missing default is explained and explicit sample/time selection remains possible; existing protected windows and IDs are not rewritten by later BPM observations

### Requirement: Processing authority follows accepted sample dependencies
Detection, Generate, manual preparation and render reuse SHALL validate accepted source, relevant timing evidence and requested settings as coherent snapshots. Instantaneous BPM evolution within an accepted map SHALL not supersede those snapshots or globally stale wet. Demonstrated alignment conflicts and actual dependency changes SHALL retain cancellation, explicit stale feedback and protection of manual/edited/frozen work.

#### Scenario: Render or Generate crosses a ramp
- **WHEN** analysis or rendering is in flight while the playhead crosses accepted tempo automation
- **THEN** unchanged accepted dependencies remain eligible to publish and complete windows can generate sample-aligned swells spanning the transition
- **AND** cancelled, restored or actually superseded work still cannot replace current state

#### Scenario: Accepted evidence is refined without changing audio
- **WHEN** new evidence improves a musical conversion without changing an existing explicit sample window or source/render inputs
- **THEN** conversion-dependent pending work revalidates its own scope, while unrelated candidates, renders and calibration are reused

## MODIFIED Requirements

### Requirement: Marker-control changes invalidate obsolete analysis
Analysis identity SHALL include detector policy, accepted source/coverage revision, coherent accepted timing evidence and current requested detector settings; provisional analysis SHALL additionally identify its originating pass and be ineligible for generation. Publication SHALL reject superseded requests and retain the last displayed candidates marked stale/updating until a matching result arrives. Default IR edits SHALL invalidate generation intent without rerunning detection. Rapid permitted Perform edits SHALL coalesce; failed/incomplete analysis SHALL NOT retire protected targets or erase manual work.

#### Scenario: Rapid sensitivity and enable changes
- **WHEN** sensitivity is dragged repeatedly in Perform and marking is disabled/re-enabled while a scan runs
- **THEN** only a complete permitted latest-request result publishes, work stays bounded and the UI/audio callback performs no PCM analysis

#### Scenario: Next-pass settings change during deferred generation
- **WHEN** an accepted earlier pass remains queued and the user changes allowed future defaults
- **THEN** the pending Generate request is cancelled on any source/settings/default mismatch; immutable capture metadata stays unchanged and only newly requested preview uses the current settings

### Requirement: Shared Generate freezes current accepted intent
Explicit Generate and eligible automatic Stop SHALL use one guarded transaction with the same final displayed candidates, accepted source/settings/timing-evidence authority and current verified Default IR. Capturing, source settlement/validation, stale analysis, invalid source/IR or another Generate SHALL prevent admission. Superseded identities SHALL prevent publication. No provisional source SHALL generate definitions.

#### Scenario: Change only Default IR
- **WHEN** the default changes after candidates settle
- **THEN** marker positions stay valid without convolution, pending generation is cancelled and a subsequent Generate uses the new accepted IR

#### Scenario: Invalid dependencies or preparation failure
- **WHEN** source validation, IR verification, resource limits or replacement preparation fails
- **THEN** no partial replacement occurs, last-good definitions/render references remain, and Learn reports a retryable failure

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
- **THEN** verified local musical conversion, fresh runtime admission and matching capture rate permit creation at the snapped arrival without a prior '+' insertion
- **AND** missing host position, invalid timing or demonstrated incompatible established timing still blocks snapped insertion

#### Scenario: Capture alone remains dry-only
- **WHEN** dry-only Learn capture commits or Perform is entered without an explicit manual request
- **THEN** those events do not authorize automatic discovery or disabled-pass convolution, and saved automatic preferences remain off

#### Scenario: Imported WavPack IR
- **WHEN** a valid imported .wv IR is accepted as the manual target's assigned or future-default IR with sufficient source
- **THEN** manual preparation verifies and uses that immutable IR, yielding a usable render without substitution

### Requirement: Regeneration supersedes pending automatic wet work
An admitted Generate SHALL cooperatively cancel superseded unfrozen automatic render work and take priority over queued renders. Completed render metadata SHALL NOT be mistaken for a newer musical edit or overwritten by an older request snapshot. Protected work and last-good assets SHALL remain intact. A failed/cancelled replacement SHALL leave interrupted accepted work eligible to resume; actual edit/settings/source changes, timing-alignment conflicts or restore SHALL still cancel publication.

#### Scenario: Reduce while high-density renders are pending
- **WHEN** the user requests a nonempty lower-density generation while an earlier automatic render is in flight
- **THEN** the earlier job yields without waiting for the collection to become Ready, the replacement commits, obsolete jobs cannot publish and surviving pending work remains honestly queued/rendering

#### Scenario: Protected render finishes after admission
- **WHEN** a protected render finishes after Generate admission but before replacement preparation starts
- **THEN** its completed render and calibration remain in the replacement, the request proceeds if musical authority is unchanged, and a genuine newer edit still cancels the request

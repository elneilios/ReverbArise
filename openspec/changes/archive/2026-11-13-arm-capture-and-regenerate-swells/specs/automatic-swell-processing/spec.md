# Spec Delta

## MODIFIED Requirements

### Requirement: New instances process captured Song audio automatically
New instances SHALL start in Capture with automatic processing and Auto markers enabled and Recapture off. An armed Song take SHALL admit dry input only during realtime playback with usable absolute position and compatible timing. Only committed source SHALL own accepted targets/renders. After useful Stop validation, Learn SHALL remain selected unless the opted-in shared generation transaction succeeds; only that commit can enter Perform, independently of pending wet work.

#### Scenario: Fresh insert and playback
- **WHEN** a new instance captures musical input during supported DAW playback and the user stops
- **THEN** useful committed coverage produces reviewable candidates; new projects have Auto-generate on Stop OFF and remain Learn until a successful explicit Generate
- **AND** Capture playback itself emits no scheduled wet and background completion does not start internal audition

#### Scenario: Sparse capture and unsupported position
- **WHEN** playback visits bars 86-90 then 1-4, or usable absolute position becomes unavailable
- **THEN** admitted spans retain absolute coordinates and unmapped gaps, invalid timing is explained and no fabricated coordinates or partial invalid pass become committed

### Requirement: Automatic targets have stable protected identities
The system SHALL distinguish suggestions, accepted automatic targets and manually created or edited targets. Successful automatic Stop generation SHALL reconcile one-to-one within a documented sample tolerance, preserve matched IDs and selection, and avoid duplicate targets. Manual edits SHALL not be moved or deleted by detection; deleted automatic targets SHALL remain suppressed for automatic reconciliation until explicitly reset. Explicit Learn generation SHALL ignore suppression.

#### Scenario: Repeated and boundary detection
- **WHEN** unchanged audio is revisited or a transient straddles capture commits
- **THEN** preview creates no definitions; subsequent automatic reconciliation retains one accepted matched identity and explicit generation produces at most one new swell for that target

#### Scenario: Manual intervention
- **WHEN** the user moves, edits or deletes an automatic target and its area is analyzed again
- **THEN** the edit and selected ID remain stable, and a deleted target does not immediately reappear

#### Scenario: Changed unedited target
- **WHEN** committed audio changes and an unedited automatic target is no longer supported by the detector
- **THEN** that target remains committed during preview and is reconciled or retired only within a successful generation transaction producing a changed nonempty viable automatic collection; obsolete jobs are cancelled with other definitions handled by the request's replacement policy

### Requirement: Generation assignment preserves protected work
Automatic Stop generation SHALL preserve stable matched identities, manual/edited/frozen IR choices, deletion suppression and accepted transition/gain edits. Additive capture SHALL NOT replace protected source-context assignments. New default or context metadata alone SHALL NOT change audio fingerprints or reconvolve existing unchanged dependencies; only a successful generation request can apply the current default to its eligible definitions.

#### Scenario: Revisit a manually overridden automatic swell
- **WHEN** a new pass/default or permitted reanalysis revisits a manually edited or frozen target
- **THEN** automatic reconciliation preserves its IR/edits/identity while genuinely new eligible targets use the request's accepted current Default IR with source provenance retained independently

#### Scenario: Context-only update with identical audio
- **WHEN** a context metadata update is accepted while PCM and existing swell dependencies remain identical
- **THEN** existing renders/calibration are reused and only an admitted generation transaction can create or replace automatic definitions

### Requirement: Shared Generate freezes current accepted intent
Explicit Learn generation and eligible automatic Stop SHALL use the same guarded transaction mechanism with their respective replacement policies with the same final displayed candidates, accepted source/settings/timing and current verified Default IR. Armed capture, recording, source settlement/validation, stale analysis, invalid source/IR or another Generate SHALL prevent admission. Superseded identities SHALL prevent publication. No provisional source SHALL generate definitions.

#### Scenario: Change only Default IR
- **WHEN** the default changes after candidates settle
- **THEN** marker positions stay valid without convolution, pending generation is cancelled and a subsequent Generate uses the new accepted IR

#### Scenario: Invalid dependencies or preparation failure
- **WHEN** source validation, IR verification, resource limits or replacement preparation fails
- **THEN** no partial replacement occurs, last-good definitions/render references remain, and Learn reports a retryable failure

### Requirement: Replacement protects work across the complete arrangement
Automatic Stop generation SHALL prepare one replacement over the union of eligible committed mapped runs, preserving manual/edited/frozen, unknown-origin and outside-scope swells. Protected targets and deletion suppression SHALL win collisions within the documented detector tolerance. Eligible automatic matches SHALL retain IDs, names, selection and activity flags, including disabled state. Empty, protected-only or identical results SHALL preserve the whole collection.

#### Scenario: Mixed protected and automatic targets
- **WHEN** candidates collide with protected targets, include duplicate positions and match disabled unfrozen automatic definitions
- **THEN** protected targets retain every saved field/asset, equal candidates deduplicate, matches retain identity/activity flags and only eligible automatic dependencies adopt the request's default

#### Scenario: Sparse source and incomplete windows
- **WHEN** committed coverage contains gaps, silence, unavailable ranges or incomplete extraction tails
- **THEN** generation uses real eligible coverage, never invents source or retires definitions merely because they fall inside a bounding interval across gaps

### Requirement: Replacement success is nonempty and atomic
A successful generation request SHALL atomically publish a nonempty viable automatic result when definitions change or eligible unmatched automatic swells retire, then enter Perform under the same authority. Retained definitions SHALL keep their renders. For explicit rebuilding, equivalence alone SHALL NOT retain prior unfrozen definitions. Empty results and unchanged automatic Stop results SHALL NOT count as success. Validation failure, cancellation and stale completion SHALL NOT partially mutate definitions. Last-good assets SHALL remain recoverable.

#### Scenario: Repeated Generate with and without change
- **WHEN** automatic Stop generation repeats with identical dependencies, then runs with a different valid IR or source dependency
- **THEN** the identical run reports no changes and stays Learn; the changed run preserves matched IDs, replaces eligible definitions atomically and enters Perform without claiming ready wet audio

#### Scenario: Reduce sensitivity after a denser generation
- **WHEN** explicit Learn generation runs with lower sensitivity after a denser generation and produces a nonempty viable set
- **THEN** the rebuilt set reflects the lower-density markers, retains frozen swells and enters Perform

#### Scenario: Empty or protected-only plan
- **WHEN** no viable eligible automatic result remains even though old automatic definitions could be retired
- **THEN** every prior definition remains, Learn stays selected and a nonfatal outcome explains empty or protected-only status

### Requirement: Regeneration supersedes pending automatic wet work
An admitted generation request SHALL cooperatively cancel render work it can supersede under its replacement policy and take priority over queued renders. Completed render metadata SHALL NOT be mistaken for a newer musical edit or overwritten by an older request snapshot. Protected work and last-good assets SHALL remain intact. A failed/cancelled replacement SHALL leave interrupted accepted work eligible to resume; actual edit/settings/source/transport/restore changes SHALL still cancel publication.

#### Scenario: Reduce while high-density renders are pending
- **WHEN** the user requests a nonempty lower-density generation while an earlier automatic render is in flight
- **THEN** the earlier job yields without waiting for the collection to become Ready, the replacement commits, obsolete jobs cannot publish and surviving pending work remains honestly queued/rendering

#### Scenario: Protected render finishes after admission
- **WHEN** a protected render finishes after generation admission but before replacement preparation starts
- **THEN** its completed render and calibration remain in the replacement, the request proceeds if musical authority is unchanged, and a genuine newer edit still cancels the request

## ADDED Requirements

### Requirement: Explicit Learn generation rebuilds the unfrozen set
Explicit Generate Swells or Regenerate Swells SHALL rebuild from the current settled markers, verified captured audio and current sound defaults. A successful rebuild SHALL retain all frozen swells, replace all prior unfrozen definitions and ignore deletion suppression. New swells SHALL have fresh identities. Frozen targets SHALL win collisions within the existing detector tolerance.

#### Scenario: Return from Perform and lower sensitivity
- **WHEN** the musician lowers sensitivity and explicitly regenerates from the same accepted Song or Loop with viable markers
- **THEN** Perform shows the newly generated set plus every frozen swell, and accepted dry audio is unchanged
- **AND** prior unfrozen manual, edited, automatic and legacy definitions are replaced regardless of provenance or their position relative to source coverage

#### Scenario: Delete all generated swells and start again
- **WHEN** deletion suppression covers the displayed eligible markers and the musician explicitly generates from the accepted source
- **THEN** viable swells are created from those markers without a Diagnostics reset
- **AND** the rebuild does not erase the saved suppression record used by automatic reconciliation

#### Scenario: All candidates are blocked by freeze or unavailable source
- **WHEN** no viable candidate remains after frozen-target collision and source-window checks
- **THEN** Learn explains the actual outcome and every accepted definition and its assets remain available

# Spec Delta

## ADDED Requirements

### Requirement: Swells list follows canonical target chronology
The Swells list SHALL display ascending canonical absolute target/arrival position, with persistent swell ID as the deterministic ascending tie-breaker. Names, insertion order, rendered bounds, wet placement and formatted bar/beat text SHALL NOT affect order. Any position label SHALL derive from canonical position using available timing, never from parsing names.

#### Scenario: Capture an earlier section after a later section
- **WHEN** a later captured section already has swells and a new swell is accepted earlier in the song
- **THEN** the earlier swell appears before later targets on the next normal UI refresh without manual reload
- **AND** targets corresponding to bar 2 sort before bar 10 regardless of their names or creation order

#### Scenario: Rename or change cosmetic and wet settings
- **WHEN** a swell receives an arbitrary name or changes status, IR label, envelope or whole-wet placement without moving its target
- **THEN** its ordering relative to unchanged targets stays the same and the user's name remains intact

#### Scenario: Multiple targets share a position
- **WHEN** multiple swells have equal target positions
- **THEN** their persistent IDs determine a stable order across refresh, reanalysis and save/reopen regardless of stored definition order

### Requirement: Chronological rows retain swell ownership
Selection, primary selection, row actions and delayed row commands SHALL follow persistent swell identity across reordering. Presentation refresh SHALL NOT submit selection edits or alter swell settings, source, assets, generation or audio scheduling. Commands from obsolete sessions SHALL be rejected or cancelled under existing ownership guards.

#### Scenario: Move the selected target across other rows
- **WHEN** the selected swell's target moves earlier or later through a supported canvas or inspector edit
- **THEN** its displayed row moves to the chronological position and the same primary and selected identities remain selected
- **AND** the inspector and subsequent row actions address those identities

#### Scenario: Reorder with multiple selection and a pending menu
- **WHEN** several swells are selected and a background update reorders rows while a row menu or rename dialog is open
- **THEN** surviving selected identities and primary are retained, the delayed action addresses its original identity or is safely cancelled, and no action reaches the replacement occupant of a row
- **AND** enabled, mute, solo, freeze, IR, shape, manual/pinned-marker metadata and unrelated swell settings remain attached to their original identities

#### Scenario: Capture reconciliation and restoration
- **WHEN** noncontiguous capture, seeks/loops, recapture, reanalysis or a supported project restore publishes accepted swell definitions
- **THEN** the list reflects current target chronology, surviving selection follows IDs and restored settings/assets retain their identities
- **AND** presentation itself neither moves targets nor changes playback placement or regenerates audio

### Requirement: Chronological updates preserve list scroll context
Routine refresh and background insertion/reordering SHALL preserve the visible list context by surviving swell identity and relative row offset where possible. Explicit primary selection and movement of a visible primary SHALL keep that primary visible with minimal scrolling. Refresh SHALL NOT repeatedly pull the user back to an offscreen selection. Missing anchors SHALL use a bounded nearby fallback.

#### Scenario: Insert above a scrolled viewport
- **WHEN** the user has scrolled through a long list and background capture inserts earlier targets above the viewport
- **THEN** the surviving top visible swell retains its relative vertical offset where list bounds permit, without jumping to the first row or an offscreen primary

#### Scenario: Follow an actively moved or newly selected primary
- **WHEN** the user explicitly selects a primary or moves a currently visible primary across rows
- **THEN** the primary stays visible using the least necessary scroll adjustment while other selected IDs remain selected

#### Scenario: Unchanged refresh and disappearing anchor
- **WHEN** identical snapshots, cosmetic-only changes or deletion of the visible anchor are refreshed
- **THEN** unchanged rows retain their scroll position and a missing anchor falls back to the nearest surviving visible row or a clamped prior viewport position
- **AND** empty lists and returning from Learn remain valid without stale row actions or out-of-range scrolling

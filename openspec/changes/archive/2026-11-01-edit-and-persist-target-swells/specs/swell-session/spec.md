# Spec Delta

## Purpose

Preserve editable swell sessions and their managed IR/render dependencies across same-PC FL Studio save and reopen without embedding long audio buffers in plugin state or losing committed source captures.

## ADDED Requirements

### Requirement: Saved projects retain swell definitions and editing preferences
Project state SHALL retain stable swell identities, arrivals, source intervals, IR references, audible bounds, fades/curves/gain, activity flags, names, selection, fixed timing, audition level, and auto-regeneration preference. It SHALL retain capture/tone state and compact asset references without embedding PCM. Mapping, recapture arms, pending jobs, and active audition SHALL not resume on restore.

#### Scenario: Reopen an edited session
- **WHEN** a project with multiple edited swells and audition level -24 dB is saved and reopened on the same PC/profile
- **THEN** its definitions, edits, assignments, selected swell, captured source, tone settings, and -24 dB audition level are restored without starting audition or capture

#### Scenario: Save while work is in flight
- **WHEN** a project is saved during import, analysis, recapture, or rendering
- **THEN** saved editing choices refer only to committed assets, unfinished work is not marked ready, and reopening can recover valid dependencies without resuming capture or playback

### Requirement: IR and full render audio have managed immutable assets
Successfully imported WAV/WavPack IR samples and complete untrimmed render results SHALL be stored as verified immutable managed assets under the instance's working store. Saved references SHALL include content identity, format/version, rate, channels, length, and render dependencies. Changing or deleting the original external IR SHALL not change an already imported assignment. Edits SHALL remain metadata over the full render.

#### Scenario: External IR is moved after import
- **WHEN** the user saves a swell, moves or removes the originally selected external IR file, and reopens the project
- **THEN** the managed imported IR remains available with the same decoded gain/channel order and the swell does not silently use a different file

#### Scenario: Shape is changed after rendering
- **WHEN** audible bounds or fades change and the project is saved
- **THEN** the full render asset remains unchanged and reopening applies the saved shape metadata

### Requirement: Reopen validates cached renders and rebuilds only when possible
Restoration SHALL verify source/IR/render identities, bounds, engine version, sample rate, and fixed timing before declaring a swell ready. A missing or invalid render with valid compatible source/IR SHALL be rebuilt in the background. Missing source/IR or incompatible timing SHALL mark only affected swells unavailable or stale and explain recovery; no replacement audio SHALL be silently accepted.

#### Scenario: Render cache is missing
- **WHEN** the saved render asset is absent but its source and managed IR are valid at compatible timing
- **THEN** the swell is visibly restoring/rendering and becomes ready only after complete verified reconstruction, without user recapture or automatic audition

#### Scenario: Managed IR or source is missing
- **WHEN** a referenced managed IR or source chunk cannot be verified on reopen
- **THEN** affected swells show the missing dependency, unaffected swells remain accessible, and unavailable audio is not presented as silence or a current preview

#### Scenario: Different host timing
- **WHEN** the host reopens at a different sample rate or known tempo/meter
- **THEN** saved sample positions and original capture rate are retained, affected swells are not auditionable as current, and no implicit stretching or reinterpretation occurs

### Requirement: State migration and malformed input do not destroy valid data
The system SHALL version M3 state and accept legacy v1/v2 capture/tone state as an empty swell collection with absent audition trim defaulting to 0 dB, preserving any explicitly saved supported trim. Unsupported versions, malformed metadata, unsafe asset paths, non-finite parameters, duplicate identities, and invalid bounds SHALL be rejected with a diagnostic while retaining the previous valid session. Parsing SHALL enforce documented resource limits.

#### Scenario: Reopen an M2 project
- **WHEN** valid v2 state is restored
- **THEN** existing capture identities/references and tone settings restore, the new swell collection is empty, and neither audition nor mapping starts

#### Scenario: Invalid newer state
- **WHEN** state contains an unknown version, path traversal, duplicate swell IDs, or non-finite envelope data
- **THEN** restoration reports the problem without publishing a partial session, deleting assets, or replacing previously valid definitions

### Requirement: Saving and editing preserve prior project snapshots
New managed assets SHALL publish completely before being referenced as committed. Saves SHALL use immutable dependency references so older saved projects and instances remain readable after later edits, Clear, or deletion of swell definitions. The plugin SHALL not automatically delete sole asset copies or perform asset I/O in the audio callback. Basic same-PC persistence SHALL not imply that the FL Studio project embeds or packs external assets.

#### Scenario: Clear after saving
- **WHEN** the user saves a session, clears capture or deletes swells, then reopens the earlier saved project without saving those changes
- **THEN** the earlier definitions and their committed assets remain recoverable

#### Scenario: Asset publication fails
- **WHEN** an IR/render write or publication fails
- **THEN** the prior committed asset references remain valid, the affected operation reports failure, and no half-written result is marked ready

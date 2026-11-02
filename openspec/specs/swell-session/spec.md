# swell-session Specification

## Purpose
Preserve editable swell sessions and their managed IR/render dependencies across same-PC FL Studio save and reopen without embedding long audio buffers in plugin state or losing committed source captures.

## Requirements

### Requirement: Saved projects retain swell definitions and editing preferences
Project state SHALL retain stable swell identities, source/arrival, IR/render references, shapes/gain/placement, activity/freeze, names/selection, timing, monitoring/automation preferences and workflow mode without embedding PCM. Restore SHALL disarm Recapture, discard in-flight operation authority and stop voices. Capture SHALL admit only a subsequent valid Play, never resume the saved pass.

#### Scenario: Reopen an edited session
- **WHEN** an edited multi-swell project with -24 dB audition level is saved and reopened on the same PC/profile
- **THEN** source, definitions, edits, assets, selection, tone preference, -24 dB audition level and saved workflow mode are retained without starting audition, host transport or the saved capture pass

#### Scenario: Save while work is in flight
- **WHEN** a project is saved during capture, Stop validation, import, analysis, recapture or rendering
- **THEN** it references committed source/assets only, unfinished work is not marked ready and reopening cannot resume a Recapture arm or publish the obsolete pass

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
Workflow metadata SHALL be versioned and validated transactionally. Supported pre-workflow states SHALL migrate to Capture with Recapture off while preserving source/creative/gain state. Unknown versions, invalid modes, unsafe paths, nonfinite parameters, duplicate identities or invalid bounds SHALL be rejected with diagnostics while retaining the prior valid session and documented resource limits.

#### Scenario: Reopen an M2 project
- **WHEN** supported valid v2 state is restored
- **THEN** capture references and tone settings survive, swells remain empty and workflow is Capture with Recapture off, without starting transport or audition
- **AND** absent supported audition trim defaults to 0 dB while an explicitly saved supported trim remains unchanged

#### Scenario: Invalid newer state
- **WHEN** workflow or creative metadata contains an unknown version, invalid mode, traversal, duplicate IDs or nonfinite values
- **THEN** restore reports the error without publishing a partial session, deleting assets or replacing valid definitions

#### Scenario: Existing musical or legacy-gain project
- **WHEN** a supported project lacking workflow mode contains valid source and prepared/frozen swells, regardless of its saved mapping/live flags
- **THEN** it opens in Capture with Recapture off and all existing source, IR/render identities, gain policy/calibration, edits and independent preferences preserved
- **AND** Live can be selected after its prerequisites validate; migration never silently chooses Live

### Requirement: Saving and editing preserve prior project snapshots
New managed assets SHALL publish completely before being referenced as committed. Saves SHALL use immutable dependency references so older saved projects and instances remain readable after later edits, Clear, or deletion of swell definitions. The plugin SHALL not automatically delete sole asset copies or perform asset I/O in the audio callback. Basic same-PC persistence SHALL not imply that the FL Studio project embeds or packs external assets.

#### Scenario: Clear after saving
- **WHEN** the user saves a session, clears capture or deletes swells, then reopens the earlier saved project without saving those changes
- **THEN** the earlier definitions and their committed assets remain recoverable

#### Scenario: Asset publication fails
- **WHEN** an IR/render write or publication fails
- **THEN** the prior committed asset references remain valid, the affected operation reports failure, and no half-written result is marked ready

### Requirement: Saved mode waits for safe readiness without resuming activity
New-format sessions SHALL restore their saved mode while verifying its dependencies and fresh timing before enabling corresponding actions/output. Missing or incompatible material SHALL remain explicit without deleting state or fabricating readiness. Recapture permission and transient pass/voice state SHALL never restore.

#### Scenario: Restore Live or Finalise with unavailable assets
- **WHEN** saved Live/Finalise state restores with missing source or incompatible timing
- **THEN** its saved mode/preference is retained with unavailable status, affected actions/output wait for validation and unaffected durable assets are preserved

#### Scenario: Restore Capture while host later plays
- **WHEN** saved Capture state restores and subsequent valid DAW Play occurs
- **THEN** a fresh additive-only pass can begin but prior mapped source remains protected and no saved Recapture arm is reused

### Requirement: Cloned instances share immutable assets without sharing future writes
Duplicated instances SHALL preserve readable committed references and creative state while isolating new capture transactions, manifest publication and completion authority. Clearing, recording or restoring one instance SHALL NOT mutate another instance's accepted map or permit stale completion into it.

#### Scenario: Clone while a pass is pending
- **WHEN** an instance is duplicated during an uncommitted capture pass
- **THEN** the duplicate restores only the last committed source and saved mode with Recapture off, and cannot inherit the original pass's publication rights

#### Scenario: Independent later recapture
- **WHEN** one copy explicitly recaptures while another copy retains the old project state
- **THEN** the old copy's source samples, manifest reference and prior saved snapshots remain recoverable and unchanged

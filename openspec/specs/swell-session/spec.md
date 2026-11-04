# swell-session Specification

## Purpose
Preserve editable swell sessions and their managed IR/render dependencies across same-PC FL Studio save and reopen without embedding long audio buffers in plugin state or losing committed source captures.

## Requirements

### Requirement: Saved projects retain swell definitions and editing preferences
Project state SHALL retain stable swell definitions/assets, source/arrival, shapes/gain/placement, activity/freeze, names/selection, timing, workflow and monitoring/automation preferences. Selected future defaults and accepted source/generation-context references SHALL persist without PCM. Restore SHALL disarm Recapture, discard provisional operation/display authority and stop voices; only subsequent valid Play can start a new pass.

#### Scenario: Reopen an edited session
- **WHEN** an edited multi-swell project with -24 dB audition level is saved and reopened on the same PC/profile
- **THEN** source, definitions, edits, assets, selection, tone preference, -24 dB audition level, saved mode and accepted future defaults are retained without starting audition, transport or a saved capture pass

#### Scenario: Save while work is in flight
- **WHEN** a project is saved during capture, Stop validation, import, analysis, Recapture or rendering
- **THEN** it references coherent accepted source/context/assets only, unfinished work is not Ready and reopening cannot resume an arm, provisional overlay or obsolete pass

#### Scenario: Reopen before deferred pass generation finishes
- **WHEN** source from pass A has accepted, a later future Default IR B has been selected, and the project reopens before A's target generation completes
- **THEN** A's new automatic targets resolve A's accepted context while B remains the selected future default

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
Workflow, capture-context and creative metadata SHALL be versioned/validated transactionally in their own domains. Supported older states SHALL retain source, gain, IRs, edits and saved/migrated mode with Recapture off. Unsupported versions, unsafe references, nonfinite controls, ambiguous/invalid contexts, duplicate identities or invalid bounds SHALL be rejected without replacing valid accepted state or deleting assets.

#### Scenario: Reopen an M2 project
- **WHEN** supported valid v2 state is restored
- **THEN** capture references/tone survive, swells remain empty and the internal Capture mode is selected with Recapture off, without starting transport or audition
- **AND** absent supported audition trim defaults to 0 dB while an explicitly saved supported trim remains unchanged

#### Scenario: Invalid newer state
- **WHEN** workflow, creative or pass-context metadata contains an unknown version, invalid enum, traversal, duplicate ID, nonfinite value or invalid reference/bounds
- **THEN** restore reports the error without publishing a partial session, deleting assets or replacing valid definitions

#### Scenario: Existing musical or legacy-gain project
- **WHEN** a supported project lacking workflow mode contains valid source and prepared/frozen swells, regardless of saved mapping/live flags
- **THEN** it opens in internal Capture mode with Recapture off and existing source, IR/render identities, gain/calibration, edits and preferences preserved
- **AND** Perform can be selected after readiness validates; migration does not silently choose it

#### Scenario: Workflow project predates pass-context metadata
- **WHEN** a supported #10-era project has a saved mode/defaults but no historical pass contexts
- **THEN** saved mode, defaults and assigned swells survive, old source remains usable and migration uses a documented legacy baseline only where historical settings are unknown
- **AND** it does not fabricate original pass choices, force recapture or rewrite existing assigned IRs

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
Copied instances SHALL retain readable accepted source/context references and creative/default preferences while isolating later transactions, settings and completion authority. Shared immutable history SHALL NOT let recording, changing defaults, clearing or restoring one instance mutate another's accepted source, generation context or swell assignments.

#### Scenario: Clone while a pass is pending
- **WHEN** an instance is copied during an uncommitted capture pass
- **THEN** the copy restores only the last coherent accepted source/context and saved mode with Recapture off, without the original pass's publication/display rights

#### Scenario: Independent later recapture
- **WHEN** one copy explicitly Recaptures with a different future default while another retains its saved map
- **THEN** only that copy accepts new source/context, and the other retains its prior samples, assignments, references and recoverable snapshots

### Requirement: Managed future defaults survive external-file changes
The selected bundled/custom Default IR SHALL be saved by immutable managed identity and label, alongside hidden generation preferences. External-file movement, catalogue ordering or later future-default changes SHALL NOT reinterpret accepted pass contexts or existing assignments. Invalid/cancelled import SHALL retain the previous accepted default.

#### Scenario: Restore a custom learning default
- **WHEN** a user imports a custom Default IR, records a pass, saves, moves the external file and reopens
- **THEN** the selected default and deferred automatic assignments use their verified managed identities without external-file dependency or substitution

#### Scenario: Hide and restore generation preferences
- **WHEN** Automatic Swells is turned off, state is saved/reopened and it is later enabled while idle
- **THEN** the prior accepted Default IR/sensitivity remain available and are not reset by hiding or restoring controls

#### Scenario: One context dependency is missing
- **WHEN** an accepted pass's generation metadata/IR cannot be verified while its dry source is valid
- **THEN** dry remains preserved, affected generation shows the dependency failure, and other valid/frozen/manual work is not discarded or silently reassigned

### Requirement: Persistent learning changes notify the host to save
Accepted changes to learning preferences/defaults and committed source/context SHALL notify the host that persistent non-parameter state changed, including with the editor closed. Notifications SHALL be coalesced outside realtime processing. Provisional progress, navigation, Recapture arming, selection and rejected/no-op changes SHALL NOT create false persistent edits.

#### Scenario: Change an idle learning default
- **WHEN** a different Default IR, Automatic Swells policy or sensitivity is accepted
- **THEN** the host receives a resave notification and an explicit save/reopen restores the accepted value

#### Scenario: Transient or rejected action
- **WHEN** the playhead/auto-fit/provisional display updates, Recapture is armed or a pass-locked/no-op edit is attempted
- **THEN** no persistent change notification is emitted for that transient or rejected action

#### Scenario: Read-only Learn inspection is ephemeral
- **WHEN** the user hovers, pins, relocates or clears a Learn inspection position, including during recording or while focus moves to a setting
- **THEN** the operation creates no persistent-state notification, capture/commit/generation request or Perform edit
- **AND** project/session replacement or editor reopen restores no pin and clears the pin flag and coordinate together

#### Scenario: Successful pass with the editor closed
- **WHEN** a useful pass accepts source/context and selects Perform without an open editor
- **THEN** host state is marked changed through the safe notification path without callback storage, GUI work or waiting

### Requirement: Retain successfully imported IR choices per instance
Creative metadata SHALL persist a bounded collection of immutable managed IR identities and display labels, independently of current assignments. Duplicate content, unsafe references and excess capacity SHALL be rejected transactionally. Supported legacy metadata SHALL seed known choices from accepted default and swell references without scanning storage or inventing unused import history.

#### Scenario: An imported IR is no longer selected
- **WHEN** a successful import is replaced everywhere and the project is saved and reopened
- **THEN** its managed identity remains selectable without the original file, while existing swell assignments and audio remain unchanged

#### Scenario: Invalid collection cannot replace accepted state
- **WHEN** restored collection metadata has duplicate content, unsafe paths or exceeds its capacity
- **THEN** restoration fails without replacing valid accepted state or deleting assets

### Requirement: Export preferences restore independently of monitoring
Export SHALL persist its own validated percentage and initialization flag. First entry without a saved Export preference SHALL seed from the effective most recent audition percentage, treating Wet Only as 100%. Later Export edits/re-entry SHALL retain its value without rewriting Perform monitoring/live preferences. Supported old sessions SHALL default to uninitialized Export state without modifying source, swells or other settings.

#### Scenario: First use and independent return
- **WHEN** Perform audition is 65% and Export has never been initialized
- **THEN** first Export entry starts at 65%; changing Export to 100%, returning to Perform and re-entering restores 100% Export and unchanged 65% Perform

#### Scenario: Legacy wet-only audition
- **WHEN** a supported old state has Wet Only with a latent 50% mixed preference and no Export settings
- **THEN** first Export entry initializes to 100%, preserving that latent preference and all source/IR/render identities

#### Scenario: Real edits notify the host
- **WHEN** Export preference initialization, a changed percentage or a completed revision reference is accepted
- **THEN** persistent non-parameter state is marked changed through the existing safe notification path
- **AND** refresh, preview cursor, failed/cancelled/stale jobs and no-op requests do not create false saved edits

### Requirement: Completed export references retain immutable history
Session state SHALL retain the accepted composition identity, completed managed WAV reference and bounded revision metadata without embedding PCM. Save during pending preparation SHALL retain coherent prior completed references and the latest desired mix separately. Restore SHALL verify references and timing without autoplay or silent replacement. Earlier file bytes and references SHALL remain recoverable after later edits or saves.

#### Scenario: Reopen a completed export
- **WHEN** a saved session with a 65% completed export reopens on the same profile
- **THEN** percentage, immutable revision and verified file reference survive, no voice starts and no new file is silently written over the prior WAV

#### Scenario: Save while a later mix prepares
- **WHEN** a 100% revision is complete, 35% is requested and the project saves before preparation finishes
- **THEN** reopen distinguishes the desired 35% mix from the older completed file, revalidates/prepares current work safely and never labels the 100% file as the ready 35% result

#### Scenario: Missing file or malformed reference
- **WHEN** a completed file is missing/corrupt or restored metadata has unsafe paths, invalid values or unsupported versions
- **THEN** missing files are reported and any recoverable rebuild publishes a new verified revision; malformed state is rejected transactionally without replacing valid state or deleting old assets

#### Scenario: Independent copied instance
- **WHEN** copied instances share an older export and later prepare different percentages
- **THEN** they preserve that immutable file while requests, new revision publication and completion authority stay instance/session scoped

### Requirement: Export inspection is ephemeral
Viewport, detail requests and previous-waveform updating presentation SHALL NOT
be serialized or notify host-save. Restore/instance changes SHALL clear these
transient owners while preserving validated Export preferences and durable WAVs.

#### Scenario: Inspect then restore
- **WHEN** a user pans/zooms or restores during pending waveform detail
- **THEN** creative state and WAV history are unchanged by navigation, old-instance display cannot publish and no preview autoplays

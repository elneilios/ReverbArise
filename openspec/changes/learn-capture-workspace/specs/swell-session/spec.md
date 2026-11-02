# Spec Delta

## MODIFIED Requirements

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

### Requirement: Cloned instances share immutable assets without sharing future writes
Copied instances SHALL retain readable accepted source/context references and creative/default preferences while isolating later transactions, settings and completion authority. Shared immutable history SHALL NOT let recording, changing defaults, clearing or restoring one instance mutate another's accepted source, generation context or swell assignments.

#### Scenario: Clone while a pass is pending
- **WHEN** an instance is copied during an uncommitted capture pass
- **THEN** the copy restores only the last coherent accepted source/context and saved mode with Recapture off, without the original pass's publication/display rights

#### Scenario: Independent later recapture
- **WHEN** one copy explicitly Recaptures with a different future default while another retains its saved map
- **THEN** only that copy accepts new source/context, and the other retains its prior samples, assignments, references and recoverable snapshots

## ADDED Requirements

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

#### Scenario: Successful pass with the editor closed
- **WHEN** a useful pass accepts source/context and selects Perform without an open editor
- **THEN** host state is marked changed through the safe notification path without callback storage, GUI work or waiting

# Spec Delta

## MODIFIED Requirements

### Requirement: Saved projects retain swell definitions and editing preferences
Project state SHALL retain stable swell definitions/assets, source/arrival, shapes/gain/placement, activity/freeze, names/selection, timing, workflow and monitoring/automation preferences. Selected future defaults and accepted source/generation-context references SHALL persist without PCM. Restore SHALL clear Arm and Recapture, discard candidate snapshots, pending Stop/generation requests and provisional operation/display authority, and stop voices. Starting a new capture requires a fresh Arm.

#### Scenario: Reopen an edited session
- **WHEN** an edited multi-swell project with -24 dB audition level is saved and reopened on the same PC/profile
- **THEN** source, definitions, edits, assets, selection, tone preference, -24 dB audition level, saved mode and accepted future defaults are retained without starting audition, transport or a saved capture pass

#### Scenario: Save while work is in flight
- **WHEN** a project is saved during capture, Stop validation, import, analysis, Recapture or rendering
- **THEN** it references coherent accepted source/context/assets only, unfinished work is not Ready and reopening cannot resume an arm, provisional overlay or obsolete pass

#### Scenario: Reopen before deferred pass generation finishes
- **WHEN** source from pass A has accepted, a later future Default IR B has been selected, and the project reopens before A's target generation completes
- **THEN** no deferred automatic generation resumes; A's accepted capture context and committed swells remain, B remains the selected default, and a new Generate explicitly snapshots the current accepted source/settings/default

### Requirement: Generation eligibility trusts durable provenance conservatively
Automatic reconciliation SHALL use persisted provenance, never names, positions or UI order. ManualCreated, manualEdited and all frozen swells SHALL remain protected. Supported older metadata without trustworthy origin SHALL remain conservatively manual/unknown and protected during automatic reconciliation. Explicit Learn rebuilds SHALL use freeze as the sole retention criterion. Malformed or unsupported provenance SHALL reject restore rather than fabricate automatic eligibility. Existing origin values and stable IDs SHALL retain meaning.

#### Scenario: Legacy or edited automatic work
- **WHEN** legacy origin is absent or an automatic swell was manually edited, renamed, disabled or frozen before save
- **THEN** during automatic reconciliation absent origin remains protected, manual edits remain protected, freeze always protects, and a valid saved automatic origin alone remains eligible subject to scope
- **AND** name and enabled state never infer origin

#### Scenario: Save during analysis or generation
- **WHEN** state is saved while candidate analysis or replacement preparation is held
- **THEN** only coherent accepted state is saved; reopening verifies assets, rebuilds read-only preview if permitted and cannot resume pending generation

## ADDED Requirements

### Requirement: A Learn rebuild retains frozen definitions verbatim
A successful explicit Learn rebuild SHALL preserve frozen identities, accepted definitions, sound and render references, activity and any still-valid selection. It SHALL clear selection references to retired unfrozen identities. Saved source and existing project format SHALL remain compatible.

#### Scenario: Save and reopen after rebuilding
- **WHEN** a project containing a frozen swell and newly generated unfrozen swells is saved and reopened
- **THEN** it restores the accepted rebuilt arrangement and its compatible source, preserving the frozen definition and resolving its existing assets

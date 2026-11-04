# Spec Delta

## MODIFIED Requirements

### Requirement: Saved projects retain swell definitions and editing preferences
Project state SHALL retain stable swell definitions/assets, source/arrival, shapes/gain/placement, activity/freeze, names/selection, timing, workflow and monitoring/automation preferences. Selected future defaults and accepted source/generation-context references SHALL persist without PCM. Restore SHALL disarm Recapture, discard candidate snapshots, pending Stop/generation requests and provisional operation/display authority and stop voices; only subsequent valid Play can start a new pass.

#### Scenario: Reopen an edited session
- **WHEN** an edited multi-swell project with -24 dB audition level is saved and reopened on the same PC/profile
- **THEN** source, definitions, edits, assets, selection, tone preference, -24 dB audition level, saved mode and accepted future defaults are retained without starting audition, transport or a saved capture pass

#### Scenario: Save while work is in flight
- **WHEN** a project is saved during capture, Stop validation, import, analysis, Recapture or rendering
- **THEN** it references coherent accepted source/context/assets only, unfinished work is not Ready and reopening cannot resume an arm, provisional overlay or obsolete pass

#### Scenario: Reopen before deferred pass generation finishes
- **WHEN** source from pass A has accepted, a later future Default IR B has been selected, and the project reopens before A's target generation completes
- **THEN** no deferred automatic generation resumes; A's accepted capture context and committed swells remain, B remains the selected default, and a new Generate explicitly snapshots the current accepted source/settings/default

### Requirement: Managed future defaults survive external-file changes
The selected bundled/custom Default IR SHALL be saved by immutable managed identity and label, alongside hidden generation preferences. External-file movement, catalogue ordering or later future-default changes SHALL NOT reinterpret accepted pass contexts or existing assignments. Invalid/cancelled import SHALL retain the previous accepted default.

#### Scenario: Restore a custom learning default
- **WHEN** a user imports a custom Default IR, records a pass, saves, moves the external file and reopens
- **THEN** the selected default and committed assignments use verified managed identities without external-file dependency or substitution; unfinished generation is not resumed

#### Scenario: Hide and restore generation preferences
- **WHEN** Automatic Swells is turned off, state is saved/reopened and it is later enabled while idle
- **THEN** the prior accepted Default IR/sensitivity remain available and are not reset by hiding or restoring controls

#### Scenario: One context dependency is missing
- **WHEN** an accepted pass's generation metadata/IR cannot be verified while its dry source is valid
- **THEN** dry remains preserved, recovery of assignments requiring that dependency reports failure, a new Generate validates its own explicit source/default snapshot, and other valid/frozen/manual work is not discarded or silently reassigned

## ADDED Requirements

### Requirement: Auto-generate on Stop has explicit migration defaults
Auto-generate on Stop SHALL persist independently per project and default OFF for fresh instances. Supported legacy state lacking the field SHALL restore ON; explicitly saved OFF/ON SHALL round-trip unchanged. New-version missing or malformed values SHALL reject restore transactionally. Hiding generation controls SHALL preserve this preference. Mode numbers, plugin identity and existing source/asset references SHALL remain compatible.

#### Scenario: Fresh versus legacy initialization
- **WHEN** a fresh plugin is created and separately an older supported project with no preference field is restored
- **THEN** the fresh instance is OFF and the older project is ON, subject to the same Automatic Swells and transaction guards

#### Scenario: Save OFF or ON with hidden controls
- **WHEN** either explicit preference is saved while Automatic Swells is disabled and the project reopens
- **THEN** that exact choice remains and no restore/toggle itself generates swells

#### Scenario: Invalid new state
- **WHEN** new-version state omits the required boolean or supplies an invalid value
- **THEN** the complete restore is rejected without replacing accepted source, swells or preferences

### Requirement: Generation eligibility trusts durable provenance conservatively
Replacement SHALL use persisted provenance, never names, positions or UI order. ManualCreated, manualEdited and all frozen swells SHALL remain protected. Supported older metadata without trustworthy origin SHALL remain conservatively manual/unknown and protected. Malformed or unsupported provenance SHALL reject restore rather than fabricate automatic eligibility. Existing origin values and stable IDs SHALL retain meaning.

#### Scenario: Legacy or edited automatic work
- **WHEN** legacy origin is absent or an automatic swell was manually edited, renamed, disabled or frozen before save
- **THEN** absent origin remains protected, manual edits remain protected, freeze always protects, and a valid saved automatic origin alone remains eligible subject to scope; name and enabled state never infer origin

#### Scenario: Save during analysis or generation
- **WHEN** state is saved while candidate analysis or replacement preparation is held
- **THEN** only coherent accepted state is saved; reopening verifies assets, rebuilds read-only preview if permitted and cannot resume pending generation

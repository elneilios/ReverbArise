# Spec Delta

## MODIFIED Requirements

### Requirement: Saved projects retain swell definitions and editing preferences
Project state SHALL retain stable swell identities, source/arrival, IR/render references, shapes/gain/placement, activity/freeze, names/selection, timing, monitoring/automation preferences and workflow mode without embedding PCM. Restore SHALL disarm Recapture, discard in-flight operation authority and stop voices. Capture SHALL admit only a subsequent valid Play, never resume the saved pass.

#### Scenario: Reopen an edited session
- **WHEN** an edited multi-swell project with -24 dB audition level is saved and reopened on the same PC/profile
- **THEN** source, definitions, edits, assets, selection, tone preference, -24 dB audition level and saved workflow mode are retained without starting audition, host transport or the saved capture pass

#### Scenario: Save while work is in flight
- **WHEN** a project is saved during capture, Stop validation, import, analysis, recapture or rendering
- **THEN** it references committed source/assets only, unfinished work is not marked ready and reopening cannot resume a Recapture arm or publish the obsolete pass

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

## ADDED Requirements

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

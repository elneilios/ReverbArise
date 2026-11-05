# Spec Delta

## ADDED Requirements

### Requirement: Inspectable build information
Settings SHALL provide About ReverbArise with numeric product version, candidate/build identifier and full Git SHA, derived from authoritative generated build metadata. Local or unavailable identities SHALL be labelled honestly. Copy build information SHALL copy the full report without modifying audio, project state, capture or render assets.

#### Scenario: Copy a bug report identity
- **WHEN** the user opens Settings About and activates Copy build information
- **THEN** the clipboard contains the displayed build's full version, build identifier and Git SHA, with no project or audio-state change
- **AND** visible accessible text confirms the copy, while the action remains enabled with the existing interactive button styling for repeated use

### Requirement: Deterministic verified build identity
Generated UI identity SHALL use the CMake product version, explicit validated release inputs and source revision/status without a build clock or random identifier. Ordinary builds SHALL clear prior release candidate inputs. Packaging SHALL reject generated metadata that disagrees with its expected version, candidate or clean source revision.

#### Scenario: Switch from candidate to local build
- **WHEN** an ordinary build reuses a candidate build tree
- **THEN** its generated identity reports local build status rather than the previous release candidate

#### Scenario: Stale candidate or revision
- **WHEN** generated build identity differs from the intended package identity or source
- **THEN** packaging fails before producing a completed release

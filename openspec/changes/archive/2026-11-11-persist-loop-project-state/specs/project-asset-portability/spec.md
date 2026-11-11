# Spec Delta

## ADDED Requirements

### Requirement: Loop recovery preserves capture-local identity
Locate and Restore SHALL verify a saved Loop's exact source identity, revision, exclusive extent, native rate, capture-local grid and dependent assets before admitting the recovered source. Recovery SHALL preserve Loop kind and keep live playback in Standby until fresh timing and Cue.

#### Scenario: Locate a moved asset folder
- **WHEN** a current-build Loop project opens with its original associated folder unavailable and the user locates a verified moved copy
- **THEN** the same accepted Loop audio, local grid, swell edits, IRs and verified render revisions become available without recapture or regeneration of valid included renders
- **AND** the next-capture preference remains independent of the recovered source kind

#### Scenario: Restore a verified backup
- **WHEN** a current-build Loop project's original assets are unavailable and the user restores its verified backup to a fresh destination
- **THEN** the recovered session uses the verified snapshot and original dependencies, and a subsequent host save and reopen can resolve that destination

#### Scenario: Invalid or incomplete recovery
- **WHEN** a candidate folder or backup is cancelled, fails verification or lacks a required dependency
- **THEN** the plug-in retains the saved identities, edits and surviving verified assets, and identifies unresolved dependencies
- **AND** unavailable dry cannot be marked ready or replaced by a Song source or a different Loop revision
- **AND** any explicit incomplete recovery preserves the existing incomplete status and complete-backup restrictions

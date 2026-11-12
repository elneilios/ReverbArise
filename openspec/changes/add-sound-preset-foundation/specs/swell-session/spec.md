# Spec Delta

## MODIFIED Requirements

### Requirement: Saved IR replacements preserve the accepted sound and shape intent

Session state SHALL persist the accepted versioned Sound assignment and exact IR, effective wet shape and the intent needed to recover fitted timing. Pending, failed and superseded replacements SHALL leave the last accepted durable definition intact. Earlier sessions SHALL preserve their saved sound and use their saved shape as initial intent where no separate intent is recorded.

#### Scenario: Save during preparation or after failure
- **WHEN** a project is saved while a replacement is pending or after it fails
- **THEN** reopening restores the last accepted Sound assignment, IR and shape, with the normal checks for saved assets

#### Scenario: Reopen a fitted replacement
- **WHEN** a project saved after a successful fit is reopened
- **THEN** the accepted Sound assignment, IR, render and effective shape reproduce the saved result in the editor and export
- **AND** the original timing remains available for a later response with sufficient support

## ADDED Requirements

### Requirement: Sound metadata migrates and restores transactionally

Supported earlier projects SHALL migrate to versioned Sound metadata without replacing verified responses, source, render identities, selection, gain, arrival, shape, freeze or activity. Restore SHALL validate new Sound metadata and its managed references before committing state. Pending requests SHALL remain ephemeral.

#### Scenario: Migrate a project with bundled, custom and frozen swells
- **WHEN** a supported earlier project is opened
- **THEN** known exact bundled identities retain their matching definitions and other managed responses retain their exact identities under Custom IR
- **AND** legacy gain and saved assets stay intact, including levels outside the current dial's display range
- **AND** missing or unassigned legacy responses preserve their recovery status without selecting a different sound

#### Scenario: Restore or clone a project with accepted Sound settings
- **WHEN** the project is reopened, copied to another instance or restored with verified portable dependencies
- **THEN** the accepted future preset and Level, per-swell assignments and retained managed imports restore coherently
- **AND** the original external import path is unnecessary, and later changes in one instance do not rewrite another

#### Scenario: Malformed or unsupported Sound metadata
- **WHEN** new-format state has unsupported versions, duplicate or invalid keys, incorrect value types, invalid gain or inconsistent asset identity
- **THEN** restore fails transactionally without modifying valid accepted state or deleting assets

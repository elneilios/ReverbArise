# Spec Delta

## ADDED Requirements

### Requirement: Saved IR replacements preserve the accepted sound and shape intent

Session state SHALL persist the accepted IR, effective wet shape and the intent needed to recover fitted timing. Pending, failed and superseded replacements SHALL leave the last accepted durable definition intact. Earlier sessions SHALL preserve their saved sound and use their saved shape as initial intent.

#### Scenario: Save during preparation or after failure
- **WHEN** a project is saved while a replacement is pending or after it fails
- **THEN** reopening restores the last accepted IR and shape, with the normal checks for saved assets

#### Scenario: Reopen a fitted replacement
- **WHEN** a project saved after a successful fit is reopened
- **THEN** the accepted IR, render and effective shape reproduce the saved result in the editor and export
- **AND** the original timing remains available for a later response with sufficient support

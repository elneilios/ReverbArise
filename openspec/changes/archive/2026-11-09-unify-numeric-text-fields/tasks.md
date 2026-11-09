# Tasks

## 1. Shared variants and native preview

- [x] 1.1 Reconcile the design's consumer table against the actual editor creation paths. Record each exposed and retained field, subtype, feedback position, binding lifecycle and disposition. Verify coverage of the six Settings value boxes, eleven inspector entries, hidden seams and timeline Height.
- [x] 1.2 Build the shared editable, embedded and read-only presentation for use in the preview. Add focused tests for focus provenance, caret and selection, temporary editor and stepper behavior, label and description associations, and bounded feedback wrapping.
- [x] 1.3 Build one interactive native family preview with the scenes in the design. Verify invalid and corrected inspector values, empty Rename recovery, keyboard and pointer focus, read-only and unavailable cases, and layout at minimum and larger sizes with 100% and 125% UI scale. Record the fixture checks separately from host results.
- [x] 1.4 Launch the native preview directly for the owner and record approval of the family variants and feedback positions before changing production consumers. Record approved metrics or specialized variants in the review evidence.

## 2. Inspector and dialog adoption

- [x] 2.1 Adopt the approved shared entry for every Perform inspector field. Route binding-supplied rejections to the approved position. Verify exact round trips, accepted and rejected Level, timing formats, cancellation, stale ownership, and feedback reflow without covering Reset. Update the C4 disposition for P7.
- [x] 2.2 Adopt the shared Name field and feedback position in Rename. Verify empty OK/Return keeps the dialog usable, correction commits once, Cancel/Escape discards, and row reorder or session replacement cannot redirect an edit. Document the approved dialog behavior and disposition.

## 3. Embedded, paired and read-only adoption

- [x] 3.1 Adopt the embedded variant for Learn Event sensitivity and Export Dry/Wet, including their temporary editors and existing stepper paths. Verify the Learn pass guard and percentage display, Export pair isolation and Character derivation, and keyboard and accessible steps. Record their feedback positions in the inventory.
- [x] 3.2 Adopt the embedded variant for the six Settings numeric fields and timeline Height. Verify existing units, precision, range, clamp and reset rules, Settings invocation behavior and display-only Height changes. Record the retained sensitivity seams and verify that their visibility and compatibility values are preserved.
- [x] 3.3 Adopt the shared read-only role for exposed Diagnostics values and verify naming, conditional visibility, precision, copying and zero edit publication. Verify the merged bar pair's accepted, rejected and unavailable paths and record it as existing shared coverage. Update the inventory dispositions for both groups.

## 4. Family completion and integration

- [x] 4.1 Reconcile the source inventory with the approved disposition of every P4 consumer. Update the shared contract, production style guide, and user guide where behavior changed. Verify documentation links, user guide currency, and absence of superseded editable-field setup for migrated consumers.
- [x] 4.2 Run the Windows Release wrapper with `-Test`, the UI standards guard, strict OpenSpec validation and whitespace checks. Record integrated coverage across the three workspaces, Settings, Diagnostics, Rename and timeline Height, with IR alignment as a regression. Record host and physical DPI limits separately and prepare the review PR with `Refs #107`.

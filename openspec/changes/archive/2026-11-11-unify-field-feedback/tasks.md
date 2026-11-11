# Tasks

## 1. Shared layout and native preview

- [x] 1.1 Extend the field presentation policy to measure and render the same wrapped lines within a single body or pair. Verify actual line and body bounds for short, long, empty and unbroken text at the approved type size, and check that the reviewed IR row and folder button are unaffected.
- [x] 1.2 Build a native preview with a single field and the Settings pair. Exercise normal guidance, multiline help and errors, correction, unavailability, focus by pointer/keyboard/accessibility and independent border feedback. Verify caption and full description associations, and check reflow and access to nearby actions at the supported minimum and a larger size at 100% and 125%.

## 2. Scoped native approval

- [x] 2.1 Launch the native preview directly for the owner and record explicit approval of the C3 look and interaction before changing the production Settings group. Carry any approved adjustments into the scoped design and evidence.

The owner confirmed the native preview was visible and approved it on 2026-11-07.

## 3. Settings Swell adoption

- [x] 3.1 Connect the Settings pair to the existing command outcome. Verify both Return and Set source from bars still submit through the same path; existing invalid input leaves the accepted range unchanged; accepted correction clears the error; context replacement cannot inherit an old error; and diagnostics, frozen behavior and other callers retain their existing behavior.
- [x] 3.2 Adopt the approved layout for Source range (bars). Verify captions above, one local message within the pair, reflow below it, red error treatment, separate focus brackets and accessible descriptions in the integrated page. Update the C3 contract, inventory and production style guidance with the approved adoption and explicitly deferred surfaces; verify documentation links and scope.

## 4. Integration checks

- [x] 4.1 Run the native Windows build and tests with `scripts/build.ps1 -Configuration Release -Test`, `scripts/check-ui-standards.ps1`, strict OpenSpec validation and whitespace checks. Record integrated Settings and IR regression evidence at the supported sizes and scales exercised. Record any untested host or physical DPI coverage separately and prepare the review PR with `Refs #107`.

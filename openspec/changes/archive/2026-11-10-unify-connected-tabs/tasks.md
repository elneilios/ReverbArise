# Tasks

## 1. Shared control and native review

- [x] 1.1 Extend `SegmentedSelector` with the approved connected body, selected fill, distinct hover and held states, and shared outer focus brackets. Extend `ControlLibraryCases` to verify seams and focus containment, pointer cancellation, keyboard and accessibility focus, explicit activation, unavailable inspection, accepted selection and wheel behavior.
- [x] 1.2 Open one native JUCE preview of the revised control with the editor strips and seven-tab Settings layout. Check selected, hover, held and focus combinations, workflow icons and unavailable choices, the compact editor, and Settings at 100% and 125%. Record owner approval of the revised component before starting section 2. Carry forward the already approved design and review only material differences or integration concerns.

## 2. Editor and Settings adoption

- [x] 2.1 Adopt the shared control in the workflow header, Learn and Perform inspector. Verify existing guarded workflow transitions, Learn and inspector page switching, and responsive layout without clipped labels, icons or focus decoration. Update the H2, L5 and P5 inventory entries with the adopted control and binding evidence.
- [x] 2.2 Use the shared strip in Settings and bind pages by stable identity with About last. Extend Settings tests to assert the full order, visible page content, direct Storage navigation, About's build information actions, page retention through refresh and resizing, and keyboard traversal of the active page. Update affected tests that target the old JUCE tab host or positional indices. Record the S7 adoption in the inventory.
- [x] 2.3 Update the production style guide, UI standards and C7 contracts to describe the implemented tab behavior and reuse boundary. Check the user guide for the Settings order and navigation and update affected references and the guide currency record. Verify the inventory still tracks list and scrolling work under #107.

## 3. Integrated verification and review

- [x] 3.1 Run `scripts/build.ps1 -Configuration Debug -Test`, `scripts/check-ui-standards.ps1`, `openspec validate unify-connected-tabs --strict` and `git diff --check`. Record the native and automated results separately from actual Windows DPI and FL Studio coverage. Confirm all four consumers use the shared control and all Settings tabs fit at the supported widths and scales, including Appearance in the responsive layout.
- [x] 3.2 Prepare a review PR referencing #107 with the native approval, consumer coverage and verification results.

# Tasks

## 1. Shared behavior and native verification

- [x] 1.1 Implement the shared dial and track states using existing tokens and focus provenance. Verify idle, hover, held, keyboard, accessibility, pointer and unavailable states, including independent focus and hit bounds for the P4 value box and steppers.
- [x] 1.2 Implement the shared coarse and Ctrl+wheel routing using each control's legal domain. Verify both directions, reversed wheel, fine versus coarse increments, clamping, discrete intervals, events outside the track, and ordinary drag with and without Ctrl on a continuous track.
- [x] 1.3 Implement optional drag-only home detent configuration and enable it for Level at 0 dB. Verify the tick, both approaches and exits, drag reversal, modifier rebasing, controls without a home, and values inside the band through fine wheel, keyboard, accessible and typed edits. Verify that the reset provider is independent.
- [x] 1.4 Compare the real shared components against the approved native preview at 900 × 600 and a larger size, at 100% and 125% UI scale. Record the scope and metrics in the style guide and dated evidence. Obtain a further owner checkpoint if the integrated interaction or appearance materially differs from what was approved.

## 2. Perform and workspace adoption

- [x] 2.1 Adopt the dial for all eleven Perform entries and record their inventory disposition. Run the Perform knob, focused Level and reset checks for grid wheel steps, Ctrl+wheel bypass, Shift fine behavior, drag rebasing, first touch of older saved Level, captured Reset actions and stale ownership. Verify fixed-width exact entry, Target hover help and inline validation, wrapping within the value column and fixed Reset layout.
- [x] 2.2 Adopt the Learn Event sensitivity and both Export level tracks. Verify percentages and their plain/fine steps, the Learn pass guard, and untouched Export level and Character derivation. Verify P4 field and stepper behavior and update the inventory.

## 3. Settings and display-only adoption

- [x] 3.1 Adopt the shared track for all six Settings sliders. Verify the listed step defaults, ranges, suffixes, whole-bar tone endpoints, audition reset and Separation's captured pass. Record the six dispositions and verify that the hidden sensitivity controls and their compatibility values stay in place.
- [x] 3.2 Adopt the zoom track in Perform and Export and the timeline Height track. Verify normalized zoom steps, anchor, limits, Fit and plus/minus behavior, and Height's display-only range and reset. Check surrounding timeline wheel behavior and record both display-control dispositions.

## 4. Integration

- [x] 4.1 Reconcile the P5 inventory and living contracts against the delivered consumers. Verify the user guide describes Ctrl+wheel and the Level detent and passes the guide currency check.
- [x] 4.2 Run the Windows Release wrapper with `-Test`, the UI standards guard, strict OpenSpec change validation and whitespace checks. Record integrated results and host and physical DPI limits, then prepare a review PR with `Refs #107`.

# Issue #107 numeric and text field preview

The P4 native fixture presents the Perform timing and Level entries, a compact
numeric slider value with its temporary editor and stepper, the Settings bar
pair, Diagnostics field states and Rename in one window. The inspector and
Rename scenes use fixture values and submission callbacks for review.

The Windows MSVC Release wrapper built the fixture on the worktree based on
`a35f156`. The focused native `--test-numeric-text-fields` check passed for
keyboard, pointer and accessibility focus, caret and selection, embedded entry
and stepper actions, exact and invalid values, correction, cancellation,
read-only behavior, and feedback associations and wrapping. It checked 900 × 600
and 1100 × 850 at 100% and 125% UI scale. The existing `--test-field-feedback`
check, UI standards guard and strict OpenSpec validation also passed.

This first run covered the family fixture and its sample bindings.

## Revised preview requested on 9 November

- Numeric input blocks letters while typing or pasting. A wholly blocked
  insertion preserves the current value and selection. The timing example
  still uses the song-position parser, with meter-aware validation.
- The compact scene includes a standalone number input with its own stepper,
  using the same entry as the slider's value box.
- Return and focus loss show the applicable validation. The Settings pair and
  Rename validate on focus loss and still require explicit submission.
- The slider initially demonstrated Ctrl-drag for the P5 gesture review. Owner
  feedback replaced this with Ctrl+wheel over the track and restored ordinary
  drag behavior. The track change is scoped to this preview. Both number boxes
  and their stepper arrows accept wheel input in one configured step per event.

The revised focused native fixture passed the letter and mixed-paste checks,
Tab and outside-click validation, the standalone stepper, the explicit Settings
and Rename submission checks.

The Windows Release build and focused native checks passed again with the
wheel changes. The checks covered both directions over standalone and embedded
steppers, smaller Ctrl+wheel changes on the slider track and equal drag results
with and without Ctrl.

The owner approved the updated native preview after the Ctrl+wheel and
numeric stepper wheel review. This completed the approval checkpoint before
production adoption.

## Production adoption

The inventory covers Learn Event sensitivity, all eleven Perform inspector
entries, Export Dry and Wet, six Settings slider boxes and the source-bar pair,
Diagnostics exact values, Rename and timeline Height. The hidden sensitivity
seams retain their visibility and compatibility values. Settings and Height
retain compact value boxes without arrow buttons. The standalone numeric
stepper remains a reusable variant. Ctrl+wheel fine slider-track adjustment
remains recorded for P5.

Inspector rejection preserves the accepted value and reflows the affected row.
Settings and Rename validate when focus leaves the field and still submit
through their explicit actions or Return. The Rename binding checks the
captured swell ID and session epoch at submission. Diagnostics retains text
selection across refreshes of the same accepted value.

The warm Windows MSVC Release wrapper rebuilt the worktree based on `a35f156`.
The integrated `preverb_swell_ui_test` and focused
`preverb_selector_popup_test --test-numeric-text-fields` both passed. The
integrated checks cover Learn pass rejection, Export level isolation, exact
Perform edits and local errors, all six Settings boxes, explicit paired
submission, Diagnostics selection and Rename across reorder and restore.
Timeline Height and IR field alignment are included in the native regression
suite. Earlier runs exposed old Rename and Level fixture assumptions and a
Learn keyboard-step ordering regression. The final implementation preserves
the existing queued step path and its pass guard.

The final `scripts/build.ps1 -Configuration Release -Test` run passed 98/98
in Windows MSVC (log directory `out/build-logs/Release/20261109-212250-0867aa88`).
The native knob-reset and exact-value readability tests passed after earlier
Windows foreground-focus timeouts. Strict OpenSpec validation, the UI
standards guard, user-guide currency and whitespace checks passed.

Coverage includes native desktop input and software UI scaling. FL Studio
integration and physical DPI variation were outside this run. The shared warm
build tree and evidence logs are retained for review.

## PR #134 review follow-up

Settings binds the source-bar pair to the displayed swell ID and session
epoch. Changing either clears the pair and its old feedback. Validation
checks the bound swell, and submission checks the epoch and the selected
ID again at the edit boundary.

The Windows Release `preverb_swell_ui_test --test-source-range-feedback`
passed. It checks submission before the editor observes a changed selection,
focus loss after that selection changes, fresh entry for the new swell, and
session restore with the same selected ID.

The full Windows Release wrapper with `-Test` passed 98/98 after the fix
(`out/build-logs/Release/20261109-214045-a5db7ada`). The knob-reset fixture
received foreground focus during the run. UI standards, strict OpenSpec,
user-guide currency and whitespace checks passed.

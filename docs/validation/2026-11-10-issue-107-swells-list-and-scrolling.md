# #107 P7 Swells list and scrolling

## Native design approval

The owner approved the native JUCE fixture on 10 November 2026. It used 32
sample swells, a compact scrolling inspector and an IR dropdown. The review
covered square Enabled checkboxes, keyboard row actions, the shared row menu,
confirmed group deletion with Cancel focused, a dimmed Rename label during
multiselect, full-width short-popup feedback and quiet disabled hover in both
the action menu and dropdown.

The preserved fixture at `out/ui-review/p7-list-scroll/build-disabled-hover`
passed its `--check` geometry, keyboard selection, Enabled, right-click
ownership, group deletion, disabled hover and inspector reveal assertions.
It used sample data and did not exercise production persistence or host input.

## Production checks

- `preverb_swell_list_test` exercised forward and reverse Tab, named
  accessibility focus and activation, square indicators, and identity and
  scroll preservation with recycled rows. Perform binding checks verified
  explicit #93 reveal and a checkbox activation without a waveform refit.
- `preverb_selector_popup_test` exercised disabled pointer behavior in choices
  and actions, keyboard inspection, checkmarks, and scrollbar-dependent row
  width at 100% and 125% software scale. The shared presenter serves the
  Swells and knob Reset menus and migrated dropdowns, including IR, Export mix
  and Settings.
- `preverb_swell_engine_test` exercised mixed manual and detected deletion,
  missing and duplicate IDs, stale epochs, suppression capacity and saved
  session round-trip. Rejection preserved the group and selection.
- `preverb_swell_ui_test` used the production editor to verify both menu entry
  points, disabled Rename, single-row scope outside the selection, Cancel
  focus and Escape, and confirmation after a selection change. It removed
  exactly the originally captured group and preserved unrelated selection.
- Responsive scale checks exercised the existing inspector, Settings,
  nested Storage and Diagnostics viewports at 100% and 125% software scale.
  Inspector checks included 900 × 600 and 1100 × 850, unchanged field text
  and fixed Reset geometry. Existing editor checks covered native focus
  reveal in Settings and the established wheel routing.

## Release verification and limits

The final Windows x64 MSVC run of
`scripts/build.ps1 -Configuration Release -Test` passed 99 of 99 CTest
cases on the warm build tree based on merged Export commit `6379834`.
CTest took 354.01 seconds. The logs are under
`out/build-logs/Release/20261110-162258-c09a41b0`. Earlier focused reruns
corrected legacy synthetic hit targets, modifier handling and an invalid
fixture reference after reopening the editor.

UI standards, strict OpenSpec validation, the user-guide currency check
and Git whitespace checks passed. Native component tests exercised JUCE
accessibility handlers. A live Windows screen reader, FL Studio hosting
and physical high-DPI displays remain unverified.

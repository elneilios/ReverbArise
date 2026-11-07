# Issue #107 action menus

The preview starts from merged `main` at `954accd` (PR #117). The cached
`preverb_selector_popup_test.exe` matches the Source, tests and CMake inputs
of that merge and passed its focused native checks, including same-process
native focus loss, unavailable inspection and owner lifetime.

[#108 PR #110](https://github.com/elneilios/ReverbArise/pull/110) is open and
draft at `7badf84`. Its revised plan is 8/13 complete and still requires its
fixed-limit implementation and native review. Its branch changes
`Source/UI/KnobValue.h` (parameter mapping and input behavior) and
`tests/PerformKnobResetCases.h`. This checkpoint uses fixture Reset actions.
Coordinate the two shared files after #108 lands or its owner rebases. The
pending hookup is limited to `KnobValue::showResetMenu` and its tests.

The supported warm Release builds `20261107-091259-1c88ef8c` and
`20261107-091530-3530fff9` passed. The focused popup executable passed after
each build. It checked role-based neutral and red row ink, held feedback,
pointer departure, keyboard inspection, absent action selection metadata,
disabled pointer/key/accessibility activation, and one-shot completion. The
final fixture uses `SwellList` and opens the same action popup from right-click
and the three-dot hit area. Long unavailable Reset rows and screen-edge
placement passed at 900x600, 1000x700, 1100x850 and 1400x1000, with 100% and
125% software scale. UI standards, strict change validation and guide currency
also passed. Full CTest, physical Windows DPI and FL Studio coverage remain
separate from these focused checks.

The native `ReverbArise - #107 action menu review` window was launched directly
for owner review. The owner approved it on 7 November 2026, including the
popup-only 14% hover and 24% held destructive blends in the preview. The
approval covers the fixture's action-row treatment and interaction.

The Swells list now owns the action popup across row recycling. Right-click and
the three-dot hit area both anchor it at the pointer. Rename retains the
existing dialog, and Delete retains the captured-ID and epoch removal path.
The native `preverb_swell_ui_test` passed in build `20261107-110641-d61df603`
on the original selector baseline. It verified both openings, pointer
anchoring, cancelled menu and dialog, rename after reorder, deletion of the
captured row after selection and order change, queued Delete across session
replacement, and editor destruction before delivery.

The integration branch also includes current merged main `ca9f7df` (PR #112).
The offline guide combines the Swells and working-folder instructions.
The earlier baseline run hit the existing Windows foreground prerequisites
for the knob-reset and exact-value readability fixtures. The combined run
uses the native fixtures from merged main with their focus checks intact.

The supported combined Release build and CTest run passed **89/89** checks
at `c2dcb11`, on merged main `ca9f7df`, in run `20261107-111450-26d74cdf`
(384.08 seconds). It includes the Swells binding, popup, editor, audio,
storage, and persistence checks. The three earlier-baseline failures
(knob focus, readability focus, and the transient engine timeout) passed
in this run. Strict OpenSpec validation, UI standards, guide currency,
and whitespace checks also passed.

OpenSpec is at 5/9 tasks. The Swells menu is implemented and verified.
The Reset hookup and its binding tests still await #108 coordination. The
combined documentation, final validation, and review-ready delivery tasks
remain open for that seam. The Windows fixture and software-scale results
are distinct from physical Windows DPI and FL Studio interaction coverage.

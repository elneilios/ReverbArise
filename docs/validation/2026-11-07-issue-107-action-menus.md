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

The initial Swells integration included merged main `ca9f7df` (PR #112).
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

### Reset hookup after #108

PR #110 landed in `4cf96d0`. The action-menu branch integrates that baseline
and keeps Level's approved revision 15, fixed edit range, legacy gain and reset
provider. Action menus use revision 16. The new Reset popup captures the
existing provider label and closure and anchors at the right-click point.
Pointer dismissal and activation return keyboard focus with the dial's brackets
suppressed. Keyboard or accessibility activation can restore those brackets.
The scope is the popup return, with ordinary dial input using the existing policy.

The built native `preverb_perform_composition_preview --test-knob-resets`
passed on `e9c3bbe`. It covers pointer, keyboard and accessibility activation,
initial/default/automatic effects, anchor and focus ink, unavailable selection,
legacy gain dismissal, and queued supersession after selection, session,
source/input, workflow pass, timing, reset revision, freeze or editor destruction.
The Windows fixture and software-scale results are distinct from physical DPI
and FL Studio interaction coverage.

The combined Windows/MSVC Release wrapper at `e9c3bbe` on main `4cf96d0`
passed configure/build and 97/98 CTests in run `20261107-135341-652219e9`
(401.19 seconds). The only failure was the Reset fixture's foreground
prerequisite (`peer=1 foreground=0`), before any Reset assertions. The first
focused CTest retry hit the same prerequisite. The final focused CTest passed
1/1 in 10.96 seconds on the same binary, with the foreground assertion intact.
The fixture finished before the planned window activation. All 98 cases
have a passing result at the same runtime revision across these runs.

The passing coverage includes Swells and Reset bindings, fixed Level and
audio paths, and storage and persistence regressions. UI standards passed
with 73 files and 59 bounded occurrences. Strict change validation, guide
currency, whitespace and changed Markdown file links passed. The final
branch diff scopes production work to the shared popup and its Swells and
Reset hookups.

[PR #119](https://github.com/elneilios/ReverbArise/pull/119) is ready for
review and references umbrella issue #107. The completed OpenSpec change
remains active for review and later sync/archive.

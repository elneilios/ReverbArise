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
for owner review. Tasks 1.1 through 1.3 are complete. Task 2.1 awaits the owner's
approval of the native preview before editor adoption.

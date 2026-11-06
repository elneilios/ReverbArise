# Issue #58 — combined upstream integration

Worktree: `6b94/k7ma-preverb`, branch `codex/58-compact-responsive-ui`.
Tested source: `807fa1b` plus this record's scale/integration working-tree patch.
This record continues the [5 November evidence](2026-11-05-issue-58-compact-ui.md).

**Latest integration:** #73 PR #100 merged as `80a12f0` during closeout. #58 was
rebased onto that accepted main. The ruler conflict retains #73's `gridTiming()`
authority and #58's component-aware responsive typography/caption clearance.
The other conflict retained both required include sets. Accepted live/offline
clock, Snap inspector/canvas bindings and Copy Diagnostics are preserved.
The combined Release gate passed on clean `e5b8b18`: **75/75 tests, 293.57 seconds
CTest**. Logs: `out/build-logs/Release/20261106-012558-ed464d70`. Earlier results
below remain evidence for their stated source. The final guide's four ordinary
production-editor captures and self-contained HTML assembly passed through the
existing release-guide helpers; review artifacts are under `out/issue58-guide`.
Guide currency, UI standards and strict OpenSpec validation pass.

During evidence inspection the compact capture helper was found to append PNG
bytes to an existing file. It now rewinds/truncates, matching the established
guide capture policy. This is a fixture-output correction only; refreshed
screenshots replace the old bytes. Product code is unchanged from the complete
combined wrapper pass.

## Accepted upstream baseline

Rebased #58 onto `6b2ae10`, the accepted #51 PR #92 merge. This also includes
#68's TransportReadout (PR #95, archive #97) and #93's explicit-selection Fit
(PR #96). Export conflict reconciliation retains independent Dry/Wet levels,
Character presets, continuous preview, accepted captions and silent notice
policy while applying the review composition and responsive text roles.

#73's new Snap repair remains local to its agent and is not incorporated.
Its quiet window was respected: one explicitly granted compile slot used at
most two compiler workers, with no UI tests or installation. That slot found
an unsupported JUCE `getSize()` call; it was corrected to width/height. #73
subsequently released the window. No system VST3 installation is performed by
#58; #73 retains ownership of that destination.

## Independent work

The review editor now wires the previously verified preference/fit policies
through a scaled content surface, Appearance controls and GUI/lifecycle saves.
Settings/Diagnostics gain work-area bounds, overflow and focus reveal. Review
default storage is transient, leaving production defaults and user profiles
unchanged. A dedicated actual-editor scale fixture covers coordinates,
preferences, recovery and preservation of musical state and waveform zoom.
The compact Export fixture now discovers the delivered Character/Dry/Wet
controls and checks their bounds and separation from the drag action.

## Verification status

- UI standards: passed, 61 files / 60 bounded definition/debt occurrences;
  debt inventory unchanged.
- `git diff --check`: passed before combined compilation.
- First sandboxed combined configure failed because Windows SDK registry access
  was denied; the rerun with SDK access passed. Existing caches were retained.
- Combined Release native preview and transport fixture compilation passed.
  The new transport fixture required its first local JUCE compilation; other
  targets reused the warm tree. At most two compiler workers were used.
- Initial focused CTest: compact geometry, policy, Perform bindings/readability
  passed; transport was not run until its missing binary was built. The scale
  fixture then exposed cumulative integer popup-anchor rounding with nested
  125% user / 150% host transforms. `SwellList::menuAnchor` now transforms the
  floating event position and rounds once. The exact screen-anchor assertion
  and complete scale fixture pass without widening tolerances.
- Final affected Release CTest: **15/15 passed, 94.01 seconds**, including
  policy, transport, full composition, navigation, gestures, bindings, Fit,
  knobs/reset, audition, readability/help, compact editor and scale integration.
  Command filter: `preverb_(responsive_(editor|scale)|perform_.*|transport_readout|presentation_policy)_test|^preverb_perform_composition_preview$`.
  This is focused MSVC/CTest evidence, not the full wrapper suite or host UAT.
- `--test-responsive` additionally captured actual component snapshots for all
  modes at [900x600 and 1400x1000](2026-11-06-issue-58-combined/). Capture passed.
  Export capture shows pending readiness rather than claiming ready-WAV coverage.
- Strict OpenSpec validation passed after retaining #93's accepted explicit
  list-navigation/render-completion scenarios in the pending modified requirement.
- The combined live native review was directly launched with `--review-responsive`.
  Computer Use observed exactly one `Issue #58 - Compact responsive UI checkpoint`
  window: 902x676 outer pixels, containing the verified 900x600 editor plus the
  review strip and native frame. The visible strip identifies #51/#68/#93.
  Window remains open for owner review; this launch is not owner acceptance.
- Task 3.3 (preference record and actual lifecycle integration) was guarded,
  checked and confirmed through refreshed vanilla Apply: **4/23 complete**.
- Full Release wrapper gate, combined 900x600 owner review, final host/DPI
  coverage and production activation: pending. #58 remains last and unfinalised.

## Learn Sound review correction

Owner review identified an empty Sound tab with Automatic Swells off. The
responsive layout incorrectly reused the automatic-generation visibility gate
for Default IR, Import IR and their labels/status. Sound defaults now follow
the selected tab independently; existing pass-lock enablement and assignment
bindings remain unchanged. Automatic generation controls keep their own gate.

The compact fixture now explicitly checks the Sound controls with automatic
processing/markers off as well as on, at every existing size. Previously its
discovery assertions used only an enabled synthetic view and missed this state.
Focused Release CTest passed **2/2, 12.79 seconds** (compact editor and scale),
and native screenshots were refreshed. UI standards and diff whitespace passed.
The first relink met the still-open review executable; after closing that one
review window, incremental relink and tests passed. The corrected preview is
reopened for review; no host installation or production activation occurred.

## Learn alignment and Sound composition review

The owner identified Source Capture's horizontal label inset and an awkward
scope-note/gap above the Sound picker. The responsive Source Capture and scope
labels now use the same zero text inset as their sibling labels. Sound begins
at the shared content-padding gap below its tabs; the scope note follows the IR
picker/status instead of occupying a separate introductory band. Regression
assertions cover the actual label insets, alignment and content gaps.
Incremental Release compile and both focused native CTest checks passed
(2/2, 14.39 seconds). Learn snapshots at 900x600 and 1400x1000 were refreshed.

Review also identified that Learn and Perform do not yet share the actual Sound
component. GitHub #80 remains open and the current model stores Default IR but
no Learn Level default. The mockup Level was previously a future-capacity slot.
The owner explicitly chose to fix layout here and handle the shared component
and persisted Learn Level separately. #80 owns the shared IR foundation; new
Learn Level semantics are a further extension. Neither blocks #58.
No dummy Level control or new default-gain behavior is introduced.

## Production integration and remaining gates

The host-facing `createEditor()` now enables the responsive composition and
user-local file storage. Direct legacy fixture construction remains available
for existing regression baselines; host construction and native review both use
the same production initialisation path. A Windows factory test redirects only
its own process's local profile to a temporary directory, exercises fresh
900x600 opening and actual file-backed close/reopen, then restores the variable.

Expanded Release compact/scale fixtures passed **2/2, 27.65 seconds**, including
16 supported/breakpoint-neighbour sizes, delivered Export ready/preparing/error
presentations with retained WAV identity, factory preferences, owned Settings
scale/reachability and Diagnostics close/hidden-refresh behavior. The first
auxiliary test searched an inactive Settings page; selecting its actual tab
corrected the fixture without changing production behavior.

This verifies task 2.1's responsive allocations/reflow across the listed sizes.
Task 2.6 uses its specified fallback: current Learn Default IR access and
Perform IR/Level access are covered, musical identities are preserved, and the
owner explicitly excludes a shared module/default Level until separate work.

Full supported Release wrapper verification is running. Final Appearance and
auxiliary interaction review and one consolidated FL Studio fit/scale smoke
remain owner checkpoints. No installation or release has been performed here.

Living presentation, workspace, UI standards and production style guidance now
describe the implemented composition and size/scale contracts. Revision 7 records
the named approved compact allocations and remaining review scope; reference
destinations exist, UI standards pass without inventory regeneration and strict
OpenSpec validation passes. Physical Windows 125/150/200% coverage is untested;
software transforms are not recorded as physical DPI passes. The offline guide
review/capture gate is tracked separately in task 4.5.

The supported wrapper `scripts/build.ps1 -Configuration Release -Test` passed
**75/75 tests, 292.89 seconds CTest**, on `0abf17a` plus the production-integration
patch. Logs: `out/build-logs/Release/20261106-005556-67831ffc`. Dependencies were
reused; previously unbuilt test/benchmark targets required their first local
compilation. No caches were cleared. Compiler parallelism was capped at two.

The final UI patch connects the shared slider's explicit value font to the same
responsive role used by other values. Actual NumericInputControl font assertions
pass at all 16 sizes. Added dense/empty 100%/125% journeys and native keyboard
focus reveal in a vertically constrained Settings window also pass. Final
focused Release CTest: **2/2, 30.72 seconds**. The first dense/empty test wrongly
expected Export admission without a ready swell; it now proves that the existing
guard rejects that entry, rather than changing production admission.

Combined evidence covers the common controls' unchanged guarded bindings from
the full suite plus compact routes/geometry, fixed Reset, source/IR/render
identity preservation, popup transforms, active exact text and Escape, actual
factory persistence, fit recovery and owned-window lifetime/focus. Tasks 1.2,
2.2–2.5, 3.1, 3.4–3.6 and 4.1 use these complementary checks. Physical DPI/host
acceptance and scoped scale-family review remain separate owner gates.

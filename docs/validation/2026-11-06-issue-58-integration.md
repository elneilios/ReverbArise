# Issue #58 — combined upstream integration

Worktree: `6b94/k7ma-preverb`, branch `codex/58-compact-responsive-ui`.
Tested source: `807fa1b` plus this record's scale/integration working-tree patch.
This record continues the [5 November evidence](2026-11-05-issue-58-compact-ui.md).

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

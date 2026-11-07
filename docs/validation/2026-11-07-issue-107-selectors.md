# Issue #107 — selector family native checkpoint

## Scope and execution map

Change: `unify-selector-states`, based on merged PR #114 (`89da1e7`) plus
planning commit `3728bf0`. The owner approved the browser design and final
transient-hover correction on 2026-11-07 and authorized Apply. Native family
approval still gates wider consumer adoption; `unify-action-menu-states` remains
a separate child, not an implementation claimed by this checkpoint.

Tasks 1.1–1.4 are medium-risk shared UI interaction/lifetime/rendering work:
focused component tests, native input/accessibility and stale/destroyed callback
oracles run before preview/adoption. Task 1.5 is low-risk API/standards evidence.
Task 2.1 requires a prepared directly launched native preview and scoped owner
approval. Tasks 3.x retain separate real binding tests and documentation per
consumer group; 4.x retain the final supported wrapper/CTest and PR gates.

The explicit `ChoiceSelector` subclass extends `DeliberateSelector`. Existing
production declarations have not been migrated or globally restyled. New code
is exercised by the dedicated `preverb_selector_popup_test` target/fixture.
No #108/#8 worktree, parameter/range, reset provider, storage, asset, model or
audio implementation was changed. Other action/context menus are untouched.

## Native verification

Warm supported Release wrapper runs on Windows/MSVC, using the retained build
tree and fetched JUCE sources:

- `20261107-004449-0f1e556d`: configure/build passed; initial popup state test
  passed, exit 0. The preceding sandboxed configure attempt could not read the
  Windows SDK registry; the supported wrapper succeeded with registry access.
- `20261107-004927-58da67b2`: compile errors in JUCE event constness and focus
  notification API; corrected without changing test requirements.
- `20261107-005134-ab944864`: configure/build and native popup checks passed.
- `20261107-005517-bee31301`: configure/build and explicit selector native
  focus/binding/lifetime cases passed, exit 0.
- `20261107-010026-11f969a8`: fixture-only compile errors (duplicate listener
  inheritance and const access to an accessibility handler); corrected.
- `20261107-010059-3ee7b8db`: configure/build and expanded geometry/state tests
  passed, exit 0, including long labels/reasons at 100%/125% user transform.

The tests establish independent hover/keyboard/checkmark state, clearing hover
into padding/outside, retained keyboard inspection after temporary hover,
one-shot completion, ineligible row inspection/no activation, actual accessible
disabled state/reason, pointer/key/AT paths, screen clamping, Escape/Tab, captured
catalogue replacement, callback owner destruction and silent refresh. State
images prove focus changes do not alter body pixels and disabled body feedback
does not respond to hover/held input. Native user-transform checks are distinct
from physical Windows DPI proof.

Final fixture refinements, preview launch and owner decision will be appended
below. Build-only success is not a full CTest result. The final change-wide
Release/CTest, production bindings and host coverage have not been claimed.

## Prepared native review checkpoint

Final warm Release wrapper `20261107-010920-04323dc3` passed configure/build,
exit 0. The native selector executable then passed all focused cases, including
pointer focus restoration without decoration, backwards Tab, owner deletion
before queued completion, destruction during preparation/commit, long text and
all four fixture sizes at 100%/125% software scale. No full CTest or new host/DPI
result is claimed at this pre-adoption boundary.

UI standards passed 70 files/59 bounded occurrences without debt regeneration.
Changed documentation links passed. The user guide was reviewed semantically:
its existing production instructions are unchanged because this is an opt-in
library/preview checkpoint; the explicit currency record was refreshed and passed.

Supporting native captures in out/selector-family/ were inspected for selector/
folder body alignment, outside brackets, wrapped menu labels, independent checkmark,
quiet hover and visible unavailable reason. The capture helper uses the existing
software-image pattern and truncates prior output; early blank captures were a
capture-helper issue and are not review evidence. Preview labels use portable
source encoding; the automated long-name test also covers an explicit UTF-8 name.

The preview was launched directly from a SHA-256-verified copy of the passing
executable: out/selector-family/selector-family-review.exe --review-selectors,
PID 54928, window title ReverbArise - #107 selector family review. Process path,
arguments, window title and responding state were verified across the sandbox
boundary; a duplicate diagnostic launch was closed, leaving the original open.
The previous completed command-family review was closed only after exact
path/argument verification.

Tasks 1.1–1.5 are verified (5/12 complete). Task 2.1 remains pending: the owner
selected "I have feedback" and the native window is preserved while that feedback
is collected. Production adoption tasks 3.x and final delivery tasks 4.x are not
started. The separate action-menu implementation is not started. This supersedes
only the earlier pending fixture-refinement note, not the owner/native gate.

## Field feedback amendment and native approval

The owner approved red validation message plus red body border and smaller help
immediately beneath the control after comparing the two interactive field studies.
The amendment is captured in the active proposal/design/delta/task 1.6 and shared
contracts. FieldPresentation supplies 12-height Inter Regular, 17-unit line
allocation, 4-unit body-to-feedback gap and existing helper/danger ink; selector
error borders remain red through hover/held while focus brackets stay aqua.

Warm Release wrapper 20261107-012955-c95c17b4 passed configure/build, exit 0.
The complete focused native selector executable passed, exit 0, including added
error border/focus state matrix, visible accessibility error/clear checks without
selection commands, all supported fixture sizes/user scales and readable disabled
explanations. Earlier added-test runs failed because the fixture queried a null
accessibility handler on a control with no native peer; the fixture now attaches
a real native window and asserts the handler exists. No critical check was removed.
Accessibility notifications are limited to showing controls and use their value
interface; error metadata remains queryable and clears without stale text.

UI standards passed 71 files/59 bounded occurrences without debt regeneration;
strict OpenSpec and semantically reviewed guide currency passed. A verified copy
was launched as out/selector-family/selector-feedback-review.exe --review-selectors,
PID 63272, responding with the expected window title. The old selector preview
was closed only after the replacement was confirmed ready. The new preview starts
on Validation error and permits an eligible IR choice to restore routine help.

The owner explicitly answered "Looks right — approve native family" for the revised
native preview on 2026-11-07. Task 2.1 is accepted; tasks 1.1–1.6 and 2.1 are now
verified (7/13). The review window remains open. This supersedes the earlier pending
feedback/native-approval status. Production adoption and final combined verification
follow; no physical Windows DPI/FL Studio coverage is inferred from this acceptance.
Text/numeric input migration and #108/#8 implementation remain separate.

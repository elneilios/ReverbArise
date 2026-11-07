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

### Shared IR adoption

Learn and Perform now opt into ChoiceSelector through IrSelector. Existing
ConvolutionIrEditor/ConvolutionIrBinding catalogue snapshots and session/input/pass
acceptance are retained. Only presentation changes: reserved vertical focus frame,
matching selector/import body height, small left-aligned local feedback and error
border/message. Disabled reasons are shown locally when no status is supplied;
healthy empty feedback still collapses. Field typography uses the native-approved
minimum 13 control value against 12-height local help, avoiding a compact-mode
inversion of that hierarchy.

Supported Release build 20261107-013814-b2715771 passed. Native
preverb_swell_ui_test --test-ir-editor passed, exit 0, with temporary capture/storage
and process-lock access outside the sandbox. The initial sandbox-only run could
not commit its temporary capture; this was recorded separately from the successful
native binding run. The checks preserve meaningful IR asset/default/selected,
reorder/captured identity, queued stale choice, readonly, persistence/audio and
body/feedback geometry oracles; arrows inspect before explicit commit now.

### Remaining selector adoption and binding checks

Perform grid, Export Character, Appearance size/scale and the four named
Playback/Swell selectors now opt into ChoiceSelector. IDs, grid divisions,
canonical gain pairs, Custom, profile storage and original Settings commands
remain intact. The latter capture a read-only session/pass predicate; curve
choices additionally capture primary/selection/input generation, preventing a
queued popup from targeting a replacement swell even before an editor refresh.
Recovery candidateBox, action/reset menus and #108 controls are unchanged.

Warm Release build `20261107-015005-e36c1f3d` passed. The actual-editor
`--test-selectors` path passed native grid inspection/cancel/commit, both curve
callbacks, stale primary rejection, routing/gain callbacks, unavailable saved
prototype inspection and session replacement rejection. The first attempted
`--test-export-levels` invocation fell through to the broad UI suite because that
focused CLI route did not yet exist; the route is now explicit. Its legacy
UiStandards arrow-key oracle was updated to require inspection before Return,
retaining its stale-epoch rejection and silent-refresh checks.

The first combined supported Release/CTest run,
`20261107-015319-e41b64e7`, built successfully and passed 85/87 checks (212.77 s).
The two failures were old IR presentation assertions: Learn expected the former
14-height help/outer-edge caption and responsive layout expected only the import
button to reserve an outer focus frame. Updated oracles retain all unrelated
label metrics and now require the approved 12-height helper, caption-to-visible
body alignment and matching selector/import bodies and frames. No production
behavior was weakened to pass these checks. A final complete run follows.

Run `20261107-015749-8826d700` subsequently reached a later Learn caption-gap
oracle that still measured to the outer selector frame. That assertion now
measures to the visible body (the approved 6-unit label gap); the outside focus
frame is reserved independently. This was a test-only correction. The previous
Learn font/alignment checks passed through to this later assertion.

### Final integration gate

The supported command `scripts/build.ps1 -Configuration Release -Test` passed
against committed runtime/test revision `8eca860`, warm retained Windows/MSVC
Release tree, run `20261107-020247-e6ab68c2`: preflight/configure/build all passed,
**87/87 native CTest checks passed**, 251.04 s, wrapper exit 0. This includes
SelectorPopup, actual SelectorBinding, ExportLevels within SwellUI, shared IR,
UiStandards, Perform navigation/bindings/reset/audition, ResponsiveEditor/Scale,
protected commands, audio, persistence and recovery regressions. The preceding
run finished 86/87, 243.45 s, with only the now-corrected caption-gap oracle.

Current merged main remains `89da1e7` (fetched and verified as an ancestor);
no active #108/#8 implementation was imported. UI standards passed, 71 files /
59 bounded occurrences, with no debt inventory regeneration. Both selector and
action-menu proposals pass strict validation. Guide prose was semantically
updated for delivered selection/focus/field-feedback behavior and its deliberate
review record passes currency checks. Change Markdown file targets and
`git diff --check` passed. Documentation/task-only completion commits follow
the tested runtime revision and do not require another native suite.

Native family acceptance covers the directly launched Windows preview and
automated component/editor/software-scale cases. Physical Windows DPI variants,
screen-reader application speech and FL Studio interaction have not been
retested for this selector change; no host or physical-DPI result is inferred
from the passing CTest suite. No release or installed host-test build is claimed.

### Separate action-menu handoff

`unify-action-menu-states` remains a separate, unapplied child (0/9). Begin its
implementation from the delivered selector adapter and current merged main;
extend ChoicePopup/PopupInteraction with action-row semantics rather than
replacing captured list/reset commands. Its native checkpoint and separate PR
remain required. Resolve the #108 KnobValue hookup overlap against that work's
delivered baseline; do not import active #108/#8 work. The umbrella #107 stays open.

The selector implementation is review-ready in
[PR #117](https://github.com/elneilios/ReverbArise/pull/117), targeting main with
Refs #107. The remote diff was checked against the named selector/shared-popup
scope; it contains no protected engine/audio/persistence, KnobValue, storage or
#108 implementation changes. The PR is open for owner review; spec sync/archive
and merge remain later delivery steps, not inferred from native family approval.

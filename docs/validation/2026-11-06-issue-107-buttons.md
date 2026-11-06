# Issue #107 command buttons — family checkpoint

Change `unify-command-button-states`; branch `codex/107-control-system-proposal`.
Apply baseline `93abe6c`, containing merged #80 `717c890` and approved contracts
revision 11. `git fetch origin main` confirmed #80 remains current main at the
start of Apply. No #108 or #8 worktree/branch is modified.

## Scope and execution map

| Tasks | Work / risk | Dependency and proof |
| --- | --- | --- |
| 1.1 | Source/evidence, low | Exact member/callback comparison, current-main check |
| 1.2–1.3 | Shared presentation/input, medium | Focused native render/input/IR/Export regressions before review |
| 1.4 | Owner family gate | Directly launched single native candidate preview; approval pending |
| 2.1–2.3 | Production command migration, medium | Blocked on family approval; actual-editor callback/identity tests |
| 3.1–3.3 | Integrated native/review/docs | Migrated consumers, supported wrapper and scope checks |

The 20 legacy commands match the design's source declarations. Existing callbacks
remain the authority: Add -> addTarget(current playhead); Delete -> removeSelected;
Regenerate selected/all -> generate()/generate(true); audition/Stop -> audition/stop;
suggest/confirm -> analyze captured span/confirm selected candidate; recapture/cancel
-> armSelected/cancelRecapture; Full response/Reset -> shapeReset(true/false);
Set bars -> applyBarRange; Clear map -> requestClearCapture; asset info -> existing
clipboard inventory; default IR -> Learn openDefaultIrFile (merged shared binding);
reset deleted targets -> resetSuppression; Open diagnostics -> setDiagnosticsOpen;
timing disclosure -> existing advancedTiming toggle/layout. Reset All is direct
resetSwell(selected ID,true), despite its resetMenu identifier.

The two latches retain existing guards: Snap uses captured navigation epoch/pass/
selection and requested/effective policy; audition Wet only uses captured session/
pass and accepted effective listening mode. No callback, eligibility refresh,
KnobValue/Level, storage/recovery, engine, audio, asset or persistence migration
is part of this isolated family checkpoint. Production legacy consumers and theme
name branches remain until owner family approval.

JUCE 9.0.4 source inspection found accessibility press uses nonvirtual triggerClick,
and accessibility toggle directly changes toggle state; both bypass the existing
ActionButton beforeInvocation capture. The candidate will share guarded activation
for pointer/keys/accessibility and test both press and toggle actions. Queued click
delivery already checks enabled state and component lifetime in JUCE; bindings
retain their own stale-context acceptance rules.

## Verification and approval

### Candidate implementation

The existing #80 adapter now delegates to one command state resolver/painter beside
the theme tokens, avoiding an include cycle. Explicit CommandVariant distinguishes
neutral, primary and destructive presentation; CommandIcon adds an explicit plus
glyph. LatchingActionButton supplies accepted toggle state to the same painter
instead of duplicating its active surface. Legacy theme branches/20 TextButton
members remain unmigrated at this checkpoint. No new visual token is introduced.

Neutral/primary retain existing roles; disabled state suppresses transient hover/
press emphasis. Destructive Reset content/outline remains danger, with contained
aqua keyboard focus. Latch off/on preserves neutral/aqua-fill state and readable
content. These revised combined/focus treatments are a **native review candidate**,
not covered by approval of the audit documents. Existing #80 neutral glyph policy
and FileDragTile's dashed/native-file variant remain regression references.

CommandButton is a small common invocation base for ActionButton/IconActionButton,
not a new command/model framework. Pointer captures on mouse-down; Return/Space
and accessibility press/toggle capture before queued JUCE click delivery. Enabled
and weak-lifetime checks surround invocation capture; existing binding snapshots
remain authoritative for stale context. Accessibility retains readable name/help,
checkable/checked state and read-only On/Off value for latches. Display is silent.

`Tests/CommandButtonCases.h` and `--test-command-buttons` cover combined state
resolution and contained rendering, icon/text edges, spoofed legacy names,
glyph/text relabeling, narrow add/play, actual native keyboard focus, long-text
accessible title, pointer/key/accessibility callbacks, stale simulated binding,
disabled-before-delivery, destruction during capture/click and queued lifetime,
silent accepted latches, accessible toggle state and FileDragTile save regression.
This entry point is also included in normal `preverb_swell_ui_test` execution.

`--review-command-buttons` opens one native state/interactive matrix, with logical
900x600, 1000x700, 1100x850 and 1400x1000 choices, 100%/125% user magnification,
main responsive or auxiliary comfortable text and explicit focus/scroll access.
The selector toolbar is outside the magnified specimen surface. This is a family
review, not integrated production composition, physical DPI or FL Studio proof.

### Native validation status

The first sandboxed wrapper configure failed before compilation because MSBuild
SDK registry reads were denied. Log: `out/build-logs/Release/20261106-173819-9ea21b48/`.
The same supported wrapper was retried with the required access; configure passed
and native compilation is in progress. Log:
`out/build-logs/Release/20261106-173846-ae5ae58e/`.
Compiler flags were inspected and retain `/EHsc`; no cache was deleted or repaired.
This is a cold local configure/build, not a measured warm improvement. Existing
source caches were inspected read-only; no other build tree/cache is copied or
mutated. No new native test success, host/DPI result or owner approval is claimed
until execution completes. The UI standards guard passes at the candidate source
boundary: 63 files / 59 bounded occurrences, with no baseline regeneration.

### Focused native checkpoint — passed

To avoid waiting for unrelated cold test-target compilation before family review,
`out/command-family-preview/build.cmd` builds a thin native runner of the same
CommandButtonCases, ExportControlCases and ConvolutionIrCases, using generated
Release include/definition/link settings and already-completed libraries read-only.
Its own main/events objects and executable stay outside the active wrapper tree;
no competing CMake/MSBuild touches that tree. It compiles JUCE events with the
fixture's modal-loop flag and retains an 8 MiB test stack. This is **MSVC native**
verification, distinct from wrapper/CTest, which is still in progress.

Runner build and all three focused case groups **passed, exit 0**. Invocation:
`out/command-family-preview/command-family-preview.exe`. Initial runner wiring
needed the unchanged native fixture helpers and a separate link response; no
production fix resulted from those harness errors. Two fixture assertions were
corrected: primary Play's dark center is expected, and JUCE accessibility handlers
require a native peer before querying. The final run includes real focused
components and accessible press/toggle, not resolver-only checks.

Tasks 1.2/1.3 are verified at this family checkpoint. No claim of full CTest,
integrated legacy migration, new FL Studio/DPI coverage or owner approval is made.

### Owner review window — open, approval pending

The prepared runner was launched directly with `--review-command-buttons`, PID
69288. Its checks passed again before opening the single review window; output
is retained in `out/command-family-preview/review.stdout.log` / `review.stderr.log`.
Computer Use window enumeration confirmed one returned #107 review window (ID
11862722); automated capture/activation then timed out awaiting app access, so
no agent screenshot inspection is claimed. The running window is preserved for
the owner and no duplicate was launched.

The running fixture's UTF-8 dash in its heading/title was decoded incorrectly by
the thin runner's default source charset. Source captions were changed to ASCII
for future builds; button rendering/behavior is identical. The existing review
window was not closed/replaced for that caption-only correction.

Progress is **3/10**, task 1.4 pending explicit family look/feel approval. Production
legacy member migration and removal of name-based theme branches have not begun.
The full cold wrapper build is still running, separate from focused native success.
UI standards, strict OpenSpec and whitespace checks pass. A scoped diff confirms
no Plugin/KnobValue/IR-binding/DSP/session edits, and no #108/#8 worktree changes.

### Owner feedback and revised candidate — 6 November 2026

The first candidate's supported Release wrapper subsequently completed **75/75
CTest passed**, 242.01 seconds, exit 0, logs `20261106-173846-ae5ae58e`. This is
the result for the first candidate, not the visual revision below.

The owner requested distinguishable hover/mouse-down/focus, destructive hover and
press feedback, and suggested icon colours following interaction accents. The
review gate remains pending: positive feedback plus requested changes is not final
approval of the revised look/feel.

Revision 2, using existing tokens only:

- Hover uses the muted aqua outer outline; focused controls retain a separate
  contained inset focus mark, so focus cannot mask hover/held state.
- Held neutral commands use separator fill and a stronger aqua outline; primary
  and on-latch hover/press fill/outline differ visibly.
- Destructive hover also gets the muted aqua outline, keeping danger text;
  mouse-down uses danger fill with contrasting dark content and strong outline.
- Neutral icons follow hover/press/focus accents. Icons on bright filled primary/
  latched surfaces remain dark for contrast. FileDragTile's specialized paint is
  unchanged. This is an explicitly requested candidate revision to #80's original
  stable-glyph policy, not a silent expansion of that approval.

CommandButtonCases now checks the revised combined-state precedence, glyph colour,
visible hover-versus-held distinction and actual independent focus stroke.
ConvolutionIrCases keeps surface parity and revises its scoped glyph oracle to
require accents for enabled hover/held and neutral ink when idle/disabled.
The updated native runner **passed, exit 0**, with command/Export/IR groups.
It is `out/command-family-preview/command-family-preview-v2.exe`; generated source,
compiler/link response files and logs remain in the ignored preview directory.

The previous window was retained while the new binary was compiled and tested.
Its executable was locked by the running review process, so output was redirected
to v2. After verifying its tests, only the exact known old review process was
replaced (executable path and --review-command-buttons arguments checked).
Revision-2 preview PID **42280**, returned window ID **10820270**, ASCII title
`ReverbArise - #107 button family review`. Window enumeration confirms a single
review window; no automated screenshot inspection is claimed. Its own pre-review
checks passed again; review logs are `review-v2.stdout.log`/`review-v2.stderr.log`.

Supported incremental Release build for revision 2 **passed, exit 0**, logs
`out/build-logs/Release/20261106-194925-bdb15a83/`; CTest was not requested in this
build-only pass. Focused native success above is separate. Strict OpenSpec, UI
standards (63/59, unchanged debt) and whitespace pass. No callback/model/audio,
#108 or #8 change was introduced by this visual refinement. Progress remains
**3/10** until scoped revised family approval; production migration stays paused.

### Revision 3 — requested outer aqua corner brackets

The owner selected the knob-style four-open-corner pattern for button focus,
always aqua, with a small outside gap. They explicitly requested the preview
update and left the knobs unchanged. The button painter now reserves existing
focus stroke plus focus-outset space around the body: a 2-pixel aqua bracket
stroke, a 3-pixel clear gap to the border and short legs using the knob pattern's
20% of the smaller frame dimension. All bracket ink remains inside the component.
The knob source is not edited or coupled to this implementation.

The native preview reserves this frame around the existing 34-pixel visible body,
rather than shrinking that body. Current production layout/migration remains a
separate gated task; this preview is not an integrated-size acceptance claim.
Hover/held border/background and revision-2 glyph behaviour remain intact. Focus
brackets stay aqua even on bright primary or destructive fills; no dark inset
ring remains. FileDragTile retains its separate specialized painter.

Focused MSVC command/Export/IR cases **passed, exit 0**, via
`out/command-family-preview/command-family-preview-v3.exe`. Tests verify bracket
colour, open sides, clear gap, no clipping, actual native focus and pixel-identical
body rendering with/without focus at each mouse state. Existing keyboard,
accessibility, lifetime and binding cases also pass. UI guard (63/59), strict
OpenSpec and whitespace pass. No wrapper/CTest rerun is claimed for this preview
refinement; prior revision-2 Release build and initial 75/75 results remain
separately scoped above.

The previous preview was retained until v3 compiled and passed checks, then only
its exact verified review process was replaced. New review logs:
`review-v3.stdout.log` / `review-v3.stderr.log` in the preview directory. Revised
look/feel acceptance remains pending at task 1.4; no legacy production migration
or #108/#8 change has started.

### Revision 4 — accepted brackets, red hover, neutral icons

The owner explicitly likes revision-3 corner-bracket focus; retain that scoped
acceptance without asking them to repeat it. They rejected aqua on destructive
hover borders and aqua icon hover. Revision 4 keeps destructive border/content in
the existing danger palette: idle red rule, stronger red hover stroke, red held
fill with contrasting content. Neutral icon ink remains off-white across hover,
press and focus; primary filled glyphs retain their dark contrast. Body mouse
feedback stays distinct from the unchanged outer aqua focus brackets.

The candidate uses existing tokens/strokes; knobs, bindings and command behaviour
are unchanged. CommandButtonCases verifies steady neutral glyphs and red hover
with visible stroke/held-fill differences. ConvolutionIrCases restores the neutral
glyph oracle, sampling inside the button body to exclude its independently
coloured border/focus. Focused native command/Export/IR groups **passed, exit 0**,
using `command-family-preview-v4.exe`. UI standards (63/59), strict OpenSpec and
whitespace pass; no full wrapper/CTest rerun is claimed for this preview-only
refinement. Prior build/test results remain scoped to their recorded revisions.

The v3 review process was preserved until v4 compiled and passed, then replaced
only after exact executable-path/argument verification. Review logs are
`review-v4.stdout.log` / `review-v4.stderr.log` under the ignored preview directory.
Full revised-family approval is still pending task 1.4; progress remains **3/10**.
No legacy production migration or #108/#8 edit is included.

### Family approval

The owner explicitly approved revision 4: “this is perfect and looks good”.
This accepts the outer aqua corner-bracket focus, gap, independent body hover/
held feedback, red-only destructive body states and steady neutral glyphs.
Task 1.4 is satisfied; production adoption may proceed within the named scope.
Prior accepted knob focus and #108/#8 boundaries remain unchanged. Integrated
composition/size acceptance remains separately tracked at task 3.2.

## Production integration after family approval

The 20 listed persistent TextButtons now use explicit ActionButton roles;
Snap/listening latches share the resolver without binding changes. The production
source diff is limited to command declarations and shared command/theme headers.
PluginEditor.cpp, PluginSettings.cpp, knob/value controls, DSP, persistence and
session models are unchanged. Inventory P1/P2/S2/D1 records adoption; other
families remain independent. No other worktree was edited.

The neutral generic theme fallback replaces all name-based command painting,
including orphaned loadIR. Native fixtures cover Rename-style OK/Cancel rendering
and one-shot delivery, name spoofing, stable glyphs and accessible full labels.
Actual-editor fixtures exercise Add/Confirm/Delete, regeneration, Play/Stop,
Snap and reset keyboard paths alongside retained IR/Export/navigation oracles.

Supported wrapper attempts (warm Release tree):
- `20261106-211835-87c62d55`: configure passed; build caught two test calls
  through JUCE Button's protected keyPressed. Corrected to shared public API;
  no CTest result claimed.
- `20261106-212045-acff4d99`: build passed; 72/75 CTest passed (233.38 s).
  Learn About still expected old held-fill colour; updated to the approved aqua
  held state. The Add test's generic `+` lookup matched Zoom; replaced with its
  stable component identity. The knob-popup fixture reported peer focus but no
  Windows foreground ownership; knob source was not changed. The exact approved
  v4 review process was closed before the next run, after owner acceptance.

Revision 12 updates the standards and C1 contract from approved revision 11,
using existing tokens. The guide documents Tab/Return/Space, independent focus,
red Reset mouse feedback and unavailable commands; its semantic review is
recorded through the guide-maintenance script. UI guard remains 63 files/59
bounded occurrences, with no debt-baseline regeneration. Physical DPI and FL
Studio checks are not represented by these native/software results.
- `20261106-212533-0175be3a`: build passed; 74/75 CTest passed (230.84 s).
  The knob foreground fixture passed unchanged. The remaining failure was the
  existing direct title assertion for Copy diagnostics after the intermediate
  accessible-label change. The final implementation retains default titles,
  follows text changes for text commands, preserves explicit custom titles and
  keeps meaningful glyph-only titles. The final wrapper run includes this fix.

The integrated native launcher is `preverb_perform_composition_preview.exe
--review-command-integrated`: it uses the actual production editor, supported
size presets (900x600, 1000x700, 1100x850, 1400x1000), Settings Appearance scale,
long-label/empty scenes and a selectable invalid-IR error from a temporary fixture.
Production bounds reserve the focus gap within their existing component frames;
the final integrated gate checks the resulting fit separately from the family's
larger specimen frames. No owner is asked to repeat passing audio/identity tests.

Final text-fit review found JUCE's native AlertWindow buttons are 28 pixels high.
The neutral fallback now draws normal-size text within the existing text padding,
without reducing font size to fit the reserved focus area. The fallback fixture
covers both 28- and 34-pixel frames. The preceding complete wrapper run
`20261106-213103-5728a9ce` passed 75/75 CTest in 223.89 s; its command family,
production bindings and layouts are unchanged by this generic text-fit correction.

The actual-editor review was directly launched from an immutable copy of that
passing build at `out/command-integrated-review/command-integrated-review.exe`,
argument `--review-command-integrated`, PID 70824, with stdout/stderr in the same
ignored directory. This avoids locking the active wrapper outputs. The isolated
v4 family window had already been closed after acceptance, so there is one
owner review window. This snapshot precedes only the neutral Rename text-fit
refinement; the scoped production command family/layout is identical. Integrated
owner acceptance is pending task 3.2. No new FL Studio or physical OS-DPI result
is claimed, and no concrete unresolved host behavior currently requires another
host smoke test.

### Final current-source verification

`20261106-213656-8866ac54`: supported Release wrapper configure/build passed;
74/75 CTest passed in 219.36 s. All changed command/component/editor, IR, Export,
responsive, audition, audio and persistence cases passed. The sole failure was
unchanged `preverb_perform_knob_reset_test`, before popup behavior, because the
fixture reported `peer=1 foreground=0`. The same fixture passed in each of the
preceding two complete wrapper runs. A standalone current-binary
`--test-knob-resets` retry again failed only that foreground precondition; log:
`out/command-integrated-review/knob-focus-retry.log`. Do not report the final run
as 75/75. Task 3.1 remains pending a successful current-source focus retry after
the owner finishes the review window. No knob or focus-test source was weakened.

Task 3.2 remains pending scoped actual-editor acceptance. The native review stays
open. Strict OpenSpec for both changes, UI standards, guide currency, changed
Markdown links and whitespace checks pass. No debt inventory was regenerated.

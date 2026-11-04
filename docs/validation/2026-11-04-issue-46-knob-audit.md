# Issue #46 — knob standards and guardrail audit

Subsequent audition refinement/build identity is recorded in the
[current audition report](2026-11-04-issue-46-audition.md); knob approvals below
remain valid, and their older bundle hashes are historical.

Current status: the owner approved the revised Level steps and corner focus.
The reset follow-up below is implemented, automated checks pass and the owner
approved its native interaction. Only final FL Studio smoke remains pending. Earlier audit
and checkpoint outcomes below are historical.

4 November 2026. Reviewed source at `cfb4d26`; this audit makes no production
changes. The owner approved adding Snap/grid to timing-knob drag/wheel input and
requested this review. The existing plan now records that extension as task 4.4;
it is not implemented or covered by the previous integrated native acceptance.

Authorities: [UI development standards](../development/ui-development-standards.md),
[production guide revision 3](../design/ReverbArise_UI_Layout_and_Style_Guide.md)
and the approved #46 exact-entry/Ctrl/Shift policy.

## Findings, in priority order

### P2 — inspector helper labels squeeze long text

The owner's side-by-side screenshot shows horizontally compressed status/IR help
text in the native preview. Both labels receive the approved helper role (Inter
Regular 14), but remain ordinary JUCE Labels with default fitted-text scaling.
Long selected names and retained/requested IR descriptions therefore compress to
fit the one-line width. Shorter release-build strings do not trigger the same
compression. This is a text-fitting defect, not an intentional condensed font or
approved density change. Keep the role's font size and horizontal scale, ellipsize
overflow and retain the full string in tooltips/accessibility. The selected-status
tooltip currently prioritizes status/calibration and does not guarantee the full
displayed name. Add long-text native/pixel or font-scale checks for these labels;
the existing geometry-only assertions did not detect the defect.

Sources: [PluginEditor.cpp](../../Source/Plugin/PluginEditor.cpp), helper-role
assignment line 264 and status updates lines 472–482; pinned JUCE Label default
`minimumHorizontalScale = 0` and LookAndFeel_V2::drawLabel fitted-text path.

### 1. P2 — active dial feedback can disagree with accepted state

`KnobValue::display` updates canonical text/fraction but deliberately skips moving
the dial while `active`; `onValueChange` discards the binding result, and only
`finish`/`restore` returns the dial to the accepted fraction. A rejected or clamped
gesture can therefore show an unaccepted dial position while the field/canvas
show the accepted state, until release. Snapping would make this disagreement
routine. This violates the requirement that dial, text and canvas agree during
gestures. Preserve a separate pointer/delta accumulator while displaying accepted
state; prove rejected/no-grid and snapped feedback with real drag events.

Sources: [KnobValue.h](../../Source/UI/KnobValue.h), `onValueChange` lines 38–43,
`display` lines 58–64 and `restore`/`finish` lines 84–92.

### 2. P2 — inspector invocation guards lag behind canvas guards

`beginValueEdit` captures primary swell, input generation, workflow pass and raw
bounds, but no assignment epoch/full selection or raw-render identity.
`commitValueEdit` checks primary/input and `sameMusicalMapping`, which does not
validate PPQ/sample origin; it then calls the unscoped `engine.edit` overload.
The selection check is outside the edit transaction. An origin change with the
same rate/tempo/meter passes this check, and a same-ID/same-generation engine
replacement is not explicitly rejected by epoch. These are concrete guard gaps;
this audit does not claim a reproduced DAW project corruption. Timing-knob Snap
must adopt the existing frozen fixed-mapping and epoch/selection transaction
guards, with tests for origin changes and same-ID replacement. Exact-entry
ownership must remain protected too.

Sources: [PluginInspector.cpp](../../Source/Plugin/PluginInspector.cpp),
`beginValueEdit` lines 20–27, checks lines 29–37 and `engine.edit` line 74;
[TransportTime.h](../../Source/Core/TransportTime.h), `transportMappingMatches`;
[SwellTimeline.cpp](../../Source/UI/SwellTimeline.cpp), `prepareDrag`/`dragTo`.

### 3. P2 — rotary controls lack explicit keyboard focus/feedback

KnobValue uses a plain JUCE Slider and never enables keyboard focus on the dial.
The pinned JUCE 9.0.4 `Slider::init` sets `setWantsKeyboardFocus(false)`.
`WorkspaceTheme::drawRotarySlider` draws no focused state, and the cell supplies
no focus container/ring. The exact TextEditor remains keyboard reachable, so this
is not a claim that the parameter has no keyboard alternative. The dial itself
has no intentional Tab/arrow/focus policy matching the library guardrail. Define
and test a deliberate keyboard path and contained focus ink before calling the
rotary family fully compliant. A visual change needs focused owner acceptance.

Sources: [KnobValue.h](../../Source/UI/KnobValue.h), constructor;
[WorkspaceTheme.h](../../Source/UI/WorkspaceTheme.h), rotary painter lines 189–201;
pinned local JUCE `juce_Slider.cpp`, `Slider::init` line 1461.

### 4. P2 — accessibility exposes internal fraction and identifier

`configure` names dial/entry with identifiers such as `sourceSampleStartKnob`;
there is no explicit human-readable title or label association. The slider uses
range 0–1 and no binding-owned domain formatter. JUCE's default accessible slider
value/range therefore describes the internal fraction rather than samples,
musical position, milliseconds or dB. The separate entry contains formatted text
but does not give the dial correct domain semantics. Add friendly names/units and
an accepted domain-value accessibility adapter without moving domain logic into
the library. Verify handlers programmatically; screen-reader use is still untested.

Sources: [KnobValue.h](../../Source/UI/KnobValue.h), range line 29 and configure
lines 51–56; [PluginInspector.cpp](../../Source/Plugin/PluginInspector.cpp),
`refreshValues`; pinned JUCE `SliderAccessibilityHandler::ValueInterface`.

## Compliant mechanisms retained

- The 11 Perform inspector cells share KnobValue interaction/edit lifetime rather
  than duplicating bindings per screen. Domain conversion/validation stays in
  PluginInspector; the component has no processor/audio/render dependency.
- Caption uses embedded Inter Medium 14 / 24 box; value uses Regular 13 / 28 box.
  Long captions ellipsize with full tooltip, and actual production rows use the
  approved 64-high composition. No new tokens or font shrink are proposed.
- Active typed text survives refresh. Return/focus-loss commit and Escape rollback
  exist. Silent display suppresses value callbacks. Existing primary/input/pass
  checks and canonical setters already provide partial stale-edit protection.
- Existing native tests exercise actual coarse/fine pointer input, wheel direction,
  limits/disabled wheel, text refresh/Return/Escape, selection/deletion rejection,
  exact large sample values, source identity and production inspector bounds.
- The two legacy marker/regeneration rotary sliders use separate setup. The
  Perform marker band is hidden; regeneration is disabled for compatibility.
  They are retained legacy consumers, not evidence of a completed global knob
  migration or a reason to change Learn/Export in this work.

## Coverage and next steps

Read-only source/test review plus `scripts/check-ui-standards.ps1`: passed,
44 files / 61 bounded occurrences. Lint checks syntax/debt inventory; its pass
does not establish focus, accessibility semantics or transaction correctness.
The previous 52/52 Release native result remains historical implementation
evidence; no new build, test, performance or DAW pass is claimed for this audit.

The present tests do not establish snapped/rejected active-dial feedback,
Ctrl+Shift or modifier rebasing on knobs, native rotary keyboard focus,
domain-correct accessibility values, same-ID restore or fixed-origin changes
during inspector edits. Some unit cells still use an old 50-high fixture, while
the caption/value line boxes total 52; production 64-high bounds already pass.
Use representative approved dimensions in new family tests.

Task 4.4 records the approved snapping work and its binding tests. Findings 1/2
directly affect safe integration. Findings 3/4 need a bounded shared-family
remediation proposal and focused native review, not an unreviewed global restyle.
No audit findings have been silently fixed. Preserve existing layout/button
acceptance and consolidate host checking with the still-pending final smoke.

## Authorized remediation — 4 November 2026

The owner subsequently explicitly requested the font fix, all audit corrections
and continued Apply of task 4.4. They approved the fixed step/precision policy,
then clarified that ordinary users should see musical position or time, not
samples. Implementation source is `a540bf71ca7fabdf449117139fbb926793eb8462`.
The original findings above remain the record of the reviewed pre-fix source.

| Finding | Implemented correction | Native evidence |
| --- | --- | --- |
| Helper font fitting | Shared EllipsisLabel for selected-status and retained/requested IR; approved Inter Regular 14 retained, full displayed status/name in tooltip | Equal prefix glyph pixels for long/short text; fixed face/height assertions; actual dense editor renders |
| Accepted dial feedback | Dial paints canonical fraction while JUCE keeps its raw drag accumulator; pointer/modifier rebasing remains independent | No-grid rejected drag leaves dial pixels identical and accepted value unchanged; Ctrl resumes valid editing |
| Invocation guards | Per-field session epoch/full selection/time/accepted mapping/input/render/frozen guards, transaction recheck and expected-epoch edit | Same-ID restore, secondary selection, mode switch, actual source revision and fixed-origin change reject stale intent |
| Rotary keyboard/focus | Dial is Tab reachable; arrows use binding steps; focus loss cancels; existing 2-pixel aqua inside-stroke policy | Native focus acquisition and focus-pixel assertion, keyboard input and cancellation |
| Accessibility | Friendly parameter/unit titles and accepted domain values/ranges/text; exact-string setter retained | Actual accessibility handler title/value/setter assertions; no 0–1 pointer fraction exposed |
| Units/steps/precision | Binding-owned fixed wheel/arrow policy and explicit units, formatted precision separate from saved values | Zoom-independent 1 ms wheel, 0.1 ms fine rounding, adjacent-grid steps, Ctrl+Shift, 0.1 dB and 0.001 fine curve checks |

The shared `UI/TimingEdit.h` extracts the existing canvas candidate policy for
both consumers. Move Wet snaps audible start once; start/end/fade knobs snap
absolute displayed anchors, not durations in isolation. Curves/level and typed
values remain independent of the grid. Existing canonical bounds/zero-fade rules
and HostGrid remain authoritative. No DSP, source capture, state schema, Export
workspace or ActionButton behavior changes.

Ordinary positions use `bar.beat.fraction`; movement/durations use ms/s. The old
unsaved sample-unit toggle is hidden in Settings and retained only as an internal
precision regression seam. Compact captions `Target (bars)`, `Src start (bars)`
and `Src end (bars)` fit fully; their titles/tooltips retain the complete source
name and format. Unitless curve range is in the full title/tooltip. Every primary
knob caption has an actual embedded-font-width assertion at minimum editor size.
No typography or geometry token changed, and the lint debt inventory is unchanged.

The [living value table](../development/perform-navigation.md) records units,
normal/fine increments, display precision, storage and range validation. Samples
remain internal: timing steps round to at least one sample; formatting cannot
discard accepted typed/saved precision. Musical fractions retain the existing
up-to-nine-digit authority; level retains its existing float representation.

### Validation and remaining acceptance

- Focused MSVC/CTest: 4/4 passed (22.31 s), logs `20261104-knob-build-v5.log` /
  `20261104-knob-tests-v5.log`; added actual pointer fine/combined and unavailable
  mapping cases subsequently passed in the full suite.
- `scripts/build.ps1 -Configuration Release -Test`: 53/53 passed, 246.28 s,
  `out/build-logs/Release/20261104-181026-7af1da62/`. This includes the original
  60-case/240-render native matrix and affected audio/state/source regressions.
- Final concise-caption refinement and full caption-width assertions: focused
  4/4 passed, 12.08 s, `20261104-knob-label-build.log` / `20261104-knob-label-tests.log`.
  Committed source `a540bf7` then passed the wrapper and all 53 tests in 167.97 s
  (`20261104-181838-54af1dda`).
- Final accessibility boundary review added validation of unavailable/non-finite
  domains and constant singleton ranges. Unavailable values expose no ranged
  interface and remain read-only; a valid constant keeps its accepted text.
  Focused native 4/4 passed in 12.46 s (`20261104-knob-accessibility-*`). This
  final guard is committed in `24ffe7b`; its final wrapper result is below.
- The `24ffe7b` full run passed 51/53 in 266.60 s
  (`20261104-182437-1d79416c`): the composition fixture failed before its first
  manual target (`engine.addTarget(48000)`), and the unchanged processor test
  timed out in `MultipleExportCases.h:18`. The new knob test passed. The fixture
  had fed 48 capture blocks at a 1-ms sleep, much faster than real-time delivery;
  it now flushes the capture worker per block rather than relying on wall time.
  No capture/Export implementation or test timeout was changed. All five focused
  editor/knob/canvas/processor checks then passed in 22.21 s
  (`20261104-knob-final-guard-*`).
- Final source `a4e9262` also preserves JUCE's own modifier-change hook alongside
  the accepted-value rebase. This keeps native fine-drag/mouse restoration under
  JUCE's existing ownership. Only docs/evidence remain dirty during final builds.
- UI standards passed: 46 files / 61 bounded occurrences. Strict OpenSpec passed
  20/20; diff checks passed. Warm toolchain/dependency sources retained; no speedup
  or cold-build claim. No Debug full build or Windows screen-reader pass claimed.
- Early build errors were incorrect namespace qualification and a JUCE Optional
  method name, both corrected. One new stale-source test initially tried changing
  an engine-owned generation counter directly; the canonical engine correctly
  recomputed it. The test now changes actual source bounds. A hidden Settings
  control lookup was corrected to test the ordinary musical caption. Earlier
  failing logs remain under `out/build-logs/Release/20261104-knob-*`; assertions
  for real stale edits, identity and source preservation remain intact.

Prior compact-layout, Snap-button and integrated pre-knob approvals are retained.
The changed knob look/feel still requires its focused native acceptance, followed
by the existing single FL Studio smoke on the final bundle. Automated/native
evidence does not claim actual Windows DPI, hardware wheel feel or screen-reader
acceptance. Task 4.4 stays open until its native review requirement is met.

Actual final-source native captures:
[selected](2026-11-04-issue-46/knobs/selected-1100.png),
[dense long names](2026-11-04-issue-46/knobs/dense-1100.png),
[empty swells](2026-11-04-issue-46/knobs/empty-1100.png).
The review executable is `out/review-issue46-knobs-final/perform-knob-review.exe`
and is refreshed from the final validated source before owner review. The final
source/bundle/preview hashes and full-suite result are recorded below.

### Final delivery validation

This records the full integration run before the owner's two focused refinements.
The current revision, focused results and bundle identity are in
[Requested revision](#requested-revision-level-increments-and-corner-focus) below.

Final implementation/test source: `a4e926262b0fff05f935a7be0bb3e2f0a4bb913d`.
`scripts/build.ps1 -Configuration Release -Test` passed configure/build/test,
exit 0, **53/53 tests**, **184.36 seconds**. Logs:
`out/build-logs/Release/20261104-183354-95be167a/`. The wrapper reports
`a4e9262 (dirty)`; only Markdown/evidence was dirty, with no subsequent production
or test changes. Final UI standards: 46 files / 61 bounded occurrences; strict
OpenSpec: 20/20; diff checks pass. No full-suite repetition is required for the
subsequent evidence/task checkbox commits.

Final Release bundle, not installed automatically:
`C:\Users\neila\.codex\worktrees\670e\k7ma-preverb\out\build\windows-msvc\preverb_plugin_artefacts\Release\VST3\ReverbArise.vst3`.
Inner `Contents/x86_64-win/ReverbArise.vst3` SHA-256:
`799F151F63EC68E887695307D697198E9191DE6A4CAD456A8737140A83D2DD32`.
Final native review executable SHA-256:
`951C16ADDBF9C3EF4F81198E6C334BB00820F784317780C8F9D37B7CB433F801`.
The directly launched window is **Issue #46 - Integrated Perform review**;
`perform-knob-review.exe --review-integrated`, PID 50500 at launch. It is preserved
for the owner's focused knob review. The captures linked above were refreshed
from this exact executable. The ordinary header/layout/Snap-button acceptance is
retained; only materially revised knob interaction needs this checkpoint.

Owner response to this focused checkpoint: **Request changes**. Specific changes
have been requested from the owner and are pending. The review window remains
open. Task 4.4 is not complete; the changed knob family is not accepted yet.

### Requested revision: Level increments and corner focus

The owner clarified two changes: Level wheel changes should be 1 dB normally and
0.1 dB with a modifier; knob focus should use four corner brackets. Their supplied
reference shows brackets around the dial. Shift remains the fine modifier, keeping
Ctrl available for Snap bypass. Normal/fine arrow steps match wheel steps; typed
precision remains two displayed decimal places with unchanged saved precision.

Revision source `b2d2ec3` changes only Level discrete-step/range metadata, the
KnobValue focus painter and associated tests/captures/guidance. The bracket variant
uses existing aqua and 2-pixel contained stroke, open sides and 20%-side-length
legs (8 pixels on the 42-pixel dial). Standards revision 4 records its exact scope;
other families and the accepted layout retain their previous focus treatment.

Focused native MSVC/CTest: **4/4 passed**, **11.09 seconds**, logs
`out/build-logs/Release/20261104-knob-bracket-build.log` and
`20261104-knob-bracket-tests.log`. Actual Level wheel/arrow events prove +1 dB,
Shift +0.1 dB and Shift-arrow -0.1 dB. Native focus pixels prove all four corners
are visible and the sides are open. [Focus detail](2026-11-04-issue-46/knobs/corner-focus.png)
and [actual editor context](2026-11-04-issue-46/knobs/corner-focus-context.png)
are native component captures; the detail is rendered at software 300% for review.
UI standards remain 46 files / 61 bounded occurrences; strict OpenSpec 20/20 and
diff checks pass. No lint inventory or unrelated component was changed.

The prior full 53-test result at `a4e9262` is retained for unchanged backend,
state/identity and waveform paths. This focused UI revision uses the affected
native regressions and a Release wrapper rebuild; the full suite is not repeated
for step-size and focus-paint-only changes. Native owner acceptance remains pending.

`scripts/build.ps1 -Configuration Release` passed configure/build, exit 0, from
clean source `b2d2ec3d7d4c554c3e10c216b36d5fe27a1f322c`; tests were not requested
by this build-only wrapper invocation. Logs: `20261104-185201-7b340127`.
The focused native 4/4 result above supplies the affected interaction coverage;
the prior full 53/53 result is retained only for unchanged paths.

Updated Release bundle remains at the same workspace VST3 path. Inner binary
SHA-256: `BC74A4D9B192D00676D9BEF4F301C9EA1A4B57BBD4E4C12E77B2172B02AAB8C8`.
The revised native window was directly opened as PID 55860, title
**Issue #46 - Integrated Perform review**, from
`out/review-issue46-knobs-corners/perform-knob-corners.exe --review-integrated`.
Preview SHA-256: `2650674A10E925B1FF94B0BD759D58121B043DE60D9CB308A5B92BE7D70ADCE7`.
The prior window was already closed; no duplicate review window was opened.
Task 4.4 remains unchecked pending this revision's native owner acceptance.

The owner subsequently approved the revised Level increments and corner brackets:
“the new changes are approved”. Task 4.4 is verified complete; prior layout and
Snap-button approvals are retained. Their additional double-click/context-menu
reset request is separate follow-up work, not rejection of this revision.

### Reset follow-up: defaults and current-editor initial positions

Owner-approved semantics: Target and Source Start/End restore the first positions
loaded for that swell in the current editor. Other controls use existing model
defaults, with Pre-roll returning to automatic IR start. Initial coordinates are
per-ID/epoch editor memory only; schema 10/envelope 8 remain unchanged.

KnobValue owns double-click, the existing-themed one-action popup, SafePointer and
invocation tickets. The Perform binding captures selection/session/input/render/
workflow/fixed-timing context, then rechecks the canonical edit transaction. Reset
bypasses Snap and modifiers. Related envelope values retain canonical constraints;
invalid source ordering rejects without mutation. Already-default actions do not
dirty the host or schedule renders. Exact text fields retain their native menus
and text-selection double-click.

Focused native MSVC/CTest: **5/5 passed in 13.16 seconds**, including the new
`preverb_perform_knob_reset_test`, existing knob, binding, gesture and swell UI
checks. Logs: `out/build-logs/Release/20261104-knob-reset-build-v4.log` and
`20261104-knob-reset-tests-v4.log`. Tests exercise real two-click sequences,
keyboard selection in the asynchronous popup, menu cancellation, defaults with
Snap On, per-bound initial restoration, constrained rejection, same-ID epoch
replacement, no-op dirty/jobs, disabled controls and stale selection/session/
mode/source/timing/other-knob/freeze/destruction actions. A real open popup is
also dismissed after its editor is destroyed. Capture revision is preserved.

The initial popup test exposed hidden Level placement/focus in the test fixture;
scrolling the inspector and focusing the right-clicked dial fixed that setup.
A new pointer-position assertion then caught a point-pair rectangle being used as
a size; using explicit x/y/1/1 anchors the menu correctly. The final five-test run
passes both checks. [Native popup capture](2026-11-04-issue-46/knobs/reset-menu.png)
is the actual component image, not a mockup.

UI standards: **46 files / 61 bounded occurrences**, unchanged debt inventory.
Strict OpenSpec validation and `git diff --check` pass. ActionButton remains
Agent 1's accepted SHA-256
`68EF2F339109F65C1044813C1CD074C5AA13E8DEE7470571452ECF2B1EEAAA16`;
ExportWorkspace, TimelineNavigation and PluginProcessor.cpp remain unchanged
against merged Export `9b8e70f`. No shared Export-specific reset behavior exists.

No repeat full backend suite is claimed for this UI refinement. The prior native
**53/53** run at `a4e9262` remains evidence for unchanged audio/state/export paths.
Actual Windows DPI, screen-reader and FL Studio coverage remain unverified.
Task 4.5 awaits focused native reset acceptance; 6.2 awaits the final host smoke.

Reset source revision: `a918f808514d12c902b19263cbc1b5c1158666eb`.
`scripts/build.ps1 -Configuration Release` passed from that clean revision, logs
`out/build-logs/Release/20261104-192615-48426eb1`; tests were not requested by this
build-only invocation. The five native tests above ran separately on the same
source. Final strict OpenSpec: **20/20**, zero failures.

Current Release bundle (not automatically installed):
`C:\Users\neila\.codex\worktrees\670e\k7ma-preverb\out\build\windows-msvc\preverb_plugin_artefacts\Release\VST3\ReverbArise.vst3`.
Inner `Contents/x86_64-win/ReverbArise.vst3` SHA-256:
`1A38813E7960935DD058C18C48F5ED60903B0C6C4B4467AEDB8DDB8A78AB6BD1`.

Directly launched native reset preview: **Issue #46 - Integrated Perform review**,
PID 44660 at launch, responding; `out/review-issue46-knob-reset/perform-knob-reset.exe
--review-integrated`. Executable SHA-256:
`CD4AE09F88F8AA2A5AD5A7D6974A6276FB1E98D30CF74F971A351C2AD3B248A8`.
The prior approved preview was closed before replacement; the reset preview is
preserved for the owner. Prior layout/Snap/knob steps/focus acceptance remains
valid. OpenSpec is **17/19**: reset acceptance (4.5) and host smoke (6.2) pending.

Owner response to the reset checkpoint on 4 November 2026: **Approve knob reset;
FL Studio smoke pending**. This completes task 4.5; refreshed Apply instructions
confirm **18/19 complete**. Prior approvals remain recorded. Only task 6.2's final
host smoke remains. Source, tests and the validated bundle hash above are unchanged;
no native rebuild or test repetition is needed for this approval-only update.

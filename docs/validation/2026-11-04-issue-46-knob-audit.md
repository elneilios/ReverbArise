# Issue #46 — knob standards and guardrail audit

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

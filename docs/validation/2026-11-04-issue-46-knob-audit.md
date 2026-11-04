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

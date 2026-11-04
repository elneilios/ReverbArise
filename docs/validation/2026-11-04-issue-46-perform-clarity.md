# Issue #46 — Perform clarity checkpoint

The subsequent [bottom hover-help checkpoint](2026-11-04-issue-46-hover-footer.md)
supersedes this revision's popup tooltip delivery. Earlier six-part refinements
and the historical test results below remain; task 5.4 still awaits acceptance.

4 November 2026. The owner requested six refinements while preserving three
columns and the 52-pixel audition strip. This extends pending task 5.4; it does
not discard previous editing, Snap, knob, reset or audition-audio validation.

## Proposed presentation and shared boundaries

1. The existing latching audition button names the selected mode: **Dry + wet**
   when Off, **Wet only** when On. Status reports readiness/playback/membership;
   accessible title and tooltip name the mode and next action. Button binding,
   saved legacy behavior, gain/protection, loop and Stop semantics are unchanged.
2. Perform composes the unchanged **MusicalTimelineRuler** painter/tick generator
   with its existing SwellViewport/SongTime. The ruler and both lanes share the
   same x bounds and sample/pixel transform; bar/beat guides use those ticks.
   Missing timing is explicit. Export's ruler/zoom API and implementation are not
   modified; no monolithic waveform component or second transform is introduced.
3. The preview inspector uses existing **control-label roles** for Timing,
   Envelope and Level groups. Source start/end cells use full-width rows and full
   names; existing knob/value cells, 64-pixel rows and spacing tokens are reused.
   Its scroll extent follows the final reset row, rather than clipping to a fixed
   height. At 1100x850 every field and both reset controls remain reachable.
4. Compact preview values trim zeroes: seconds up to 3 decimal places, ms/dB 1,
   curves 2, musical fractions 3. Exact focus uses decimal round-trip strings
   (core musical grammar remains exact); Return without a change is a no-op.
   KnobValue accepts an optional exact string alongside compact text and keeps a
   separate typing baseline. Unchanged consumers retain existing display behavior.
   Stored sample/float/double precision, increments, parsers and validation remain.
5. Lane copy is Dry / Wet (plus preparing/stale state), with a concise Target
   anchor. Reference, amplitude and drag explanations move to native tooltips;
   gesture/Snap warnings remain visible. A JUCE TooltipWindow host makes those
   explanations usable, using existing theme background/secondary/outline roles.
6. UI-only musical formatting delegates to the core exact formatter, then rounds
   display with beat/bar carry. Footer explicitly says **Bar / Beat**, with clean
   tempo text. Durable names/IDs and the core formatter/parser are unchanged.
   Overview now uses the same section-heading Label/font/border mechanism as
   Audition, aligning horizontal text-box margins as well as font role.

No new visual token, colour value, font face, density exception or control painter
is introduced. The 52-pixel/grouped metric profile remains instance-only native
review until owner acceptance; normal construction retains the prior profile.
The previous host bundle remains unchanged. No audio, serialization, Export,
Learn-generation or cache implementation changed in this refinement.

## Focused native validation

**9/9 native MSVC Release CTests passed, 37.18 s**. Logs:
`out/build-logs/Release/20261104-perform-clarity-build-v5.log` and
`20261104-perform-clarity-tests-v5.log`. These cover swell UI, navigation, actual
editor binding, gestures, knobs, knob reset, audition, strip geometry and clarity.

The clarity test covers full Source captions without squeezing, grouped field/
reset reachability, compact/exact focus, zero commits/dirty events on untouched
Return, typed curve precision, source/render/capture identities, shared ruler
pixel equality and lane alignment across three view ranges, tooltip content,
musical negative positions/6-8/carry and readable footer. Existing strip coverage
retains 18 bounds cases / 72 software-scale renders and now compares Overview
against a reference Label using Audition's actual font and border.

Initial build caught a test header first including the standard char-conversion
header inside the fixture namespace. Required headers now load at file scope.
Focused tests then caught full Source labels being used in the old narrow layout;
full labels are correctly staged with the preview's full-width cells.

The no-op focus test identified **zero value commits and unchanged shape**, but
one delayed `performUi` update from prior viewport changes. Root cause:
`ScrollBar::setRangeLimits` defaults to asynchronous notification, which could
escape the synchronous updating flag and later constrain/persist the view.
Internal range refresh now explicitly uses `dontSendNotification`, consistent
with its existing silent current-range refresh. A dispatch-and-retain-view test
proves the correction, and untouched exact focus now produces zero dirty events.
User scrolling retains its normal callback. No assertion was weakened.

UI standards: **47 files / 61 bounded occurrences**, unchanged debt inventory.
Strict OpenSpec: **20/20**; diff checks pass. The previous full **55/55** native
Release run at `4e7aff9` remains evidence for unchanged audio/state/export paths;
no new full-suite or actual host/DPI acceptance is claimed for this UI checkpoint.

## Native visual checkpoint

[Selected at 1100x850](2026-11-04-issue-46/perform-clarity-selected.png) and
[dense at 1100x850](2026-11-04-issue-46/perform-clarity-dense.png) are actual native
production-component captures, visually inspected. The native review strip
provides empty/dense scenes and supported sizes. The completed previous strip
window was closed before directly launching this refinement; the current window
is preserved for owner review. It has no audio device.

Executable: `out/review-issue46-perform-clarity/perform-clarity.exe`;
SHA-256 `6A59F655CEBF3477E807C647EC00BDD7C6D346D0B4FE145EE841337EDEE0FC21`.
PID 36544 at launch. The review strip reads **Perform clarity checkpoint**;
the native window retains its **Issue #46 - Audition strip layout checkpoint**
title. Task 5.4 remains pending actual visual acceptance/adoption; task 6.2 retains
final host acceptance. No merge, issue closure or OpenSpec archive was performed.

Native clarity source revision: e6a76b40e9bb3efdf99356d1d3666731f7c55e00. The nine focused tests and native
executable above use these identical source files; only this evidence line follows.
Final strict OpenSpec: 20/20; worktree diff checks passed.

# Perform navigation foundations — issue #46

Implementation status: the owner approved the revision-4 compact composition.
`UI/PerformLayout.h` records that scoped profile, consumed by the native fixture.
Production editor adoption and navigation/gesture/persistence bindings remain
pending #6's merge for the reserved editor/decoder paths. Agent 1 released isolated
SwellTimeline/SwellViewport work in its explicit ownership handover. The approved shared Zoom/Fit
interface is documented in [shared timeline controls](shared-timeline-controls.md).

`Core/HostGrid.h` is a standalone fixed-mapping quantizer, not a second viewport.
It accepts the session's accepted `SongTime`, a current host observation, actual
capture rate and a stable `GridDivision`. It reuses `transportMappingMatches`,
`songOrigin` and `resolvePpqSample` rather than inferring a tempo map or bar origin.
Compatible seeks/loops retain the mapping. Changed tempo, meter, rate or origin,
invalid/non-finite timing and incompatible capture rates return unavailable.

The seven divisions are denominator beats (1/16, 1/8, 1/4, 1 beat) and numerator
bars (1, 2, 4 bars). PPQ remains quarter notes. Quantization rounds absolute grid
indices relative to PPQ zero, with half ties away from zero, then performs one
checked integer-sample conversion. Grid indices beyond reliable unit precision
on Windows double/long-double return unavailable rather than claiming a snapped
coordinate. This helper performs no edits, starts no jobs and persists no state.
Actual gesture-domain bounds, stale-gesture cancellation and unavailable-state
messages will be supplied by the existing Perform binding during integration.

`SwellViewport::snapToGrid` now exposes this checked grid through the existing
coordinate authority, preserving the old API for adapters not yet migrated.
Unavailable timing returns an empty optional, never an invented snapped position.
`snapToGridWithin` additionally finds a valid grid point inside a closed sample
domain or returns unavailable when none exists. It never substitutes an off-grid
clamped endpoint. Its fixed-work adjacent-index search accounts for the final
integer-sample rounding, including sub-sample grid spacing. This domain foundation
is tested independently; handle/translation binding remains task 4.2.

Perform timeline navigation now uses opt-in `constrain` / `zoomBounded` methods;
Export's unbounded viewport arithmetic and its adapter-owned limits remain
unchanged. Limits cover known capture, selected/source definitions, real render
frames and requested shape extents plus existing 4% Fit margins. Minimum span is
one sample per logical source-lane pixel (or the complete context when shorter),
with an additional double-precision floor at extreme absolute coordinates.
The horizontal fraction is logarithmic from complete context (0) to closest (1).
Toolbar zoom anchors a visible selected target, then a visible host playhead,
then center; Ctrl-wheel anchors at the pointer. Overview/pan stay inside the same
limits without seeking the host. Navigation never edits source/model coordinates.

`UI/Controls/LatchingActionButton.h` extends the accepted ActionButton family
without modifying it. The shared control owns accepted pressed-state presentation,
keyboard/focus/accessibility and invocation; its owner retains requested preference
and computes effective state. The standalone Snap example uses
`effective = requested && !ctrlHeld`; held modifiers are not preferences. Saved
Off remains Off after Ctrl release. The native control-family preview exercises
that feedback, not timeline editing. Shift fine drag, Ctrl+Shift, focus lifecycle,
gesture baselines and persistence still require production binding tests.

Focused native tests: `preverb_host_grid_test` uses an independent integer oracle;
`preverb_timeline_controls_test` exercises pointer/keyboard input, effective display,
silent refresh, disabled callbacks, native focus and accessible checked state.
`preverb_timeline_controls_test --review-snap` opens the directly launched family
checkpoint. The full Perform native matrix retains geometry, divider-pixel and
live refresh assertions. See [dated evidence](../validation/2026-11-04-issue-46-perform-composition.md)
for approvals, current status, exact commands/results and untested host/DPI scope.

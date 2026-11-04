# Perform navigation and editing — issue #46

Implementation status: the owner approved the revision-4 compact composition.
`UI/PerformLayout.h` records that scoped profile, consumed by the production editor
and native fixture. Integration follows merged Export PR #50 (`9b8e70f`), preserving
its dedicated workspace, schema-10 creative state and processor envelope 8.
The approved shared Zoom/Fit
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
The Perform binding freezes this mapping at gesture start and cancels further
publication after session, selection, mode, source support or timing changes.

`SwellViewport::snapToGrid` now exposes this checked grid through the existing
coordinate authority, preserving the old API for adapters not yet migrated.
Unavailable timing returns an empty optional, never an invented snapped position.
`snapToGridWithin` additionally finds a valid grid point inside a closed sample
domain or returns unavailable when none exists. It never substitutes an off-grid
clamped endpoint. Its fixed-work adjacent-index search accounts for the final
integer-sample rounding, including sub-sample grid spacing. This domain foundation
is tested independently and through actual handle/translation gestures.

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
and computes effective state. The production binding uses
`effective = requested && !ctrlHeld`; held modifiers are not preferences. Saved
Off remains Off after Ctrl release. Ctrl bypasses horizontal Snap; Shift scales
pointer deltas by 0.1; Ctrl+Shift combines both. Modifier transitions rebase from
the last accepted position and pointer without publishing an edit. Focus loss
cancels gestures and clears transient keys; active-editor refresh reconciles the
actual modifiers. Ctrl-wheel still zooms horizontally at the pointer. Exact
inspector entries remain unsnapped; left/right handle nudges use the adjacent grid
point, or one sample with Ctrl. Vertical tension remains continuous.

Fit includes selected dry source, requested envelope, placed raw tail and overlapping
wet context. Pending renders use source/request metadata. Without selection it
fits captured extent, including gaps; empty uses a finite neutral view. Initial
Perform entry Fits once when source/definitions arrive. Selection and background
refresh do not refit; mode return retains the view. Explicit Fit resets context.
The separate **Height** control changes waveform display amplitude in dB only.

Horizontal edits quantize absolute samples before canonical setters. Whole-wet
translation quantizes audible start once and preserves internal timing. Closed
domains require a valid grid point; zero fade-out is a distinct valid endpoint.
No valid point means rejection, never an off-grid clamp. Unavailable fixed timing
shows a reason and blocks snapped horizontal edits; Ctrl or Snap Off permits
sample edits. Navigation and vertical edits remain available. No tempo map is
inferred from changed current tempo/meter/origin.

## Optional state

`SwellSession::performUi` serializes an optional `performUi` JSON object containing
`snap`, stable `grid` key and optional `view: { first, last }` absolute samples.
The actual schema-10 decoder validates each field independently. Missing/malformed
UI values default to requested Snap On, quarter beat and initial Fit, preserving
valid musical and Export state. View endpoints must be finite, representable and
at least one sample apart; restore constrains the view to current context.
Only deliberate navigation/preference edits publish through an epoch-guarded
engine setter and existing coalesced host-state notification. Held Ctrl, focus,
gestures and cursors never persist or dirty the host. Editor close/reopen, mode
return and processor save/restore retain preferences independently per instance.
Restore starts no audition, recapture or rendering solely for these UI fields.
Schema and envelope versions are unchanged; an older schema-10 reader may ignore
the object and lose these preferences on resave. Readers supporting only schema 9
still cannot read schema 10; this is not a downgrade mechanism.

Focused native tests: `preverb_host_grid_test` uses an independent integer oracle;
`preverb_timeline_controls_test` exercises pointer/keyboard input, effective display,
silent refresh, disabled callbacks, native focus and accessible checked state.
`preverb_timeline_controls_test --review-snap` opens the directly launched family
checkpoint. The full Perform native matrix retains geometry, divider-pixel and
live refresh assertions. See [dated evidence](../validation/2026-11-04-issue-46-perform-composition.md)
for approvals and the [integration report](../validation/2026-11-04-issue-46-integration.md)
for final commands/results and separately recorded host/DPI scope. Actual processor
bindings, Fit and gestures run as `preverb_perform_binding_test`,
`preverb_perform_fit_test` and `preverb_perform_gesture_test`.

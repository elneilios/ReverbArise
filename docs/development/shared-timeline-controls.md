# Shared timeline controls — #46 / #6 handover

This independently mergeable library slice of [#46](https://github.com/elneilios/ReverbArise/issues/46)
supports Agent 1's [Export handover](https://github.com/elneilios/ReverbArise/issues/46#issuecomment-6629874432).
The parent OpenSpec Apply remains `realign-perform-workspace` in [PR #48](https://github.com/elneilios/ReverbArise/pull/48).
No Perform compact geometry, production workspace integration or state format is adopted here.

Issue #6 now consumes this library in `ExportWorkspace` (integrated native
approval recorded on 2026-11-04). Its binding exposes `TimelineNavigation::minimumSpan()` and
`maximumSpan()`, refreshes the row from accepted post-clamp geometry and guards
queued intents by session/pass/displayed composition. The shared files remain
unchanged from PR #49. See the [Export reference](../design/export-workspace.md).

## Interface and ownership

`Controls/TimelineZoomControls.h` composes Zoom −, horizontal slider, Zoom + and Fit.
It reuses Agent 1's existing `ActionButton.h` verbatim and the existing
`InvocationSlider` input mechanism. The inherited workspace theme supplies ink,
embedded typography and slider painting. Use `controlGeometry::height` and at
least `TimelineZoomControls::minimumWidth()`; no new density tokens are introduced.

```cpp
controls::TimelineZoomControls zoom;
zoom.display({fraction, canZoomIn, canZoomOut, canFit}); // silent accepted refresh
zoom.onZoomStep = [&](double factor) { navigation.zoom(factor); }; // .8 / 1.25
zoom.onZoomFraction = [&](double fraction) { /* owner maps its zoom domain */ };
zoom.onFit = [&] { navigation.fit(); };
```

Fraction increases from widest (0) to closest (1). For an accepted finite span
`S`, owner-defined minimum `A` and maximum `B` with `0 < A < B`:

```cpp
const auto fraction = std::log(B / S) / std::log(B / A);
const auto desiredSpan = B * std::pow(A / B, fraction);
navigation.zoom(desiredSpan / navigation.viewport().span());
```

Agent 1 should expose accepted minimum/maximum spans from its existing
`TimelineNavigation` clamp, rather than recalculate or duplicate those limits in
`ExportWorkspace`. This PR does not change that unmerged adapter. For a degenerate
domain disable zoom and display fraction 0. The binding must use the adapter's
accepted post-clamp view when refreshing, and retain its existing `onChange`
handler (repaint/update this row there). Never Fit from a routine refresh.

`beforeInvocation` is available to pin workspace/session ownership before mouse,
wheel or keyboard input; bindings retain authority to reject stale intents.
`display` suppresses notifications, cancels a queued slider change when superseded
by accepted display state, and rechecks button availability at callback delivery.
Disabled parent state also suppresses intents. This is not a session-epoch guard:
owners must cancel/rebind across workflow/session replacement, as with the existing
invocation controls. Human slider input follows JUCE's wheel and arrow behavior;
buttons support Return/Space with explicit accessible names and contained focus ink.

`Controls/MusicalTimelineRuler.h` provides:

```cpp
paintMusicalTimelineRuler(graphics, rulerBounds, navigation.viewport(), acceptedSongTime);
```

The painter accepts absolute sample coordinates, uses `SwellViewport::pixel` and
`makeSongBarRuler`, and clips all ink to its bounds. Bar labels use `bar.1`; meter
denominator beats appear as minor ticks when space permits and `bar.beat` labels
at closer zoom. Sparse long views retain the existing adaptive bar stride.
`musicalTimelineTicks` exposes the same data for focused tests. Invalid timing
produces no musical ticks; the workspace retains ownership of any unavailable
timing message. Fixed accepted tempo/meter only; no tempo-map inference or Snap.

## Integration boundaries

- Export retains its read-only waveform, click/drag preview positioning,
  pointer-anchored zoom, right-drag pan, keyboard navigation, current-file readiness,
  last-good pending view, and once-per-accepted-mix Fit. Fit covers the accepted
  audible mix plus margins, independent of WAV song-origin duration.
- Perform retains editable dry/wet lanes, envelope gestures and future Snap/grid.
  Production layout and materially revised controls still require their review
  checkpoints. Its independent amplitude zoom is not horizontal timeline zoom.
- Learn retains captured dry coverage and inspection. No controls or generation
  behavior are migrated by this PR.

Keep waveform rendering/overlays in their existing focused components. There is
no generalized waveform viewer, transport controller, render cache, persistence
or DSP dependency in this library slice. Broader renderer extraction is deferred.

## Validation and review

`preverb_timeline_controls_test` runs native component input and binding checks,
silent/queued refresh, disabled delivery, finite fraction handling, bounds and
software-scale renders. It also checks denominator-aware beat ticks, narrow views
and unavailable timing. `--review` opens the same tested row with a tiny viewport
binding; it starts no audio device. This isolated family review does not approve
Perform compact layout metrics or Export integration. Native visual/feel acceptance
must precede wider rollout; retain Export's existing accepted composition.

Export selects `TimelineNavigation::PanGesture::rightDrag`: right-drag pans without
positioning preview, left-drag positions, and only left-double-click invokes Fit.
The default left/Shift-left pan policy is unchanged for other consumers. Mouse-up
and ownership/range changes cancel active gestures. Horizontal/Shift-wheel and
keyboard navigation are unchanged. The gesture policy has no audio/model dependency.

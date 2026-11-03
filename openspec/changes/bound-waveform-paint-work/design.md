# Design

## Context

See proposal.md for motivation and the Issue #25 audit for exact source references
and native measurements. `SwellTimeline::paint/drawWave` scan capture/wet peaks;
`SwellOverview::paint` redraws all available swell summaries. The editor refreshes
at 10 Hz and repaints the overview. Disabling the inline overview only moves its
rectangle offscreen; existing loops still compute/draw its content. Source peaks
have 512-frame resolution; each wet summary has about 2048 peaks. No audio change
is needed to address the measured software drawing cost.

## Goals / Non-Goals

**Goals:** Bound repeated drawing by visible resolution, reuse stable waveform
layers and preserve sparse coverage, extrema, exact geometry and accepted binding.
Use existing JUCE facilities and the same message-thread ownership for GUI objects.

**Non-Goals:** Change DSP, playback limits, worker scheduling, capture persistence,
shared control families, workspace layout, #11's unmerged Learn implementation,
asset cleanup or saved formats. No new renderer/library is proposed.

## Decisions

1. **Prepare pixel-column summaries and reusable waveform layers.** For dry data,
   collect real minima/maxima of intersecting peaks, keeping unavailable, silence
   and unmapped ranges separate. For wet overview, combine displayed shaped
   extents at the existing amplitude convention; preserve narrow extrema rather
   than averaging them away. Detail drawing begins with a binary search into
   ordered peaks and touches only the visible interval. A cached image or compact
   column list is appropriate; per-peak draw calls every refresh are not. A GPU or
   full waveform data-model replacement would expand scope without evidence.
2. **Separate waveform content from interaction overlays.** Cursor, highlight,
   target and handle geometry continue to use canonical sample coordinates.
   A cursor-only update reuses waveform content. Inline overview loops are skipped
   when that view is disabled. Keep the existing separate overview transform.
3. **Invalidate from accepted data and display identity.** Keys cover source and
   render identity/revision, availability, selected ID, shape/gain/placement,
   viewport/global extent, size/DPI, theme/display scale and waveform gain. Carry
   revision/dirty information from existing bindings; avoid hashing all PCM or
   rebuilding complete summary maps on each paint. Never hold engine/capture locks
   while rasterizing. Old snapshots may stay pinned until their view releases
   them, but cache entries must be bounded and replaced on revision change.
4. **Keep construction costs observable.** Measure dirty rebuilds, cache-hit paint
   and the complete 10 Hz refresh separately. Initial summary construction can
   depend on peak count; repeated unchanged paint must not. This design does not
   move rendering into the audio callback or add another worker. If first-dirty
   work still stalls interaction, report that measured stage before expanding the
   design to asynchronous preparation.
5. **Use matched evidence as the acceptance gate.** Rebaseline after the workspace
   work has merged, then use the audit harness at identical compiler flags,
   dimensions and fixture density. Target at least 50% lower p95 for the
   five-minute/200-swell full-view case, with cursor-only overview p95 below
   16.7 ms on the audit machine. These are implementation acceptance targets,
   not universal timing guarantees. Report all maxima, misses, cache memory and
   any target failure; do not trade waveform fidelity for a timing pass.

## Risks / Trade-offs

- [Per-column aggregation hides narrow peaks or bridges gaps] → Test impulses,
  negative lead-in, unavailable ranges and silence at multiple zooms; compare
  geometric extrema to an independent reference and inspect matched native images.
- [Stale cache after a render/edit/selection race] → Binding tests cover each key
  dependency and accepted-generation changes while a field/gesture is active.
- [Cache memory offsets CPU savings] → Bound entries by displayed layers and
  dimensions; record private-byte/peak change and release replaced layers promptly
  on the message thread. Keep them out of audio publication ownership.
- [Synthetic software timings differ from interactive rendering] → Preserve that
  label, measure actual editor refresh and consolidate remaining host/DPI checks
  into the existing final smoke test. No DAW performance claim from software alone.
- [Concurrent workspace changes move these components] → Rebase and rerun the
  same fixture against merged code before implementation; adapt ownership to the
  settled components without editing the other agent's unmerged files.

## Migration Plan

Obtain owner approval for this proposal before applying it. Implement in an
issue-specific follow-up branch, preserve canonical specs until that change is
implemented, and open a separate owner-reviewable PR. No data migration is needed.
Rollback restores drawing/binding code; captures, renders and DAW state are
untouched. The current audit neither applies nor archives this change.

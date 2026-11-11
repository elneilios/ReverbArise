# Proposal

## Why

Issue #25 found substantial native software paint cost for the existing Perform
timeline/overview with full-song capture and 200 swell summaries. The audit traces
this to repeated traversal and drawing of subpixel or invisible peaks; its raw
measurements and host limitations are recorded in
`docs/validation/2026-11-03-issue-25-performance-audit.md`.

## What Changes

- Prepare display summaries at visible pixel resolution and reuse unchanged
  waveform layers across cursor-only refreshes.
- Restrict detail work to visible ranges and avoid drawing the disabled inline
  overview; keep the existing separate overview truthful.
- Preserve sparse coverage, real peak extrema, sample coordinates, shapes,
  display scaling, controls and interaction geometry.
- Add component/binding fidelity checks and rerun the audit's native paint matrix
  before claiming an improvement. Rebaseline the latest merged workspace first.

This is a future proposal requiring owner approval. The audit PR changes no
production implementation. #11's unmerged Learn workspace is outside its scope.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: require display-bounded waveform paint work, correct summary
  invalidation and measurable responsiveness without altering audio or state.

## Impact

Expected future implementation touches `Source/UI/SwellTimeline.cpp/.h`, the
existing editor snapshot binding and focused UI tests. No new dependency, FFT,
audio scheduling, thread architecture, persistent format or asset-storage change
is proposed. Existing component and DPI/host coverage remain applicable. Estimated
effort is 2–4 engineering days, subject to rebasing against the settled workspaces.

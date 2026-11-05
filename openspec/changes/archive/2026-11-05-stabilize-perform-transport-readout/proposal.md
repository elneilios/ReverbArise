# Proposal

## Why

Perform's centred transport label changes its text width as bar, beat, tempo and meter values update, visibly moving its separators and neighbouring values. GitHub issue: [#68](https://github.com/elneilios/ReverbArise/issues/68) requests a stable, readable informational footer during playback and seeking.

## What Changes

- Present Bar, Beat, BPM and meter in independently reserved slots whose geometry depends on available space and embedded font metrics, never the current value.
- Keep separators and the complete status group anchored across playback, seeks, tempo/meter and availability changes; align numeric values consistently.
- Retain fractional display precision, truthful unavailable states, exact-position context help, existing dark styling and supported size/scale behavior.
- Add native geometry, text-fit and editor-binding regression coverage and a scoped owner preview checkpoint before production rollout.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: Require stable, independently sized transport fields in the existing informational bottom band.

## Impact

GUI-only work in the editor's `hostInfo` setup, layout and refresh, a focused reusable read-only control under `Source/UI/Controls/`, and presentation formatting in `PerformText.h`. Extend the existing native swell UI/readability harness, production standards and Perform documentation after scoped acceptance. No transport, DSP, capture, persistent identity, saved-state or host-parameter changes; no external dependencies. Planning does not claim a tested fix.

# Proposal

## Why

GitHub issue: [#6](https://github.com/elneilios/ReverbArise/issues/6). Export currently provides guarded preview over the Perform layout, without a dedicated mixdown or durable WAV delivery. Producers need a trustworthy preview of the exact song-aligned wet layer or dry/wet replacement they can save or drag into FL Studio.

## What Changes

- Deliver the dedicated Export composition: approximately two-thirds waveform/preview and one-third mix/export controls, retaining the approved header and shared Inter/control styling. Include exact Dry–Wet percentage, preview Play/Stop, Drag WAV to Playlist, Save WAV and Return to Perform; omit Export Summary and the Perform editor.
- Prepare one immutable, bounded full-timeline composition for preview, waveform and file writing, reusing accepted source/render assets, absolute positioning, envelopes, calibration and disclosed protection. Mix changes do not reconvolve or alter live routing.
- Support genuine 0/50/100% dry/wet endpoints using linear gains. On first use initialise from the effective Perform audition percentage; thereafter preserve Export's own saved percentage/revision.
- Preview/fit the audible region while exporting from song origin, retaining leading silence and complete nonnegative tails. Uncaptured or unavailable dry regions become silence with a persistent explicit warning, as approved by the owner; coverage classification remains truthful.
- Publish verified immutable WAV revisions off the callback before enabling external dragging. Save WAV uses the same prepared composition, with cancellation/failure/overwrite protection and durable history.
- Permit Export entry with an enabled, eligible verified compatible wet render even when dry is missing; dry alone still cannot unlock it. Separate mode eligibility from a current mixdown being ready. Maintain dry DAW passthrough and no capture/scheduled wet in Export.
- Persist export preference, dependency/revision identity and completed managed-file references; restore without autoplay, overwriting prior files or publishing obsolete work.

## Capabilities

### New Capabilities

- `song-export`: full-timeline composition, equivalent manual preview, warned dry-gap substitution, bounded preparation, song-aligned WAV publication and durable file delivery.

### Modified Capabilities

- `swell-workspace`: replace temporary Export content with the dedicated mix-aware waveform and controls while preserving approved Learn/Perform behavior.
- `swell-session`: retain validated Export preferences and immutable completed revisions independently of monitoring and creative edits.
- `workflow-modes`: distinguish compatible wet-based Export entry from source-ready Perform entry and mixdown/file readiness.

## Impact

Affects `PluginEditor`, workspace layout/shared controls, `SwellEngine`/session serialization, source readers, prepared mixing/voice and managed asset publication. Reuse arithmetic/gain and bounded worker/publication patterns from `AuditionMix`, `TransportPlayback` and capture assets; no new runtime library is assumed.

Issue #5 remains owner of continuous Perform audition; this change establishes a reusable composition seam without changing that UI or its selected-window semantics. #38's explicit Generate workflow, #8's portability/cleanup, header redesign, new DSP, plugin identities, Learn capture/provenance and host offline-bounce routing remain outside scope. The #11 archive is not edited or re-archived. Planning only; implementation requires a subsequent Apply request.


## Approved review refinement — 2026-11-04

The owner's shorter nine-point review supersedes the earlier consolidated review.
Retain the accepted 66/33 composition and 0/50/100 audio behaviour. Refine the
WAV tile proportions, add colour-keyed layer legend and restrained read-only
Fit/pan/zoom. Keep a labelled, dimmed last-good waveform while requested work is
pending or fails, without enabling its stale playback/file actions. Distinguish
preview audible duration from song-origin WAV duration. Preserve header, gain
law, persisted state and Learn/Perform; no new audition/convolution engine.

## Approved preview positioning refinement

Implement the owner's requested waveform click/drag preview positioning now.
Stopped seeks park without autoplay; playing seeks resume through bounded worker
prefetch and anti-click ramps. Shift-drag retains panning until #46's shared
controls arrive. Keep positioning ephemeral and separate from DAW seeking,
export composition and WAV contents. Shared navigation/ruler integration remains
deferred, with no competing toolbar or waveform infrastructure refactor.

## Shared timeline integration — PR #49

The owner requested completion now that #49 is merged. Replace Export's lone Fit
button and local bar-label painting with the approved shared Zoom − / slider /
Zoom + / Fit row and musical ruler. Reuse existing viewport limits, accepted
timing and complete-audible Fit; retain click/drag preview positioning, pan,
mix/readiness/file semantics and the approved composition. No generalized viewer
refactor, Snap/grid or Perform redesign is included.

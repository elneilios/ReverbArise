# Proposal

## Why

[GitHub issue #73](https://github.com/elneilios/ReverbArise/issues/73) now has an identified host reproduction: on clean build `02a3a17`, wet remains audible while a stable approximately +16-sample playback-origin offset disables Snap; Stop restores it. Normal playback should not require disabling Snap or clearing source merely because a validated playback clock has a different origin from the captured grid.

## What Changes

- Allow Perform Snap during a validated constant-tempo live run whose origin differs from accepted capture timing. Reuse the existing independently validated run clock; do not add an offset allowance, hard-code 16 samples or change the shared one-sample mapping predicate.
- Preserve the accepted captured grid as the musical editing coordinate system: no automatic relocation of grid points, source, swells, renders or persisted timing on Play/Stop.
- Make canvas, inspector knobs/wheel/keyboard, snapped target insertion, availability and diagnostics consume the same grid decision and provenance. Exact entry and temporary bypass retain their existing behavior.
- Retain genuine drift rejection, stale gesture/session guards, stopped and empty-session behavior, and the already integrated offline export fix. Grid availability remains independent of audible wet, output enable and asset readiness.
- Keep [#71 tempo-map support](https://github.com/elneilios/ReverbArise/issues/71) separate. Its accepted local-mapping/coverage policy will replace the fixed-tempo assumption later; this repair does not infer a map from instantaneous BPM or claim steps/ramps are supported.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: trustworthy rhythmic snapping accepts an independently validated fixed-tempo run offset while retaining captured-grid coordinates and genuine invalidation safeguards.

## Impact

`HostGrid`, the dedicated `LiveTransportClock` control snapshot, `SwellEngine` editor snapshot, `SwellTimeline`/viewport, inspector timing invocations and existing diagnostic formatting; focused core/component/processor/persistence tests and living timing guidance. Reuse current Snap button, grid selector, KnobValue, timeline, Diagnostics and context-help text roles. No new control family, visual variant, dependency, persistent state or DSP policy.

Continue locally on the existing #73 branch with investigation/evidence `e803323` and combined offline implementation retained. The diagnostic-only change remains a completed historical investigation, not rewritten as a repair. Installed build remains unchanged during proposal. No tempo-map implementation, historical live-rejection repair, push, PR, archive or issue closure is included.

# Proposal

## Why

GitHub issue: [#46](https://github.com/elneilios/ReverbArise/issues/46).
The working Perform editor still uses legacy composition and navigation while Learn and the shared header have approved production roles. A focused Perform pass must preserve real two-lane editing while supplying the approved horizontal Zoom/Fit/Snap toolbar.

## What Changes

- Recompose the chronological Swells list, central dry/wet editor, Selected Swell inspector, audition band and single compact Overview using production Inter/semantic roles and shared controls.
- Reuse #4's merged identity-bound `SwellList` (PR #47), including its existing selection behavior; add no separate sort or bulk-edit model.
- Propose a Perform-only compact spacing profile, with measured native dense/empty previews and explicit owner look/feel approval before production rollout. Preserve approved Learn/header geometry and fonts.
- Replace Prev/Next and page-span navigation with one restrained row: Zoom minus, horizontal slider, Zoom plus, Fit, independent Snap toggle button and beat/bar grid selector. Holding Ctrl temporarily bypasses snap and shows the button off; releasing restores the remembered enabled state. Retain disclosed amplitude scaling separately from horizontal zoom.
- Extend the existing viewport and gesture paths for bounded zoom, contextual Fit and host-coordinate snapping of horizontal target/envelope timing. Preserve exact numeric entry and continuous vertical edits; fail honestly when reliable musical timing is unavailable.
- Preserve source PCM, durable IDs/assets, workflow guards and existing audition authority. Handle pending/failed renders, sparse coverage and return from other modes without inventing ready audio.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: production Perform composition, toolbar, horizontal navigation and safe host-grid gestures with truthful states and compatibility.

The existing `ui-design-system` review requirements already govern scoped density/control approval; they are not weakened or duplicated by this change.

## Impact

Expected areas: `Source/Plugin/PluginEditor.*`, `Source/UI/WorkspaceLayout.h`, `UiGeometry.h`, `WorkspaceTheme.h`, `SwellTimeline.*`, `SwellViewport.h`, `EnvelopeHandles.h`, affected shared controls and native UI tests. Optional backward-compatible UI preferences use existing session serialization, without changing plugin IDs or the processor envelope version.

Planning baseline: fetched main `33cd54b`, production standards revision 2 (superseding the issue's revision-1 wording). #6 remains separately owned; its branch currently points to `33f1871` and supplies no final Export API to integrate. Shared composition/preview changes require a concrete owner-coordinated contract before implementation. #38's future Generate transition is not implemented here. #5, #9, #25, Pattern policy and new DSP remain separate.

This proposal stops before Apply. A fresh user request authorizes the initial native review fixture; compact production metrics and materially revised families still require explicit native approval before broader implementation.

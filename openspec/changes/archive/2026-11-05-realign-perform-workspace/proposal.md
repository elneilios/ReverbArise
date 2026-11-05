# Proposal

## Why

GitHub issue: [#46](https://github.com/elneilios/ReverbArise/issues/46).
The working Perform editor still uses legacy composition and navigation while Learn and the shared header have approved production roles. A focused Perform pass must preserve real two-lane editing while supplying the approved horizontal Zoom/Fit/Snap toolbar.

## What Changes

- Recompose the chronological Swells list, central dry/wet editor, Selected Swell inspector, audition band and single compact Overview using production Inter/semantic roles and shared controls.
- Omit the obsolete Perform auto-marker/sensitivity band, as clarified in the owner's 4 November divider reference review. Preserve saved detection preferences and Learn's controls without implementing #38 generation behavior.
- Reuse #4's merged identity-bound `SwellList` (PR #47), including its existing selection behavior; add no separate sort or bulk-edit model.
- Propose a Perform-only compact spacing profile, with measured native dense/empty previews and explicit owner look/feel approval before production rollout. Preserve approved Learn/header geometry and fonts.
- Replace Prev/Next and page-span navigation with one restrained row: Zoom minus, horizontal slider, Zoom plus, Fit, independent Snap toggle button and beat/bar grid selector. Holding Ctrl temporarily bypasses snap and shows the button off; releasing restores the remembered enabled state. Retain disclosed amplitude scaling separately from horizontal zoom.
- Extend the existing viewport and gesture paths for bounded zoom, contextual Fit and host-coordinate snapping of horizontal target/envelope timing. Preserve exact numeric entry and continuous vertical edits; fail honestly when reliable musical timing is unavailable.
- Owner-approved follow-up: timing-knob drags and wheel changes follow the same Snap/grid using corresponding absolute timeline anchors. Ctrl bypasses, Shift retains fine drag and Ctrl+Shift combines both. Typed values remain exact and unsnapped; curve and level knobs remain continuous. Audit existing knob standards/guards before this binding change; wider family remediation requires its own scoped review.
- Preserve source PCM, durable IDs/assets, workflow guards and existing audition authority. Handle pending/failed renders, sparse coverage and return from other modes without inventing ready audio.
- Owner-requested final knob refinement: double-click reset and a right-click reset menu. Use model defaults for neutral/relative values and automatic wet start; Target/Source restore positions first loaded for that swell in the current editor. Explicit resets are unsnapped, guarded and limited to the chosen control's canonical operation; initial positions are ephemeral, not new project state.

Owner-requested audition visual follow-up: one inline status/actions row, equal
button gaps and proposed 52-pixel strip, gated by actual native metric approval;
Overview reuses the established section-heading typography.

Further owner-requested clarity pass preserves the three columns and 52-pixel
preview strip: the audition button names the selected mode; the editor reuses the
shared ruler; inspector groups/full Source labels and compact readouts retain
exact editing/storage; essential lane copy/tooltips, musical footer and aligned
section heading margins complete the native visual checkpoint. No audio or Export
behavior changes. Reuse existing roles/tokens; any new exception remains gated.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: production Perform composition, toolbar, horizontal navigation and safe host-grid gestures with truthful states and compatibility.
- `audition-monitoring`: owner-requested finite-window looping, Wet only toggle with live-level balance, removal of the row's trim reset and truthful concise status. Preserve legacy monitoring preferences and Export independence.

The existing `ui-design-system` review requirements already govern scoped density/control approval; they are not weakened or duplicated by this change.

## Impact

Expected areas: `Source/Plugin/PluginEditor.*`, `Source/UI/WorkspaceLayout.h`, `UiGeometry.h`, `WorkspaceTheme.h`, `SwellTimeline.*`, `SwellViewport.h`, `EnvelopeHandles.h`, affected shared controls and native UI tests. Optional backward-compatible UI preferences use existing session serialization, without changing plugin IDs or the processor envelope version.

Planning baseline: fetched main `33cd54b`, production standards revision 2 (superseding the issue's revision-1 wording). #6 remained separately owned during planning; final integration now uses merged #50 at `9b8e70f`, with the explicit Agent 1 handover and owner merge notice satisfying the shared-file gate. #38's future Generate transition is not implemented here. #5, #9, #25, Pattern policy and new DSP remain separate.

Apply is authorized. The owner approved the actual revision-4 compact composition and latching Snap family on 4 November 2026; scoped layout values live in standards revision 3. Shared Zoom/Fit/ruler PR #49 is approved and merged. Agent 1's explicit handover releases isolated SwellTimeline/SwellViewport work and reserved final PluginEditor/serialization integration until #6 merged. That gate is now satisfied; preserve its schema-10 Export state and envelope 8.

Latest owner steering replaces pop-up tooltips with one plugin-wide bottom hover
help line, below the musical footer. All control and graph help uses that full
width at the normal helper font; no graph overlay, small-font exception or popup.
Existing control help metadata is reused through a generic recursive listener,
including owned Settings/Diagnostics windows, without changing control actions.

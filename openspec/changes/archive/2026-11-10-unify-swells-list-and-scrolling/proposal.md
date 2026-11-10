# Proposal

## Why

Finish the lists and scrolling slice of [#107](https://github.com/elneilios/ReverbArise/issues/107). Swell selection already survives refresh, but Enabled and More actions need direct keyboard and accessibility access, and the row menu needs a clear, confirmed bulk delete.

## What Changes

- Give Swells rows square Enabled checkboxes and a three-dot action with the approved keyboard focus treatment. Up/Down selects a swell and Tab reaches its row actions, then the compact inspector. Right-click and the three dots open the same menu.
- Delete the full captured selection only after confirming its count when the menu opens on a selected row. An unselected row remains a single deletion target. Disable Rename during multiselect and show the dimmed label by itself.
- Keep disabled items visually quiet on mouse hover in shared dropdowns and action menus. Preserve independent keyboard inspection and accepted choice checkmarks. Let the highlight reach the inner right edge of a short popup.
- Preserve stable row identity, multiselection, scroll anchoring and [#93 contextual waveform reveal](https://github.com/elneilios/ReverbArise/issues/93). Verify that the compact inspector, Settings and Diagnostics reveal off-screen focus without editing values.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace` defines row action access, multi-selection command scope and confirmed deletion.
- `ui-design-system` defines disabled popup hover, the scoped Rename presentation and the extent of row highlighting.

## Impact

The approved native preview covers the Swells list, compact inspector and one IR dropdown. The planned work affects the virtualized list, guarded editor and engine deletion, the shared popup presenter, and verification of existing focus viewports. It builds on the existing editor-presentation overflow contract. P8 canvases, native choosers, auxiliary window chrome and Export file gestures are separate.

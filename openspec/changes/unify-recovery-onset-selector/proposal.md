# Proposal

GitHub issue [#107](https://github.com/elneilios/ReverbArise/issues/107), P2 follow-up.

## Why

Settings → Recovery → Onset suggestions still uses a plain JUCE ComboBox. The selector rollout deferred it while #8 was active. Adopting the shared control completes this consumer’s part of the UI audit.

## What Changes

- Apply the approved C2 dropdown presentation and pointer, keyboard and accessibility interaction to Onset suggestions.
- Keep the selected sample through refresh while it remains valid. Preserve viewport filtering, source-lane selection, and the existing fallback when the selected sample disappears.
- Keep selection separate from Confirm. Prevent a stale popup from selecting a different sample through a reused row ID.
- Verify the scoped adoption through automated native checks. The owner approved reuse of the existing dropdown design for this migration.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system` gains explicit C2 coverage and sample identity rules for the Recovery selector.

## Impact

Settings Recovery layout and editor binding, the existing shared selector and popup, focused native validation and the UI control inventory. The onset analysis and confirmation engine stay outside the change scope.

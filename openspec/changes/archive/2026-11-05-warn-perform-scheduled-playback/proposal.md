# Proposal

## Why

Perform plays prepared swells independently of current source input, so FL Pattern playback or muting an upstream Playlist track/instrument can leave unexpected wet audible. Issue #1's investigation in PR #74 found no reliable low-friction automatic discriminator; the owner requests an immediate, presentation-only warning using the existing message bar.

## What Changes

- Show a persistent warning-level fallback in Perform: **“Caution: In Perform, swells can remain audible during DAW playback, even if the source is silent or muted.”**
- Explain saved Song timing, silent lead-ins and explicit effect/mixer muting in the existing accessible/help detail.
- Let every existing actionable/error/transient notice, including debounced IR preparation, replace the fallback; restore it when that notice clears.
- Show the fallback throughout Perform, including stopped, empty and playback-disabled states. It describes the mode, not detected host state. Preserve Learn and Export notice policies.
- Reuse shared NoticeBar, approved warning semantics and reserved header geometry; no new control family, token, dismissal preference or visual exception.
- Preserve all playback, capture, routing, saved state, source/render assets and realtime behaviour. No host detection, MIDI connector, input gate or new automation parameter.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `swell-workspace`: Permit and require the Perform safety fallback within the existing conditional notice row, with higher-priority replacement and accessible detail.

## Impact

Refs #1; investigation evidence remains in [PR #74](https://github.com/elneilios/ReverbArise/pull/74). Plan on a separate branch/PR from the investigation. Primary implementation surface: `Source/UI/WorkspaceNotice.h`; focused cases: `tests/WorkspaceNoticeCases.h` and existing `swell_ui_test` header/notice coverage. Update living workflow/help documentation during Apply. Existing NoticeBar styling and export-specific notice composition remain reusable without redesign. No processor/DSP/transport or durable schema changes are required. The defect is not fixed and issue #1 must remain open.

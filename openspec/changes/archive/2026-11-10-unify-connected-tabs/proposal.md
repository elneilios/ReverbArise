# Proposal

## Why

The editor's segmented selectors and the Settings window use different tab styles and focus feedback. This P7 child of #107 brings both onto the connected tab design approved in the native preview and keeps About at the end of the Settings strip.

## What Changes

- Adopt connected tabs in the Learn/Perform/Export workflow header, Learn's Capture & Generate/Sound selector, the Perform Swell Inspector's Timing/Envelope/Sound selector, and Settings.
- Keep the accepted tab visibly selected. Give hover and mouse-down distinct body feedback, and show separate aqua corner brackets with a small gap for keyboard and accessibility focus.
- Let keyboard users inspect tabs without switching pages, then activate an available tab explicitly. Preserve workflow availability, rejection and accepted selection.
- Order Settings as Playback, Processing, Swell, Audition, Recovery, Appearance (when available), Storage, About. Keep About last and bind each tab to its existing page.
- Leave Swell list interaction and viewport/scrolling changes for a separate P7 child under #107.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system` gains the connected tab presentation, deliberate selection and Settings page order contracts.

## Impact

The shared segmented control, its editor consumers, the Settings tab strip and its page bindings, focused UI tests, and the production UI guidance are affected. Existing workflow rules, Settings page content, window sizing and persisted data retain their contracts. [Issue #107](https://github.com/elneilios/ReverbArise/issues/107)

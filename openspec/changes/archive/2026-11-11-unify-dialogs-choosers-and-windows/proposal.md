# Proposal

## Why

The owner approved the native P8 dialogs, choosers and auxiliary-window preview
under #107. The remaining production surfaces need its consistent keyboard,
cancellation and focus-return policy while retaining their existing guarded
operations and Windows-native pickers.

## What Changes

- Adopt the approved dialog presentation and lifecycle for Rename, confirmed
  Swell deletion and asset/metadata recovery warnings. Preserve their actual
  messages, actions, validation and captured targets.
- Make Tab/Shift+Tab, eligible Return/Space activation, Escape, Cancel and
  title-bar close coherent, including safe initial focus and one completion
  per invocation.
- Return focus to the surviving opening control or its appropriate visible
  owner, with input-appropriate decoration and no stale-window resurrection.
- Preserve native IR, Storage and working-folder pickers, their filters,
  overwrite/cancel handling, asynchronous lifetimes and operation guards.
  Retain Export Save and drag semantics, checking their lifecycle integration.
- Apply the same closing/focus policy to Settings and Diagnostics, preserving
  comfortable text, one user-scale transform, work-area fit, scrolling,
  selected pages and accepted values.
- Complete the named P8 consumer inventory and update shared contracts,
  standards, guide currency and scoped verification evidence.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system`: define the approved dialog, native-picker and auxiliary
  window lifecycle, keyboard access, cancellation and focus-return contracts.

## Impact

The affected owners are `PluginEditor` dialogs, `PluginSettings` and
`WorkspaceWindow`, `ConvolutionIrBinding`, `PortableStoragePanel`,
`WorkingFolderPanel`, and the existing Export Save lifecycle integration.
Reuse shared commands, fields, feedback, text, connected tabs and focus-aware
presentation/scrolling. The approved specimen is in
`out/ui-review/p8-windows/`; the design records its approval and limits.

This requires no new DSP, asset format, recovery capability or third-party
dependency. File selection does not bypass existing session, identity,
revision, operation or lifetime guards. Other family migrations and remaining
umbrella/spec housekeeping are separate work.

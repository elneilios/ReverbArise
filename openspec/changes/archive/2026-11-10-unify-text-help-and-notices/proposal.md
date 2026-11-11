# Proposal

## Why

The interactive families under #107 now share approved controls, while some
screen text still uses individual font, wrapping and overflow setup. The owner
approved the P3 native text review and softer aqua footer help, giving this
remaining family a consistent presentation to adopt across the plugin.

## What Changes

- Adopt shared text roles and explicit wrap, ellipsis or scroll policies for
  captions, scope explanations, empty states, statuses and diagnostic detail.
- Give the hover-help footer the existing softer aqua `hover` ink
  (`#00C2CC`), with its full message available through accessibility.
- Carry readable local errors, notice severity and full diagnostic descriptions
  through compact layouts and 100%/125% UI scale.
- Account for every visible consumer in Learn, Perform, Export, Settings,
  Diagnostics and Storage/Recovery through migration or a verified existing
  shared or native presentation.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system` adds the approved shared text overflow and aqua contextual
  help requirements.

## Impact

The main owners are `WorkspaceTheme`, shared label/help/notice controls,
`PluginEditor`, `PluginSettings`, the mode workspaces and Storage/Recovery
panels. Existing field feedback, fixed transport readouts and notice precedence
remain their established contracts. The remaining P8 window and chooser
interaction review has its own scope. This change requires no DSP, persistence
format or workflow authority changes.

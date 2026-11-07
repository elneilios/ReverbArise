# Proposal

## Why

Issue #107's action/context menus still rely on generic popup styling while selection and reset actions carry specialized lifetime and eligibility guards. The owner approved quiet menu feedback, transient pointer hover, persistent keyboard inspection and red destructive feedback on 7 November 2026; this slice applies that design to existing menus without changing their commands.

## What Changes

- Reuse the popup-row policy delivered by `unify-selector-states` for the Swells-list Rename/Delete menu and existing per-value reset popups.
- Open the same Swells item menu from a right-click on its row or a click on its three-dot icon.
- Clear pointer highlight when it leaves a row/menu; retain the actual keyboard-navigation highlight while its focus remains. Ordinary action rows do not acquire selected checkmarks or a latched background after invocation.
- Give Delete its explicit destructive red text/edge and restrained red hover/held surfaces; retain quiet neutral feedback for Rename and per-value reset. Labels and component names do not choose semantic variants.
- Preserve generated reset labels, captured actions, popup anchors, cancellation, deletion checks, one-shot dispatch and all current stale session/selection/input/pass/reset-revision rejection.
- Present long action labels and unavailable reasons readably and accessibly, with inspection distinct from permission to invoke.
- Validate and directly launch the scoped native menu preview before rollout, then verify actual context/reset bindings.
- Exclude dropdown migration, new actions, rename-dialog fields, knob/value interaction or geometry, #108 Level semantics, #8 storage/recovery/working-folder menus, #94 Settings organization, native choosers and other control families.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system`: Add explicit action-menu presentation, transient inspection and guarded invocation requirements.

## Impact

Reuse the selector child's shared popup presenter; scope adoption to `PluginEditor.cpp`'s Swells context menu and `KnobValue::showResetMenu`. Preserve `PluginInspector.cpp::prepareValueReset` providers and command bodies. Extend `PerformKnobResetCases` and actual Swells context-menu tests, including native lifetime/focus/anchor evidence. Only the popup hookup in KnobValue is in scope: coordinate a shared-file overlap with #108 instead of editing its worktree or parameter implementation. Update family standards/contracts/inventory and dated validation evidence during Apply. No new dependency on #8, DSP changes, durable-state migration or automatic umbrella closure.

Refs #107. Apply after `unify-selector-states` supplies the shared popup policy; deliver as a separate implementation slice/PR.

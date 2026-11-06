# Proposal

## Why

Merged #80 already aligns shared icon actions with command surfaces, but legacy main-editor and Settings/Diagnostics buttons still obtain glyphs, primary fill or destructive ink from component names. This first child of #107 implements the approved C1/P1 contract so remaining commands share predictable focus, state feedback and activation without changing their effects.

## What Changes

- Migrate the 20 remaining persistent legacy TextButton command members listed in the design to the existing shared command family, with explicit content and neutral/primary/destructive roles.
- Reuse merged #80's shared surface for text and icon commands; consolidate LatchingActionButton's duplicated active-state painting while preserving accepted effective state.
- Retire all name-based button appearance branches in WorkspaceTheme, including the orphaned `loadIR` glyph path. Keep component names for discovery and existing tests.
- Preserve current command bindings, eligibility, async/session guards and existing destructive classification. New keyboard/accessibility paths must issue the same guarded command once.
- Add native state/keyboard/accessibility/binding regression coverage and a directly launched family preview. Obtain scoped owner look/feel approval before migrating production consumers; update standards/evidence together.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system`: Establish semantic command state/content independence, equivalent guarded activation and accepted-state latch rendering for the migrated command scope.

## Impact

Baseline: merged #80 `717c890`, plus approved #107 audit/contracts at standards revision 11. Reuse `ActionButton::paintSurface`, IconActionButton, LatchingActionButton, existing editor callbacks, native control/Export/IR/audition/compact fixtures and existing tokens. Scope includes persistent legacy commands in Learn-adjacent Settings, Perform, Settings and Diagnostics, with already-shared header/Learn/Export actions serving as regression references.

No knob, value-field, Level range, selector, checkbox, storage/recovery window, shared-engine, DSP, capture, persistent-state or asset migration. Active #108 and #8 remain untouched. Dynamic Rename dialog internals, FileDragTile's specialized rendering and numeric step/segmented/list subcontrols retain their own family ownership. No new dependency or token is proposed. #107 remains the umbrella; this proposal authorizes planning only, not production implementation or new GitHub issues.

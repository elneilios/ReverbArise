# Proposal

## Why

Issue #107 and its owner scope extension identify inconsistent control construction and state feedback across ReverbArise, beyond the original button hover discrepancy. Existing production standards require shared controls, but developers still lack a complete consumer inventory and explicit family contracts for reusable composition, interaction and feedback.

## What Changes

- Keep [#107](https://github.com/elneilios/ReverbArise/issues/107) as the umbrella for a full UI audit and shared contracts, with separately proposed, reviewed implementation slices by control family.
- Establish a traceable inventory covering Learn, Perform, Export, the header, Settings and Diagnostics, including compact composition and user scaling. Record actual consumers, owning primitives/policies, state differences, approval status and remaining debt.
- Define family contracts for commands/icons/latches, selectors/menus, labels/help/notices, numeric/text fields with validation, knobs/sliders, switches, navigation/lists/scroll surfaces and specialized timeline interactions.
- Specify semantic variants and applicable state/interaction matrices using production standards revision 7 and existing approved exceptions. Appearance in migrated consumers must be independent of incidental names and visible labels.
- Define bounded family delivery packets with named consumers, reuse, compatibility safeguards, native review gates and automated evidence. The first recommended slice is button state consistency and name-based styling retirement, reconciled with #80 before any duplicated work.
- Deliver the audit, contracts and family roadmap through this umbrella change. Production control migrations belong to child OpenSpec changes and owner-reviewed PRs referencing #107.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system`: Require complete consumer ownership/state inventories, explicit reusable family contracts and traceable incremental delivery that preserves model and audio behavior.

## Impact

Umbrella implementation adds living UI guidance under `docs/design/` and `docs/development/`, linked through `docs/README.md`, and dated audit evidence under `docs/validation/`. It extends the existing `ui-design-system` specification without changing production widgets or canonical specs during proposal creation.

Later family changes will reuse `Source/UI/Controls/`, `WorkspaceTheme`, `KnobValue`, existing invocation mechanisms and native fixtures such as `Tests/ControlLibraryCases.h`. Screens retain composition and bindings retain domain/session authority. No DSP, commands, persistence format, asset migration, new sonic controls, new design tokens or whole-plugin redesign are included. GitHub child issue creation is a separate action; this proposal records slice definitions without creating issues or posting messages.

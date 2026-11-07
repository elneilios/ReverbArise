# Proposal

## Why

Issue #107's audit and command-button family are delivered by merged PR #114, but dropdowns still combine shared deliberate selectors with plain Settings ComboBoxes. The owner approved a quiet menu highlight and separate selector body/focus states on 7 November 2026; this slice makes that design reusable without changing selection authority.

## What Changes

- Extend the existing C2 selector family with reviewed idle, hover, held and disabled bodies, steady off-white text/chevrons, and aqua outer corner brackets for keyboard/accessibility focus only.
- Establish a reusable project-owned popup-row policy: quiet background/aqua edge for inspection, distinct held feedback, selected checkmark independent of highlight, pointer highlight cleared on leaving, and retained keyboard-navigation highlight.
- Support readable long choices, full accessible names, discoverable unavailable reasons and inspection without activation; preserve accepted selection until the binding accepts an edit.
- Migrate Learn/Perform shared IR, Perform grid, Export Character, Appearance selectors and retained Playback/Swell Settings selectors. Keep no-wheel selection deliberate and preserve existing IDs, invocation hooks and callbacks.
- Require focused native component/binding checks and a directly launched JUCE family preview before consumer rollout. The approved interactive mockups establish design intent, not native/DPI/host verification.
- Defer `candidateBox`/Recovery, storage/working-folder controls (#8), all Level/numeric/knob changes (#108), Settings reorganization (#94), navigation segments and native file dialogs.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `ui-design-system`: Add concrete C2 selector surface, popup inspection/selection, readable choice and guarded-delivery requirements.

## Impact

Extend `Controls/DeliberateSelector.h`, `IrSelector.h` and `InvocationComboBox` using existing focus observation, geometry and semantic tokens. Reuse `ConvolutionIrEditor`/binding and `ExportLevelsControl`; migrate the exact consumers listed in design.md. Introduce only the popup presentation/inspection seam needed by these selectors, reusable by `unify-action-menu-states`; no global popup rollout. Extend focused control/IR/Export/presentation tests and update standards, contracts, inventory, guide review and dated evidence during Apply. No DSP, assets, persistence formats, new choices or domain validation changes.

Refs #107; the umbrella remains open. Deliver selectors before the separate action-menu change.

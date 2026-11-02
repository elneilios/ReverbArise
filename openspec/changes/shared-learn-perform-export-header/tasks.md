# Tasks

Prerequisites: merged #10, #15 and UI development standards. This change owns #16;
#11's dedicated Learn workspace remains separate. No implementation is complete
merely because the proposal or a mockup exists.

## 1. Display mapping and guarded presentation

- [x] 1.1 Add one Learn/Perform/Export presentation mapping and reason/status adapter backed by existing workflow authority; verify unit cases for all modes, recording, settlement, failure, missing/incompatible source and frozen/ready-render eligibility without duplicating guard predicates.
- [x] 1.2 Add explicit saved Capture/Live/Finalise round-trip and metadata-free legacy tests alongside presentation assertions; verify state versions, identities, host notifications and the approved Recapture Stop policy remain unchanged.
- [x] 1.3 Document the durable/display-name boundary and reason mapping in the living workflow guide; verify examples distinguish future Export delivery from currently supported dry bypass/explicit preview.

## 2. Library controls and component acceptance

- [x] 2.1 Implement library-owned SegmentedSelector with silent model refresh, pointer and keyboard activation, visible focus, selected/unavailable descriptions, accessible reasons and wheel/active-selection no-ops; verify actual event tests including rejected/stale requests and zero callbacks during refresh.
- [x] 2.2 Implement a semantic Settings IconActionButton using existing visual tokens; verify Enter/Space/pointer activation, focus/hover/disabled states, safe lifetime and icon selection independent of component name.
- [x] 2.3 Add a reusable native fixture for both component families and document their interaction policies and API in the UI development guide; verify normal/active/hover/focused/disabled states at representative logical sizes and record remaining unmigrated families.
- [ ] 2.4 Component checkpoint A: ask for brief owner look-and-feel approval using the native fixture, without requiring FL Studio; use automated keyboard/disabled/wheel tests as primary evidence, record approval/adjustments/untested items and resolve material adjustments before further integration.

## 3. Shared header and visible terminology

- [ ] 3.1 Compose one reusable WorkflowHeader from the approved controls, existing wordmark/tagline and concise mode status; replace the dropdown and bind requests/Settings through existing APIs, verifying accepted-mode selection, active-mode no-op, source-ready automatic Perform and no layout overlap at 1100x850 and supported larger sizes.
- [ ] 3.2 Update mode-related visible labels, help, accessible strings and rejection feedback across the existing editor/Settings/Diagnostics paths; verify a targeted text audit preserves functional Source Capture/Recapture/live-wet wording and does not alter serialization keys or historical records.
- [ ] 3.3 Update native UI fixtures from ComboBox assumptions to mode/control behavior, including empty, dry-only, ready, recording, delayed settlement, error and frozen recovery; verify mode guards, canvas/value edits, Settings actions and the merged knob-wheel/scroll regressions continue to pass.
- [ ] 3.4 Update product vision and living UI/workflow guidance with the new visible names, shared-header ownership and deferred workspace boundaries; verify links, no broad historical renames, and no claim of a complete control-library migration.
- [ ] 3.5 Integrated checkpoint B: present actual native header captures in all three modes at minimum and larger sizes and ask the owner about brand balance, readability, focus and switching feel; resolve requested changes and record explicit approval before final host sign-off.

## 4. Integration and acceptance

- [ ] 4.1 Run appropriate integrated state/processor/UI regressions and strict OpenSpec validation after final changes; verify capture protection, useful/empty/silence Recapture outcomes, frozen recovery, incompatible render rejection, preview/live/export routing and callback safety retain their existing numerical behavior.
- [ ] 4.2 Build the Release ReverbArise VST3 and record its hash plus native layout/scale evidence; verify the artifact identity is unchanged and separate offscreen scale checks from actual host DPI validation.
- [ ] 4.3 Ask for one brief final FL Studio smoke test of control feel/focus, a Learn → Perform → Export/return journey and save/reopen; rely on automation for detailed guards/routing/protection/edge cases, record untested host/DPI coverage, and request more UAT only for failures or concrete unresolved host behavior.
- [ ] 4.4 Assemble dated evidence and the issue #16 PR with requirement/task results, both component and integrated approval records, and remaining control migration; verify no runtime/state change exceeds the approved scope and leave owner review/merge outstanding.

## Workflow follow-up

- After acceptance, sync/archive through the existing OpenSpec lifecycle when requested.
- Reconcile the separate committed #11 proposal with the delivered shared header and incremental control migration before applying its dedicated Learn workspace.

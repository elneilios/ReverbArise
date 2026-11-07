# Design

## Context

See [proposal](proposal.md) for motivation and [delta](specs/ui-design-system/spec.md) for observable requirements. Baseline is merged PR #114 on main (`89da1e7`); C1 command controls already own keyboard/accessibility-only aqua corner focus via `Controls/FocusVisibility.h`. C2 currently has `DeliberateSelector : InvocationComboBox`, disabling wheel selection, while several Settings members remain plain ComboBoxes. `WorkspaceTheme` sets a filled aqua popup highlight through JUCE colours; it does not own selector state/inspection semantics.

`ConvolutionIrEditor` snapshots choice identities before invocation and restores accepted selection before calling the binding. `ExportLevelsControl` derives Character from its accepted Dry/Wet pair; Custom is a displayed but unavailable choice. These mechanisms remain authoritative. The pinned JUCE source shows `ComboBox::keyPressed` nudging selection with arrows, and `PopupMenu` keyboard highlighting requires `item.isEnabled`. Consequently the approved inspect-before-commit and unavailable-row inspection cannot be achieved by changing colours alone.

## Goals / Non-Goals

**Goals:** A bounded C2 selector implementation, shared popup-row state/presentation and an explicit native inspection lifecycle. Reuse existing invocation, focus, typography, layout and binding ownership. Make the popup presenter usable by the separate action-menu slice.

**Non-Goals:** A generic UI framework, JUCE fork, global LookAndFeel rollout, whole-plugin selector conversion or new model API. No behavior changes to #108 parameter definitions/knob entry, #8 recovery/storage flows, native choosers or unrelated C1/C3–C8 families. No Settings tab/label/removal decisions from #94.

## Decisions

### Approved design and durable handoff

The owner explicitly approved the live design on **2026-11-07**, including the subsequent transient-hover correction, and requested these proposals. The approved references are the conversation previews `quiet-selector-states.html` and `quiet-menu-edge-cases.html`; their behavior is captured here so implementation does not depend on machine-local preview paths.

- Body: dark idle surface/neutral outline; hover raises the dark surface and uses the darker aqua border; mouse-held strengthens background/border. Off-white content/chevron stays steady. Opening a popup is not a latched hover/focus style.
- Focus: four aqua outer corner brackets with the C1 small gap; keyboard/explicit accessibility focus only, including already-focused requests. Pointer clicks hide decoration while actual focus remains. Pointer movement alone does not clear focus visibility. Disabled precedence and focus containment are independent of hover/held state.
- Menu: quiet dark raised row with a thin aqua leading edge, off-white content and stronger dark held feedback. A selected checkmark does not imply row highlight. Use no persistent hover background after pointer departure or activation.
- Track pointer hover and keyboard navigation separately. Pointer enters temporarily inspect that row; leaving a row (including menu padding) clears hover. Restore any still-valid keyboard navigation highlight. Clear keyboard highlight on focus loss/dismissal or pointer-origin focus, without erasing selection. Keyboard navigation replaces stale hover emphasis. Start pointer-open menus without a synthetic highlighted row.
- Closed long values ellipsize and expose full names through existing contextual help/accessibility. Open rows wrap without smaller text, preserve check alignment, and grow/clamp within screen bounds. Reasons remain visible and accessible. No popup tooltip host is introduced into the plugin.
- Unavailable rows can be inspected with keyboard/accessibility and expose truthful disabled state/reason, but have no selection permission. Entirely disabled selectors remain ineligible with an adjacent/associated explanation. Export Custom remains a valid accepted display value, never an invocable preset.

These are approved design directions. Browser fonts/pixels and simulated menu behavior are not validated JUCE dimensions, physical DPI coverage or native acceptance. Resolve exact tokens using existing production roles; any necessary new popup-only metric/variant needs named scope, native evidence and owner approval before rollout. Keep the command/IR folder-button alignment accepted in #114.

### Extend the selector and provide one scoped popup adapter

**Native-feedback amendment, 2026-11-07:** The owner approved the side-by-side
field study's **red message plus red border** treatment and smaller help text
immediately below the input. Capture that anatomy here and in the selector
delta/tasks. `FieldPresentation` supplies the initial scoped policy (12-height
Inter Regular, 17-height line box, 4-unit gap, normal helper ink versus semantic
danger ink). Align local feedback and captions to the visible body edge, outside
the reserved bracket margin. Binding-supplied errors replace routine guidance,
remain red during hover/held states and are exposed accessibly; the border is
red while the outer keyboard/accessibility brackets remain aqua. Disabled input
does not dim its explanation. Clearing validation restores normal body/help
without a model command. These are field presentation roles, not new validation
or commit rules. The native fixture starts on a clearable missing-IR error scene.
Text/numeric fields should use the same approved anatomy in their own C3/C4 work;
this change does not migrate them or introduce a universal field/editor wrapper.

Keep `DeliberateSelector`/`InvocationComboBox` and their callback-facing interface. Add a narrow project-owned popup presentation/inspection adapter for explicit C2 consumers. Share typed row identity, accepted/inspected/eligible state, reason and semantic role, plus geometry and painting, rather than rediscovering semantics from names or labels. Use current command focus geometry/observer and existing Inter/responsive roles.

The adapter owns an anchored JUCE component popup with explicit row navigation and dismissal; it separates navigation eligibility from activation eligibility and does not delegate those decisions to JUCE's `isEnabled` item flag. Reuse JUCE component/accessibility, screen placement, focus and asynchronous lifetime mechanisms, without patching vendored code. Keep the adapter small: one-level choice/action rows, wrapping, reasons, selection marker, guarded completion and cancellation. Native file dialogs, nested arbitrary menu frameworks and domain commands are outside it. An opt-in view prevents recolouring unrelated recovery/storage/native popups through `WorkspaceTheme`.

This is chosen over colour-only JUCE overrides because those cannot satisfy the inspected native behaviors. A wholesale custom selector hierarchy is rejected: existing fields, callbacks, models and scene composition remain. The first task proves the adapter against the specific native keyboard/focus/accessibility cases before any consumer migration, not a new discovery phase or a permissive fallback that silently skips requirements.

### Consumer and binding boundary

| Consumer | Existing owner / retained behavior | Work in this change |
| --- | --- | --- |
| Learn `learnWorkspace.irEditor.selector` (`learnDefaultIR`) and Perform `irEditor.selector` (`selectedIR`) | Shared `ConvolutionIrEditor`/`ConvolutionIrBinding`; catalogue identities, captured pass/session, accepted refresh and import sibling alignment | Reuse one selector/popup adapter; no new unavailable IR catalogue or recovery flow |
| Perform `gridBox` | DeliberateSelector; fixed grid choices and timing context | Presentation and inspect/commit lifecycle; retain grid meaning/guards |
| Export `mix.selector` | ExportLevelsControl; canonical pair presets and non-invocable Custom | Presentation/lifecycle only, same IDs and paired edit |
| Appearance `presentationSizeBox`, `presentationScaleBox` | DeliberateSelector and presentation preferences | Retain supported profiles/100%–125% software scaling and preference refresh |
| Playback `liveRoutingBox`, `gainPolicyBox`; Swell `inCurveBox`, `outCurveBox` | Plain ComboBox members and existing callbacks | Adopt deliberate no-wheel selector and shared popup; preserve choices, disabled prototype item, mapping and callbacks |
| Recovery `candidateBox`, #8 working-folder/storage controls, native chooser boundaries | #8/recovery owners | Deferred explicitly; do not silently count as migrated |

Read actual declarations at Apply start after current-main integration; implementation does not sweep newly discovered consumers into scope. For legacy Settings, adopt no-wheel deliberately, arrow inspection without eager callback, explicit Return/Space commit and Escape/Tab cancellation/traversal. Keep existing callback effects and add binding checks for this changed input route. Opening, inspection and accepted-state refresh never mutate the model.

Capture binding context before human invocation once, retain stable row IDs/keys, and route at most one eligible commit through the original callback. Preserve async owner deletion checks and close/reconcile a menu on incompatible catalogue refresh; no stale index reinterpretation. The popup owns transient presentation only, never an asset loader, processor pointer or independent parameter range. Accessible show/focus/select paths use the same eligibility boundary. Preserve full accepted text and the existing no-forced-speech policy.

### Delivery and validation boundary

Deliver this change before `unify-action-menu-states`. Only this change introduces the shared popup adapter and C2 choice behaviors; the action change reuses it and adds destructive action semantics, avoiding competing implementations. Do not globalize popup styling or implement action-menu consumers here.

Extend `ControlLibraryCases`, `UiStandardsCases`, `ConvolutionIrCases`/binding cases, `ExportLevelsCases` and actual editor/Appearance coverage. Prove independent hover/keyboard/selected state, no sticky click highlight, pointer exit into padding/outside, keyboard-hover-return, disabled inspection/no activation, Enter/Space one-shot, Escape/Tab, wheel scrolling, explicit accessibility focus, reorder/stale/owner deletion and silent refresh. Include long labels/reasons and contained brackets with original IR body alignment.

Use a lightweight native family preview first, launched directly for the owner; preserve its window during review. After native approval, migrate only listed consumers and run `scripts/build.ps1 -Configuration Release -Test`, `scripts/check-ui-standards.ps1`, strict change validation and guide review/currency. Record baseline 900x600/100%, 1000x700, 1100x850, 1400x1000, long/empty/error/disabled cases and supported 100%/125% user magnification. Windows DPI and FL Studio remain separately passed/failed/unsupported/untested; request extra host UAT only for a concrete unresolved behavior.

## Risks / Trade-offs

- [Native popup ownership can disturb focus return, screen-edge placement or dismissal] → Prove opening origin, repeated focus, Tab/Escape, outside click, owner destruction and screen clamp in the family fixture before migration; do not replace the accepted bindings.
- [Separating inspection and activation could accidentally expose disabled permission] → Distinct typed eligibility and inspection state, accurate accessible disabled state and negative pointer/keyboard/accessibility tests, including accepted Export Custom.
- [Shared geometry can clip brackets or alter the IR folder-button match] → Keep body alignment invariant; reserve contained outside ink, preserve caption/feedback spacing, verify actual shared field pixels/bounds.
- [Shared-file overlap with active work] → Leave #8 consumers and all #108 files/semantics out of selector migration; rebase onto their merged work without porting implementation from their worktrees. #94 organization remains separate.

## Migration Plan

Implement adapter/family fixture, obtain scoped native approval, migrate named selectors, run combined gates, and update standards revision/contracts/inventory and dated evidence together. Review the user guide semantically before refreshing its currency record. Deliver a focused PR with `Refs #107`. Roll back presentation/adoption together if needed; there is no state or asset migration and the canonical delta is synced/archived only after delivery approval.

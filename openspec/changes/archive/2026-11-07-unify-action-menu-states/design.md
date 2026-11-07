# Design

## Context

See [proposal](proposal.md) and [delta](specs/ui-design-system/spec.md). The prerequisite [selector design](../2026-11-07-unify-selector-states/design.md) introduces the scoped popup adapter, readable row geometry and independent pointer/keyboard/selection state. Reuse it after that slice is delivered; do not create a second menu framework or bundle the two implementations in one PR.

Observed main consumers: `PluginEditor.cpp::showSwellMenu` creates Rename/Delete items, captures a SafePointer, swell ID/name and assignment epoch, and opens Rename's existing modal text field only after valid selection. `KnobValue::showResetMenu` obtains a generated `ResetAction`, anchors its popup at the pointer, uses owner deletion checking and invokes the captured closure once. `PluginInspector.cpp::prepareValueReset` captures selection/session/input/pass/initial positions/reset revision and revalidates before model mutation. Those providers, including existing Level reset behavior, are not presentation policy.

## Goals / Non-Goals

**Goals:** Reuse approved quiet inspection language in the two named action-menu seams, add explicit destructive popup semantics and retain exact action authority/lifetime boundaries. Verify native context and reset behavior independently of selector binding tests.

**Non-Goals:** Dropdown migrations, changing reset defaults/ranges/precision or knob entry/drag/wheel behavior, relabeling commands, changing Rename dialog input, adding menu actions, Settings cleanup, storage/recovery/working-folder workflows, native choosers or a JUCE fork.

## Decisions

### Approval record and menu anatomy

The owner approved the designs and transient-hover correction on **2026-11-07**, then requested focused proposals. `quiet-menu-edge-cases.html` in this conversation demonstrated action/reset, long-label and unavailable-choice treatment; the following record is authoritative for this plan without relying on a machine-local file.

- Neutral Rename/reset rows retain off-white content and the selector adapter's quiet dark hover/keyboard row with thin aqua leading edge. Held feedback strengthens the dark surface; it does not turn the whole row aqua.
- Hover belongs only to the row currently under the pointer and clears on departure into another row, padding or outside the menu. A clicked row gains no persistent focus-like background merely from pointer focus.
- Keyboard navigation has its own current row and persists only while its navigation focus remains. Temporary hover may supersede its presentation; leaving restores the actual keyboard row. Keyboard navigation clears obsolete pointer emphasis. No synthetic initial pointer highlight, accepted checkmark or latched state belongs to an ordinary command.
- Delete is explicitly destructive: red content and red leading edge/quiet red hover/held surfaces. Rename/reset remain neutral. This does not reclassify existing C1 buttons (some Delete/Clear buttons intentionally remain neutral).
- Wrap long labels/reasons at the named normal text role, without arbitrary shrink. Unavailable actions remain inspectable and truthfully disabled to accessibility; visible/accessible reasons originate from their provider. Inspection is never activation permission.
- Preserve input-appropriate focus return. Keyboard/accessibility focus may show the existing aqua outer brackets on the owning control; pointer restoration must not do so. Menu row navigation feedback is distinct from a separate bracket around every item. Once open, the menu supports native keyboard inspection/activation without altering the knob's existing value input contract; new list-row navigation/opening affordances remain C7 work.

Browser design approval does not establish native row heights, screen-edge behavior or host/DPI coverage. Use the selector adapter's approved tokens; any new destructive popup-only roles/metrics require scoped native evidence and owner approval before consumer rollout. Shared notices continue using ordinary accessibility metadata/events without forced speech or a new popup tooltip host.

### Reuse one presenter with explicit role and captured completion

Pass typed row descriptors (ID, label, reason, neutral/destructive role, eligible state and optional selection marker) to the selector child's adapter. Action menus use no selection marker. Keep permission separate from keyboard inspection; unavailable actions have no invocable closure. Result delivery is one-shot and uses the original captured owner/action context, not fresh lookup of whichever swell is selected when a menu closes.

Use the adapter's existing screen clamping, Escape/outside cancellation, keyboard/Space/Return/accessibility invocation, focus return and safe owner lifetime. Preserve each call site's anchors: Swells uses the original row/event screen area, resets use the original pointer anchor and owner lifetime. Revalidate eligibility on activation/delivery through the existing model guards; styling does not grant acceptance.

Both Swells pointer entry points use the existing row callback. Right-click on
the row body or click its three-dot icon to open Rename/Delete for that item.
The native fixture uses the actual list component so both paths can be tried
before editor adoption.

Rejected alternatives: change global PopupMenu colours (would affect deferred #8/native boundaries and cannot distinguish role/inspection), infer Delete from its text/name (violates explicit ownership), or rebuild reset providers around a generic command API (would jeopardize exact captured authority). A new button or confirmation flow is not part of this change.

### Exact adoption and coordination

| Consumer | Presentation change | Invariants |
| --- | --- | --- |
| Swells-list Rename/Delete popup | Opt into shared action presenter, explicit neutral/destructive roles | Captured row ID/name/epoch, SafePointer, existing rename dialog and removeTarget path |
| Per-value popup in `KnobValue::showResetMenu` | Replace popup presentation/setup with shared adapter and passed-through ResetAction | Generated label, nullable apply, pointer anchor, deletion check, closure lifetime and invocation once |
| `PluginInspector::prepareValueReset` and reset bodies | No redesign or parameter edit | Existing initial/default/automatic targets, full session/selection/input/pass/reset-revision checks and native regressions |
| #8 Recovery/Storage/working-folder popups, native choosers | Deferred | Their own flows, bindings, pending work and separate approvals |

The per-value row includes the existing Level reset because it shares the same presenter, but changing its number, range, parameter mapping, precision, saved-value handling or provider is explicitly excluded. This work does not implement or amend #108. At Apply start, compare current main with the active #108 shared-file scope. If `KnobValue.h` still overlaps, prepare the shared presenter/fixture and context-menu work independently, and coordinate the minimal reset-popup hookup after that file's owner lands/rebases. Leave that hookup task pending until genuinely integrated and tested; never overwrite #108 work, cherry-pick its implementation or claim the family fully migrated early. No whole-feature dependency on #8 or extra parameter migration is created.

### Verification and native checkpoint

Extend shared menu state tests and `PerformKnobResetCases`; add focused actual Swells context-menu binding cases. Exercise pointer exit into padding/outside, pointer click then leave, keyboard row plus temporary hover then departure, focus loss/dismissal, disabled pointer/key/accessibility rejection, wrapped labels/reasons and explicit role/name invariance. Ordinary action rows must never inherit selector checkmarks.

Actual reset tests retain existing initial/default/automatic results, selection/source/session/pass/reset-revision supersession, destroyed owner, one invocation, same menu anchor and unaffected fields/assets. Context cases prove Rename opens only for the captured valid target, dialog cancellation does not edit, Delete cannot remove another target after stale epoch or destruction, and no selection/index reinterpretation occurs. Keep meaningful foreground/lifetime oracles; do not weaken them to avoid native test coordination.

Directly launch the prepared native menu fixture for scoped owner review before broader adoption; previous approved body/focus direction need not be re-approved as abstract mockups. Verify family geometry at 900x600/100%, 1000x700, 1100x850 and 1400x1000 with supported 100%/125% user magnification, long/unavailable rows and screen-edge anchors. Run supported Release wrapper/CTest, UI guard, strict change validation and guide review/currency; record actual Windows DPI and FL Studio coverage separately. Additional host UAT requires a concrete unresolved issue.

## Risks / Trade-offs

- [Popup presentation could capture current rather than original selection] → Keep closures/IDs/epochs from providers and run actual stale-menu tests before adoption.
- [Inspectable unavailable action looks eligible] → Truthful disabled content/reason/accessibility, independent inspection, no activation closure and negative tests for every input path.
- [Shared file conflict with #108] → Limit changes to popup presentation hookup, coordinate that seam as above and preserve Level behavior from its independently reviewed baseline.
- [Menu role changes leak into command buttons or deferred menus] → Explicit opt-in scope; test C1 button invariance and retain inventory debt for #8/other consumers.

## Migration Plan

Start from delivered selector adapter, extend typed destructive/action-row presentation in a fixture, obtain scoped native approval, adopt the two named popup seams and run the combined gates. Update versioned standards/contracts/inventory and dated evidence, then semantically review guide currency. Open a separate PR using `Refs #107`; keep the umbrella and unrelated work open. Roll back presenter adoption with its popup-only roles if needed; no asset, saved-state, parameter or DSP migration is required. Sync/archive the delta only after implementation delivery is approved.

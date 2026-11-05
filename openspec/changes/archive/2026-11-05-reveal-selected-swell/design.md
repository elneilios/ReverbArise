# Design

## Context

See proposal.md and issue #93. SwellList already emits stable selected/primary IDs and suppresses selection callbacks during snapshot display. PluginEditor applies selection through the epoch-guarded engine. SwellTimeline already owns complete-context Fit, viewport bounds, shared coordinates and navigation-change publication.

During native review the owner rejected span-preserving reveal and explicitly requested the same pan/zoom as Fit after selection. That decision supersedes the original zoom-preservation requirement. The initial span-preserving implementation and its tests are replaced rather than retained as an unused parallel navigation policy.

## Goals / Non-Goals

**Goals:** Explicit list intent frames the accepted primary identically to Fit, including available source, requested envelope, placed wet render and overlapping context. Re-clicking after pan/zoom restores that framing. Refresh remains passive.

**Non-Goals:** A new framing algorithm, alternate Fit policy, bulk editing/undo, new controls/styling, DSP, assets or persistence schema changes.

## Decisions

1. **Use the existing Fit action directly.** After accepted list selection and coherent refresh, validate Perform mode, session epoch and primary ID, then call the same timeline Fit operation used by the toolbar. This guarantees the requested pan/zoom and avoids duplicating context bounds, margin, pending-render or overlap logic. Remove the first implementation's custom span-preserving helper and reveal method. A translation-only policy was rejected by the owner because it could leave much of the swell offscreen.

2. **Keep explicit intent separate from refresh.** A separate stable-ID/epoch-bearing list reveal intent handles same-row reactivation. Pointer selection suppresses duplicate reveal while normal row activation emits it after accepted selection; keyboard selection uses the same binding. Checkbox/menu actions do not request Fit. Empty, obsolete-session or mismatched-primary intents do nothing. Snapshot refresh, delayed rendering, mode return and restored selection never issue this intent. Generic primary-change observation was rejected because it would override retained navigation.

3. **Retain navigation ownership.** Existing Fit publication updates scrollbar, painting, toolbar, both lanes, ruler, handles, Overview and optional saved Perform navigation together. No independent lane offsets, copied time mapping or new engine navigation API. Repeating Fit on already-fitted content remains visually idempotent.

4. **Reuse approved UI revision 5.** Affected families are SwellList, specialized SwellTimeline/SwellOverview, TimelineZoomControls and musical ruler. Keep visual tokens, geometry, multi-selection and row policies. No new family, variant or exception. Reopen the directly launched integrated native navigation-feel checkpoint for the owner after implementing the requested behavior; preserve existing review windows until review finishes and defer relinking a running preview. Any material family/style exception requires separate owner approval before wider rollout. Consolidate host checks into one brief final smoke.

## Risks / Trade-offs

- [Explicit selection changes a user's close zoom] → This is the owner's requested behavior; manual navigation is retained until the next explicit list selection or Fit.
- [Stale row identity or delayed publication frames another swell] → Revalidate epoch/mode/accepted primary and test reorder, rename, restore and late refresh.
- [Repeated pointer callbacks or checkbox/menu input causes unwanted Fit] → Separate explicit reveal intent, suppress pointer duplicates and exercise real input.
- [Framing diverges from Fit or persistence is skipped] → Reuse the exact production Fit operation and compare automatic endpoints with manual Fit in binding tests, including mode return/reopen.

## Migration Plan

No state migration. Implement and verify on codex/93-reveal-selected-swell, update guide currency and dated evidence, then open one issue-linked PR for owner review. Reverting the binding restores prior list navigation without altering musical state or assets. Archive only on a separate request.

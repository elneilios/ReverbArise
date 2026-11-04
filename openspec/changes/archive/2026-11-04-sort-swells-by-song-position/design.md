# Design

## Context

See `proposal.md` for issue #4's motivation. Baseline: main `33f1871`.
`Swell` already owns signed 64-bit absolute `arrival`, persistent UUID `id`, source bounds, independent wet `placement`, provenance and detector anchor. `Session` stores selected IDs and a primary ID. `SwellEngine::snapshot()` copies the model under its lock; the editor refreshes at 10 Hz. Automatic target reconciliation appends new definitions and can update automatic arrivals. Restore retains serialized definition order. None of these paths guarantees presentation order.

`PluginEditor.cpp` currently indexes `state.session.swells[row]` for painting, clicks and selection. Its refresh rebuilds selected row indices from IDs with notifications suppressed. It has no identity-based scroll anchor. Context menus already capture the original ID, which must survive the refactor. Existing tests in `swell_ui_test.cpp` cover a virtualized 500-row list and popup anchoring. Canonical `swell-workspace` specifies identity-bound selection and separate target/wet placement but no chronological list contract; this change adds that contract without replacing those requirements.

## Goals / Non-Goals

**Goals:** One deterministic display order and consistent row-to-ID resolution for every interaction, stable scroll context, no new persisted authority, and bounded message-thread work. A small reusable list component owns affected interaction mechanics while processor bindings remain in the editor.

**Non-Goals:** Reordering stored definitions, changing detector reconciliation or scheduling, changing default names, adding position columns or sort controls, redesigning Perform, implementing bulk operations, or migrating unrelated controls. The separate active `bound-waveform-paint-work` proposal owns waveform performance changes.

## Decisions

### 1. Derive the order only for the list

Build an immutable ordered row projection from each accepted editor snapshot using `(arrival, id)` ascending, comparing coordinates directly rather than subtracting them. Use a defined case-sensitive ordinal comparison for IDs. IDs are already persisted and unique in valid sessions. Source start, detector anchor, wet placement, bars and names are not ordering keys. Reuse canonical musical-position formatting only where a position label already exists; names remain user content.

Keep projection data valid for the snapshot lifetime and resolve row commands to owned ID values. Compare ordered IDs to detect actual row changes; rename/progress-only updates repaint without reinstalling order or resetting the viewport. A transient cache is derived data only. A bounded O(n log n) projection for at most `maximumSwells` is acceptable on the UI thread; do not add work or locks to the audio callback.

Alternatives rejected: sorting the durable model could change ordinal-dependent scheduling, serialization and default-selection behavior; sorting only one add/restore path misses other arrival changes; parsing labels fails on arbitrary names and numeric positions such as 2 versus 10.

### 2. Extract the affected specialized list family

Introduce a focused reusable Swells list component under `Source/UI/Controls/`, backed by JUCE's existing virtualized ListBox. It owns projection, rendering, multi-selection mapping, row hit handling and scroll policy. Feed it immutable row presentation values and selected/primary IDs; callbacks return identities and user intent. It must not include the processor, do asset I/O or mutate sessions. Keep `swellList` and ListBox-compatible test/accessibility access stable. The editor owns workflow/session guards, menus/dialog lifetime and calls to the existing engine APIs.

Move the existing list setup/row mechanics once rather than maintaining separate sorted and unsorted paths. Preserve existing theme text roles, row appearance, enabled checkbox, context-menu hit region, keyboard/multi-selection and popup screen anchor. Other library families, Settings and Diagnostics have no duplicate Swells list consumer to migrate. Timeline and inspector continue using model-owned IDs. No new token, visual variant or styling exception is proposed; preserve legacy Perform appearance without promoting its measurements to production defaults. Any newly necessary exception requires separate owner review and matching standards/tests changes.

Alternative: an editor-only index vector would fix the immediate order, but leave selection/scroll mechanics distributed across screen callbacks and fail the affected-family consolidation policy.

### 3. Refresh atomically without emitting edits

Capture old viewport and identity context before replacing row data, install the complete projection, and restore selected rows by ID with notifications suppressed under an update guard. Preserve the primary separately from the set of highlighted rows; programmatic rebuilding must not turn the last inserted row into the model primary. No callback can observe new rows with old indices. Editor row actions capture ID and applicable session/restore epoch before asynchronous completion, using existing engine validation; deleted/obsolete objects are rejected or cancelled. Preserve the current selection/deletion policy for truly removed IDs.

Every row consumer must use this projection: painting, enabled toggle, pointer and keyboard selection, context actions, accessibility and scroll targeting. Do not simply sort a temporary copy for painting. Returning from hidden Learn rebuilds from the current accepted snapshot. Reopening reconstructs presentation from saved arrivals/IDs; it needs no schema/version bump. Restore clears ephemeral row/menu/scroll ownership so an ID reused in another session cannot receive a stale command.

### 4. Preserve the reader's viewport, follow deliberate edits

Before a structural update, retain top visible ID plus its pixel offset and nearby visible IDs for fallback. After installing rows, anchor the same surviving ID/offset subject to viewport bounds. If it vanished, use the nearest surviving old visible row; otherwise clamp the old viewport position to the new extent. Empty lists reset safely.

Explicit primary selection scrolls it into view. A primary that was visible before its target moved remains visible with the minimum adjustment, taking priority over the top anchor. Background changes retain the anchor; a primary already deliberately scrolled offscreen does not pull the viewport back every 100 ms. Cosmetic-only or identical refresh leaves scrolling untouched. Learn/Perform transitions retain the prior Perform scroll context within a session; restore/editor recreation starts fresh.

Alternative: always scrolling to the primary makes background updates disruptive; retaining numeric scrollbar position moves the reader to different content when rows are inserted above it.

## Risks / Trade-offs

- [Missed row consumer edits another swell] -> Route all list interactions through one projection; test checkbox, keyboard, menu, delayed rename, multi-selection and primary after reorder.
- [Refresh becomes a user action] -> Notification suppression plus update guard; assert no model selection churn, dirty notification, extra generation or scheduling changes from presentation-only updates.
- [UI sort masks an upstream reconciliation bug] -> Test real automatic/capture/restore paths with surviving IDs and settings, asserting order only from accepted definitions rather than changing source reconciliation.
- [Floating timing or overflow influences order] -> Compare signed sample coordinates directly; test wide coordinates and missing musical timing. Bars are display conversions only.
- [Extraction changes legacy behavior or virtualization] -> Reuse existing native tests, preserve 500-row bounded child count and popup positioning, and add component/binding tests before production integration.
- [Untested native/host interaction] -> Prepare a lightweight native list preview for owner look/feel approval before dependent rollout, open it directly and keep it open during review. Verify integrated native composition, then request one brief final FL Studio smoke test; record untested physical DPI/host coverage honestly.

## Migration Plan

After a fresh Apply request, implement and test the pure projection and reusable list in a native fixture, then obtain the scoped owner interaction checkpoint before production integration. Integrate all affected list bindings and automated capture/persistence regressions. Update the list-family inventory/policy in `docs/development/ui-development-standards.md`; add dated evidence under `docs/validation/` and link it from `docs/README.md` where useful. If a standards revision is needed for new documented interaction policy, update the production guide and component coverage together without introducing new visual values.

Run `scripts/check-ui-standards.ps1` without regenerating the debt inventory, and `scripts/build.ps1 -Configuration Release -Test` using the existing wrapper lock and warm cache where available. Resolve any extraction-induced lint line/debt issues through compliant reuse, not inventory bypasses; report a genuine required exception for owner review. Record focused cases and native results separately from fixture, Debug, physical DPI and FL Studio checks. Leave unperformed checks open. No migration of state or assets is required; rollback restores the prior list presentation without rewriting saved projects. Deliver one implementation PR for owner review; no self-merge, auto-close, spec sync or archive.

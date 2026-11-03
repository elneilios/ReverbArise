# Design

## Context

See proposal.md for the owner's brief, scope and copy decision. The original
screenshot is a reference for preserving the existing composition; its conceptual
diagram does not authorize moving the branding or tagline.

At base `09c566b`, `WorkflowHeader` owns the wordmark, tagline, selector, Settings
and role label. `WorkspaceLayout` reserves separate notice and description bounds
in the same 32 px lower row inside an 82 px header. `SegmentedSelector` owns model-
accepted selection, borders, focus and accessibility. `NoticeBar` owns severity
presentation. Existing tests/review modes cover these components.

The current description uses 16 px italic Inter; contextual body text uses 14 px
regular Inter. Global secondary ink is #EDF1F5, whereas the requested header body
ink is #C7CDD4. `WorkflowHeaderCases` explicitly asserts italic/16 px today, and
the canonical header requirement also says italic. The delta deliberately changes
those expectations; global typography and colour tokens remain stable.

## Goals / Non-Goals

**Goals:** Add optically balanced mode icons, an accented regular description and
matched information-row typography without changing external layout or guarded
navigation. Preserve the Learn agent's clipping corrections during integration.

**Non-Goals:** Workspace redesign, new export capability, brand/tagline relocation,
DSP/state/workflow changes, theme-wide token edits, changes to the waveform audit
proposal, or editing the other agent's unmerged working files.

## Decisions

1. **Prepare reusable semantic vector glyphs independently.** Add a focused helper
   under `Source/UI/Controls/`, using normalized JUCE paths/strokes and an explicit
   semantic icon role. Start at a 16 px visual box, 1.6 px stroke and 8 px label
   gap; allow optical adjustment within the owner's 14–16 / 1.5–1.8 px ranges at
   native review. Learn uses varied bars. Perform uses a smooth accelerating rise,
   short release and three outlined handles; the path does not fill over handles.
   Export uses an audio page/outward arrow. Raster assets, emoji and icon inference
   from visible labels/component names are rejected.
2. **Extend the existing selector's content only after safe integration.** Append
   an optional semantic icon field with a no-icon default to `SegmentChoice`, so
   existing aggregate callers retain their behaviour. `WorkflowHeader` supplies
   the three roles explicitly. Centre the measured icon-plus-label group within
   the existing content box, with foreground colour/disabled alpha inherited
   from the current active/inactive state. Reuse current event and accessibility
   handling; icons create no children or focus targets. Preserve the incoming
   border, join, clipping and focus code rather than replacing the file.
3. **Keep screen layout and severity ownership.** Draw the description's 2 px aqua
   accent within its current right-column bounds; inset regular body text by
   another 8–10 px and centre it within the existing row. Use the existing 14 px
   regular Inter body role for both sides. Supply #C7CDD4 through a narrowly
   scoped header-body appearance setting on the notice component, keeping its
   default presentation for other consumers. Severity stripe/icon paint remains
   unchanged. Do not silently change global `secondary` or description-size
   constants. Keep the existing left notice allocation and inter-column gap.
4. **Keep text complete in meaning.** Use the proposal's three mode descriptions.
   Standard descriptions must fit the current narrow navigation column at the
   shared body size. Long contextual messages remain bounded with existing
   tooltip/accessibility detail; showing or hiding one must not resize the header.
   Export retains preview wording until actual WAV delivery is implemented.
5. **Stage review before rollout.** Extend `ControlLibraryCases`,
   `WorkflowHeaderCases`, `NoticeBarCases` and existing binding tests. Update the
   native `--review-messages`/`--review-controls` fixture to show all three modes,
   interaction states and short/long notices without changing production geometry.
   Launch one prepared native preview for owner approval of the materially revised
   selector family and information row before production binding/integration.

## Risks / Trade-offs

- [Concurrent edits overwrite clipping fixes] → Keep helper work on this isolated
  branch. Before any overlapping integration, inspect the Learn agent's published
  commit/diff and integrate against that version, preserving its geometry/focus
  tests. If no safe published version exists, defer overlapping edits and record
  the exact merge points; do not manufacture replacement selector geometry.
- [Text plus icon clips at minimum width or DPI] → Test measured content bounds,
  fonts and stroke extents at supported sizes/scales, then inspect native focus/
  hover states. Keep segment dimensions fixed and tune content optical sizing.
- [Header ink leaks into unrelated notices] → Opt in only the header consumer;
  verify default NoticeBar consumers and severity markers in component tests.
- [Screenshot approval is mistaken for interaction approval] → Record native
  look/feel, automated bindings, interactive DPI and FL Studio coverage separately.

## Migration Plan

No state/data migration. After an explicit apply request, implement isolated glyphs
and the review fixture first. Preserve/consume the concurrent clipping correction
before shared-file integration. Following owner component approval, integrate the
header, update living UI guidance and submit a focused owner-reviewable PR.
Consolidate host checks into one brief final smoke test. Rollback is limited to UI
content and presentation; existing audio/assets/state remain valid.

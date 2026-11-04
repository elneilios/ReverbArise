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

1. **Use the prepared SVGs as canonical artwork.** The original final vector
   sources are [Learn](assets/icons/learn.svg), [Perform](assets/icons/perform.svg)
   and [Export](assets/icons/export.svg). Later add a small semantic loader/cache
   under `Source/UI/Controls/` for these exact resources; do not manually redraw
   them into a second path implementation. They share a 16-unit viewBox and
   1.6-unit rounded stroke. Perform has a smooth rise, short release and precisely
   three outlined handles with transparent centres. Raster icons, emoji, dynamic
   SVG parsing in paint and identity inferred from visible labels are rejected.
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

## Exact proposed visual specification

All dimensions below are logical pixels at 100% scale. They describe internal
content, not replacement segment or focus geometry. Agent 1's final accepted
outer bounds remain authoritative; verify these recorded base values against
that revision before integration.

| Item | Proposed value / rule |
| --- | --- |
| Canonical artwork | SVG canvas `0 0 16 16`; nominal display 16×16; 14×14 compact QA sample |
| Icon stroke | 1.6 units at 16 px, 1.4 px when scaled to 14; round joins/caps; transparent fill; no fixed non-scaling stroke |
| Icon optical placement | Canvas centre `(8,8)` on the content line centre; per-icon translation `(0,0)` |
| Icon-to-label gap | 8 px from the 16 px canvas edge to the measured text bounds |
| Label | Existing embedded Inter SemiBold, 18 px; preserve current baseline and state handling |
| Group positioning | Centre the combined width `16 + 8 + measured label width` in each segment's existing content bounds; minimum 5 px side inset; no outer-bound changes |
| Inactive / active ink | `#F8FAFC` / `#0F1418`; active background remains `#00EFFF`; existing disabled alpha 0.4 and hover/focus policy |
| Header reference bounds | Existing height 82; primary row 46; navigation height 36 at local y=5; preserve branding and Settings alignment |
| Lower information row | Existing local y=46, height 32; both body text boxes vertically centred at y=62 |
| Description stripe | At right-column local x=0, row-local y=8, width 2, height 16: header-local y=54–70; fill `#00EFFF` |
| Description text box | Right-column x=10, row-local y=0, width `columnWidth - 18`, height 32; exactly 8 px after the stripe and 8 px trailing inset |
| Information body text | Embedded Inter Regular, 14 px, `#C7CDD4`; no italic, synthetic weight or global font override |
| Body baseline alignment | Use the same font and 32 px centred-left text-box alignment for both columns; derive baseline from shared font metrics rather than independent y nudges |
| Contextual marker | Preserve existing severity stripe at x=0, y=4, width 3, height 24 within the notice row; keep severity icon, its ink, and existing 38 px body-text inset |
| Column separation | Preserve the existing 16 px gap; keep the entire description/accent in the right column and the notice in its allocated left column |
| Decoration | No description icon, background fill, pill, border, shadow or extra status panel |

The exact asset stroke bounds are Learn `[1.2,0.7]–[14.8,15.3]`, Perform
`[0.25,0.85]–[15.75,14.95]` and Export `[1.2,0.7]–[15.0,15.3]` (conservative
axis-aligned bounds including stroke). Different silhouettes use the same canvas
centre; do not enlarge one icon independently to fill all available pixels.
The Perform handle centres are `(2.4,12.8)`, `(9.3,3.0)` and `(13.6,10.4)`, radius
1.35, with paths meeting their outer rims rather than crossing the holes.

At base `09c566b`, the mode column is 330 or 360 px, with Settings 36 px and a
12 px gap. For a 1100 px plugin the header width is 1068, mode column x=690,
notice width 674 and inter-column gap 16. At 1400/1600 px the respective header
widths are 1360/1560 and mode-column x values 952/1152, with notice widths
936/1136. Preserve the existing threshold based on header width, not a new size
breakpoint. These figures are reference assertions for integration, not permission
to overwrite any geometry refinements made by Agent 1.

Mode descriptions remain single-line at the standard widths, with full tooltip
text and the proposal's truthful Export copy. Verify actual embedded-font advance
widths in JUCE; do not silently squeeze text, reduce font size or move controls to
make it fit. Long contextual text uses its existing bounded/ellipsis presentation
and full tooltip/accessibility detail; it never borrows right-column space.

## File-level integration plan and Agent 1 dependency

| Future integration point | Intended minimal edit | Preserved ownership / dependency |
| --- | --- | --- |
| `assets/icons/*.svg` → proposed `resources/icons/navigation/` | Promote these exact reviewed SVG bytes when implementation is authorised | No copy into production resources during this preparation; no raster replacement |
| `CMakeLists.txt`, `preverb_ui_resources` list | Embed the three SVG resources alongside existing UI resources | Defer; no build configuration changed now |
| New focused loader/cache under `Source/UI/Controls/` | Parse embedded SVG once; keep originals immutable and cache separate foreground-ink copies outside paint; use scoped transforms | No cumulative recolouring of a global shared drawable, duplicate geometry, paint-time parsing or processor dependency |
| `Source/UI/Controls/SegmentedSelector.h` | Append optional semantic icon field and change icon-plus-label content paint only | Consume Agent 1's published version first; preserve `resized`, joins, `outlineFor`, focus/clipping, hit bounds, keyboard/accessibility and guarded request handling |
| `Source/UI/WorkflowHeader.h` | Supply semantic roles and draw the accented regular description within its existing allocation | Preserve published header bounds, branding, Settings and selector placement |
| `Source/UI/Controls/NoticeBar.h` | Expose a narrow opt-in body appearance if the final component has no equivalent | Preserve notice severity glyph/stripe, focus/announcement behaviour, default styling and other consumers |
| `Source/Plugin/PluginEditor.cpp` | Opt in the header notice's body appearance; retain current display bindings | Integrate against Agent 1's final editor; no workspace or model logic edits |
| `Source/UI/WorkflowPresentation.h` | Update only the approved Perform description; retain truthful Learn/Export text | No durable mode-name or eligibility change |
| `Source/UI/WorkspaceLayout.h`, `WorkspaceTheme.h` | Read-only reference; no planned geometry/token edit | Agent 1's final allocations and fixes remain authoritative |
| Existing header/control/notice tests | Extend icon, appearance, bounds and unchanged-interaction checks | Preserve Agent 1's complete-outline regressions; update only obsolete italic/16 px expectations |
| UI standards and workflow guidance | Document the implemented scoped appearance and icon roles later | No living documentation claims that this pending UI is implemented |

Before editing overlapping files, record Agent 1's published commit/PR, the exact
files/hunks containing clipping corrections, and the associated passing regression
checks. None was supplied in this request; no final revision is assumed here.
If not available, preparation is ready but shared-file integration stays blocked.
This preparation performs no merge or cherry-pick. Any later branch integration
must be explicitly authorised and must not replace files wholesale. If final
geometry differs, revise the internal fit assertions against it before applying.

## Validation checklist and acceptance status

**Completed preparation:** XML/style/primitive checks and static browser inspection
of the canonical SVGs at 14, 16, 20, 24 and 32 px, plus 96 px detail; active dark
ink on aqua, inactive off-white and 40% disabled examples. See the asset
[review sheet](assets/icons/preview.html) and [rendered evidence](assets/icons/preview.png).
Browser scale samples are not interactive Windows DPI or native JUCE acceptance.

The following checks are deliberately pending production integration:

- [ ] Load all three embedded SVGs with JUCE; verify non-null drawables, exact
  viewBox/stroke extents, transparent interiors, active recolouring and no paint-time
  parse/reallocation. Render at 14/16 px and compare with the canonical artwork.
- [ ] Exercise Learn, Perform and Export as selected, inactive, hovered, pressed,
  disabled and keyboard-focused choices; inspect icon/label spacing, clear Perform
  handles and every outer/internal border and focus-outline pixel at both end segments.
- [ ] Test pointer, Tab, arrows, Home/End, Enter/Space and accessibility activation;
  preserve accepted-state selection, rejected/stale requests, disabled explanations,
  repeated-active and wheel no-ops, and existing accessible labels/focus targets.
- [ ] Pair each mode with no notice and information/warning/error notices; include
  short, column-width and overlong text. Verify severity colours, regular shared
  body typography/baseline, full tooltip/accessibility detail and zero column collision.
- [ ] Verify notice appearance/disappearance and repeated refresh preserve header,
  workspace, focus, active editing, state identities, audio and host-notification
  behaviour; changing the visual icon must not request a mode or render.
- [ ] Check plugin sizes 1100×850, 1400×900, 1400×1000 and 1600×1000; include the
  existing 1068/1360/1560 header fixtures and threshold boundary widths so icon
  content does not depend on a single width or overwrite Agent 1's layout policy.
- [ ] Cover 100%, 125%, 150% and 200% Windows scale (16 px icons become 16/20/24/32
  device px), including a monitor/DPI transition and fractional origins. Inspect
  text, stroke margins and focus visibility; label unperformed host/DPI cases.
- [ ] Preserve Agent 1's clipping-fix regression checks and existing default/no-icon
  selector and non-header NoticeBar consumers. Assert global styling tokens unchanged.
- [ ] Run the repository wrapper and component/binding tests after implementation;
  keep structural/browser checks separate from MSVC/CTest and host results.
- [ ] Launch one prepared native review window for owner look-and-feel approval,
  preserve it until review ends, then consolidate only unresolved host behaviour
  into one final FL Studio smoke test. A browser raster does not pass this gate.

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

No state/data migration. Asset preparation is complete without production
embedding. After a later explicit implementation request, load/cache these exact
SVGs and prepare the native review fixture. Preserve/consume the published
concurrent clipping correction before shared-file integration; no merge or
cherry-pick is authorised by preparation alone. Following owner component
approval, integrate the header, update living UI guidance and submit a focused owner-reviewable PR.
Consolidate host checks into one brief final smoke test. Rollback is limited to UI
content and presentation; existing audio/assets/state remain valid.

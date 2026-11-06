# Design

## Context

See proposal.md for motivation. The owner approved **Option A revision 4**, then requested implementation. The canonical review artifact is `docs/design/compact-ui-900x600/README.md` with v4 screenshots and browser checks. This approval selects the composition; native interaction/typography and host/DPI acceptance remain separate gates.

The inspected and freshly fetched main is `84894ee`. No duplicate open #58 PR was found. Current editor minimum/default is 1100x850. Perform has 180/304-wide sidebars, a 472-wide navigation requirement and roughly 860 pixels of inspector content in a 460-pixel viewport. Learn stacks non-scrollable controls; Export has its original single crossfade. No general local presentation preference store exists. `performUi` is musical navigation and must remain independent.

#51 is implemented separately in draft PR #92, pending owner host acceptance; it is not merged into this branch. Its two-level/character layout was included in the mockup as a capacity contract. #80 and new DSP are also separate. Preserve delivered behavior and adapt existing controls, without absorbing unmerged audio changes or shipping hypothetical widgets.

## Goals / Non-Goals

**Goals:** implement the approved 900x600 composition, continuous bounded type growth, full normal notices, persistent labelled access, comfort clearances and independent sizing/magnification preferences. Reuse the existing editor, controls, bindings and native harnesses.

**Non-Goals:** DSP/gain-law changes, source or asset migration, new host parameters, plugin identity changes, separate compact workflow, whole-library migration, or production Character/Tone/filters/Stretch placeholders. Future mockups prove capacity only.

## Decisions

### 1. Use the approved revision-4 geometry

At width 900: outer side margins 12; list 160; centre 416; inspector 276; column gaps 12. Shared header starts at (12,12), height 64. Main work starts at y84 after an 8-pixel gap and ends y464. Audition is 48 high; Overview 48; position/help are 20 each. Larger heights grow detail/inspector space; Overview grows proportionately to a bounded 100. Larger widths principally grow the timeline, with navigation groups sharing one row when the real 472-pixel requirement fits.

The inspector reserves 24 for its heading, 32 for activity controls, 32 for section navigation, 8 before content, and fixed reset actions with 8 pixels above the next divider. Its selected content has 8-pixel side insets (259 usable width) and 236 height at minimum. Exact text entries retain 6-pixel horizontal insets. Keep focus strokes contained; controls must not touch separators.

Subsequent owner clarification retains standard named padding/margin tokens and requires an 8-pixel divider-end clearance: perpendicular rules must not touch. Draw vertical separators inset from horizontal band boundaries in every mode; do not use full-height CSS/component border edges that form T junctions. Keep divider clearance distinct from content padding even where both currently equal 8.

Use named geometry and role policies alongside `UiGeometry`/`PerformLayout`; layout calculations remain testable independently of painting. Current supported maximums may remain subject to screen fit. No constant in a browser mockup becomes a production token without the documented review/evidence path.

Alternative rejected: simply lowering the minimum or shrinking the old complete raster. It preserves undiscoverable overflow and violates useful control geometry.

### 2. Introduce explicit responsive text roles

Reference role heights (JUCE logical font heights, with native optical verification):

| Role | 900x600 | 1100x850 | 1400x1000 |
| --- | --- | --- | --- |
| Main heading | 16 | 18 | 21 |
| Navigation | 14 | 16 | 18 |
| Body/button/tab | 12 | 13 | 14 |
| Caption/value | 11 | 12 | 13 |
| Notice | 12 | 13 | 14 |

Interpolate using `small=clamp(min((w-900)/200,(h-600)/250),0,1)` and `large=clamp(min((w-1100)/300,(h-850)/150),0,1)`. A large width alone must not overgrow text in a short window. User magnification is distinct. Preserve embedded Inter faces, semantic colours, field precision, input increments and accessible names. The optional compact KnobValue policy uses a 36-pixel dial, caption above the dial/value pair and 26-high entry; ordinary actions are 32 high. Unaffected auxiliary/legacy consumers retain existing defaults unless explicitly migrated.

Extend shared role APIs with a scoped responsive policy rather than mutating process-wide defaults. Existing library components obtain roles from their inherited presentation context; bindings keep model ownership. Update fonts on actual layout/theme changes, preserving active typed text. Do not rebuild controls in the polling timer.

Alternative rejected: fixed 14/21 heights at all sizes. The owner explicitly preferred the quieter compact type and growth at larger sizes. Native optical differences from browser metrics must be surfaced, not silently compensated with arbitrary literals.

### 3. Correct the shared header

Reuse the original wordmark artwork/proportions; fit its actual drawing bounds to remove the transparent-canvas displacement. Align visible ink to the x12 content edge. Put the mode description beside the wordmark in the primary row, permitting two lines. Retain mode names/icons and Settings on the right; omit the decorative tagline as approved.

Give notices the full reserved row below. The normal Perform warning must be complete at minimum size without ellipsis. Use bounded adaptation/wrapping and existing full accessible/help details for unusually long messages; never indefinitely reduce text or change notice priority. Showing a notice does not shift workspace bounds or steal focus. Maintain at least the approved 12-height normal notice role; the reviewed two-line fallback has an 11-height floor. Auxiliary detailed diagnostics remain outside the primary workflow.

### 4. Use the chosen tabbed composition

Perform has persistent Timing / Envelope / Sound selectors. Timing owns target, placement and source bounds; Envelope owns supported pre-roll/offset/fades/curves; Sound owns current IR/import/status and Level. Keep selection/activity controls above and Reset/reset options below. Retain stable component IDs, reset meanings and epoch-bound commands. Switching sections safely cancels or completes the existing edit lifecycle without accidental publication and never changes musical state itself.

Learn has Capture & Generate / Sound tabs. Capture policies, detection and sensitivity remain Learn-only. Current Default IR lives in Sound; Generate/readiness/manual guidance stay pinned below both tabs. Preserve Automatic Swells visibility, pass locks, explicit generation provenance and default-versus-selected-swell independence. Provide focus reveal/discoverable overflow for exceptional text or future delivered controls.

Reuse SegmentedSelector, ActionButton, KnobValue, Switch, SliderControl/NumericInputControl, deliberate IR/grid selectors and existing guarded bindings. Extract only the affected viewport/section composition; remove duplicate interaction setup in touched consumers. New/revised families require a directly launched native look/feel checkpoint before wider rollout. No separate whole-plugin migration is needed.

The future Sound capacity specimen is 259x236 in both contexts. Its hypothetical Character/engine disclosure plus Tone/HP/LP/Stretch are not implementation tasks. #80's later shared editor must fit this boundary. Likewise Export adapts its delivered mix controls to the approved dominant-left-preview/right-actions composition; #51 remains the sole owner of independent Dry/Wet gain semantics.

### 5. Keep sizing, magnification and host scale distinct

Use a responsive content root inside the host-facing editor. All main painting, header/notices/help and screen children belong to the same logical composition. Apply user magnification once at that boundary; retain the host scale path independently. At 125%, the minimum 900x600 content needs 1125x750 presentation space before external DPI. Do not rasterize/stretch the complete editor.

Provide size presets 900x600, 1100x850, 1400x900 and 1600x1000 plus free resizing, and fitting 100%/125% magnification. Below-100% choices remain unavailable unless separately accepted; current approval concerns the responsive role scale at 100%, not 75% shrinking. Keep waveform zoom/viewport semantics independent of size changes. Verify popup anchors, transformed input coordinates and all waveform cache invalidation paths, including Learn/Export's independent drawing.

### 6. Persist presentation preferences locally

A small versioned user-local record stores desired logical size and UI magnification. Missing profile defaults to 900x600/100%. Validate fields independently; use coalesced atomic persistence outside the audio callback. No per-mouse-move writes, project dirty notification or processor/session serialization. Failures retain a usable in-memory editor.

Only the invoking open editor changes size; later openings use retained preferences. Other open instances do not jump. Screen-fit uses the peer/display work area and actual chrome allowance. Disable non-fitting choices and recover to fitting reviewed sizes/scales without destroying the desired preference when a temporary smaller screen forces recovery. If the baseline itself cannot fit, expose accessible recovery where the host permits and record the combination unsupported.

Apply the magnification/fit policy to owned Settings/Diagnostics without changing native chooser/chrome policy or hidden diagnostics polling. Do not perform disk I/O, allocation or blocking work in the audio callback.

## Risks / Trade-offs

- **Small text and narrower columns** -> preserve usable targets and full precision; review actual native resolved Inter metrics, long names, focus and typed edits. Mockup approval is not a native test.
- **Tab changes while editing** -> retain existing cancellation/publication guards and test selection/session replacement.
- **Header warning lengths** -> prove the actual normal warning fits, bound adaptation and retain accessible detail without routine ellipsis.
- **Two-row toolbar competes for height** -> measure real lane/handle/ruler room at 900x600 before production adoption.
- **Host/user/device scale composition** -> test input/menu anchors, auxiliary windows, cache resolution and reopen/display changes separately from software images.
- **#51/#80 work in flight** -> adapt delivered controls; do not merge unapproved work or add fake controls. Record integration gaps explicitly.
- **1080p at high Windows DPI may not fit** -> record actual owner DPI/work area; distinguish unsupported combinations from untested coverage.

## Migration Plan

**Landing-order gate:** The owner explicitly wants #58 to follow the other active
UI work. Retain interim layout work in this isolated branch while #51, #68 and
#93 complete their existing reviews and land. Rebase onto the accepted merged
baseline, reconcile shared controls/layout and test hooks, and preserve #73's
transport semantics where relevant. Do not absorb unfinished branches or widen
their scopes. In particular, retain #51's notice accessibility/speech policy,
independent mixer bindings and preview behavior; #68's stable transport slots;
and #93's automatic Fit on explicit selection. Re-run affected regression and
native checks, then directly launch the combined 900x600 preview for owner
acceptance before finalising. Interim previews remain available for layout
feedback and are labelled with their older baseline and missing integrations.

1. This reconciliation records the already-approved static revision and subsequent Apply request. Refresh vanilla Apply and read its context before code changes.
2. Extend focused regression/native harnesses for the approved current-control composition, responsive roles, fixed actions and full warning. Directly launch one native checkpoint and preserve its review window.
3. Adopt approved families/metrics incrementally, integrate all current modes, sizing/preferences and auxiliary recovery, and verify each task through refreshed Apply instructions.
4. Update production standards revision/living guidance with scoped native approval; retain dated browser/native/host evidence distinctly. Run Release wrapper tests, UI standards and guide currency.
5. Consolidate owner host acceptance into one brief 1920x1080 size/scale, Reset, mode-journey and reopen smoke test; open an issue-specific owner-reviewable PR, never self-merge or auto-archive.

Rollback ignores the optional local presentation record and restores prior UI code. No capture/render/IR/session/plugin identity migration or managed-asset deletion is required.

## Open Questions

- Actual Windows DPI and FL Studio chrome/work-area measurements remain validation evidence to collect, not scope decisions.
- Native optical adjustments to match the approved composition and new-family interaction review remain acceptance checkpoints. Any material departure must be reported before rollout.

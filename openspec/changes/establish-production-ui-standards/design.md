# Design

## Context

See [proposal.md](proposal.md) for motivation and #37 scope. Discovery baseline:
`main` at `2848fcd1faed56b4ba7c741abcb1620f8f102035` (PR #40), including Learn
PR #39 (`185f00f721d4372554c4c1d4961adfba0ee30526`). Both prerequisite issues
are closed as completed. No production file is changed by this proposal.

Relevant authorities:

- `docs/design/ReverbArise_UI_Layout_and_Style_Guide.md`: approved visual intent,
  but version 1.0 labels numerical suggestions provisional and includes older
  three-column/global-knob material that is not the Learn production layout.
- `docs/development/ui-development-standards.md`: existing ownership/interaction
  policy, numeric-family approval on 3 November and header #35 look/interaction
  approval on 4 November 2026.
- `docs/validation/2026-11-03-learn-capture-acceptance.md`: approved Learn A/B,
  focus/pin corrections and usual-scale host smoke; actual OS-DPI matrix untested.
- `docs/validation/2026-11-03-issue-35-header-navigation.md`: baseline and native
  test evidence; its candidate-era pending approval entry is historical. Use the
  later living standards approval record, not a fabricated rewrite of that entry.
- `openspec/specs/swell-workspace/spec.md`: current workspace contracts. At
  proposal discovery its italic description lagged the completed header delta.
  Implementation integrated main `fd33ae5` / PR #43, which already synced and
  archived that delta at `archive/2026-11-04-enhance-header-mode-navigation`.
  Its regular-description contract now agrees with production. This change does
  not repeat that sync/archive; `bound-waveform-paint-work` remains untouched.

### Observed inventory and gaps

| Pattern | Existing owner / consumers | Proposed disposition |
| --- | --- | --- |
| Embedded fonts, semantic inks, LookAndFeel | `WorkspaceTheme.h`; all workspaces | Extend existing role policy, retain explicit cached Regular/Medium/SemiBold/Bold/Italic faces |
| Spacing, focus and Learn geometry | `UiGeometry.h`; `LearnWorkspace`, `WorkspaceLayout` | Document scope, retain constants; distinguish breakpoint inset from future density |
| Section titles and labels/helpers | repeated setup in `LearnWorkspace`; custom timeline text | Small role application helpers in existing theme layer; use same roles for Label and Graphics |
| Switches | `Controls/Switch.h`, `InvocationToggle.h`; Learn | Reuse approved track, focus, invocation and keyboard behaviour |
| Slider + numeric up/down | `SliderControl.h`, `NumericInputControl.h`, `InvocationSlider.h`; Learn | Preserve range/format/notification path and active-entry ownership; remove redundant consumer defaults |
| Mode navigation | `SegmentedSelector.h`, `ModeIcon.h`; `WorkflowHeader` | Reuse accepted IDs, cached SVGs, shared-edge/focus painting and silent refresh |
| Settings/file actions | `IconActionButton.h`; header/Learn | Reuse semantic roles; no incidental name/text styling |
| Notices/information | `NoticeBar.h`, `HeaderInformation.h`; header and notice binding | Preserve header-only ink opt-in, severity and reserved row |
| IR selector | `InvocationComboBox` in `WorkspaceTheme.h`; Learn/other pickers | Extract named no-wheel IR presentation/default policy without moving managed-IR or pass/session guards |
| Empty states | Learn custom paint plus existing workspace prompts | Shared text/icon presentation roles; domain copy and visibility remain screen-owned |
| Knobs and other controls | `KnobValue.h`, legacy LookAndFeel button-name branches, settings/diagnostics, lists/windows | Inventory and retain explicit debt; no bulk migration or redesign |
| Specialized waveform controls | Learn canvas, `SwellTimeline`, Overview | Keep sample/hit geometry and coverage semantics local; share visual roles where touched |
| Tests/review | `ControlLibraryCases.h`, `ModeIconCases.h`, `WorkflowHeaderCases.h`, `NoticeBarCases.h`, `WorkspaceNoticeCases.h`, `LearnTimelineCases.h`, `swell_ui_test.cpp` | Extend actual event/geometry checks and existing review modes; no parallel demo application |

### Measurements observed in approved code

These are extraction inputs with consumer scope, not newly approved universal
defaults. Implementation must confirm against the unchanged baseline native fixture
before labelling the resulting production table validated. Units are JUCE logical
pixels/font heights, not CSS point sizes or actual Windows device pixels.

| Role | Observed value | Authority/scope |
| --- | --- | --- |
| Surface/accent/timing | `#0F1418` / `#00EFFF` / `#FFC700` | `WorkspaceTheme.h`; existing semantic roles |
| Primary/separator/outline/danger | `#F8FAFC` / `#4A5058` / `#6B737C` / `#FF6B5E` | Theme definitions; preserve distinct warning/data roles |
| Learn section / control label | 21 SemiBold / 14 Medium | `LearnWorkspace` using embedded faces |
| Learn helper / ruler | 14 Regular / 14 Medium; 20 line box | `learnTypography` and Learn paint/layout |
| Learn helper ink | `#C4CCD4` | Learn descriptions only |
| Navigation | 18 SemiBold; 16 icon + 8 text gap | `SegmentedSelector`, canonical SVG canvas |
| Header body | 14 Regular, `#C7CDD4`; 32 row | `HeaderInformation`, header NoticeBar opt-in |
| Header accent | 2 x 16; 8 gap and 8 trailing inset | `HeaderInformation`; keep distinct from helper ink |
| Tagline | 21 Italic | `WorkflowHeader`; do not apply regular description role |
| General control / small / caption | 13 / 12 / 11 | Existing theme roles, not proof every legacy consumer is accepted |
| Learn empty heading | 24 SemiBold in 32 box | Learn empty-state composition |
| Comfortable rhythm | tight 4, label-help 6, related 12, group 16, section 24 | `spacing`; actual Learn usage governs |
| Control / label / heading | heights 34 / 24 / 30 | `controlGeometry`; not universal line heights |
| Numeric entry | 74 x 28, stepper strip 18 | `controlGeometry`; preserve keyboard targets |
| Switch | track 44 x 24; component 56 x 36 | 6 focus padding each side |
| Focus | stroke 2, outset 3, padding 6 | Existing controls; bounds include complete stroke |
| Rule / radius | 1 / 4 | Theme; switches and joined segments retain own geometry |
| Learn settings / column gap | 304 / 28 | Learn comfortable composition |
| Outer inset / header | 16 below width 1200, otherwise 20 / height 82 | `workspaceGeometry`; header has its own inner-width breakpoint |

No generic line-height ratio exists: record font height and each actual line box,
including heading, helper, empty-state and information-row boxes. Retain separate
Learn `#C4CCD4`, header `#C7CDD4` and general `secondary` roles. Do not unify close
colours or replace header geometry under the guise of extracting tokens.

## Goals / Non-Goals

**Goals:** one traceable production authority, small reusable role/default helpers,
enforceable checks and preserved native appearance/interaction. Supply a clear
dependency handover before Perform visual implementation.

**Non-Goals:** a universal widget framework, all-consumer migration, new compact
numbers, rewriting waveform geometry, changing sample/state/asset identities,
audio behaviour, host automation or generated OpenSpec skills/configuration.

## Decisions

1. **Extend existing code authorities.** Keep theme roles in `WorkspaceTheme.h`
   and geometry in `UiGeometry.h`. Introduce named text roles and narrowly scoped
   application helpers for headings/labels/helpers/empty states, preserving exact
   face, size, colour, justification, borders and line boxes. Keep drawing-only
   token definitions separate from screen bindings. Centralise IR no-wheel
   presentation defaults using the existing invocation-aware combo mechanism;
   session/pass validation and chooser lifetime remain with existing bindings.
   A second token file/library or universal component would duplicate ownership.
   Apply the same helper to affected Settings/Diagnostics consumers only when
   they share the extracted policy; list all untouched alternatives explicitly.

2. **Publish a production section in the existing guide.** Retain branding and
   historical intent, clearly label provisional tables and superseded layout
   examples. Add role/value/unit/code/evidence/scope tables and control ownership.
   Update `docs/README.md`, development standards and concise `AGENTS.md` links.
   Creating another competing style guide would make agent lookup ambiguous.

3. **Comfortable is approved Learn spacing; compact is a future review state.**
   Document a profile contract with spacing/row/group/column fields only; current
   Learn constants implement comfortable. Do not add an unused runtime selector,
   user preference or fabricated compact constant set. Future compact adoption
   requires a named proposal against representative Perform dense/empty/error
   content, minimum/larger bounds, waveform room, hit targets, overflow and keyboard
   operation while preserving invariant font/colour/focus/control roles. The
   existing `compactInset` name is a width-breakpoint inset, not an approved
   compact-density profile. Unrelated legacy Perform values remain legacy.

4. **Use a scoped PowerShell lint without a new dependency.** Add
   `scripts/check-ui-standards.ps1`, command-fixture coverage in
   `tests/ui_standards_lint_test.ps1` and CTest registration alongside existing
   validation. Scan `Source/UI` plus actual editor consumers identified by the
   inventory. Check raw colour literals/construction, direct/default/system font
   construction and literal styling/interaction setters outside named definitions.
   Report path, line, rule and preferred role/helper. Token-definition files are
   explicitly owned; arbitrary literals in consumers are not permitted.

   Baseline remaining debt by exact path/rule/normalised occurrence and count,
   with rationale and follow-up scope. Do not blanket-ignore legacy files or all
   numbers. Check missing/stale exceptions and changed/new occurrence counts;
   adding another identical violation must fail. Comments and non-style geometry
   arithmetic must not trip the check. Approved exceptions carry a stable ID,
   owner-review reference and scope. Test negative examples, approved examples,
   duplicate occurrences, stale exemptions and paths with spaces. This is a
   targeted guard, not a full C++ parser or proof that visual review is unnecessary.
   Broad "ban all numeric literals" lint would obstruct legitimate layout work.

5. **Extend existing native tests and fixtures.** Use `preverb_swell_ui_test`
   and its `--review-controls`, `--review-messages`, `--review-learn-empty`,
   `--review-learn` and `--review-workspace` modes. Add role/embedded-face and
   component/binding tests for any helper migration. Preserve existing shared-edge,
   invocation/typed-refresh, keyboard, focus/pin and notice-boundary regressions.
   Check full focus ink containment, child bounds, text extents/line boxes, empty
   prompts, long notices/IR labels and generation on/off at 1100x850, 1400x900,
   1400x1000 and 1600x1000, including both sides of existing width breakpoints.
   Software raster 1/1.25/1.5/2 scale checks supplement geometry tests; use semantic
   assertions/tolerances rather than whole-image hashes or golden screenshots.

6. **Preserve acceptance and version changes deliberately.** Initial extraction
   must match the approved baseline; no broad appearance refresh. Production
   standards revision 1 is separate from product/plugin and persistence versions.
   Subsequent accepted changes increment the standards revision and record changed
   roles, consumers, reason, owner reference and evidence in the guide alongside
   the code change. New tokens/variants/exceptions require explicit owner review;
   materially revised/new control families require native look and feel approval
   before further rollout. Open an existing prepared preview directly, avoid
   duplicate windows and preserve it until review finishes. Future OpenSpec UI
   plans cite standards revision, reuse, exceptions and gates through ordinary
   artifacts; generated workflows/config stay unchanged.

## Risks / Trade-offs

- Extraction changes font metrics or clipping -> compare unchanged native baseline
  and test actual embedded face resolution/line bounds; preserve accepted roles.
- Lint false positives or broad grandfathering -> scoped rules, exact-count debt,
  fixtures and explicit diagnostics; do not claim semantic C++ analysis.
- Historical pending approval conflicts with later approval -> cite both with dates
  and scope, preserve old evidence, record any unresolved acceptance explicitly.
- Software scaling mistaken for OS/host coverage -> separate evidence columns for
  native owner approval, software raster, Windows DPI and FL Studio; never infer.
- Scope expands into a whole-plugin control rewrite -> extract only affected
  presentation/default policies and inventory remaining consumers for later work.

## Migration Plan

1. After proposal review and a fresh Apply request, confirm the baseline commit and
   prerequisite evidence, capture unchanged native fixture measurements and commit
   the scoped inventory/production tables before widening extraction.
2. Extract named roles/defaults in small increments; migrate Learn/header duplicate
   setup with component and binding checks. Preserve model paths and domain units.
3. Add lint, bounded debt and fixtures; extend native geometry/event coverage.
4. Run `scripts/build.ps1 -Configuration Release -Test` (and Debug when affected
   assertions/debug-only paths require it). Record lint fixtures separately from
   native MSVC/CTest. Inspect minimum/larger native render, then obtain any required
   new-family approval before wider integration.
5. Record the visual checklist for 100/125/150/200% Windows DPI, monitor transitions
   where feasible, and one brief final FL Studio smoke only for remaining concrete
   host behaviour. Preserve prior passing acceptance; untested matrix entries stay
   untested. Record an explicit handover and separate implementation PR for #37.

Rollback is an ordinary revert of a cohesive UI/tooling/docs increment; no saved
state or asset migration is introduced. Proposal merge alone neither closes #37
nor establishes that the production implementation is complete.

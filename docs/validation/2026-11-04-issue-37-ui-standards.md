# Issue #37 — production UI standards

Issue: [#37](https://github.com/elneilios/ReverbArise/issues/37). Change:
`establish-production-ui-standards`. Proposal PR: [#42](https://github.com/elneilios/ReverbArise/pull/42).
Implementation branch: `codex/37-production-ui-implementation`.

## Baseline and scope

Production baseline is main `2848fcd1faed56b4ba7c741abcb1620f8f102035`, plus
proposal-only commit `57cef97`. The extraction changes no DSP, model validation,
persistent state or assets. The completed header change and waveform-paint proposal
remain separate. The header's unarchived delta owns the regular-description spec
correction; this implementation does not archive or sync it.

Prior acceptance is supported by records, not merely closed issues:

- [Learn final acceptance](2026-11-03-learn-capture-acceptance.md) records owner A/B,
  focus/pin/readout acceptance and the prior usual-scale FL Studio smoke.
- [UI development standards](../development/ui-development-standards.md) record
  the numeric family approval on 3 November and header #35 native look/interaction
  approval on 4 November 2026. The earlier header candidate report's pending entry
  is historical; its test results do not prove actual Windows DPI or host approval.

## Execution map

| Tasks | Work / risk | Verification boundary |
| --- | --- | --- |
| 1.1–1.2 | Baseline/docs, low | Unchanged native fixture, measured roles, evidence and links |
| 2.1 | Shared typography/presentation, medium | Embedded faces, metrics, geometry, native equivalence |
| 2.2 | Shared interaction defaults, medium | Real wheel/key/typed entry, stale intent and existing binding suite |
| 2.3 | Family acceptance, medium | Preserved native geometry/events; revised families reopen owner gate |
| 3.1 | Lint/build integration, medium | Positive/negative command fixtures, bounded debt, clean scan |
| 3.2 | Agent/change governance, low | References, example lifecycle and unchanged generated tooling |
| 4.1–4.2 | Native regression/integration, medium | Size/scale geometry, render inspection, wrapper/CTest |
| 4.3 | Handover/review, low | Separate implementation PR, explicit remaining coverage |

## Affected pattern inventory

| Pattern | Owner and affected consumers | Remaining scope |
| --- | --- | --- |
| Fonts and text roles | `WorkspaceTheme.h`; Learn titles/labels/helpers/empty text, header tagline/body | Legacy Perform, Settings/Diagnostics text is not silently restyled |
| Spacing/focus | `UiGeometry.h`; Learn/header control bounds | Existing width insets are not compact-density approval |
| Switch | `Controls/Switch.h` and `InvocationToggle.h`; Learn | Legacy checkbox families remain distinct |
| Slider/numeric up/down | `SliderControl`, `NumericInputControl`, `InvocationSlider`; Learn | KnobValue/legacy sliders retain their current domain behaviour |
| Selector | `InvocationComboBox`; Learn and selected-swell IR use the same no-wheel policy | Other menus/selectors keep their existing semantics |
| Mode navigation/icons | `SegmentedSelector`, `ModeIcon`; `WorkflowHeader` | Accepted segment geometry/joins/cache remain unchanged |
| Action buttons | `IconActionButton`; header Settings and Learn file action | Legacy button-name theme branches remain inventoried debt |
| Notices/information | `NoticeBar`, `HeaderInformation`; header | Header-only body ink opt-in and severity remain distinct |
| Empty/timeline drawing | Learn, `SwellTimeline`, Overview | Reuse text roles; preserve domain copy, timing/hit geometry and coverage |
| Settings/Diagnostics/windows/lists | existing editor and specialized controls | No all-plugin migration; only shared affected defaults migrate |

## Validation results

Unchanged baseline: `scripts/build.ps1 -Configuration Release -Test`, with
`CMAKE_BUILD_PARALLEL_LEVEL=6` and `PREVERB_UI_M3_OUTPUT=out/issue37-baseline`
(absolute path supplied). Elevated native execution used the installed VS 2026
toolchain, with a fresh per-worktree dependency/build tree. Build succeeded;
`preverb_swell_ui_test` passed (18.30 s); full Release CTest passed 38/38 (60.88 s).
Logs: `out/build-logs/Release/20261104-020919-ee643337/`.

Inspected unchanged `fresh-1100.png` (1100x850) and `fresh-1400.png` (1400x1000):
headings, two-line helpers, switch tracks, complete column/viewport outlines,
numeric stepper and header information row are readable and contained. Existing
native tests verify actual static Inter faces, 14-pixel description, 2x16 accent,
line/focus geometry, keyboard and shared-edge behaviour. These are extraction
measurements, not a new look/feel approval or physical-DPI test. The production
guide's role/value/unit/authority tables record the corresponding code values.

The baseline generated 203 native/software-rendered fixture PNGs, including empty,
mapped, generation on/off, all header modes and error/focus states. The initial
empty IR label can be temporarily not ready while asynchronous preparation runs;
this existing truthful state is not a styling difference.

Actual Windows 100/125/150/200% scaling, monitor transitions and new FL Studio
checks have not been performed for #37. Software raster scaling is separate.

## Text-role extraction (task 2.1)

Release wrapper build passed (`20261104-022211-777f46df`); focused native CTest
`preverb_swell_ui_test` passed 1/1 (19.66 s), with renders in
`out/issue37-text-roles/`. `UiStandardsCases` checks nine roles against the prior
faces/heights/inks/line boxes, actual glyph widths and label/custom-paint pixel
equivalence. Existing native geometry and binding tests also passed.

Reinspected the 1400x1000 Learn render. The sampled `header-empty-1068.png`,
`header-ready-1360.png` and `fresh-1400.png` are byte-identical to the unchanged
baseline. This is extraction evidence, not a permanent screenshot hash gate.
No family look or interaction changed, so prior scoped owner acceptance remains.

## Shared defaults and family review (tasks 2.2–2.3)

Release wrapper build passed (`20261104-022441-4de98ad6`). Focused native CTest
passed 1/1 (20.70 s), including wheel/key/typed numeric binding, existing pass locks,
IR preparation/failure and session ownership cases. Added actual wheel/arrow-key
tests verify IR wheel no-op, silent refresh, accepted selection and a queued key
intent rejected after its binding epoch changes. The same prior invocation-aware
combo mechanism is now `controls::IrSelector`; Learn and selected-swell IR share
its single no-wheel default. No additional Settings/Diagnostics IR wheel policy
exists; their unrelated selectors and KnobValue retain current behaviour.

The same three sampled header/Learn PNGs remain byte-identical after consolidation
(`out/issue37-shared-defaults/`). Switch, NumericInputControl, SliderControl,
SegmentedSelector, IconActionButton, HeaderInformation and NoticeBar retain their
existing geometry/input/disabled/focus/shared-edge tests and pass. IrSelector
extracts the existing accepted selector policy; it introduces no new appearance
or interaction family. No family is materially revised, so no new owner checkpoint
is required and prior passing look/feel acceptance is not requested again.

## Styling enforcement and governance (tasks 3.1–3.2)

The repository scan passes across 32 governed files, with 61 exact bounded
occurrences in 48 entries: 38 shared definitions, two shared defaults and eight
retained consumer-debt entries. Entries are not a claim that legacy consumers
have all been visually approved. The inventory contains baseline/review reference,
rationale and scope; changing/removing or duplicating an occurrence fails.

Command fixtures pass **35 assertions each** on Windows PowerShell
5.1.26100.9818 and PowerShell 7.6.9. Registered CTest lint/check tests pass 2/2.
During review, an initial heuristic mistook rectangle `withHeight` operations and
font-returning declarations for font construction; the rules were narrowed and
fixtures added. Ordinary layout arithmetic now passes without those exemptions.
The default-path invocation was also corrected for PowerShell parameter binding.
These development failures were not recorded as passing runs.

The guide and `AGENTS.md` now point to the same revision/controls authority.
Development standards document command scope, parser limitations, exact debt,
versioning and a worked future Perform exception lifecycle. Local links pass;
generated OpenSpec skills/configuration are unchanged.

## Geometry and visual checklist (task 4.1)

Release wrapper build passed (`20261104-022735-985fa51d`); focused registered
CTest passed 3/3, with native UI 1/1 (21.64 s). `UiStandardsLayoutCases` checks
empty and mapped Automatic Swells on/off at 1100x850, 1199x900, 1200x900,
1239x900, 1240x900, 1400x900, 1400x1000 and 1600x1000. These include outer and
inner-header breakpoint boundaries. Actual text widths, line boxes, child bounds,
empty groups and complete switch focus ink are checked, with 1/1.25/1.5/2 native
software snapshots. Existing family, notice and editor tests cover keyboard/edit
lifecycle, long content, focused/disabled/error states and rejected intents.

Inspected `out/issue37-validation/standards-automatic-on-1199x900.png` and
`standards-automatic-off-1100x850.png`, along with earlier minimum/larger empty
renders: text, controls, focus/column/viewport outlines and reserved header row
remain contained; the dry canvas keeps its size and dominance. The reusable native
checklist and test tolerances are in UI development standards. No brittle permanent
screenshot gate was added. These results do not establish physical Windows DPI
or FL Studio acceptance.

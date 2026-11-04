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

# Issue #35 — header navigation candidate

Issue: [#35](https://github.com/elneilios/ReverbArise/issues/35).
Change: `enhance-header-mode-navigation`. Branch:
`codex/35-header-mode-navigation`.

## Integration baseline

Proposal-only PR #34 merged as `4f0f62d`. Learn implementation PR #39 merged
as `185f00f721d4372554c4c1d4961adfba0ee30526`; this branch starts there.
The Learn source commit is `598b998`. Its published selector changes remove
per-child border strokes and paint all segment borders in `paintOverChildren`,
with hovered/pressed borders last. This protects the overlapping shared edge
against neighbouring selected fills. `resized`, `outlineFor`, keyboard focus,
child order, hit rectangles and model-owned request handling remain unchanged.

Preserved regression: `ControlLibraryCases` loops over all adjacent selected/
hovered pairs and checks the shared edge, top/bottom edges, selection/focus and
child order. The Settings inset-focus geometry assertion is also retained.
The candidate selector diff adds only semantic content and cached drawing.

## Execution map

| Task | Work / risk | Dependency and verification |
| --- | --- | --- |
| 1.1 | SVG embedding/cache, medium | Exact approved SVG copies; native parser, geometry, handle transparency, ink and 14/16/20/24/32 px raster tests |
| 1.2 | Integration baseline, medium | Published PR #39; preserved shared-border regressions |
| 2.1 | Selector visual/interaction, medium | 1.1/1.2; existing interaction suite with icons and without icons |
| 2.2 | Information-row visual, medium | Native candidate fixture; bounded columns, regular Inter, severity, short/long text and unchanged geometry |
| 2.3 | Owner checkpoint | Launch native candidate; obtain look and interaction acceptance before rollout |
| 3.1 | Production binding, medium | Owner acceptance; real header/notice/model regression tests |
| 3.2 | Living documentation, low | Implemented/approved appearance; dated screenshots and evidence |
| 3.3 | Final build/review | Wrapper, native tests and honest Windows DPI/host gaps; separate implementation PR |

## Candidate scope

`ModeIconCache` loads exact SVG copies embedded from `resources/icons/`, keeps
parsed originals immutable and prepares separate inactive/active drawables before
paint. Draw transforms map the original 16-unit canvas directly, preserving its
margins rather than stretching each icon's painted bounds. No SVG parsing or
recolouring occurs in paint. Icons add no focus targets or accessible labels.

`HeaderInformation` draws the candidate 2×16 aqua accent and 14 px regular Inter
body. `NoticeBar` has an explicit header-body appearance opt-in; other consumers
retain their existing colour. Candidate composition lives in `--review-messages`.
Production `WorkflowHeader`, description copy and notice binding are unchanged
pending the required owner checkpoint.

## Validation status

- Release wrapper build passed. Full CTest ran 37/38 successfully; the added UI
  test exposed a test constness error during compilation, then image-context
  lifetime and antialiasing expectations during native pixel checks. Those test
  defects were corrected. Final focused CTest `preverb_swell_ui_test` passed
  (1/1, 6.44 s); the other 37 passing tests' source inputs did not change.
- Final build log: `out/build-logs/Release/20261103-160153-f4d2bbc7/`.
  Full-suite log: `out/build-logs/Release/20261103-155613-26b5a589/test.log`.
  Final focused CTest log: `out/build/windows-msvc/Testing/Temporary/LastTest.log`.
- An initial sandboxed configure could not read the Windows SDK registry. The
  subsequent elevated wrapper configured successfully. Its serial compile was
  deliberately stopped with its compiler children, verified stopped, and resumed
  using `CMAKE_BUILD_PARALLEL_LEVEL=6`. No raw CMake/MSBuild build overlapped the
  wrapper. These failed/interrupted attempts are not test passes.
- SVG resource copies match the proposal sources byte for byte. Strict OpenSpec
  validation and `git diff --check` pass.
- Native render checks cover 14/16/20/24/32 px, source/active/disabled ink, finite
  in-bounds paths, and three transparent outlined handles. At 16/20 px the hole
  pixels retain partial antialiasing; at 32 px all sampled centres are transparent.
- Both icon/no-icon interaction suites pass, including preserved joined-border
  regression tests, rejected/stale requests, keyboard and accessible state.
- Information-row checks pass at header widths 1068/1199/1200/1360/1560, all
  modes, short/long notices, all severities and software raster scales
  100/125/150/200%. Default NoticeBar marker pixels remain unchanged.
- Inspected native candidate screenshots at minimum/larger header widths:
  [Perform](2026-11-03-issue35-header-perform.png),
  [Learn, large](2026-11-03-issue35-header-learn-large.png),
  [Export](2026-11-03-issue35-header-export.png).
- Owner look/interaction acceptance: pending.
- Native `--review-messages` window launched for the owner, process 53744,
  title `ReverbArise control review`. It remains open; do not relink its executable
  until the owner has finished. Task 2.3 stays unchecked pending acceptance.
- Real Windows 100/125/150/200% scaling, monitor transitions and FL Studio: untested.
  Software raster scale samples do not establish OS or host acceptance.

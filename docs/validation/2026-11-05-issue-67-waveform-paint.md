# Issue #67 — waveform painting

## Approval and matched baseline

The owner invoked `$deliver-issue #67` on 5 November 2026 after creation of the
focused sub-issue. This authorizes Apply of the existing proposal
`bound-waveform-paint-work`; no new control family, visual token, audio/state
contract or asynchronous rendering architecture is authorized.

Baseline production source: fetched main `7fc2f08`, in Windows worktree `11b7`.
The only source additions while building/measuring were an unused display helper
and its independent test; existing production sources were unchanged. Windows
10.0.26300, AMD Ryzen 5 7600X (6 cores / 12 logical processors); same audit
machine as #25. MSVC 14.51.36231, Visual Studio 18 2026, C++20 Release `/O2 /MD`,
JUCE 9.0.4; no audio driver or FL Studio is involved in the software measurements.

Initial all-target wrapper `20261105-013219-f167425c` configured successfully and
compiled the production library, but was intentionally stopped before completing
the redundant baseline test-target build. Its identified process tree was
stopped and absence of remaining MSBuild processes verified. No test result is
claimed for this invocation. Dependency download/native compilation in this
worktree was cold; its usable outputs were retained. The succeeding
`scripts/build.ps1 -Configuration Release -PluginOnly` passed, log
`20261105-013941-b7e5d4f5`, reusing those outputs. CTest was not requested there.

Reused the [#25 UI fixture](2026-11-03-performance-harness.md#disposable-software-ui-benchmark),
adding per-observation CSV recording. Timeline **800×380**, inline Overview
disabled; separate Overview **1050×125**. Nine combinations of 3/5/20 minutes and
10/100/200 swells, full view and zoom/scroll, five warmups and 50 timed paints per
component: **36 rows / 1,800 timed observations**. p50/p95 use sorted indices 24/46 as in the
original audit, maximum index 49. This is component paint cost, not host latency
or complete editor-refresh time.

The inherited harness used JUCE's default image type, despite its documentation
calling it software rendering. JUCE 9's Windows implementation can select
Direct2D, and native image output was not complete while its Graphics context
remained alive. Initial inherited command timings (five-minute/200-swell p95
141.024 / 151.820 ms) are retained separately, **not used as completed-software
raster evidence or the final improvement baseline**:
[summary](2026-11-05-issue-67/inherited-native-summary.csv),
[raw](2026-11-05-issue-67/inherited-native-raw.csv).

Corrected both revisions to explicit `SoftwareImageType`. Reconstructed the
unchanged main timeline/header from `git show 7fc2f08` and compiled them directly
for the baseline, ahead of unchanged non-UI library dependencies. The candidate
uses the same fixture/compiler/dimensions. Runs were serial, with no competing
build. The corrected five-minute/200-swell full-view baseline p50/p95/max is
**55.5272 / 59.1587 / 63.6150 ms** for timeline and
**39.8416 / 43.2419 / 45.5640 ms** for Overview.

- [Baseline summary](2026-11-05-issue-67/baseline-summary.csv)
- [Raw baseline distributions](2026-11-05-issue-67/baseline-raw.csv)

## Reference test development

Before changing the drawing path, the independent native pixel oracle passed
for 4,000 synthetic peak slots, one-sample impulses, silence/holes, negative
lead-in, translated data, narrow/full views and 1/31/800/1600 columns. A separate
int64-boundary translation check passes on MSVC. The oracle intersects every
pixel interval with every input peak independently of the production reducer.
It first exposed an exact-pixel-boundary rounding leak, then MSVC's double-width
`long double` translation precision; both were corrected and the native suite
passed before wiring the reducer into the timeline. No audio code was changed.

## Implementation and review corrections

Three replaceable GUI-owned images preserve dry/wet paint ordering and leave
rulers, targets, handles, envelopes, viewport and cursors live. Visible peak
search/reduction and cache metadata comparisons never perform PCM analysis,
asset writes or work in the callback. No dependency, saved format, DSP, control
family, token, layout or worker architecture changed. See the
[living ownership/invalidation guide](../development/waveform-display.md).

Focused native component checks pass for disabled-inline and distant-range work,
cursor reuse, immutable pointer/source publication, selection, shape/gain/placement,
activity/phase, availability and retained-detail fencing, restore/session epoch,
Height, theme, pan/zoom, resize and 125/150/200% software scale. Display operations
leave the independent engine's definitions and render count unchanged. Existing
native processor/editor tests retain actual model bindings and persistence/audio
coverage.

The owner rejected the first native preview: the dry lane looked empty and the
wet waveform looked jagged/blocky on zoom. That preview used constant test input
and 200 overlapping copies, rather than representative musical transients. The
review fixture now uses decaying oscillatory attacks and one prepared swell;
timed editor benchmarks keep their original input for comparability. The renderer
also no longer stretches a coarse peak bucket across multiple pixels in detail
lanes: it retains the established narrow, sample-anchored stroke at deep zoom.
The independent helper and real component now test that rule. No fake finer PCM
or viewport-driven source-loading architecture was added.

The corrected native window is directly launched and its process kept attached
until owner close. A software screenshot is supporting evidence; it does not
replace owner acceptance. On 5 November the owner reviewed the corrected native
window and replied **“Looks correct now”** for dry/wet appearance and zooming.
This is native visual acceptance, not FL Studio or VST3-host acceptance.

## Measurements and memory

The first candidate matrix, before the owner-requested close-zoom correction,
met the full-view targets but showed six zoom-scroll timeline rows slower than
baseline. Its [summary](2026-11-05-issue-67/pre-review-candidate-summary.csv) is
retained as intermediate evidence, not the final result.

The final corrected Release binaries were measured after the final wrapper
exited, with no competing build. The native review window remained available;
these measurements still exclude DAW/compositor/input latency. The nine-case
matrix has 36 summary rows / 1,800 timed candidate observations, plus 180 recorded
warmup/first-build observations:

| Five minutes / 200 swells | Baseline p95 ms | Final p95 ms | Reduction |
| --- | ---: | ---: | ---: |
| Timeline, full | 59.1587 | 1.8311 | 96.905% |
| Overview, full | 43.2419 | 0.5174 | 98.803% |
| Timeline, zoom/scroll | 3.3795 | 3.3154 | 1.897% |
| Overview, zoom/scroll | 44.6547 | 0.4056 | 99.092% |

Both design targets pass: full-view p95 falls by at least 50%, and unchanged
Overview p95 is below 16.7 ms. The highest Overview p95 across all rows is
0.5599 ms. **This is not a universal speedup**: five zoom-scroll timeline rows
regressed, with candidate p95 2.4961–4.6585 ms versus baseline 1.4712–3.5995 ms.
They are 3-minute 10/100/200-swell and 5-minute 10/100-swell cases. Changing a
viewport rebuilds images; small visible scenes can cost more than the original
few-peak direct paint. No target failure, maximum or slower row is omitted.

- [Final summary](2026-11-05-issue-67/candidate-summary.csv)
- [Every baseline/final p95 comparison](2026-11-05-issue-67/comparison.csv)
- [Raw final distributions, including first builds](2026-11-05-issue-67/candidate-raw.csv)
- [Cache builds/hits, visited peaks, emitted columns and image bytes](2026-11-05-issue-67/candidate-cache.csv)

Construction is separate from cache hits. Final first-build observations for
200 swells at 3/5/20 minutes were 7.34/7.40/8.34 ms timeline and
12.77/14.07/16.08 ms Overview; these are single cold-layer observations, not p95s.
The benchmark geometry retains **2,957,000 bytes** of images (two timeline layers
plus Overview), approximately 2.82 MiB at 100%, four times that bitmap footprint
at 200%. Peak/column counters describe the last rebuild; build/hit counters are
cumulative across the matrix's reused components.

Fresh-process five-minute/200-swell memory supplements are separately sampled,
not a complete allocation trace. Full-view setup-to-Overview private-byte growth
was 7,618,560 bytes baseline versus 10,600,448 candidate (difference 2,981,888).
Cumulative peak working set was 34,574,336 versus 38,092,800 bytes. These precede
the narrow-stroke visual correction; it adds no retained cache storage. Raw
[baseline](2026-11-05-issue-67/memory-baseline.csv) and
[candidate](2026-11-05-issue-67/memory-candidate.csv) retain all sampled stages.

The full production-editor fixture separately captures 4.096 seconds of real
disposable input and restores one/200 definitions with verified asset references.
It times refresh plus the full 1100×850 software paint; it is not the full-song
component matrix. Baseline p50/p95/max were 3.8797/4.6893/5.2126 ms (one swell)
and 3.5980/4.7112/4.8865 ms (200). Final values were 3.4282/4.2081/5.5591 ms and
3.3184/3.8551/4.7716 ms respectively. Candidate checks verify unchanged encoded
session and source reference across refresh/painting. Raw
[baseline](2026-11-05-issue-67/editor-baseline.csv) and
[final](2026-11-05-issue-67/editor-candidate.csv) retain all 50 observations per
scene. The one-swell maximum rose despite its lower p95; no claim removes that
tail. These are software results, not real-song or FL Studio throughput.

## Native validation status

- Release plugin-only wrapper passed, log `20261105-014921-17289681`.
- First full Release wrapper `20261105-020002-20bded9e` passed **68/68**, 183.64 s.
  Its plugin library preceded the close-zoom correction; this is retained
  intermediate evidence, not substituted for the final source's gate.
- Corrected helper/component native tests pass using a directly compiled current
  timeline object and existing unchanged dependencies under ignored `out/audit/`.
- Final corrected-source Release wrapper `20261105-021712-b679b3cc` passed
  **68/68 CTest**, 178.08 s. The plugin library rebuilt after the final production
  edits; both new tests and all existing processor/UI/audio/persistence suites
  pass. Four build workers reused the existing native tree and wrapper lock.
- UI standards passed: **51 files / 61 unchanged bounded occurrences**.
- `openspec validate bound-waveform-paint-work --strict` passes. Global strict
  validation has the eight existing description-length warnings recorded by the
  [housekeeping audit](2026-11-05-openspec-housekeeping-audit.md); this issue does
  not restructure those unrelated approved requirements.
- `git diff --check` passes. [Validated source/module hashes](2026-11-05-issue-67/source-hashes.json)
  identify the final production/test sources and Release module; bookkeeping
  edits after the gate do not change those sources. Source hashes describe the
  validated working bytes before any Git line-ending normalization.
- Debug, FL Studio, physical Windows DPI and screen-reader coverage are not
  claimed. One consolidated host smoke, if required for acceptance, should check
  dry/wet visibility, selection and zoom/pan during ordinary Perform playback;
  it should not repeat passing automated edge cases. No user installation,
  cache, capture, IR, render, export or saved project was changed.

## Reproduction and review state

After `scripts/build.ps1 -Configuration Release -Test`, create `out/issue67/`
and run the optional Release executables serially, with a fresh measurement
prefix: `preverb_waveform_paint_benchmark.exe out/issue67/run-raw.csv` (stdout is
summary CSV) and `preverb_waveform_editor_benchmark.exe` (stdout contains raw
full-editor refresh/paint observations). Their C++ fixtures are tracked under
`tests/`; the full historical baseline compilation uses the same include/define/
link settings documented by #25, substituting `git show 7fc2f08` timeline/header
files ahead of unchanged dependencies. No raw CMake/MSBuild operation overlaps
the wrapper. Disposable compiler response files and intermediate binaries remain
ignored under `out/`.

The native preview uses `preverb_waveform_editor_benchmark.exe --review`; keep
its launching process alive until owner close. `--capture` produces this
[supporting software raster](2026-11-05-issue-67/native-review.png). The first
preview's constant input, summary stretching and superseded harness linkage
attempt are not presented as accepted visual evidence. The owner accepted the
corrected native appearance/zooming. No actual FL Studio or physical-DPI result
is claimed, and no issue closure, OpenSpec sync/archive or self-merge is performed.

Delivery: [PR #69](https://github.com/elneilios/ReverbArise/pull/69), open for owner
review and merge. Production/test/evidence commit `5c0eba2` contains the validated
implementation; the follow-up is delivery tracking only. Refreshed vanilla Apply
confirms **7/7 tasks complete / all_done**. The existing change remains active
for the owner-controlled sync/archive lifecycle.

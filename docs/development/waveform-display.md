# Perform waveform display

Issue #67 applies `bound-waveform-paint-work` to the existing Perform timeline
and separate Overview. Learn and Export retain their own drawing paths. No audio,
asset, persistent-format, control-family or layout change is involved.

`Source/UI/WaveformDisplay.h` owns GUI-only peak reduction and replaceable image
layers. `SwellTimeline` owns one dry/inline-Overview layer and one wet layer;
`SwellOverview` owns its own content layer. Their original paint ordering is
retained. Rulers, target/selection markers, handles, envelopes, viewport and
transport/audition cursors remain live and use canonical sample coordinates.

## Reduction and fidelity

Ordered nonoverlapping peak intervals are searched from the first visible
intersection. Dense data reduces to minima/maxima in logical pixel columns;
it does not average away impulses. Empty columns remain empty. Captured silence,
unavailable ranges and gaps keep their existing classifications. Selected dry
detail is still clipped to currently available captured audio.

At deep zoom, a coarse bucket must not become a solid rectangle spanning its
whole duration. Detail lanes retain the existing narrow stroke at that peak's
sample-derived position when it spans more than a pixel. Overview continues to
summarize interval coverage and shaped wet extent. Neither path fabricates finer
PCM between summary points; the existing approximately 2,048-peak wet/detail
resolution remains a limit at extreme zoom. Viewport-driven PCM-detail generation
would require a separate scoped design, rather than interpolation presented as
actual samples.

The reducer checks half-open interval intersections after candidate column
rounding, avoiding leakage across exact pixel boundaries. Translated samples use
checked integer-space addition before conversion where representable, including
on MSVC where `long double` has double precision. Existing amplitude roles,
normalization and wet envelope evaluation remain in their consumers.

## Ownership and invalidation

All images and temporary columns are built, replaced and retired on the GUI
thread, after snapshots leave engine/capture locks. They never participate in
audio publication or asset writes. A layer is one image, replaced rather than
accumulated. Each image is bounded to at most 16 million pixels, including
rounding/skinny-dimension guards; supported 200% editor rasters retain their
physical resolution. Temporary column storage is proportional to lane width.

`SwellTimeline::setData` compares accepted metadata, not PCM or peak contents:

- Capture reference, manifest revision, project/instance, rate, range identities,
  range availability and peak-vector sizes. Producers must retain the established
  contract that changed accepted peaks carry a new source/range identity/revision.
- Session epoch, primary identity, immutable shared raw/shaped/source-detail and
  summary pointers, plus each swell's identity, source/arrival, placement, shape,
  activity, phase, generations and render identity/bounds.
- Timing validity/rate/tempo/meter where these affect display extents.

Changed metadata invalidates the three consumers. Timeline layers also key their
viewport; the separate Overview keys the global extent and therefore survives
ordinary pan/zoom and cursor-only refresh. Size, physical graphics scale, display
Height, inline-Overview enablement, LookAndFeel and colour changes invalidate the
appropriate layers. Source and wet still use their existing shared Height policy.
An unchanged cursor update reuses images without traversing waveform peaks.
Disabled inline Overview does no waveform reduction/drawing.

`waveformPaintMetrics()` and `wetPaintMetrics()` expose native-test observations:
build/hit counts, last-build visited peaks/emitted columns and retained bitmap
bytes. They are GUI-thread observations, not product controls or audio metrics.
Existing snapshot copying and bounded swell/range metadata scans remain; this
change does not migrate the snapshot model or add a worker.

## Verification

- `preverb_waveform_display_test`: independent pixel-interval oracle, extrema,
  impulses, gaps/silence, negative and int64-boundary placement, visible search,
  and the narrow-stroke deep-zoom rule.
- `preverb_waveform_paint_test`: real components with accepted snapshot fixtures;
  hidden-inline/visible work, cursor reuse, publication/selection/edit/availability
  invalidation, restore, theme, pan/zoom, resize and software scale.
- Existing native processor/UI/persistence suites retain actual model bindings,
  source/render identities, gesture guards and callback safety coverage.
- Optional `preverb_waveform_paint_benchmark` measures the 3/5/20-minute ×
  10/100/200-swell matrix into **explicit `SoftwareImageType`** surfaces. Its output
  path argument records raw observations and cache bytes; stdout is summary CSV.
  Optional `preverb_waveform_editor_benchmark` records full production-editor
  refresh-and-paint timings with disposable real capture and verified renders.
  `--review` opens a disposable production editor; `--capture` writes a supporting
  native raster without opening a window. Review uses decaying transient input;
  the matched editor benchmark retains its original constant-level test input.

Create `out/issue67/` first, run benchmarks serially after the build wrapper exits,
and retain revision, compiler, dimensions and raw distributions. Construction,
unchanged paints and full editor refresh are different costs. Native-image
backends may defer drawing, so a default JUCE image is not a software benchmark.
See [dated evidence](../validation/2026-11-05-issue-67-waveform-paint.md) for results,
trade-offs, owner feedback and actual host/DPI coverage.

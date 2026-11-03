# Issue #25 performance audit — 3 November 2026

## Executive summary

Audit of [Issue #25](https://github.com/elneilios/ReverbArise/issues/25), against
fetched `origin/main` **752b3a33865ca6ec3e2e7b4b13baac839509b8bb**. Production DSP,
UI, workflow, persistence, user audio and existing caches were not changed.
The independent checkout is `issue-25-performance-audit/k7ma-preverb`, branch
`codex/25-performance-audit`. Unmerged #11 Learn workspace work is excluded.

- Perform does **not** recapture input. Learn alone queues source PCM; Export
  bypasses DAW wet scheduling. This is implemented mode isolation, not evidence
  that all three future workspaces or dedicated WAV export are complete.
- No plugin-owned allocation, lock, file access, wait or final shared-buffer
  destruction was found on the inspected **real-time** callback paths. This is
  source review plus focused C++ allocation guards, not a universal host guarantee.
- Playback has a **64 simultaneous-swell limit**, independent of its 4096 total
  definition capacity. 100/200 total swells are meaningful tests; 100/200 fully
  overlapping swells exceed the implemented contract and are explicitly refused.
- The **256 MiB budget is not an instance RSS cap**. Admission estimates include
  render scratch and playback pins, but capture, UI snapshots, executable/resource
  mappings, allocator overhead and several local metadata objects live outside it.
- **Confirmed software UI bottleneck:** five-minute/200-swell full-view paints
  took 151 ms p95 for the timeline and 154 ms for the overview. The evidence
  justifies the proposed future `bound-waveform-paint-work` change.
- Native Release tests passed **35/35**. The one-swell full-processor baseline
  had no deadline misses across 27,000 callbacks, but broader component sweeps
  recorded rare elapsed-time deadline exceedances. Host timing remains unresolved;
  no FL Studio performance or dropout acceptance is claimed.

## Environment and reproducibility

Windows 11 Pro 10.0.26300 (build 26300), AMD Ryzen 5 7600X, 6 cores / 12 logical
processors, 33,436,258,304 bytes installed RAM reported by Windows (~31.14 GiB).
The active Windows power plan was High performance. Benchmarks used ordinary
application threads without audio-driver/MMCSS scheduling or CPU affinity.
VS 2026 Community, MSVC 19.51.36260.0 (tools 14.51.36231), Windows SDK
10.0.26100.0, x64 Release, JUCE 9.0.4 and repository-pinned WavPack 5.9.0.
No package or production dependency was added. A first build in this new checkout
populated the repository's pinned dependencies; no second clean build was used.

Build command: `./scripts/build.ps1 -Configuration Release -Test`. Its exclusive
build-directory lock was respected. Logs:
`out/build-logs/Release/20261103-015020-3f6dbb89/`; native CTest log:
`out/build/windows-msvc/Testing/Temporary/LastTest.log`. The initial configure took
68.9 seconds. Tests use their own `PvM2-<uuid>` temporary directories; audit
executables and CSVs use ignored `out/audit/`. No user cache was benchmarked.

The [disposable harness and compiler commands](2026-11-03-performance-harness.md)
reproduce actual scheduler and software UI component measurements. QPC timing
surrounds unchanged functions; samples/counters and CSV output are outside calls.
See the accompanying raw metrics for values, not rounded numbers alone.

## Native results and limitations

Raw evidence is in [2026-11-03-performance/](2026-11-03-performance/). All values
below are native x64 Release results, using synthetic disposable inputs through
the actual implementation. No simulated DSP algorithm or FL Studio run is
included. Timers are not subtracted from results.

### Callback and scheduling measurements

The three shared-PCM scheduler sweeps ran after the Release build/test wrapper
completed, approximately 02:11–02:12 UTC; UI, processor and asset runs finished by
02:15:19 UTC. No compiler/MSBuild was detected at the run boundaries. A different
Debug build began at 02:16:18 UTC; subsequent distinct-PCM and cycle diagnostics
are explicitly labelled **compiler contention**, not part of that baseline.
Normal desktop/OS activity was not controlled.

| Native fixture | Observed result | Evidence / scope |
| --- | --- | --- |
| Release wrapper + CTest | **35/35 passed, 39.28 s** test time | [Native log](2026-11-03-performance/native-ctest.txt). Correctness tests, not callback timing derived from test duration. |
| Full processor, one prepared swell | 27,000 timed callbacks; **0 deadline exceedances, 0 C++ allocations/frees**; largest observation **364.4 µs** across all rows | [Processor CSV](2026-11-03-performance/processor.csv). 44.1/48/96 kHz, 64/128/512 frames, Learn stopped, Perform playing and Export playing. At 96 kHz/64, Perform p50/p99/max = **7.4/14.2/19.2 µs** against 666.7 µs deadline; idle Learn p50 0.3 µs, Export p50 0.2 µs. Synthetic host, one swell, no editor. |
| Scheduler, total 0/10/50/100/200, up to 64 overlap | **567,000 timed intervals**, 3 sweeps, **0 C++ allocations/frees**; **8 elapsed-time deadline exceedances**, maximum **10,356.9 µs** | [Run 1](2026-11-03-performance/scheduler-1.csv), [run 2](2026-11-03-performance/scheduler-2.csv), [run 3](2026-11-03-performance/scheduler-3.csv). Rows include 1/2/4 serial component players and workflow bypass. Shared PCM reduces memory pressure; these are not multiple complete plugin instances. |
| 200 total / 64 overlap, 96 kHz / 64 frames | One-player p50 **20.9–24.2 µs**, p99 **32.5–46.7 µs**; four-player p50 **84.6–85.7 µs**, p99 **151.8–185.4 µs** | Same three runs; no exceedance in these particular rows. All scheduled definitions are prepared before timing. Overlap, rather than total definitions alone, drives the hot mixer cost. |
| Same 200/64 case, distinct PCM per voice, compiler contention | 18,000 timed intervals across rates/blocks and 1/4 players; **0 exceedances, 0 C++ allocations/frees**. At 96 kHz/64, one/four-player p99 **51.0/185.7 µs**, maxima **106.8/270.8 µs** | [Distinct-buffer CSV](2026-11-03-performance/scheduler-distinct-compiler-contention.csv). About 64 MiB PCM per player; normal-priority background Debug compiler active. Smaller sample count cannot disprove rare tails. |
| Sparse tail diagnostic, compiler contention | 1,000,000 96 kHz/64 calls; p50/p99 **8.5/18.2 µs**, max **4164.2 µs**, **9 exceedances**. Median charged cycles **40,890**, cycles at maximum wall sample **103,870** | [Thread-cycle CSV](2026-11-03-performance/thread-cycles-compiler-contention.csv). Largest wall sample is ~490× median elapsed time but ~2.5× median charged cycles: consistent with a substantial scheduling/wait contribution, not proof of its cause. |

The baseline exceedances occurred in four-player 48 kHz/128, two-player
96 kHz/512, sparse one-player 96 kHz/64 and other dense rows. They are not confined
to maximum overlap. The earlier compiler-contention preflight also recorded
**7 exceedances / 189,000 intervals**, max **34,797.6 µs**; its complete rows are
retained in [the preflight CSV](2026-11-03-performance/scheduler-compiler-contention.csv).
Do not discard these tails or infer a hard real-time guarantee from low p99.
They are ordinary-thread, unpaced elapsed-time observations, **not measured DAW
dropouts**. Thread scheduling/page-fault attribution and paced host-priority
measurement remain open. No C++ allocation/free or plugin lock was found to
explain them. The cycle diagnostic adds evidence for external elapsed-time delay
but does not identify a driver or fault. [Microsoft documents charged thread
cycles and cautions against converting them to elapsed time](https://learn.microsoft.com/en-us/windows/win32/api/realtimeapiset/nf-realtimeapiset-querythreadcycletime);
no such conversion is used here.

### Waveform rendering

These are native **software component paints** into reused images, 50 observations
after five warmups per row. Timeline is 800×380, separate overview 1050×125,
48 kHz source summaries at 512-frame stride and about 2048 peaks per four-second
swell. Values are synthetic nonzero extrema; this deliberately exercises dense
waveform drawing. It excludes the actual editor refresh, interactive input,
compositor, DPI scaling and DAW scheduling. The components are painted separately,
so their times are not a measured complete-frame latency.

| Five-minute song | Full timeline p50 / p95 | Full overview p50 / p95 | Zoom/scroll timeline p95 | Zoom/scroll overview p95 |
| --- | --- | --- | --- | --- |
| 10 swell summaries | 17.63 / 22.96 ms | 16.45 / 23.13 ms | 2.52 ms | 24.26 ms |
| 100 summaries | 72.86 / 87.47 ms | 75.72 / 88.17 ms | 3.65 ms | 90.46 ms |
| 200 summaries | 138.46 / 151.45 ms | 140.03 / 153.89 ms | 6.46 ms | 167.27 ms |

[All 36 UI rows](2026-11-03-performance/ui.csv) include three- and twenty-minute
fixtures. At twenty minutes/200 summaries, full timeline/overview p95 reached
**208.80/182.97 ms**. This is a confirmed component drawing bottleneck, not a
measured FL Studio input-latency claim. `drawWave` scans before clipping; detail
painting visits every other wet summary, with repeated linear ID lookup. Overview
draws each summary's peaks even when they collapse into the same pixels. Disabled
inline-overview geometry does not skip its traversal. This combination justifies
the narrow [OpenSpec proposal](../../openspec/changes/bound-waveform-paint-work/proposal.md)
for visible-range/pixel summaries and cached waveform layers. Proposal, delta,
design and unchecked future tasks pass strict validation; none is applied or
archived. There is no evidence here for replacing the FFT or threading design.

### Convolution, resampling, resource ownership and overlap

[IR metrics](2026-11-03-performance/ir-library.json) cover six real bundled stereo
responses at 44.1/48/96 kHz with a **32-frame source**. Managed first install uses a
new fixture store; Windows file/resource caches are not flushed. Results do not
represent a long-source/full-song convolution.

| Monumental, ten-second stereo IR | IR preparation | Convolution | Estimated render bytes |
| --- | --- | --- | --- |
| 44.1 kHz | 350.21 ms | 84.58 ms | 103,171,192 |
| 48 kHz, native IR rate | 1.21 ms | 79.04 ms | 103,795,192 |
| 96 kHz | 663.38 ms | 243.57 ms | 136,641,016 |

The mismatched-rate sinc preparation is a measured cold cost, greater than FFT
time in this fixture. It is not paid every callback. At 96 kHz the short Whisper
fixture prepared/convolved in **37.61/8.54 ms**. Monumental first/repeat managed
installation took **31.37/16.94 ms**, demonstrating reuse still has verification
cost. Native mono/WavPack cases passed correctness tests; long mono IR latency,
CPU utilisation and fine transient-allocation traces were not measured.

The [96 kHz streaming test](2026-11-03-performance/ir-streaming.json) uses a
one-second disposable capture and two swells, retains accepted Shimmer Cloud while
one changes to Monumental, and drives 512-frame callbacks at approximately host
pace. First assignment to playable took **1217.89 ms**, repeat same assignment
**87.44 ms**; **229 paced blocks had no missing pages**. Sampled accounted peak:
**144,758,520 bytes (138.05 MiB)**. Process cumulative peak working set:
**86,880,256 bytes (82.86 MiB)**. Cache grew from **15,670,235 to 35,640,190 bytes**
over the fixture's replacement/restore/offline phases; this is not a per-render
write-byte measurement. This directly exercises render/playback overlap, but not
a FL Studio driver or 64 simultaneous rendering swells.

The full processor resource regression exercised 128 definitions with 64 overlap
and a far loop destination, then sparse placements. Accounted resident bytes
fell from **61,020,048 to 10,108,504**; a 65th simultaneous contribution was
explicitly rejected. [Resource stdout](2026-11-03-performance/native-resource.txt)
preserves the measurements. These tests support safe retirement and sparse-page
behaviour for those fixtures; they do not certify every temporary allocation or
an aggregate process RSS cap.

### Capture writes and immutable revisions

[Capture/asset CSV](2026-11-03-performance/assets.csv), actual transactional
CaptureSession at 48 kHz stereo, one five-second PCM block reused throughout:

| Fixture / phase | Elapsed | Total files | Total managed bytes |
| --- | --- | --- | --- |
| Three-minute first capture | 663.06 ms | 37 | 69,132,066 |
| Protected replay, changed input, Recapture off | 6.30 ms | 37 | 69,132,066 |
| Explicit three-minute recapture | 892.54 ms | 74 | 138,264,132 |
| Five-minute first capture | 1121.42 ms | 61 | 115,220,004 |
| Protected replay, changed input, Recapture off | 11.35 ms | 61 | 115,220,004 |
| Explicit five-minute recapture | 1541.05 ms | 122 | 230,440,008 |

This is unpaced storage throughput, not real-time Learn callback latency. It
confirms protected replay adds no files and explicit recapture retains immutable
old audio and manifests. In the five-minute process phase, sampled private bytes
were **5,521,408** after first capture and **6,688,768** after recapture; process
cumulative peak working set reached **18,702,336**. This component has no processor
capture queue/editor, so those numbers cannot describe whole-plugin memory.
Two identical direct IR writes produced equal content IDs but different paths,
adding four files / **1114 bytes**. The bundled installer and same-assignment reuse
are separate higher-level mechanisms. Revision retention is expected, not a
demonstrated leak. Long-session cleanup remains #8's scope.

## Source inventory and thread ownership

References below are pinned to the audited revision. Ranges identify the entire
relevant implementation, including branches omitted from a short quotation.

| Path / owner | Work and boundary | Finding |
| --- | --- | --- |
| [Processor entry][processor], host audio thread | Scoped denormal mode; host position query; bounded diagnostics read; workflow snapshot/CAS; capture branch; optional diagnostic tone; transport/audition mix; surplus output clear | No plugin-owned heap/lock/I/O on the live route. Host `getPosition()` is external code and remains a timing dependency. Work scales with host block size. |
| [Diagnostics publication][diagnostics], audio writer / worker and UI readers | Host query occurs before sequence publication; readers try at most eight times | No unbounded seqlock spin. Unknown timing is allowed when a coherent snapshot cannot be read. |
| [Workflow authority][workflow], atomic shared state | Single lock-free uint64 word; audio `begin`/`stop` use one strong CAS | Audio admission is bounded. `reset`/`cancel` retry loops exist, but are non-audio control operations, not reachable from `processBlock`. |
| [Capture queue][queue] and [capture worker][captureworker] | Callback copies at most 8192 stereo frames into one of 32 preallocated slots. Worker pops, combines up to five seconds / 1,920,000 frames, checks/commits assets and publishes snapshots | Full/oversized queue returns failure, marks pass failed and preserves accepted capture; it never waits for disk. Queue is ~2 MiB per instance regardless of mode, outside preview budget. Small host buffers provide only 32 blocks of queued time. |
| [Transport wrapper and memory ownership][engine], audio thread | Publishes atomic transport/loop/pass hints, stops audition on Song priority, calls live mixer then audition | No capture snapshots, strings, shared_ptr copies or worker locks in this path. `endOffline()` only clears an atomic flag. |
| [Prepared mixer][player], audio consumer | Four publication slots, 64 fixed voice channels, generation validation, segment binary search, absolute PCM offsets and bounded ramps | Callback uses raw pinned views. Clears old views **before** marking old publication retired. `collect()` destroys buffers only on producer/control path. No per-block shared_ptr destruction. |
| [Audition voice][voice], audio consumer | Eight fixed commands; consumes a frozen producer write index; raw current/transition buffers; 5 ms ramps; atomic acknowledgements | Command loop cannot chase a continuously growing producer. Pins are collected outside callback after coherent ownership acknowledgement, including crossfade ownership. |
| [Background coordinator][worker], SwellEngine worker | One serialized worker per instance; inspect, default IR, automatic dispatch, page preparation, imports/resets/analysis, render, shape, monitoring | Locks, heap, file operations and destruction allowed here. Default JUCE normal thread priority; no process-wide job cap or memory governor. Capture has its own normal-priority worker. |
| [FFT and IR preparation][fft], worker | Full zero-padded complex FFT, per-request plan and three arrays; stereo sequential transforms; windowed sinc resampling | Not partitioned real-time convolution. No evidence justifying algorithm replacement. Source and IR lengths determine peak memory and FFT power-of-two steps. |
| [Source reader][reader] / [source visitor and fingerprint][visitor], worker | Complete interval PCM, bounded chunk decode, finite checks and source hashing | Repeated verification can reread chunks; none is in live processing. Missing source never silently becomes zeros. |
| [Transport preparation and offline path][transport], worker / host-marked non-real-time thread | Worker reads shaped WAV pages, builds segments and aggregate gain plan; offline path takes `workLock`, may render synchronously, allocates and reads pages | Offline work is explicitly exempted by the [current specification](../../openspec/specs/transport-swell-playback/spec.md). Must never be called from a live path based only on a UI mode label. |
| [Timeline and overview][timeline] / [editor refresh][editor], message thread | Capture/session snapshots, min/max peak drawing, marker/selection work, zoom/scroll; refresh at 10 Hz | UI takes worker snapshot locks but audio does not. Full-song summary loops and per-swell overview loops can compete for CPU/cache and delay input/worker progress. |
| [Immutable managed assets][assets] / [capture transactions][capture], worker | Hashing, float WAVs, descriptors, atomic manifest publication, prior revision retention | Disk and hashes stay off live callback. Intentional old revisions persist; RAM eviction is not disk deletion. |

The JUCE Windows fallback FFT uses an instance-local spinlock and plan tables
([JUCE 9.0.4 source](https://github.com/juce-framework/JUCE/blob/9.0.4/modules/juce_dsp/frequency/juce_FFT.cpp#L98-L137)).
Those calls are background/offline only; finding this lock is not a live callback
violation. The older `PreviewEngine` also has a worker, but the current processor
constructs `SwellEngine`, not a separate active legacy preview engine.
Default JUCE host bypass uses the base processor's bypass path; no custom
`processBlockBypassed` override was found. Host suspend/bypass policy is untested.
Host save/restore entry points are separate control operations:
[`getStateInformation`/`setStateInformation`](https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Plugin/PluginProcessor.cpp#L447-L526)
allocate XML/JSON, copy snapshots and take locks; they are not called by
`processBlock`. A host invoking state operations on its processing thread is not
covered by the live-path safety conclusion. Restore fences generations and
requests background capture verification.

## Stage isolation, scheduling and cancellation

`processBlock` queues input only when **not offline**, mode Capture, phase
recording, playing, timing compatible and mapping admitted. Perform input passes
to output/mixing without capture enqueue. The worker may still finish accepted
Learn work, import IRs, render replacements and prepare playback pages during
Perform. “Read-only scheduling” therefore describes the source/audio-consumer
boundary; it does not mean the entire instance stops writing managed renders.
Export/Finalise admits existing explicit audition, but no host-scheduled wet.

The [mode guide](../development/workflow-modes.md) records merged #10 authority
and #16 header. Dedicated Learn/Export layouts and #6 WAV delivery are future
work at this revision. The host's non-real-time **Perform bounce** is a separate
implemented path: it freezes accepted IR fallbacks, waits for worker serialization,
prepares missing work synchronously and mixes bounded pages even when the host
Playing flag is false. It may block for a long time; this is not a live dropout
finding and is not an implemented dedicated Export workspace.

Mode separation removes capture copies, queue pressure, source comparison, source
commits and capture-driven invalidation from Perform. Prepared playback avoids
convolving every host block. Export bypass removes Song wet scheduling while
allowing preview. No pre-#10 comparison was run, so no percentage CPU saving is
claimed. Mode changes do not reduce a cold convolution's scratch peak or limit
other instances' workers.

Background jobs are serial per SwellEngine, with reset/import/analysis priority
over ordinary queued rendering and selected shape/monitor preparation. Automatic
analysis alternates opportunities with other work. Input/session/shape generations
and source signatures reject superseded publication. Recapture outside a source
dependency does not invalidate it. Shape and placement edits generally rebuild
prepared shape/pages without FFT; extended support or source/IR changes can require
new convolution. Existing tests distinguish those cases.

During a render, cancellation callbacks service live pages roughly every 40 ms at
available checkpoints, with a conservative active-job reservation. Pages cover
`min(rate*2,96000)` frames ahead and one eighth behind, plus separate loop pages.
At 96 kHz this is one second ahead, not two. Each complex FFT call, some decode,
hash, asset-write and waveform stages can finish before another cancellation/page
check. The 40 ms check is **not** a hard service-time guarantee. Stale results are
rejected, but a completed obsolete immutable write can remain on disk. Measure
longest non-yielding stage before changing thread scheduling.

## Memory and disk accounting

`estimateRenderMemory` checks
`occupied + 8*(N + M + N+M-1) + 48*nextPowerOfTwo(N+M-1) + 64 MiB`.
It includes stereo source/prepared IR/output, three complex arrays, conservative
plan/scratch and bounded capture decode allowance. The original decoded IR is
added as render overhead by SwellEngine. Decode/preparation have independent
available-byte checks. Oversized work fails instead of truncating source.

`occupiedLocked()` counts audition pins, all live publications (including retired
ones awaiting collection), offline pins, runtime raw/shaped/monitor buffers and
waveforms, plus active-job reservation during nested page service. It deliberately
double-counts some shared buffers. `available()` collects retired publications,
drops non-primary full PCM and source detail, then evicts non-primary waveform
summaries by age when summary storage exceeds 16 MiB. Live caches hold weak PCM
references; durable renders remain on disk. No global decoded/resampled IR-spectrum
cache or shared cross-instance renderer was found.

This is conservative admission accounting, **not a measurement of all live
allocations**. `activeJobBytes` is a temporary reservation during nested page
service, not continuous telemetry for every FFT local allocation. Sampling
`residentBytes()` therefore cannot establish a transient allocation peak.
The accounting excludes the capture queue, capture worker pending capacity,
committed/staged/prepared capture snapshots, editor copies, model/maps, allocator
bookkeeping, external GUI snapshots pinning old waveforms and JUCE/resource/code
memory. Local scratch must be considered separately at each call site. No tested
RSS observation below 256 MiB can prove a universal cap. Conversely, process RSS
above it alone does not prove render-budget oversubscription. Four instances have
four independent budgets (1 GiB nominal render allowance), plus those exclusions;
their normal-priority workers can run simultaneously. Repeated full-plugin
multi-instance peak/private-byte testing remains required before claiming a global
resource bound.

Capture audio is immutable float32 stereo, nominally `rate*seconds*8` bytes plus
WAV/metadata. A nonsilent five-minute capture at 48 kHz has 115,200,000 PCM bytes
(109.86 MiB), about 60 five-second chunks; silence uses metadata only. These are
**format calculations**, not measured disk totals. Source overview has one peak
per 512 frames, not full-song PCM. Wet summaries are bounded to about 2048 peaks
per render; overview painting still visits all retained summaries.

Managed writes compute content identity but append a random suffix to each WAV
path; there is no content-addressed existence check in `SwellAssetStore::write`.
Thus an actual repeated write can create a second equivalent asset. Higher-level
same-IR and valid-render checks can avoid that write entirely; distinguish reuse
from deduplication. Bundled-IR installation additionally searches up to 32 matching
descriptors and verifies decoded equality before reusing an immutable IR; it is
not an unrestricted global content store. Shape-only playback preparation reads existing render WAVs
without committing a shaped WAV. Old manifests/renders, canceled jobs after write
and imported assets may accumulate intentionally. This audit recommends measuring
and reducing unnecessary writes only with reference safety; retention, portability,
GC and deletion belong to [Issue #8](https://github.com/elneilios/ReverbArise/issues/8)
and its separate PR #31. Nothing was purged or relocated.

## Reproducible scenario matrix

| Scenario | Fixtures and measurements | Coverage / next validation |
| --- | --- | --- |
| Native baseline | Release CTest; allocation guards; render/cache metrics; sparse/dense resources | Exact results below; correctness duration is not callback latency. |
| Prepared scheduling | 0/10/50/100/200 definitions; 0/1/10/50/64 overlap; 44.1/48/96 kHz; 64/128/512 frames; 1/2/4 component players | Disposable native harness; p50/p95/p99/max, deadline misses, new/delete counts. 65 overlap refusal checked explicitly. |
| UI | 3/5/20-minute synthetic capture peaks, 10/100/200 swell summaries; full view and zoom/scroll; actual software component painting | Native fixture timing, not measured input/compositor latency. Test 100/150/200% DPI and interactive selection separately. |
| FFT / resampling | Bundled short/long stereo IRs at 44.1/48/96 kHz; native mono/oracle cases; cold/warm install | Existing metrics isolate decode, resample and convolution; 32-frame source is a small-source stress case, not a full-song render. Extend to .1/1/5/15 s sources and mono/stereo 0.1/2/8/12 s IRs. Preserve explicit rejected combinations. |
| Render during playback | 96 kHz long-IR replacement with paced 512-frame blocks; retained old IR, second unaffected swell; repeat same IR | Existing streaming fixture checks missing pages and accounted bytes. Add paced 64/128 blocks, adversarial seeks, 64 voices, CPU/private-byte samples and whole-callback QPC percentiles. |
| Multiple full instances | 1/2/4 instances, cold simultaneous render; staggered edits; editor open/closed; 20 repetitions | Not established by component-player scaling. Sample process private bytes/working set every 5–10 ms, CPU time, phase timestamps and post-idle retention. Separate peak delta from shared module mappings. |
| Capture / history / storage | Disposable 3/5-minute nonsilent and silent songs; fragmented passes; repeat protected playback, explicit recapture, Clear, restore older state, canceled jobs | Record bytes/files by segments/IRs/renders/manifests/parts/export. Compare reference/content identities, not just file names. Full long-session growth experiment not run. Never use an existing user asset root. |
| Transport / transitions | Seek inside/outside warm pages; short/far loops; rapid start/stop; missing timing, sample-rate/BPM/meter changes; Capture/Perform/Export and host bypass | Native regression coverage is narrower than DAW acceptance. Pattern vs Song correctness and real bypass timing require host observation. Do not infer FL Studio mode from synthetic valid Song time. |
| Export | Hot/cold host-marked offline bounce with irregular/large blocks, pending IR edits and dry/wet extremes | Existing native offline tests cover supported bounce. Dedicated WAV mix/generation/output profiling is not applicable until #6 is implemented. |

For each additional run record exact commit, build flags, CPU/power mode, sample
rate/block, instance count, cold/warm definition and background workload. Use at
least three repeats; report all tails and failures. Compare callback time to
`frames/rate` (64 frames: 1.451 ms at 44.1 kHz, 1.333 ms at 48 kHz, 0.667 ms at
96 kHz). Export full distributions and identify page misses independently of CPU
deadline overruns. Do not substitute process average CPU for callback timing.
For Windows CPU measurement use process/thread CPU-time deltas divided by elapsed
time and logical processors; this audit does not invent CPU utilisation from wall
latency. Read/write operation counts and file totals are different metrics.

## Priorities and recommendations

These priorities incorporate the native results above. Confirmed mechanisms are
separate from suspected impact; no production optimisation is authorised here.

- **Critical:** no confirmed live callback RT violation or host dropout. Host
  callbacks, page faults, driver scheduling and untested DAW paths remain outside
  this conclusion. Component elapsed-time exceedances were escalated during the
  audit and are retained with their reproducer; timing is not certified.
- **High, suspected host timing risk:** trace the rare elapsed-time tails under
  paced host-priority playback, recording scheduling/fault events and callback
  phase/CPU time (small–medium, 1–2 days). The charged-cycle diagnostic supports
  a substantial external delay contribution, but cannot identify its cause.
  Expected benefit: distinguish host/system stalls from a costly plugin path
  before authorising a DSP or scheduling change.
- **High, suspected:** aggregate full-instance memory/CPU peaks are not globally
  governed. First add disposable full-instance peak/retention runs (small–medium,
  1–3 engineering days); benefit is trustworthy admission evidence. A global
  governor/thread redesign is not justified by bookkeeping exclusions alone.
- **Medium, confirmed mechanism:** UI paint traverses all capture peaks and all
  wet overview peaks, even when many map to the same pixels; detail drawing scans
  before clipping. The measured 151/154 ms p95 full-view timeline/overview cost
  justifies `bound-waveform-paint-work`, with all planning artifacts ready for
  review and implementation tasks unchecked. Expected complexity:
  medium (2–4 days including binding/component regressions); expected benefit:
  at least 50% lower p95 on the matched dense fixture and cheap cursor-only layer
  reuse, subject to the proposal's fidelity and editor-refresh acceptance gates.
- **Medium, measured cold cost / candidate reuse:** Monumental resampling to
  96 kHz took 663 ms versus 244 ms convolution for a tiny source. First measure
  consecutive same-IR/many-swell jobs and cache-hit opportunity (small, ~1 day).
  A bounded prepared-IR reuse proposal could then target repeated preparation
  cost, with content/rate identity, cancellation and budget tests; no shared cache
  or retained-spectrum redesign is justified by the current single-job data alone.
- **Medium, suspected:** render/page service delays across FFT, decode/hash/write
  checkpoints can starve moving windows. Instrument stage maxima outside audio,
  then extend paced small-block/overlap tests (1–2 days). Benefit: identify the
  exact long stage; do not introduce a thread pool or alternate FFT speculatively.
- **Medium, confirmed mechanism:** live mixing performs fixed 64-channel bookkeeping
  and accumulation per sample even with few/no active members. Any future active
  set fast path must preserve ramps, channels, generation invalidation and exact
  sample placement. Complexity medium (2–4 days). Expected benefit is lower sparse
  callback work; use native full-callback/host measurements before prioritising.
- **Medium, confirmed storage behaviour:** repeated actual immutable writes may
  duplicate content and historical assets accumulate. Cross-reference #8; do not
  solve retention here. Measure avoided writes/cache hits first (small, 1 day).
  Any deduplication needs durable-reference/concurrent-writer tests and separate
  approval; expected benefit is disk and hashing time, not cheaper live mixing.
- **Low:** snapshot copying and linear session/dependency scans increase UI/worker
  work with song length/definitions. Profile refresh/lock duration before altering
  snapshot ownership. No evidence here justifies a state-format migration.

Already sound: capture isolation; asynchronous durable writes; bounded callback
queues; pinned publication retirement; generation/source-signature checks; sparse
page storage and separate loop destinations; no full-song PCM retention for the
source overview; no FFT for ordinary envelope/placement edits. Preserve these
mechanisms in any follow-up.

## Approval and remaining host coverage

This report and raw evidence are for owner review. Issue #25 stays open. No merge,
archive, user-asset cleanup or production behaviour change was performed. One brief final
FL Studio smoke test should eventually consolidate Song playback with 64/128
buffers, a long IR replacement while playing, seek/loop recovery, mode isolation
and suspend/bypass behaviour. Only failures or concrete unresolved host behaviour
should expand that UAT; native edge cases need not be repeated manually.

[processor]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Plugin/PluginProcessor.cpp#L63-L137
[diagnostics]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Plugin/PluginProcessor.cpp#L139-L179
[workflow]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Core/WorkflowState.h#L30-L85
[queue]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Capture/AudioCaptureQueue.h#L8-L69
[captureworker]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Capture/CaptureWorker.cpp#L1-L287
[engine]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/DSP/SwellEngine.cpp#L329-L416
[player]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Playback/TransportPlayback.cpp#L1-L129
[voice]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Playback/AuditionVoice.cpp#L1-L138
[worker]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/DSP/SwellEngine.cpp#L539-L835
[fft]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/DSP/ReverseConvolution.cpp#L18-L164
[reader]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Capture/SourceRangeReader.cpp#L35-L70
[visitor]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Capture/SourceVisitor.cpp#L8-L55
[transport]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/DSP/SwellTransport.cpp#L149-L398
[timeline]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/UI/SwellTimeline.cpp#L19-L420
[editor]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Plugin/PluginEditor.cpp#L384-L505
[assets]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/DSP/SwellAssetStore.cpp#L51-L141
[capture]: https://github.com/elneilios/ReverbArise/blob/752b3a33865ca6ec3e2e7b4b13baac839509b8bb/Source/Capture/CaptureSession.cpp#L187-L313

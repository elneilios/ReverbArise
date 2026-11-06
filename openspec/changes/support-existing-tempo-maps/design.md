# Design

## Context

See [proposal.md](proposal.md) for scope and [issue #71](https://github.com/elneilios/ReverbArise/issues/71)
for acceptance. Investigation performed 6 November 2026 on merged main
`80a12f0`; this is a source/API investigation, not a new host experiment.
There is no existing active tempo-map change or issue discussion to reuse.
The issue's named planning change is scaffolded here for the first time.

Current constraints found in the delivered code:

| Path | Fixed-tempo assumption to replace for map-enabled state |
| --- | --- |
| `Source/Plugin/PluginProcessor.cpp` | `SongTime.valid` combines sample, PPQ, BPM and meter availability; capture compares every block with one pass mapping. Loop PPQ bounds are converted using current BPM. |
| `Source/Capture/AudioCaptureQueue.h`, `CaptureWorker.cpp`, `CaptureManifest.h` | Queue already carries block timing for diagnostics, but accepted manifests retain source/context, not durable tempo evidence. Workflow manifests use schema 2. |
| `Source/Core/SongTime.h`, `TransportTime.h`, `HostGrid.h` | Bar conversion and inferred origin are affine; one observation cannot represent a varying-tempo arrangement. |
| `Source/Analysis/LearnCandidates.*`, `AutomaticGeneration.cpp`, `AutomaticTargets.cpp` | Candidate identity carries a timing tuple; source defaults use one quarter-note duration from current BPM. |
| `Source/DSP/SwellEngine.cpp` | `service()` globally stales ready/rendering work on BPM mismatch; rendering, manual audition and Export preview also use instantaneous tempo guards. |
| `Source/Playback/LiveTransportClock.h`, `LiveDiagnostics.h`, `Source/DSP/SwellTransport.cpp` | Live and offline admission compare fixed asset/run mapping. Offline subpages and error predictions synthesize PPQ from current BPM. |
| `Source/Core/SwellSession.*`, shared UI timeline/inspector bindings | Stored timing/render metadata, reopen eligibility, names/rulers and musical entries assume fixed timing. |

The #73 Snap/offline fixes are merged and owner accepted. Their tests and
[causal evidence](../../../docs/validation/2026-11-05-issue-73-clock-investigation.md)
remain regression authorities. They do not demonstrate tempo-map support, and
this proposal does not reopen that completed investigation.

### Host/API evidence and its limits

- The pinned JUCE **9.0.4** VST3 wrapper's `getPosition()` reads sample position
  plus optional BPM, PPQ, meter, bar start and cycle points from the processing
  context. Source inspected at
  `modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp`, around
  lines 2999 and 3572. The public
  [PositionInfo API](https://docs.juce.com/master/classjuce_1_1AudioPlayHead_1_1PositionInfo.html)
  describes optional observations; it supplies no full future tempo-map query.
- [VST3 ProcessContext](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/structSteinberg_1_1Vst_1_1ProcessContext.html)
  describes timing at processing-block boundaries. Hosts can split blocks for
  timeline jumps, but are not required to do so. Its tempo and musical-position
  fields do not supply an exact automation curve or an intra-block event list.
- Image-Line documents
  [tempo automation](https://cluster.image-line.com/fl-studio-learning/fl-studio-online-manual/html/toolbar_panels.htm),
  but this establishes the musician workflow, not the precision delivered to a
  VST3. No new FL Studio step/ramp trace, plugin-delay comparison or automated
  bounce has been captured in this planning turn.

Consequently two sample/PPQ points, even with matching BPMs, do not prove the
curve between them. A numerical fit or repeated successful test does not by
itself certify arbitrary ramp interiors.

## Goals / Non-Goals

**Goals:** preserve sample identity as the audio authority; make musical
availability local and precision-aware; retain all original state on conflicts;
keep constant-tempo #73 behavior intact. Support both automatic and manual
sample-defined swells across existing tempo boundaries.

**Non-Goals:** infer an unexposed host map, implement a new stretch algorithm,
accept time-signature automation, detect Song/Pattern mode, alter Export gain
policy or change production geometry/control families. See proposal for the
remaining exclusions.

## Decisions

### 1. Use accepted observations, not a reconstructed global tempo curve

Introduce a `TempoEvidence` domain alongside source coverage. Its immutable
snapshot identifies project/instance/source revision, evidence schema/revision,
recorded rate and constant meter. Records contain absolute block start/extent,
available PPQ/BPM/meter flags, observation/pass/run provenance and discontinuity
markers. Distinguish audio coverage from musical evidence coverage; captured
silence can carry evidence, while an uncaptured gap remains unknown.

Reuse the existing bounded capture queue and worker transaction rather than
recording from an editor timer or serializing diagnostics. Preserve raw
observations needed to audit conversion. Worker-side indexing can normalize
duplicates and sort sparse runs without inventing intermediate observations.

Reject the alternative of integrating instantaneous BPM across the arrangement:
it accumulates error and confuses seeks, tempo steps and unreported ramps.
Do not automatically label a local evidence revision as the host's tempo-map
revision; no such host revision identifier has been established.

### 2. Separate three kinds of authority

1. **Sample audio:** immutable source/render dependencies, explicit sample
   intervals, compatible processing rate and accepted constant-meter policy.
   Fresh runtime session/reset ownership and actual host sample ranges still
   govern live admission. Missing PPQ/BPM alone does not invalidate aligned PCM.
2. **Musical conversion:** accepted local evidence and its precision/coverage.
   A query returns a coordinate, a bounded sample interval/error and provenance,
   or unavailable/conflicting/ambiguous. Every editing binding uses this result.
3. **Comparison:** sufficient accepted and current evidence under comparable
   provenance. A differing instantaneous BPM alone proves neither a new map nor
   changed audio. Unsupported rate/meter or demonstrable boundary/alignment
   conflict remains an explicit failure.

Keep the legacy affine authority for old fixed-tempo sessions, including the
existing independent run baseline and one-sample guard. Map-enabled state uses
the new authority explicitly; do not globally relax `transportMappingMatches`
or silently change existing fixed-tempo behavior. A stopped-to-playing offset
does not translate stored samples. Never convert an arbitrary PPQ discrepancy
to a trustworthy sample delta using the current BPM across a ramp.

New source captures collect evidence by default. An explicit successful Capture
or Recapture may accept new evidence for its admitted intervals in an older
project; it does not certify previously unobserved regions or overwrite the
protected legacy authority for old source. No new user-facing enable switch is
needed, and mere playback never upgrades persisted mapping evidence.

### 3. Default precision is conservative and deterministic

An observed anchor supports its recorded coordinate. Interior conversion needs
an independently justified model/error bound; endpoints and equal BPMs alone
are insufficient. A query may also succeed when all admissible sample results
round to the same integer. Otherwise exact musical entry and Snap are
unavailable locally. The fixed-tempo affine path retains its current rounding
and meter-denominator semantics. Nearest-grid ties remain away from PPQ zero;
inverse lookup must reject multiple possible locations or unsupported coverage.

Initially do not enable fitted linear-ramp or piecewise-constant extrapolation
for new map intervals without a documented host guarantee/precision basis.
Synthetic fixtures with an exact model test conversion arithmetic; they do not
upgrade FL Studio's real observations to exact map knowledge. Future hosts may
provide stronger evidence through the same query contract without changing
sample scheduling.

Rulers omit unsupported musical marks rather than drawing a uniformly spaced
fictional map. Bar/range and relative-beat edits resolve both endpoints through
this service, not a BPM multiplication. Samples/seconds remain independent.
Host footer BPM/PPQ continues showing actual host observations. Seek/loop bounds
in musical coordinates become sample prefetch/fitting hints only after valid
conversion; absent bounds must not shorten source tails. Observed contiguous
sample pass boundaries remain separately usable under existing policy.

### 4. Source-window default follows the owner's planning decision

Owner decision, 6 November 2026: use tempo observed near the arrival to choose a
fixed sample duration when an exact beat endpoint is unavailable, and disclose
the duration policy rather than claiming exact beat alignment.

Use a verified endpoint for arrival PPQ plus one quarter note when supported.
Otherwise select valid BPM from the original observation block containing the
arrival, with its provenance; compute `round(rate * 60 / bpm)` once as a duration
default. Do not interpolate a more precise arrival BPM, borrow Stop/playhead
tempo, or carry a BPM across an unobserved interval. If the containing block has
no valid BPM, require explicit sample/time bounds instead of inventing a default.
Apply existing next-target, genuine completed-pass, source-coverage and protected
edit rules. Hold the resulting bounds stable through later playhead movement.

Onset detection remains sample-domain. Candidate/Generate identity pins accepted
source and relevant map/evidence authority, settings and IR. Progress through
the same accepted automation must not cancel a Generate/render or globally stale
swells. A change to a conversion used by a pending operation cancels/revalidates
that operation; adding unrelated evidence need not invalidate the arrangement.
Keep raw rendering/calibration keys sample-based where tempo was only a validity
guard. Retain old dependency-hash versions rather than relabel old assets under
a new hash law or rewriting audio merely to migrate metadata.

### 5. Reuse sample scheduling and transactional publication

Retain the current sample-positioned mixer, 5 ms admission/stop/handover ramps,
gain protection, bounded PCM budget, IR replacement and frozen recovery rules.
Change the validity inputs, not the audio placement algorithm. Live/worker
publication pins source/evidence/session authority coherently; obsolete worker
or restored-session results cannot publish. Tempo changes that preserve accepted
sample dependencies do not revoke retained IR contributions or frozen wet.

Offline entry freezes the same accepted assets/evidence/routing snapshot. Its
subpages advance **sample offsets only**; do not synthesize linear PPQ for a ramp
or validate a subpage against fabricated timing. Real callback timing remains
separate from internal pages. Handle both Playing flag values, variable blocks,
silent input, seeks, loops and independent instances. After all required wet
ends, preserve the #73 completed-tail behavior; an actual conflict before then
remains visible. Internal audition and plugin WAV composition remain sample
aligned and must not stop solely on ordinary BPM progress.

### 6. Persist evidence as part of accepted source, with explicit limits

Plan a manifest schema 3 evidence reference and separately versioned creative
state support. Keep schema 1/2 readers and legacy state migration. An immutable
evidence asset carries checksum, version, bounded record count, source binding
and precision/provenance; session references are metadata, not PCM. Source,
generation context and evidence publish atomically. Ordinary capture cannot
replace old source or its evidence. Recapture changes only explicitly encountered
intervals, preserving old snapshots/exports. Overlapping conflicting observations
are not silently averaged or used to overwrite protected evidence.

Proposed initial evidence budget: at most 1,048,576 compact records and 64 MiB
per accepted evidence snapshot, plus explicitly accounted bounded staging and
publication overhead. Keep PCM's existing 256 MiB budget unchanged and report
metadata separately. The implementation must pin/test the actual record size,
bounded lookup steps and total coexistence before enabling collection. Reject
an over-cap pass atomically with an actionable reason; do not drop arbitrary
anchors and claim full precision. Storage/index work runs on the worker; no
full-history scan, file I/O, allocation, deallocation or locks on the callback.

Unsupported or malformed new-format evidence cannot replace valid state.
Missing referenced evidence reports unavailable provenance without deleting
PCM or falling back to an invented affine grid. Legacy state retains its saved
fixed authority. Restore clears transient run/gesture authority and does not
autoplay, arm Recapture, choose a new mode or resume pending Generate.

### 7. Recognize conflicts without claiming unobservable map edits

Compare only exact or sufficiently bounded corresponding positions with
validated run provenance. Keep evidence conflicts separate from grid gaps and
preparation misses. Demonstrated conflicts suppress affected live scheduling
and musical operations while retaining render bytes and permitting inspection
of the original sample-domain result. Recovery uses explicit existing source
selection/Recapture and regeneration commands; no new automatic recovery mode.

Do not treat a seek, new bounce block partition, PPQ-origin variation or unknown
region as proof of a map edit. Nor may a seek/conflict create a replacement
baseline to conceal actual drift. New/unverified regions may play independently
safe absolute PCM with the mapping limitation disclosed. Exact identification
of arbitrary later edits cannot be promised without additional host evidence.

### 8. Gate production integration on measured host behavior

The first Apply stage is a bounded diagnostic experiment, not a production
timing-policy rollout. Reuse #73's copied diagnostics and add a test-only bounded
trace collector as needed. Record actual FL Studio/JUCE/build, rate, block sizes,
plugin-delay state, sample/PPQ/BPM availability and observations for a fixed
baseline, tempo step and curved/linear ramps, seeks/loops, and realtime versus
offline partitions. Include a change located inside a host block if reproducible.
No repeated #73 fixed-tempo acceptance is requested as fresh UAT.

Before production metadata/admission changes, present the measured capability
table, exact-only fallback, comparison/conflict policy and resource limits for
explicit owner review. Default support remains conservative if the host supplies
only boundary points; no requirement depends on proving exact ramp interiors.
If stronger precision would require a JUCE fork, FL-native bridge, user map
import or broad architectural expansion, stop and revise the proposal for review.

### 9. Reuse existing UI controls and verify at the binding boundary

Use shared timeline/ruler, `NumericInputControl`/`KnobValue`, grid selector,
`TransportReadout`, NoticeBar, context-help and Diagnostics components. Change
their domain bindings and status data, not production tokens, geometry or control
families. Name local grid precision and the approximate source-duration default
in existing accessible helper/full-text paths. Never show missing musical
coverage as generic audio failure. Reuse accepted focus/modifier/exact-entry
policies. If implementation needs a new variant/token, stop for scoped native
owner approval and standards revision before rollout. Run UI standards plus
component/binding tests; guide maintenance occurs after delivered implementation,
not by describing this future feature as current behavior.

## Risks / Trade-offs

- **Sparse evidence limits musical usefulness** -> exact-only fallback is honest;
  audio/sample workflows remain useful, and local unavailable reasons identify
  what the host did not establish.
- **Different partitions make comparisons inconclusive** -> compare supported
  positions only; distinguish unverified from conflicting, never choose arbitrary
  drift tolerance to force a match.
- **A host hides an intra-block seek** -> record the protocol limitation; do not
  claim sample-perfect scheduling for an unreported discontinuity. Validate the
  behavior FL Studio actually supplies and disclose unsupported cases.
- **Later map edits cannot always be detected** -> preserve captures and report
  the observable comparison state rather than claiming full-map identity.
- **Dense tiny-block sessions exceed metadata capacity** -> measure long/sparse
  fixtures, disclose the explicit cap and reject transactions without damaging
  prior state; do not silently increase PCM budgets.
- **Migration and in-flight ownership are cross-cutting** -> version independent
  domains, keep historical assets, inject storage/restore/cancellation failures
  and verify coherent publication before enabling host use.

## Migration Plan

1. Approve this proposal; a new Apply request is required before any test harness
   or production implementation.
2. Complete the host evidence experiment and owner policy gate. Keep production
   behavior unchanged until that gate is satisfied.
3. Add evidence domain/serialization and legacy readers with fault/persistence
   coverage; integrate bounded capture and coherent accepted references.
4. Replace map-enabled processing/admission consumers incrementally, retaining
   fixed-tempo regressions. Integrate shared musical bindings last.
5. Complete focused native/audio oracles, full Debug/Release wrappers, UI/spec
   checks and semantic guide review. Install one local build through the existing
   test-install workflow and consolidate final FL Studio capture/playback/bounce
   plus save/reopen into one smoke test after automated gates pass.
6. Deliver a reviewable PR with actual evidence and host limits. Sync/archive only
   when requested after implementation; do not merge as part of planning.

Rollback means retaining the prior binary and immutable project/assets. Older
binaries need not parse new evidence schemas: use a retained pre-upgrade project
snapshot rather than downgrading or overwriting a newer state. No destructive
cleanup or automatic schema downgrade is part of this change.

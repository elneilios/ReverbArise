# Design

## Context

See [proposal.md](proposal.md) for scope and [issue #71](https://github.com/elneilios/ReverbArise/issues/71)
for acceptance. Original source/API investigation performed 6 November 2026
on merged main `80a12f0`; the authorized diagnostic follow-up below now includes
actual host measurements, while production timing policy remains unchanged.
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
  bounce was captured in the original proposal turn. The subsequently authorized
  diagnostic-only investigation is now recorded in the
  [task 1.3 evidence packet](../../../docs/validation/2026-11-06-issue-71-checkpoint.md),
  with compressed raw numeric trace and metrics: 51,013 loss-free records in
  FL Studio 26.1.7.5654 at 48 kHz, nominal ASIO 512/256, step/linear/curved cases
  and offline bounces. It establishes observed boundary values, not exact ramp
  precision. In particular, reported step BPM changed before the configured
  PPQ boundary, and identical absolute samples across partitions had small raw
  PPQ differences. Do not infer exact transition boundaries from BPM changes or
  treat raw-anchor equality as a guaranteed host contract.

Where a ramp's host-coordinate uncertainty has no justified bound, even an
exact-sample anchor comparison with a small PPQ difference remains unverified;
the measured differences do not define a new drift tolerance. Automatic later
edit detection in such regions is consequently limited. Prove conflicts through
an established authority/bounded comparison or retain the unknown state; do not
claim detection of every upstream edit from these traces. This is an explicit
task 1.3 owner trade-off alongside retaining absolute-sample playback.

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

Fresh constant-tempo capture retains the existing affine authority and today's
Snap/ruler/bar-entry behavior under the unchanged fixed-tempo consistency policy,
not the stricter ramp exact-anchor-only fallback. New-format storage records the
authority kind explicitly. On demonstrated variation, split affected authority
at observed boundaries; do not retrospectively certify the interval containing
an unobserved transition. Unaffected affine regions remain usable. This preserves
the existing fixed-tempo operating assumption; it does not turn repeated equal
BPM observations into a guarantee of an arbitrary automation curve. Any proposal
to demote fresh fixed-tempo editing requires a separate owner decision.

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
per accepted evidence snapshot (64-byte records), plus explicitly accounted bounded staging and
publication overhead. Keep PCM's existing 256 MiB budget unchanged and report
metadata separately. The implementation must pin/test the actual record size,
bounded lookup steps and total coexistence before enabling collection. Reject
an over-cap pass atomically with an actionable reason; do not drop arbitrary
anchors and claim full precision. Storage/index work runs on the worker; no
full-history scan, file I/O, allocation, deallocation or locks on the callback.

Capacity failure has one production rule: lost required records, queue overflow,
record/byte exhaustion or evidence-write failure abort the entire pending
source/evidence transaction. Ordinary host fields reported unavailable are valid
records with missing flags, not capacity failure, and may commit a musical gap.
No capacity-triggered precision downgrade may accept new PCM. Earlier accepted
state stays intact. An optional diagnostic trace is different: it retains a
bounded prefix, explicitly records overflow/omissions and never controls capture
acceptance. Its measurements are unusable as a complete trace after any loss.

Worst case, one unique record per callback, empty snapshot, no compaction, 48 kHz:

| Frames/block | Source duration at 1,048,576 records |
| ---: | ---: |
| 32 | 11 min 39 s |
| 64 | 23 min 18 s |
| 128 | 46 min 36 s |
| 256 | 1 h 33 min 12 s |
| 512 | 3 h 6 min 25 s |
| 1024 | 6 h 12 min 50 s |

At 96 kHz these durations halve; at 44.1 kHz multiply by 48000/44100.
The table assumes a future packed production record no larger than 64 bytes;
that layout is not implemented by this diagnostic stage. The diagnostic's richer
raw record is 112 bytes and has its separate 65,536-record/7 MiB queue. If the
production record exceeds 64 bytes, the 64 MiB byte limit reduces these durations
and must be recalculated for review, not increased silently.
Mixed/variable blocks consume actual records, not nominal duration. Existing
accepted records reduce remaining capacity. Affine compaction, if added later,
must preserve the declared authority/provenance; no longer capacity is promised
here. Before production enablement, account for old/new snapshots coexisting,
indexes, queue and staging in a measured total; 64 MiB is a snapshot cap, not a
whole-process memory claim. The short tiny-buffer duration is an explicit owner
trade-off at checkpoint 1.3, not an approved unlimited-song promise.

Unsupported or malformed new-format evidence cannot replace valid state.
Missing referenced evidence reports unavailable provenance without deleting
PCM or falling back to an invented affine grid. Legacy state retains its saved
fixed authority. Restore clears transient run/gesture authority and does not
autoplay, arm Recapture, choose a new mode or resume pending Generate.

### 7. Recognize conflicts without claiming unobservable map edits

Use this deterministic comparison/admission table for each callback range and
each contribution's dependencies, independently in realtime and bounce:

| Situation | Comparison and admission |
| --- | --- |
| Fixed/affine authority, different block boundaries | Evaluate the accepted affine mapping at the actual callback start; validate the unchanged one-sample run residual. Never require identical capture block boundaries. |
| Map authority, zero or calibrated offset | Query accepted evidence at the corresponding sample. Only an exact anchor or justified interval with adequate error bounds is comparable. Do not integrate BPM to manufacture a matching ramp anchor. |
| Ramp callback has no comparable anchor at this partition | `sample-aligned / musical-unverified`; permit verified sample-positioned PCM if no known fence/rate/session failure, disclose lack of comparison and disable unsupported musical conversions. Neither success nor conflict of the entire map is inferred. |
| Comparable evidence contradicts accepted alignment beyond its declared uncertainty | Fence dependent material before admission and report a demonstrated conflict. An arbitrary tolerance cannot force a match. |
| Existing conflict fence intersects a contribution | Reject that whole contribution, including a seek into its wet lead-in/tail; retain other independently safe contributions. A new run/bounce cannot erase the fence. |

A map run can calibrate a **sample-domain observation offset** only using two
distinct comparable observations inside an accepted affine region, with matching
rate/BPM/meter and the same offset within the existing one-sample residual.
For signed offset `d = runtime origin - accepted origin`, comparisons query
accepted musical evidence at `runtime sample - d`; playback still addresses
wet at `runtime sample`. No PCM, source bounds or saved grid is translated.
Each playback or bounce run calibrates independently; capture's offset is not
blindly reused. An offset uncalibrated at a ramp start remains unknown. Do not
infer it from one ramp BPM/PPQ pair or choose a fitted offset to hide a mismatch.
If no calibration exists, an incompatible observation is ambiguous between host
phase and a changed map; classify it unverified unless independent comparable
evidence proves a conflict. Fixed-tempo #73 validation remains unchanged.

Conflict ownership is project/instance + accepted evidence revision, not a
callback/run. Record the contradictory observations, provenance and affected
half-open source interval as durable bounded metadata via the worker. Until
durable publication completes an in-memory fence already suppresses affected
contributions. If worker-side conflict storage fails, retain an instance-wide
admission latch, report the failure and serialize that bounded fail-closed latch
inline in plugin state with the prior accepted references. Host Save is not
blocked or silently treated as clean; reopen must restore the latch before
admission. Never delete or rewrite the original timing evidence or audio.

Scope a contradiction to its established conflicting interval; when either
boundary cannot be established, conservatively extend to the nearest verified
unchanged anchors, or to the end of the accepted contiguous source run. A tempo
edit upstream can shift all later musical positions: fence the suffix until a
verified convergence anchor rather than just the single callback. Do not cross
sparse gaps and certify later independent regions unchanged. A contribution is
affected if its extraction/support interval, target/placement musical authority
or positioned wet interval depends on the fenced region; retain its full stored
PCM but suppress the entire contribution. This includes wet before its source
arrival. For sparse downstream coverage with no evidence of convergence, mark
the downstream comparison unverified and retain a conservative conflict fence
where the demonstrated edit's effect cannot be bounded safely.

Recovery is an explicit user command, not Play/Stop, seek, reopen or new bounce:
Recapture/reselect source and regenerate affected definitions against newly
accepted evidence, or explicitly restore the prior DAW map and validate the
whole fenced dependency before clearing it. Validate recovery atomically; a
failed/cancelled/partial recovery retains the fence and all originals. Acknowledge
must not mean ignore-and-admit. Persist fences across save/reopen and clones
with their accepted revision; subsequent changes in one instance stay isolated.

Do not treat a seek, new bounce block partition, PPQ-origin variation or unknown
region as proof of a map edit. Nor may a seek/conflict create a replacement
baseline to conceal actual drift. New/unverified regions may play independently
safe absolute PCM with the mapping limitation disclosed. Exact identification
of arbitrary later edits cannot be promised without additional host evidence.

### 7a. Mixed legacy and new evidence

Keep authority spans, not a global map-enabled flag that weakens legacy checks.
For a swell extracting across legacy affine source into newly accepted map
source, verify both source spans and their boundary compatibility independently.
An affine boundary provides a musical comparison anchor only where that authority
was valid; it cannot certify the new ramp interior. With compatible sample rate,
constant meter, boundary evidence and no fence, explicit sample-defined source
and wet can span both authorities. Musical edits spanning them require a valid
conversion at each endpoint plus compatible authority at the join. Missing or
contradictory join provenance blocks that spanning contribution/operation with a
specific reason, while wholly verified neighbors remain usable. An implicit
authority upgrade cannot resolve the mismatch. Save/reopen retains both authority
kinds, original references and the boundary decision without relabelling legacy
assets as newly captured evidence.

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

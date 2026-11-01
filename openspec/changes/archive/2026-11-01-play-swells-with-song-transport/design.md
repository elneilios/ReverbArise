# Design

## Context

See proposal.md for motivation. M3 is complete at 42/42 tasks. The automatic workspace is synced and archived as `2026-11-01-automatic-lean-swell-workspace`; its accepted audition, gain and automatic-processing contracts are already in main specs. Reconcile and sync/archive M3 without rolling those contracts back, then archive transport only after its remaining centre-line endpoint host check (14.3) passes. The transport reverse-convolution delta retains the accepted audition scenarios and adds Song priority and explicit frozen-render recovery.

Observed components: `Source/Plugin/PluginProcessor.cpp` captures incoming dry before ScheduledTestTone and SwellEngine mixing. Host diagnostics provide playing and sample/PPQ/mapping fields, but `SongTime` has no playing flag. `SwellEngine::processAudio` currently drives a sequential AuditionVoice; `transportPlaying` only informs preparation. SwellSession v4 stores positioned wet edits, activity, monitoring preferences and immutable asset references. Worker rendering validates dependencies and shares a 256 MiB budget with resident PCM and pinned audition buffers. On-demand audition hydration currently evicts nonselected PCM, so live playback cannot depend on selection residency.

`SwellShape`, placement arithmetic, asset checks and MonitoringSafety are reusable. `ScheduledTestTone` demonstrates absolute scheduling, but does not solve multi-asset residency. Existing allocation guards, exact convolution oracles, processor tests and offscreen UI tests provide verification infrastructure. The current baseline has 23 passing CTest targets; that is regression evidence, not evidence that transport exists.

## Goals / Non-Goals

**Goals:** Deterministic positioned mixing with bounded callback work and worker-owned data; independent live routing; honest readiness; continuity during normal recapture; focused handle-regression fixes.

**Non-Goals:** Predicting uncaptured audio, changing the host transport, arbitrary tempo maps, rewriting convolution, exporting files, making all long-song PCM resident, or marking prior host checks complete. No promise of processing while FL Studio has suspended the effect.

## Decisions

### 1. Separate absolute transport rendering from sequential audition

Add a coherent block transport snapshot containing playing, position, mapping and validity, taken from the same diagnostics read. Do not bolt a second cursor onto AuditionVoice. For block [P,P+N) and positioned swell [A,B), mix only their intersection at render offset (sample-A), using checked arithmetic and existing shaped/placed semantics. Fixed sample-to-PPQ origin compatibility is checked with a documented rounding tolerance, including nonzero host origins. Selection is irrelevant; activity uses existing global solo semantics.

An enabled playing Song takes priority over manual audition. Starting it requests the existing bounded stop transition and inhibits new manual starts. Disabled live playback permits existing independent preview, including host seeking. Stopping transport never starts audition. This avoids two sustained wet paths while retaining diagnostic preview when requested.

Alternative: trigger Audition at each arrival. Rejected because preroll, mid-swell seeks, negative placement, overlaps and loops require absolute reads, not trigger timing.

### 2. Prepare immutable timeline pages outside the callback

Create worker-owned immutable shaped/stereo playback pages per swell, indexed by absolute sample ranges and a compact eligibility/dependency schedule. Mix admitted independent pages in the callback so one stale or edited contribution can retire without interrupting valid overlapping neighbours. Bound simultaneous mixing to 64 swell contributions; report excess coverage unavailable rather than dropping unreported members. Reuse verified raw assets and no-FFT shaping. Support larger host blocks by bounded slicing without allocating. Report unsupported bounds explicitly.

Approved implementation refinement: premixed shared pages cannot remove a newly invalid member while keeping its neighbours audible during a slow worker rebuild. Independent pages solve that contract conflict. Callback cost is bounded by block frames times admitted overlap, rather than a scan or mix of all 4096 session definitions. Admission detects excess overlaps before publication.

Prefetch a rolling forward window and known host loop destinations when supplied; while stopped prepare around the current position. Charge pages, scratch, raw hydration, render work and all retired/pinned publications to the existing 256 MiB budget. Reserve admission before allocation and evict only unpinned data. No second untracked PCM cache. Budget-pressure tests must run concurrent render/audition/preparation and a logical long song.

A warm seek plays at its destination immediately. An arbitrary cold seek cannot load disk synchronously: report missing coverage, keep routing deterministic and join current absolute time after worker preparation. Never replay elapsed lead-in. UI readiness describes specific prepared coverage, not a whole-song promise. Whole-song precomposition/residency is rejected because long sparse songs exceed the budget.

### 3. Bound publication and ownership

Publish complete immutable schedule/page revisions with dependency generations using bounded callback-safe commands and external lifetime ownership, following the existing AuditionVoice pin/retirement pattern. The callback observes raw pinned views, never destroys a last shared owner. Coalesce worker requests and cancel obsolete generations on restore, capture clear and incompatible timing. Page replacement cannot expose a partially filled buffer.

Use maximum 5 ms complementary old/new revision transitions; old valid pages stay pinned through the transition. A seek fades from a bounded retained last-output state into the destination rather than continuing an old cursor. Stop, disable, misses and invalidation similarly discharge wet within 5 ms. Page seams within one revision have exact contiguous values and do not introduce per-page fades. Existing 2 ms wet crop safety is applied once per swell, not once per page; admission/handoff ramps are separately disclosed. Very short events scale ramps to available length.

### 4. Keep capture and live routing independent

Capture original stereo input before every generated branch and before Wet Only suppression. Default Dry + Wet adds prepared wet to unity live dry; never add captured dry in transport mode. Live enable, routing and wet level are distinct from audition isolation, trim, blend and tail. Wet Only suppresses dry only during enabled playing transport, with a 5 ms routing ramp; stopped/disabled returns dry. No limiter or automatic gain affects the default live dry path.

Store new typed fields in a versioned session migration (current JSON/plugin state v4). Absent fields select enabled Dry + Wet, 0 dB user wet gain. Restore clears voices/pages and waits for a fresh coherent host snapshot and asset validation; no host transport request. Reject malformed values transactionally as existing session decoding does.

### 5. Protect aggregate wet without pumping

Calculate a conservative aggregate peak bound per schedule revision from interval-overlapping shaped per-channel peaks after each swell's saved gain and crop safety. An interval sweep gives the largest sum of absolute bounds; one fixed attenuation min(1, -6 dB amplitude / bound) covers that schedule, with no quiet boost. It can be conservative but is deterministic, keeps relative gains and requires no callback peak scan or per-block normalization. Publish attenuation and predicted bound separately from user wet gain. Level changes ramp 5 ms; complementary revision crossfades preserve the bound. The combined dry/wet output can exceed full scale and is not falsely labeled protected.

Alternative: inherit audition trim/blend or normalize each page. Rejected because it would change the live balance with monitoring preferences or create level steps/pumping across pages. Revise the bound only on meaningful schedule changes, not residency shifts or identical capture commits.

### 6. Distinguish verified freshness from recapture churn

Reuse fingerprint-based dependency checks. Identical and unrelated recapture keep the last verified generation eligible while checks run; an actually changed source invalidates only affected contributions through bounded callback-visible member generations. Do not call an unverified replacement current. Rebuild affected independent pages outside the callback and publish complete replacements; a slow replacement can mean an explicit omission of that member while valid neighbours continue. Unknown upstream edits remain unknowable until recapture and must be described honestly.

Missing/corrupt assets and incompatible rate/tempo/meter/origin suppress affected wet. Manual cached-Wet-Only recovery is not a transport eligibility shortcut. Existing extended wet support, endpoint before/after arrival, tension and placement must pass through unchanged. Fresh automatic preparation may enter at the current Song offset; it cannot produce earlier audio retroactively.

### 7. Integrate the existing workspace and fix bounded gestures

Add prominent live enable/mode/level and compact actual readiness/miss/attenuation status, keeping Audition clearly separate. Draw host playhead in detail and overview from snapshots; do not let UI polling drive audio. Update the current 'coming later' wording only when implementation exists.

The screenshot reports odd extreme drags, not a proven root cause. Reproduce against `EnvelopeHandles.h` packing and SwellTimeline gesture ownership: zero/full-length fades, crossed endpoints, maximum tensions, extreme positive/negative body placement, snap on/off and viewport edges at supported sizes. Define stable active-handle identity for a gesture; clamp accepted model values without switching targets or moving the source anchor. Keep distinct zero-fade hit targets, finite on-curve geometry and working numeric controls. Add event-driven tests for reproduced failures before fixing; do not redesign the inspector or claim screenshot diagnosis as a fix.

## Risks / Trade-offs

- Cold seek outruns disk/preparation -> explicit temporary wet omission, bounded forward prefetch and immediate correct-offset admission when ready; never callback I/O.
- Conservative overlap bound makes large dense schedules quieter -> display fixed attenuation; preserve neutral user gain and relative levels, with no hidden block pumping.
- FL Studio smart-disable/silent routing suppresses callbacks -> host validation for routed silence and idle inserts; document actual observed limitations instead of promising sound without callbacks.
- Host offline bounce outruns asynchronous preparation -> use bounded synchronous preparation only when explicitly marked non-real-time; prepare all available wet, warn about skipped members and report fatal failures. Realtime callbacks remain nonblocking.
- Recapture/edit churn invalidates composed pages -> content fingerprints, coalescing, per-dependency generations and bounded replacement transitions; exercise pressure tests.
- Archive order differs from implementation order -> preserve the already synced workspace contracts when syncing M3, retain recorded acceptance results, and finish transport host acceptance before its archive.

## Migration Plan

Performance/export feedback: normal automatic recapture can compare bounded stereo multiband energy profiles with the retained committed PCM instead of requiring bit identity. A separate saved Regeneration sensitivity selects exact mode at 100%; scoped recapture stays exact. Thresholds include an absolute quiet floor, preserve meaningful local additions/deletions and avoid cumulative drift by not replacing ignored baselines. This is a documented heuristic, not musical-key detection.

Offline bounce is a new explicit exception to real-time preparation restrictions. JUCE's VST3 wrapper maps host kOffline to isNonRealtime. That path skips capture, pauses normal background scheduling, completes/validates the prepared definitions, freezes an export snapshot, and synchronously prepares bounded pages before their samples are mixed through a separate transport renderer. A work mutex serializes worker operations and offline preparation; real-time callbacks never acquire it. Frozen assets/settings remain stable for a bounce, resource accounting includes offline owners, and missing/unrenderable data produces a persistent export diagnostic rather than a successful silent omission. Returning to real-time resumes normal scheduling and retires offline buffers on the worker. Host-marked offline blocks use absolute Song time even if Playing is false. A host not identifying offline mode retains the real-time contract and requires separate host validation.

Loop-feedback refinement approved 2026-11-01: sample-identical normal capture is a no-op at the managed capture layer, including no new commit notification. Worker timing-read gaps do not invalidate assets; host getPosition runs before the short diagnostics publication so the prior coherent snapshot stays readable. Valid rate/tempo/meter changes still invalidate incompatible assets. Current block timing continues to gate wet output.

Untouched automatic extraction/default overlap can fit an explicit host loop boundary or an observed completed backward transport pass. Record bounded per-instance closed-pass intervals outside PCM state; saved source/shape metadata already persists the resulting bounds. Apply them during automatic window reconciliation only when captured contiguous input does not continue beyond that boundary. Future genuine coverage can restore the ordinary automatic window. Never infer completion merely from a growing coverage edge or an unmapped gap; never rewrite manual ownership. This refines the predecessor's full-window requirement with the user's approved closed-pass exception, not zero padding.

Implement in independently checked tasks below, preserving old-state fixtures and the current regression suite. Complete automated checks and publish a specifically identified VST3 build before FL Studio validation. Keep this change and predecessor host checks open until actually confirmed. Rollback uses the previous VST3 and pre-upgrade project copy; older readers need not accept the new state version. Disabling live playback provides an immediate runtime fallback to dry plus optional manual audition without changing prepared assets.

### Adjustable regeneration and frozen wet

Store regeneration sensitivity separately from marker sensitivity. Map 0–100% to 30–0% coarse spectral-envelope difference in 50 ms windows; 100% uses exact PCM. Compare against retained committed source, with a small absolute quiet floor, never against previously ignored input. Explicit recapture remains exact. These are signal-change heuristics, not musical key detection.

Freeze render is saved per swell and admitted only for ready committed wet. It changes automatic provenance to manually edited and bypasses later captured-source/IR dependency checks for that verified baked render. Frozen wet still requires the original sample rate/tempo/meter and a valid managed render asset. Shape, gain, enabled/mute/solo and placement remain editable; changing source/IR or extending render support requires unfreeze. Unfreezing queues current dependency validation and generation. Capture clear/reopen can restore frozen wet even without captured source; Dry + Wet audition still requires genuine captured dry context. Offline export uses the same frozen wet semantics.

### Final-loop and multi-instance export regression

Real capture metadata at 82 BPM/48 kHz reaches sample 1264391 while the 36-quarter loop rounds to sample 1264390. A single captured rounding sample must not invalidate a completed boundary. Preserve the existing one-sample host timing tolerance while rejecting genuine coverage extension beyond that allowance; manual/frozen windows remain protected. Advertised loop bounds take precedence over approximate observed backward-pass ends.

Offline resources, snapshots and failure state belong to each engine instance. Test separate source/IR/level paths in interleaved and concurrent offline callbacks and repeated bounces. User-approved export policy: unavailable individual swells are skipped with a prominent named warning while all valid audible contributions continue rendering; never reject an entire instance solely because one member is waiting/missing. Fatal mapping, I/O-after-admission or budget failures remain explicit incomplete-export errors. Surface the exact per-instance result prominently and provide offline callback counts in Diagnostics so a host skipping one insert can be distinguished from a plugin validation refusal. Do not infer that the host called both instances from success on one track.

Export validation builds a fixed accepted-member list, preserving original eligibility/solo decisions before dropping unavailable members. Page entries and members stay aligned; later pages cannot accidentally index a skipped definition. Skipped names/reasons (bounded detail plus total count) remain visible through the bounce and afterward. Wet Only with no available members produces silence with a warning; Dry + Wet still passes dry unchanged. Collect outstanding completed-pass observations synchronously at export start before automatic boundary reconciliation.

### Offline completion diagnostic refinement

The host has no explicit per-export completion callback here. A positive offline block after the maximum admitted wet endpoint can still contain dry/tail samples while PPQ resets or freezes. If absolute samples are beyond all admitted intervals and valid rate/tempo/meter remain unchanged, preserve the completed wet result and apply the frozen dry routing directly; no wet read or mapping reinterpretation is needed. Do not extrapolate this exception into a block with current/future wet, invalid timing, or changed rate/tempo/meter. Retain true failures and include block position, PPQ and remaining wet endpoint in their diagnostic. A reset during an actual wet interval remains an error. The user's apparently complete WAV is consistent with this host-tail case, but the actual FL Studio cause remains to be confirmed.

### Envelope and audition acceptance feedback

User acceptance groups 1,2,5,6,7,8 passed; group 3 remains failed and group 4 partially passed. Check only fully covered host tasks, preserving mixed envelope/audition tasks. The screenshot shows collision callouts displaced horizontally before the envelope start, with crossed connectors; keep time coordinates aligned and separate collisions primarily vertically, with a compact fallback only when the lane cannot accommodate them.

Publish an audition cursor using a short atomic sequence independent of long audio-block ownership guards. It reports the absolute next sample of actual preview playback, not the host playhead, and disappears after stop/completion. Draw it in a distinct color in detail/overview without driving audio from the UI.

Add saved isolateDry=false in state version 7. In Dry + Wet mode this optional control limits captured dry to [sourceStart,sourceEnd), rather than the surrounding listening context. It does not separate overlapping mixed notes or suppress live plugin input. Keep the listening window/tail, wet membership/isolation, blend and safety rules unchanged. Show the actual replayed dry interval; old sessions retain contextual behavior. Scope source-dependency checks to the dry span actually read.

### True-position stacking refinement

User prefers z stacking to zoom-dependent repacking, with offsets only for intrinsically coincident controls. Record the exact logical sample coordinate of every handle and compare that identity before pixel conversion; equal or close floating pixel coordinates do not establish genuine coincidence. Zero fade-out is anchored at the exclusive wet endpoint, like the end handle; zero fade-in shares the start. Retain actual positions for all distinct times, even if glyphs overlap. Reverse paint-order hit testing selects the topmost target; users zoom in for individual access. Small offset callouts are reserved for actual coincidence groups and remain bounded, independent of horizontal zoom. Existing saved metadata and audio are unchanged. This supersedes the previous visual-proximity separation policy.

### Waveform-centred endpoint refinement

The user confirms crazy positions are resolved but requests permanent waveform-centred Start/End squares. Endpoint time remains exact while their vertical anchor is always lane centre; fade/tension controls follow the normalized envelope. Genuine coincidence now requires both sample identity and equal vertical anchors, so zero fades at unity naturally occupy the upper curve rather than receiving a callout merely for sharing an endpoint time. Draw the envelope's zero-outside boundary links to the centre-line endpoints. Audio, metadata, gesture deltas and z stacking remain unchanged.

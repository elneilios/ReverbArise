# Design

## Context

See [proposal.md](proposal.md) for motivation and the four capability deltas for acceptance. GitHub issue #6 and owner comment [6578857827](https://github.com/elneilios/ReverbArise/issues/6#issuecomment-6578857827) describe the dedicated Export layout and 0/50/100% comparison. Comment 6588016355 adopts Learn / Perform / Export display names without changing persisted enums. The owner explicitly selected **allow missing dry as silence with a warning** on 2026-11-03 and authorized this planning scope on 2026-11-04.

Baseline is merged #11 at `185f00f` on `origin/main`. Current `PluginEditor` uses a guarded Perform-shaped Export view. `SwellEngine::audition()` requires a primary selected swell and its prepared finite listening window. `AuditionMix` already defines linear dry/wet weights, separate wet/sum attenuation and policy ceilings. `TransportPlayback` and `SwellEngine` have accepted render versions, interval scheduling and bounded offline window-reading mechanisms. These are useful primitives, not an existing standalone WAV export pipeline. `processOfflineAudio` serves host bounce and must not become the normal realtime preview callback.

`SourceRangeReader` currently rejects incomplete dry ranges, appropriately for existing Perform audition. Add a separate composition reader policy for #6; do not weaken that reader's callers or the strict selected-window audition contract. Source manifests distinguish exact silence and missing/gapped coverage. Session creative metadata is v9 with processor envelope v8, imported IR retention and capture-context schema 2. New Export state needs a versioned additive creative-schema extension; no PCM belongs in plugin state.

Approved typography and reusable controls are documented in `docs/development/ui-development-standards.md`. The Learn refinement established stable helper text, Medium labels, Regular descriptions, visible focus ink and the wheel-enabled SliderControl/NumericInputControl family.

Visual references reviewed: on 2026-11-04 the owner supplied both original mockups directly in this conversation after the GitHub attachment URLs returned 404. The first is the large dedicated Export layout; the second is the four-panel sheet showing 0%, 50%, 100% and a single-event waveform comparison. The reference-retrieval blocker is resolved; native implementation approval remains a later checkpoint.

Apply their waveform-to-controls composition: a large left preview with heading/caption, song ruler, wet/dry legend and internal Play/Stop plus elapsed/duration; a right column with exact mix, prominent dashed aqua WAV drag tile, Save WAV and Return to Perform. Retain concise song-alignment and DAW-wet-bypass guidance. Use current Learn / Perform / Export names, approved header, bundled Inter and flat charcoal/aqua styling rather than reproducing obsolete labels, gradients, outer frames or unsupported actions.

Reference consistency: the first large mockup is labelled 100% Wet but still depicts white dry attacks. The second sheet and written #6 contract explicitly resolve the endpoint: 0% shows neutral dry only; 50% shows scaled dry plus rising pre-arrival aqua wet; 100% shows aqua wet only, with the dry legend optionally subdued. Use actual prepared audio, not the illustration's idealised repeated shapes. These references confirm the existing delta requirements; they do not add a comparison screen or new controls.

## Goals / Non-Goals

**Goals:** one dependency-complete composition and audio kernel for accurate preview, waveform and durable WAVs; bounded work for long/sparse songs; explicit warning and publication authority; independent saved Export preferences; native visual checkpoints and one brief real Playlist-drag acceptance.

**Non-Goals:** #5's full-timeline Perform controls/start-selection semantics, #38's candidate/Generate workflow, #8's portability/cleanup, new DSP or gain calibration, header-mode icon/information redesign, new host parameters or changing host offline bounce. Preserve current successful capture Stop → Perform. Share new composition primitives with later #5 without delivering its UI behavior implicitly.

## Decisions

### 1. Dedicated Export view, common header

Retain the approved header exactly. Below it, allocate roughly 66% to Export Preview and 33% to actions with a thin separator; maintain 1100x850 default/minimum and existing larger sizes. No list, inspector, duplicate Overview or Export Summary. Ruler, caption, viewport and preview controls align on the left. On the right: Dry–Wet slider and exact percentage, large Drag WAV tile, Save WAV, Return to Perform. Inline mix/readiness/file text and contextual notices supply necessary feedback without a new status panel. Explain bypass/layer/replace behavior in concise persistent guidance.

Reuse SliderControl/NumericInputControl and shared icon/notice/font/spacing policies. Inventory command buttons, file-dialog invocation and external drag tile; extract missing reusable action/drag interactions under Controls with screen-independent callbacks and tests. Screen layout owns no file I/O. A new specialized Export waveform control owns cached peaks, mapping, truthful coverage, fit and preview cursor; it has no creative handles or host-seek action. New/revised control families receive owner look/feel approval before rollout. Untouched Perform controls remain outside this migration.

Alternatives: a modal overlay hides the arrangement and contradicts the approved layout; exposing old engineering controls or inventing a format/range panel expands the UX unnecessarily.

### 2. Immutable composition plan and bounded shared kernel

Introduce a composition model separate from selected-swell monitoring: accepted capture reference/timing, ordered eligible wet identities and accepted render/shape/placement/calibration versions, effective activity/solo membership, wet percentage, rate, origin/range, warning spans, protection measurements and algorithm/schema revision. Pin metadata/assets for the lifetime of a job or preview. Retain the last accepted sound during a pending IR replacement if it independently verifies; name the pending version. A missing required wet member blocks the requested complete mix rather than silently skipping it. This stricter standalone contract does not alter the available-member policy of host bounce.

Factor/reuse existing checked interval arithmetic, envelope/calibration and edge-gain functions. A deterministic window mixer takes an absolute half-open range and emits interleaved stereo from verified source/wet windows. Source is mixed once, not once per swell. Page boundaries are never audible boundaries: apply 2 ms wet safety ramps only at true member bounds; preview-only 5 ms start/end/Stop/handoff ramps are separate from stored mix samples. Preserve custom-IR gain, accepted energy baseline and user Level. Export does not inherit live wet level/routing or selected-window isolation/tail/trim.

Use bounded worker-owned pages within the existing shared 256 MiB working budget, not a separate 256 MiB allowance per Export/preview/write. Reuse scheduling/window-reading mechanisms rather than retaining entire songs or every full render. Explicitly account for resident live/monitor buffers, preview prefetch, write buffers and peak metadata. Keep sparse zero spans sparse in the plan; write required zeros progressively. Refuse overflow/unsupported file size early. Test stable summation order and page-partition independence.

Alternative: extending `prepareAuditionMix` by allocating a whole-song WetRender would exceed memory for long sessions. Creating a second convolution/export engine would risk gain and timing divergence. Do not call blocking offline-bounce processing from the normal callback.

### 3. Defined gain, coverage, origin and extent

For w = percentage/100, use dry=1-w and wet=w. Preserve saved per-member gain/envelope and derive one aggregate wet guard and one final-sum guard with the existing saved policy ceilings, without upward normalization. Measurements and the final render use the same exact window mixer and member set. Stream measurement passes before declaring a gain plan complete; retain disclosed attenuation in the revision and waveform. No per-page normalization. Preview excludes the independent Perform audition trim; its only difference from WAV interior samples is disclosed manual boundary/handoff ramps.

Use the verified fixed musical mapping to calculate song origin, with checked integer frame bounds and the existing no-resampling/no-stretch compatibility rule. The file interval starts at origin and ends at max(origin, accepted dry extent, eligible positioned wet end), independent of the viewport/mix so alignment is stable across endpoint comparisons. Clip any wet before origin without delaying the dry event. No end-of-song length is fabricated from DAW position. Keep complete known tails even if dry extends beyond wet; trailing exact zeros remain part of this declared range.

Within that range, enumerate captured audio, captured exact silence, uncaptured gaps and missing/corrupt dry-asset intersections separately. Missing dry contributes zeros **only in this Export composition**, with a persistent warning listing/counting ranges and explaining substitution. An entirely absent dry branch at w<1 is a warned silent branch, not a reason to invent data or force 100%. At 100% bypass dry reads/dependencies and suppress irrelevant dry-substitution warning. Invalid timing, unsafe references or unrepresentable ranges remain fatal; the warned-silence policy does not authorize malformed metadata or unavailable wet.

Alternatives: blocking gaps was rejected by the owner; quietly treating gaps as captured silence would violate coverage truth. Extra export-format/normalization controls are unnecessary for the approved UI.

### 4. Preview and current revision readiness

Separate desired mix, verified composition plan, prebuffered preview and fully published WAV. Enter Export when at least one enabled/unmuted/solo-eligible committed compatible wet render qualifies. Do not require dry; independently valid frozen-wet recovery is supported. Keep creative and source-readiness rules unchanged elsewhere. After entry, 0% remains selectable and does not eject Export. Saved modes retain their existing guarded restore behavior.

On first Export use only, seed w from effective audition mode/percentage (Wet Only means 100%, not its latent mixed value). Thereafter Export owns its saved percentage; mode re-entry must not copy over edits. Build jobs are keyed by instance/session epoch, request generation and complete input dependency identity. Rapid gestures coalesce; stale completions cannot become current. Do not show an older file as ready for the newly requested percentage. Keep its immutable history intact. Only a fully published current WAV enables Drag/Save.

Preview consumes prepared pages for the same composition. Fit on initial entry to the actual nonzero mixed region, with modest visual margins; the preview starts at its first nonzero sample and retains original song ruler coordinates. Use exact nonzero bounds rather than a hidden silence gate that trims quiet content. An all-zero result has an honest empty audible region and disabled Play; it can still produce a valid zero-sample-value WAV over the declared nonempty file interval. A changed mix recomputes these bounds. Do not repeatedly refit while the cursor advances or change Perform/Learn navigation.

Play/Stop represents explicit internal intent. If a new mix becomes ready while already playing, safely hand off at the same absolute cursor with the disclosed monitor ramp; if not prebuffered, stop and disclose preparation instead of silently skipping pages. Stop, mode exit and restore revoke continuation; worker completion alone cannot start a voice. Export never schedules wet to follow DAW transport, captures input or seeks/starts the host. Detect underruns and stop/report; they never alter the verified file.

### 5. WAV publication, external drag and Save

Use stereo IEEE float32 WAV at the verified composition sample rate. This avoids irreversible integer clipping/quantization and matches existing stereo sample conventions. Use the supported RIFF size limit initially; reject excessive ranges before writing, with a precise error instead of silently truncating or adding RF64 scope. Container length/byte arithmetic is checked. Streaming verification covers header, rate/channels, frame count, finite samples and content identity, without loading the whole file.

Write to a unique unpublished sibling/staging file in the managed working store; close, validate and publish atomically to a mix-identifying immutable name such as `Swells - 65pct-wet-<revision>.wav`. Only then publish the reference. A duplicate identical composition may reuse a verified immutable file. Accepted edits, Clear, canceled drags and later renders cannot delete/rename/replace exported files. Cancellable owned scratch can be retired safely; do not implement broad retention cleanup. Disk-full, short-write, close/rename/verification errors leave prior files intact.

External drag offers the actual ready managed path through JUCE's native external-file drag facility. Reverify existence before initiating; never hand over a future path or delete after a successful or canceled drag. Never fabricate drag success. The file carries song-origin lead-in; users place it at song origin. Any platform helper needing a message-thread call receives only the already published path. A native test covers invocation/lifetime; real FL Studio Playlist acceptance must confirm float32 file handling, origin placement and cancellation. Save WAV stays available as the fallback.

Save uses the same published composition through the native chooser, with existing-file confirmation and transactional destination replacement. Pin request/session/revision to the chooser invocation, reject stale completion and never write into another restored session. Preserve the managed original even on a user-confirmed overwrite of an external destination. Show exact chosen percentage and layer/replacement guidance; at intermediate percentages advise muting/replacing the original dry track. Advise remaining in Export while the delivered clip plays, preventing scheduled-wet doubling.

### 6. Saved preferences, complete references and compatibility

Extend creative JSON from v9 with a validated Export section (new creative schema version; preserve processor envelope v8 and existing mode/parameter/plugin IDs). Store initialization flag, percentage, desired composition identity and last completed managed reference, plus bounded immutable revision descriptors. Bound metadata by rejecting excess new descriptors safely; never remove old audio to make room. Keep durable file identity/rate/channels/frames/format and input/protection version metadata. No progress, voice cursor, chooser state, staging path or PCM is serialized.

A save during a pending mix keeps the latest desired setting and coherent older completed reference separately. On restore, verify or prepare the desired composition; do not relabel/rewrite an old file or autoplay. If a completed export file is missing but exact inputs survive, rebuild as a newly verified immutable publication, preserving historical references. If inputs no longer survive, keep the desired preference and explicit unavailable status. Reject malformed state transactionally, not by partially accepting a new Export section over an old session. Copies share readable immutable history while future requests/publications remain independently owned. Actual accepted preference/reference changes notify host-save through the existing coalesced non-parameter path.

### 7. Validation and approval sequence

Use independent sample oracles for impulse/asymmetric IRs, sparse capture, negative pre-roll, 0/50/65/100%, opposite/correlated overlaps, frozen/pending replacements, origins, complete tails and page seams. Compare decoded WAV and preview interior samples; isolate monitor ramps and dry host passthrough. Assert render counts/asset hashes and per-swell settings do not change for pure mix edits. Exercise job supersession, restore/copy, save mid-job, missing assets, write/close/rename failures, invalid metadata and unsupported-size rejection.

Native UI fixtures cover reference state layering, shared slider/type/stepper/focus behavior, truthful pending/errors, Play/Stop cursor, minimum/larger sizes and representative raster scales. Owner checkpoint A covers revised reusable control families; checkpoint B covers integrated Export 0/50/100 states and waveform/readiness behavior, using a directly launched preview. Retain prior approvals for unchanged header/control families. Record native raster checks separately from OS DPI/host acceptance.

Run focused affected tests while implementing; run one final appropriate Debug/Release wrapper validation with `scripts/build.ps1 -Configuration Debug|Release -Test` after changes settle. Avoid overlapping wrappers or raw build commands. Consolidate FL Studio into one short final flow: preview/export parity, real Drag WAV placement/cancel or Save fallback, bypass/double-dry guidance, and save/reopen. No repeat of automated fault/edge cases absent a concrete host failure. Document exact tested versions/configuration and untested DPI/host coverage.

## Risks / Trade-offs

- [Full-song gain measurement takes time] → cancellable bounded scans, cached measurements keyed by immutable dependencies, immediate pending feedback and coalesced mix intent; no false-ready audio.
- [Capture gaps may omit expected dry notes] → owner-selected zero substitution is persistent and range-specific; coverage stays distinct, and dry-inclusive guidance explains a replacement can be incomplete.
- [Unreadable assets change after verification] → pin verified immutable handles/identities; fail and retain prior completed files if writing cannot match the plan.
- [Drag/drop host behavior differs] → native invocation/lifetime tests plus one real FL Studio drag test; Save is a tested fallback, not a claimed universal drag guarantee.
- [Long files and immutable history consume disk] → preflight capacity/format limits, bounded streaming and explicit errors; defer cleanup/portability to #8 and never delete sole Playlist assets.
- [Concurrent main UI plans] → retain the current approved header, coordinate only the shared mode/notice integration seam with `enhance-header-mode-navigation`; leave its icons/style changes and `bound-waveform-paint-work` implementation to their changes.
- [Illustrative mockup inconsistencies or obsolete styling] → use the owner-supplied comparison sheet and written mix contract for audio/layer behavior, preserve current shared styling/header and review the actual native result at checkpoint B.

## Migration Plan

1. Implement/test the additive Export preference/identity schema and legacy migration before publication paths; keep backups of earlier FLP/cache state for binary rollback.
2. Land bounded composition/mixer and verified file publication with independent oracles, then explicit preview/voice integration under existing mode guards.
3. Integrate the approved dedicated workspace and shared controls behind real readiness, retaining current Learn/Perform behavior and saved preferences.
4. Complete native approvals and consolidated host smoke, update living guidance/evidence, then sync/archive only when explicitly authorized. Do not alter #11's archive or implement #38 as part of migration.
5. Roll back by restoring earlier project/cache copies with the earlier binary; do not promise older binaries read the newer creative schema, delete assets or destructively rewrite references.


## Approved refinement: navigation and last-good presentation

The shorter nine-point review is authoritative. Propose a screen-local roughly
4:3 WAV target rather than filling the right column vertically; group Save and
Return beneath it using existing spacing. No new universal geometry/density or
guardrail definitions. A compact legend keys aqua Wet and neutral Dry to their
actual displayed gains, including inactive endpoints, and retains shared scale.

Extract a reusable read-only navigation interaction over `SwellViewport`, retaining
`SongBarRuler`/accepted song mapping. Fit includes exact earliest/latest audible
samples across the complete composition plus the helper's visual margins (which
may extend outside file bounds); never clamp a relevant peak to a viewport edge.
A compact Fit action and double-click/Home restore it. Wheel zoom anchors at the
pointer; drag or horizontal/Shift-wheel pans; +/- and arrow keys provide focused
keyboard equivalents. Bound navigation numerically and by known composition
extent. View state is ephemeral, independent of file range, and survives ordinary
refresh/playback. A newly accepted composition may fit once; pending work never
repeatedly refits. No editing, scrubbing, host seeking or manual swell creation.

Retain one previous accepted waveform while preparing/failing the requested mix.
Show its actual previous percentage with Updating preview, dim only its plot and
keep labels legible. Requested percentage updates immediately; Play/Save/Drag
remain admitted only against the current fully verified requested revision.
Publish matching metadata/waveform/readiness together; stale results cannot take
over. Session/instance reset clears prior presentation references together.
Count retained summaries and bounded viewport-detail work under the shared budget.
Viewport detail reuses the accepted measured mixer/asset readers and gain plan;
it never reconvolves or rewrites the WAV. Coalesce/cancel detail requests separately
from mix/file intent, service preview prefetch, and retain no full-song PCM.

Label Preview position/duration as the exact audible span, not viewport or file
length. WAV duration remains its full declared song-origin interval. The reviewed
2.048 s dry/mixed versus 1.020 s wet-only fixture is legitimate when its enabled
wet ends sooner; tests must establish coverage/tails, not force equal durations.
Keep muting/layering and stay-in-Export guidance in restrained existing surfaces.

Checkpoint B is reopened for the revised proportions, legend, navigation family,
last-good pending/error presentation and duration labels. Existing header and
unchanged control approvals are retained. Validate minimum/larger bounds and
software raster scales separately from real Windows DPI/FL Studio acceptance.

## Preview positioning (owner requested implementation now)

Unmodified left-click or left-drag sets the internal preview position using the
existing absolute-sample viewport mapping, clamped to the accepted audible range.
A stopped seek only parks the cursor. Play begins there; without a parked cursor
it retains the first-audible default. During playback, seeking supersedes old
prefetch intent and continues at the new position with existing boundary ramps.
Rapid drag requests coalesce by ticket; Stop, session/mode/mix changes revoke
delayed starts. Stop parks the current position; completion returns to the chosen
start. Position is ephemeral and cleared with accepted-mix/session/mode resets.
No stale/last-good waveform can seek while the requested mix is pending.

Shift-left-drag and horizontal/Shift-wheel retain panning; wheel zoom and Fit
remain unchanged. Right-click does not seek. The cursor and time readout follow
the requested position immediately, then the actual callback cursor, without
showing the outgoing seek's fade-out cursor. No DAW seek, scrubbing engine,
capture, render/file mutation or saved-state change is introduced. Integrate the
future #46 controls later against this existing viewport/preview boundary.

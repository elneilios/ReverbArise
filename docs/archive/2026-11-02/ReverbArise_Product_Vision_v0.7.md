# ReverbArise — Product Vision and Design Specification

**Version:** 0.7 — ReverbArise naming and branding alignment (supersedes v0.6)  
**Status:** Product vision and proposed architecture, not an implementation contract  
**Primary target:** Windows x64 VST3 **audio effect** in FL Studio  
**Technology:** C++20, JUCE, CMake, MSVC, VS Code, Codex, OpenSpec  
**Product name:** **ReverbArise** (selected working brand; trademark and availability checks remain outstanding)
**Former internal codename:** Preverb (retained only where existing file/repository references have not yet been migrated)

## Branding and document references

- **User-facing product identity:** ReverbArise, using the approved wordmark with the waveform-shaped **A** and aqua-accented “Arise”.
- **Product-facing copy, examples and new design documents** should use ReverbArise. Existing code identifiers, plugin IDs, cache folders or OpenSpec change IDs may still use the older codename until an intentional migration is performed; do not rename durable state identifiers casually.
- **Approved UI direction:** `ReverbArise_UI_Layout_and_Style_Guide.md` plus its accompanying layout/style reference images. The product vision defines behavior; the design guide defines the visual system.
- **Historical competitor reference:** “Low End Candy Preverb” is a separate named commercial product, not the name of this project.

## 1. Vision

Build a **live, marker-based reverse-reverb editor implemented as a mixer-insert audio effect**. The user places the plugin on the source's mixer channel, plays the arrangement to capture audio, confirms or edits suggested onset markers, chooses which notes/chords receive swells, and hears generated reverse-reverb swells together with the original signal on subsequent playback. Each swell remains visually editable: its source region, start, arrival/end position, fades, curve and gain.

**Core promise:** *Capture once, target the musical moments, shape the swells visually, and hear them in context without bouncing and reversing clips manually.*

The design takes inspiration from Low End Candy Preverb's TARGET editor, but is an independent design rather than a claim about its internals. Our distinctive focus is a **progressively discovered, sparse song timeline** that is built non-contiguously as users play different regions, editable per-swell regions and envelopes, with wet-only and source-plus-wet export.

### 1.1 Key product decision

**Primary format is an FX VST3, NOT a MIDI instrument.** The older plan for automatically mapping swells to MIDI notes becomes a possible future companion instrument, not an MVP dependency. Re-recording the arrangement through a mixer insert is the main input path; file import remains useful for tests and editing without playback.

## 2. Primary user workflow

1. Insert ReverbArise on a dedicated FL Studio mixer channel receiving a synth, instrument or audio track.
2. With **Song Mapping** enabled, start FL Studio transport at **any song location** and play. ReverbArise records the incoming dry input together with host position metadata into a **sparse song map**, progressively filling only the regions visited. You may jump from bar 86 to bar 1 and capture both independently; an uncaptured gap is not treated as recorded silence.
3. ReverbArise displays the captured waveform in a song-timeline editor. Offline onset/transient analysis proposes **candidate arrival markers**. Suggested markers are not automatically required to produce swells.
4. Confirm, delete, move or add arrival markers; enable only the musical events that should receive a reverse swell.
5. Select a convolution impulse response (IR), change tone/IR tail parameters and **Generate**. Regenerate affected swells automatically after a successful recapture if enabled; never block the audio callback.
6. Show each reversed wet result underneath the original waveform, anchored to its selected note/chord. Permit dragging **swell start**, optional **audible end offset**, fade-in/out handles, curve and individual gain. These edits are nondestructive and don't require reconvolution.
7. On subsequent FL Studio playback, output the original input plus the prepared wet swells at their song positions. A wet-only monitoring mode supports routing the effect to a dedicated track/bus.
8. Export either **Wet only** or **Source + wet** as a timeline-aligned audio file. Optionally export selected range / individual swell in later milestones.
9. When the source arrangement changes, use **Regenerate Selected**, **Regenerate All**, or **Recapture & Regenerate** with an explicit range; optional host-controlled transport may reduce manual steps when supported.

### 2.1 Live audio caveat

This is a **capture/prepare/replay** workflow. Pre-rendered swells can lead into a known note on later playback, but the plugin cannot predict future notes in a first, unrecorded live performance. Before capture is ready, audio must pass through normally (subject to documented monitoring modes). If input or MIDI changes, affected swells may be **stale** until recaptured/regenerated. Never promise fully automatic anticipation of arbitrary newly played notes.

## 3. Full-song capture and timeline (essential distinction)

A short 1-, 2-, 4- or 8-bar loop is a convenient *view*, not a maximum supported song length. **MVP supports Song mode only**; any Pattern mode handling is out of scope and must not be assumed to map to Playlist instances. Low End Candy documents TARGET loop lengths of 1/2/4/8 bars; its public material does **not** establish full-song editing with navigation across independent sections. **Our product design explicitly supports multi-section/full-song capture.**

### 3.1 Navigation

- Global **overview ruler** representing the entire captured range, with zoom and a minimap.
- Configurable **detail view** showing 1, 2, 4, 8 or 16 bars, plus freely zoomable continuous scrolling.
- **Previous / next page** moves by the chosen display span; clicking the overview jumps to any bar.
- Captured regions and targeted swells use absolute song time/beat/sample anchors, not local page indices.
- Swells may start in the previous page and end in the next. Page boundaries **never trim or reset audio**.
- Multiple non-contiguous capture passes may fill different song ranges; revisiting a section may refresh it according to an explicit replacement policy. Preserve other captured sections.
- Display **uncaptured**, **captured silence**, **captured audio**, and **stale/render-invalid** regions distinctly; never silently fill missing sections with zeros and present them as captured.
- At song start, the editor can show negative/lead-in time for swells that precede the first captured note.

### 3.2 Host transport behavior

- Query tempo, meter, playback/record status and **host musical/absolute position** using the relevant JUCE host-playhead facilities; investigate precision and discontinuities in FL Studio.
- Maintain the association between captured audio frames and timeline positions, including transport seek, stop, loop, playback from the middle and tempo changes. Discontinuities start a new mapped segment rather than stitching unrelated blocks together.
- Schedule prepared swells from **absolute song-sample position**, independent of whether a dry signal is currently arriving at the mixer insert. A swell may begin in an unmapped/silent section and must still output there if the host processes the plugin.
- Distinguish **song-position locked** playback (main use) from optional **loop-relative** behavior (later).
- On seeking into the middle of a pre-rendered swell, playback must join at the correct sample offset, not necessarily retrigger the swell from its start.
- After stopping, resuming or looping, avoid duplicated or missing tails; test carefully around host buffer boundaries.
- Decide how to respond if a tempo edit changes the beat positions of captured material; explicitly mark affected captures stale and require verification/recapture rather than silently time-stretching.

### 3.3 Capture strategy and scalability

- For full arrangements, prefer **streamed/managed, sparse capture assets on disk** and compact waveform peaks rather than duplicating the entire song as unbounded RAM buffers or stuffing large audio blobs into plugin state. Store covered ranges independently (or use a sparse-file/index design) with a coverage bitmap/interval index.
- Store a stable project identifier, source sample rate/channel count, captured timeline start and ranges, onsets, IR identity, per-swell settings and cache version/hash.
- Support a passive **Map while playing** mode, explicit **Recapture selection** / **Recapture current section** / **Clear capture**, with warnings for edited source regions. Audio capture must never record this plugin’s own generated wet output.
- Plan for stale-capture detection based on transport / captured region updates; arbitrary changes to upstream audio cannot always be detected automatically without re-recording.
- Save the editing state with the FL Studio project, with an asset-cache/sidecar policy robust to project moves and reopening. Detailed storage and restoration requirements appear in §10.

## 4. Audio routing and monitoring

**Default mode: Source + Wet.** Pass the incoming input through and mix in generated swells according to their absolute song positions. The source pass-through should not be replaced by an old recording in normal monitoring; use current live input as dry. Since the rendered swells come from captured source audio, warn or indicate when they no longer match the currently playing input.

**Alternate mode: Wet Only.** Output only the rendered swells, enabling an independent FX mixer channel where supported. Do not assert routing or plugin latency behavior until tested within FL Studio.

**Monitoring states:**
- **Idle / no capture:** dry pass-through; clear status.
- **Capturing:** dry pass-through while source is recorded; no promise of a swell before yet-unseen incoming notes.
- **Rendering:** continue glitch-free playback of previous valid swells or temporarily bypass affected regions, with visible status.
- **Ready:** play pre-rendered wet swells in song sync alongside current live dry input (or wet only).
- **Stale:** retain previous rendered swells if user chooses, but visibly mark them as not necessarily matching new input.

A full-song pre-swell can begin before the first musical event; FL Studio must start transport early enough to play that lead-in or the user must export it as an audio file. Do not promise audio before the start of host playback.

## 5. Editor and marker model

**Editor lanes:** (1) captured source waveform and arrival markers, (2) generated wet swell previews either stacked or overlaid, (3) optional mix/overview waveform. Shared horizontal position, grid, zoom, snap, scroll and transport playhead. Suggested onset markers are visually distinct from confirmed/active markers.

**Marker/region concepts:**
- **Arrival/target marker:** musical point at which the swell leads into a note/chord, stored at sample-accurate position in captured source coordinates and on the absolute host timeline.
- **Source capture region:** editable part of audio starting near the target event and extending forward, with an independent end. Prevent unintended inclusion of neighboring events.
- **Rendered full wet region:** generated reverse reverb before any trimming or fades.
- **Swell start:** where the audible swell begins; movable nondestructively.
- **Swell audible end:** default near arrival marker, optionally earlier/later for gap or overlap.
- **Fade in/out:** lengths and curve (e.g. linear, smooth, exponential) adjustable independently; optional end fade, with the option to remain at full gain up to arrival.
- **Per-swell gain, enable/mute/solo and rename.**

All marker moves operate nondestructively. Changes in marker selection, capture region, IR or IR preparation invalidate the relevant renders; trimming/fades/levels should update without reconvolution.

## 6. Onset detection

Run offline analysis on captured regions and suggest likely onset markers. Initial implementation can use spectral flux/local-peak picking or a suitably licensed library. Provide sensitivity, minimum separation and regenerate suggestions while preserving confirmed manual edits. Test synthetic impulses, piano/plucks, soft pad attacks and dense material. Do not promise perfect polyphonic note detection.

## 7. Convolution and alignment

Use a licensed/resellable IR library and JUCE convolution facilities or a testable offline convolution engine. For source segment `x` of length `N` and IR `h` of length `M`, full linear convolution yields `N + M - 1` frames. Two mathematically equivalent workflows for linear time-invariant convolution (with correct boundaries):

```text
reverse(source) → convolve(normal IR) → reverse(full wet output)
source → convolve(reversed IR)
```

If the source segment begins at timeline sample `P`, its complete reversed-convolution buffer begins at `P - (M - 1)`, assuming full-length convolution and no additional resampling/latency offsets. Store alignment at integer sample precision. Audible energy peak need not coincide with the arrival marker because IRs can have initial silence, predelay or several peaks. Offer visual marker/fade adjustment.

**Engine requirements:** Full-tail flush; sample-rate conversion of IR; defined mono/stereo policy; no render work or allocation on real-time audio callback; background generation; immutable render cache handoff; handling overlap without unwanted clipping. Tests must cover buffer seams, song seeks, loop wraps, moving targets and long IRs. A deliberately finite IR makes tail duration deterministic.

## 8. IR controls

- Initial categories: room, hall, plate, cathedral, ambience (subject to redistribution rights), plus custom WAV IR import.
- IR duration/tail shortening, tail fade, level; simple HP/LP tone shaping and stereo width if feasible.
- Reverb generation at 100% wet internally, separate dry input mix. Avoid doubling the source.
- Changing IR or its preparation invalidates affected swells and schedules regeneration.

## 9. Export

Two required modes:

1. **Wet Only:** all active swells summed, with per-swell fades, trim and gain, no original audio.
2. **Source + Wet:** **captured source audio** plus identical wet-swell timeline at configured dry/wet gain. Unlike live monitoring, this is a fixed offline render of the selected capture; label this distinction clearly.

Exports share the same sample rate, origin, timeline span and sample alignment. If swell audio starts before the captured source, extend the export range to include the leading swell and offset captured source by the exact number of frames. Full-length source must not be inadvertently truncated. Prefer 32-bit float WAV by default; do not auto-normalize/limit without opt-in. Provide source-origin placement metadata/instructions for FL Studio, especially for wet-only exports.

**Acceptance:** For unity gain and no mastering, composite = properly aligned captured dry + wet-only output (within floating-point tolerance). Test negative-time lead-ins and several overlapping swells.

## 10. Session assets, storage and project portability

**Default:** Hybrid RAM + disk persistence. Store short active playback buffers and cached waveform peaks in RAM; persist captured source audio and rendered swells in a **sparse, disk-backed session asset store**. Do not embed long PCM recordings in VST3 plugin state or write audio to the VST3 installation directory.

- **Working asset location (Windows default):** `%LOCALAPPDATA%\K7MA\ReverbArise\Projects\<project-id>\<instance-id>\` is a candidate, not a final externally stable API. Distinguish sessions and plugin instances using durable UUIDs. Names should be treated as examples until tested.
- **Source assets:** Store precisely positioned captured regions as lossless 32-bit float WAV (or an equally lossless internal format), with explicit source start sample, sample rate, channels, coverage intervals and capture revision. **Captured silence is covered audio; an uncaptured gap is not.**
- **Rendered swell assets:** Store full, untrimmed wet convolution results per swell, with render hashes/versions of input capture and IR parameters. Nondestructive start/end cropping, fades, curve and gain remain metadata applied during playback/export.
- **State in `.flp`:** Through VST3/JUCE state persistence, store compact session/instance IDs, relative or resolvable asset references, song anchors, capture coverage, target markers, source dependencies, IR identity and edits. Never assume that an `.flp` automatically bundles external media.
- **Atomicity and safety:** Background-write new assets to temporary files and atomically publish manifests/replacements. Never overwrite a sole valid render mid-playback, delete in-use assets or perform file I/O on the real-time audio thread. Explicitly version manifests to support migrations.
- **Project portability:** Provide a future-facing **Pack Project** function that copies source, render and IR dependencies to an export folder with a manifest and checksums. On reopen, verify linked assets, offer **Relink**, and report missing/stale resources without silently replacing them with silence. Test Save As, duplicate `.flp` files, moving to another PC and crash recovery. Because ordinary VST3s may not know the `.flp` path, do not assume assets can be placed automatically beside the project.
- **Retention:** Working caches contain project-critical data until backed up or packed; never automatically purge sole copies. Include storage usage, cache health and safe manual cleanup controls.

### 10.1 Explicit regeneration operations (MVP)

| Operation | Input required | Behavior |
|---|---|---|
| **Regenerate Selected** | Existing captures | Re-render selected swells only; no DAW playback needed |
| **Regenerate All** | Existing captures | Re-render every swell affected by current source/IR settings; no DAW playback needed |
| **Recapture & Regenerate** | Fresh input on transport playback | Arm a chosen song range, overwrite only covered samples received from FL Studio, compare capture revisions or content, then asynchronously re-render only dependent swells |

- Optional **Auto-regenerate after recapture**, **off by default** until reliable invalidation and background render are proven. Changes solely to start/end markers, fade shape or level are nondestructive and do not trigger convolution.
- **Dependency graph:** Each swell records source capture interval/revision, IR identity/hash, and convolution settings. Recaptures invalidate only swells whose source dependency overlaps actually modified audio. Prefer block/region hashes and explicit user-controlled refresh; the DAW does not notify a standard FX plugin about arbitrary upstream MIDI edits.
- **State machine:** `Ready → Recapture Armed → Capturing → Capture Committed → (Unchanged | Stale) → Queued → Rendering → Ready`; failure/cancellation retains the last valid render with an obvious **stale** label and retry action. A manual Generate command can start at `Queued` from stored captures. Processing never writes the plugin's own wet output back into the source map.
- **Real-time safety:** Render on background workers, prepare complete new buffers and atomically switch snapshots at safe boundaries. Coalesce repeated edits, allow cancel/retry and show progress. Never block callback waiting for disk, locks, IR processing or UI.
- **Recording rules:** Recapture only the explicitly armed range (or selected captured region), preserve unrelated mapped areas, detect seek/loop discontinuity, and mark partial coverage distinctly. An incomplete capture must not be presented as a complete refresh. The user controls whether to retain a known-stale render until the new one succeeds.

### 10.2 Optional host transport requests (feasibility only)

The user may request **Recapture & Regenerate** without leaving the plugin UI. We should *probe* whether FL Studio supports a suitable, host-provided transport-control interface to seek/play/stop. Current host support is **unverified**, and a standard plugin is never entitled to command transport. Standard JUCE playhead access is primarily for **reading** host timing; do not mistake methods available in an abstraction for guaranteed control in FL Studio.

- Discover/verify transport-control capability and its available actions (seek, play, stop, possibly loop/record). If an optional VST3 interface is available through the SDK, test it separately and check JUCE's wrapper integration before architecting around it.
- Any control action requires an explicit button press or other user intent. Show a clear capture range, refuse unsafe automatic jumps while the user is recording, and never alter the transport invisibly.
- **Supported path:** With the user's permission, request seek → play → capture range → stop (only if safely controllable) → regenerate. Confirm observed state/position after requests; requests can fail or be ignored.
- **Fallback path (required MVP):** Display **Move playhead to bar X and press Play**; automatically record when the DAW enters the armed region, then regenerate after adequate samples have been captured. Do not attempt keyboard/mouse simulation as default host integration.
- Host transport control is an **experimental enhancement**, **not** an MVP dependency. All capture and rendering operations must work with manual transport.

## 11. Proposed modules

```text
ReverbArise/
  docs/
  CMakeLists.txt
  Source/
    Core/          # Session model, timeline, markers, serialization
    Capture/       # Real-time input capture, host position tracking, disk assets
    Analysis/      # Onset suggestion engine
    DSP/           # IR load/prep, convolution, alignment, envelope shaping
    Playback/      # Position-aware wet-swell scheduling and dry/wet mixing
    UI/            # Overview, waveform lanes, target editor, fade handles
    Plugin/        # VST3 effect wrapper and host state
  Tests/           # DSP, transport/timeline, edits, persistence, export
  Resources/       # Redistributable IRs and synthetic test media
```

C++20 + JUCE + CMake; Windows VST3 effect first. Keep separate headless processing library to test render behavior without FL Studio. Optional standalone/import editor later if worthwhile.

## 12. Revised incremental milestones

### M0 — FL Studio transport, progressive capture and silent-input output spike (FIRST TASK — architecture gate)

**Goal:** Prove that a Windows VST3 **audio effect** can both (a) map input to precise, non-contiguous FL Studio Song positions and (b) output pre-scheduled audio at those positions **without any incoming mixer audio**. This is a feasibility spike, not the beginning of the reverb UI.

**Implement only:**
- **Transport-control probe (diagnostics, not a requirement for success):** Detect any supported plugin-to-host seek/play/stop capabilities, add explicit UI test buttons if safe, log request versus observed transport behavior, and verify graceful fallback when absent or ignored. Do not depend on these capabilities for mapping or scheduled audio.
- Build a minimal C++20/JUCE/CMake Windows x64 VST3 **effect**, with transparent stereo passthrough, a diagnostic UI and clear logs of host transport status, sample/PPQ position, tempo, time signature and block sizes when supplied.
- Use the host's per-block absolute song-sample position to place captured dry input into a **sparse in-memory or temporary-disk song map**, recording explicit coverage separately from captured silence. Preserve prior sections when the playhead jumps.
- Allow capturing non-contiguous ranges, e.g. play bars 86–90, seek to bar 1 and play bars 1–4; display or log both mapped intervals and the unmapped gap. The spike need not provide a full waveform editor.
- Include a tiny **scheduled audio generator** that produces a deterministic **440 Hz sine tone** at **−18 dBFS peak** over a user-configurable absolute bar interval, initially **bar 8 through the start of bar 10** (two bars). Apply short click-free fades at boundaries. The tone must be mixed into the plugin output even when input samples are all zero and no source notes are playing.
- Derive oscillator phase from **absolute sample position relative to event start**, not the amount of uninterrupted playback since triggering, so seeking into the middle gives the correct waveform phase and looping/replaying is repeatable. For bar scheduling, calculate musical bar boundaries using host beat/meter data; verify behavior with a fixed tempo/time signature first and explicitly document limitations when tempo maps or meter changes are present.
- Make diagnostic controls for **schedule start/end**, **enable tone**, **enable mapping**, **clear map**, plus optional display of host sample position and observed input peak. Avoid allocations, disk writes, logging or blocking operations on the real-time audio callback: stage capture blocks into an appropriate lock-free/real-time-safe queue for background persistence/visualization.
- Test a mixer insert with an **assigned but currently silent** source, and separately a **completely unused insert with no source routed**. Determine whether FL Studio calls the effect and delivers its generated output in both cases; don't assume it does.
- Test **Song mode** (Pattern mode not required), transport start/stop, seeking, replay, mid-tone start, loop-wrap, first-note pre-roll, project save/reopen, buffer-size changes, and offline render/export if feasible. Record actual host behavior and discrepancies.

**Acceptance gates (all required to claim the architecture viable):**
1. In FL Studio Song mode, diagnostic host position corresponds to visible Playlist location and does not drift across multiple passes.
2. Capturing bars 86–90 followed by bars 1–4 preserves both segments, with the gap marked *unmapped* rather than silence; mapped samples are correctly aligned when compared to a known test signal.
3. A tone scheduled for bars 8–10 is audible and measurable **before any source note** and with zero-valued mixer input, with stable sample boundaries and no audible clicks.
4. Starting playback midway through the scheduled tone outputs the **correct sample offset** rather than retriggering the oscillator at phase zero.
5. Repeated/looped playback, seeking, and saving/reopening maintain deterministic scheduling and don't duplicate events. Test the same event in an offline bounce if supported.
6. Report explicitly whether a **completely unused** mixer insert runs its audio callback and emits the tone; failure in that case is not necessarily a blocker if a routed but silent source works. Document any required host-specific workaround.
7. Record results of the optional host transport-control probe, including whether seek/play/stop requests are possible, accepted, ignored or unavailable. **Lack of host control does not fail M0**; manual controls remain supported.

**Failure strategy:** If FL Studio does not invoke the effect during silent sections of the normal routed mixer path, investigate routing, always-on processing or a dedicated signal-source companion before proceeding. Do **not** claim success merely because the tone works while audio is flowing. If host timing is incomplete, defer architecture decisions until the Song-mode transport behavior is measured.

### M1 — Persistent progressive song map and waveform overview
- Promote the M0 capture experiment into managed, crash-tolerant on-disk audio segments with coverage metadata, stable instance IDs, atomic manifests, and sensible overwrite/recapture rules.
- Display source waveform/overview; capture one later region then an early region without overwriting either, with clear uncaptured gaps.
- Navigate via overview and previous/next page; preserve map/diagnostics across project save and reopen using an asset-storage policy.
**Accept:** Non-contiguous segments remain accurately placed and replayable, stale/covered/unmapped states remain distinct, and no feedback of this plugin's own wet output enters the capture map.

### M2 — DSP reverse-convolution proof
- IR import/load and headless offline render of source-region swells.
- Regression tests for exact placement, full tails and negative/leading time.
**Accept:** Reference signal comparisons and no truncation.

### M3 — TARGET markers and swelling editor
- Onset suggestions, manual confirmation, source regions, multiple swells, generated waveform lanes, independent start/end and fades.
- Regenerate Selected / Regenerate All / Recapture & Regenerate actions with dependency invalidation, stale-state indicators, background rendering and optional auto-regenerate after recapture. Manual transport fallback is required; automatic host control optional.
**Accept:** Visual editing stays responsive; unrelated swells unchanged.

### M4 — Live FX playback
- Play prepared swells in sync with current live input and FL Studio transport.
- Wet-only/source-plus-wet modes, seeking into swells, overlapping tails, loop boundaries, stale indicators.
**Accept:** Audible, glitch-free playback at correct song positions after seek/loop; no stale claims.

### M5 — Export and persistence polish
- Wet-only and source-plus-wet timeline exports, Pack Project / Relink or defined portability workflow, asset management, robust state reopen.
- FL Studio guide and real-song testing.
**Accept:** Exports align sample-for-sample and sessions reopen reliably.

### Later candidates (NOT MVP)
- MIDI triggerable swell instrument or dedicated sample-player companion.
- Live zero-capture reverse effect with host latency/lookahead, if feasible.
- More sophisticated transient/onset classification; auto recapture/diff detection.
- Whole-session stem export, draggable WAV clips, macOS, algorithmic reverb, third-party VST hosting.

## 13. Major risks

- **Host timeline accuracy:** FL Studio might report changing or discontinuous positions; test early before UI investment. Map **Song mode only** in the MVP; do not infer pattern-instance information from transport positions.
- **Capture state:** captured source and currently playing upstream signal can diverge; clear stale state and recapture workflow.
- **Long full-song assets:** disk streaming and cache management rather than massive plugin-state blobs.
- **Loop/seek behavior:** voice scheduling at buffer boundaries and correct mid-swell offset are critical.
- **Project portability:** sidecar/cache storage, missing media, moves/renames, autosave and crash recovery; local cache is not disposable if it contains sole source recordings.
- **Host transport control:** Seek/play/stop requests may be unsupported, ignored, or unsafe in current host context. Manual transport remains mandatory fallback.
- **Regeneration correctness:** Changed source audio must invalidate dependent swells; preserve valid old renders until replacements are fully ready, without writing from the audio callback.
- **Silent-section execution:** FL Studio may suspend a VST3 effect if its mixer path is considered inactive/silent; M0 must independently prove scheduled tone output on a routed-but-silent channel and assess fully unused inserts.
- **Pre-roll:** the plugin cannot play prior to DAW transport start; early swells require earlier playback or export.
- **IR licenses and JUCE licensing:** verify before public distribution.
- **Naming and release clearance:** ReverbArise is the selected name; check trademark, audio-plugin market collisions, domains and distribution metadata before release. Low End Candy already markets a different product called Preverb.

## 14. Open decisions

1. Final cache location, Pack Project asset format, backup strategy and safe relinking policy (core hybrid disk/RAM design is agreed).
2. Default page length, zoom behavior and capture-overlap replacement policy.
3. MVP **Song mode only**; whether to add pattern-relative repetition in a future release.
4. Behavior when a captured note shifts to a new bar: manual recapture, assisted marker shift, or optional re-analysis.
5. Whether source+wet monitoring can be run safely in parallel FL Studio routing without double-dry paths.
6. IR stereo formats, default library redistribution and resampling methods.
7. Initial default for optional auto-regeneration, change-detection granularity and stale-render playback policy (manual regenerate actions are agreed).
8. How to handle sample-rate/tempo/meter changes to existing captured material.
9. Trademark/domain availability of **ReverbArise**, plus distribution/licensing approach; name selected but not legally cleared.
10. Does the current FL Studio VST3 host offer plugin-initiated seek/play/stop, and through which API if any? Must be measured in M0; never assumed.
11. Scope and UX of Pack Project / Relink in MVP versus immediate post-MVP, without compromising basic reopen reliability.

## 15. OpenSpec starting prompt

> Use `docs/ReverbArise_Product_Vision_and_Design_Specification.md` (version 0.7) as product context. Propose change `spike-fl-song-map-and-silent-output`: create a Windows C++20/JUCE/CMake VST3 **audio effect** that (1) logs FL Studio Song-mode playhead positions, (2) stores non-contiguous incoming dry-audio captures as a sparse song map with coverage metadata, and (3) schedules a 440 Hz, −18 dBFS peak test tone from bar 8 to bar 10 even with silent mixer input. Prove correct seek/loop/mid-event timing, silent-channel execution and project reopen. This is a narrow feasibility spike; do **not** implement convolution, transient detection, full waveform editing or export. Additionally probe, but do not depend on, any host seek/play/stop interface with explicit user-triggered diagnostic buttons. Record supported/unsupported/ignored outcomes and manual fallback before committing to the wider architecture.

---

**Product principle:** *A reverse-reverb effect that learns the musical material, preserves a full-song edit timeline, and lets the musician decide exactly where each swell starts and arrives.*

**Research note:** Public Low End Candy documentation confirms TARGET grid loop sizes of 1/2/4/8 bars but does not establish a whole-song navigation workflow. Our full-song design is a proposed differentiation, not a statement about that product's internal implementation.


## Automatic by default (added in v0.6; deferred until core engine is validated)

**Priority:** Product direction and follow-up OpenSpec work; **do not interrupt or invalidate the in-progress engine implementation**. The earlier workflow steps requiring manual Generate, Analyze or Regenerate describe the original engine/bootstrap experience, not the intended final UX. After core functionality is available, supersede those manual actions with the automatic workflow below.

- New instances load a legally redistributable default IR, map dry input during normal Song-mode playback, automatically detect suitable onsets and automatically enqueue convolution rendering for eligible markers.
- Once captured and rendered, pre-note swells play at absolute song positions on subsequent passes, including on silent incoming audio. An unencountered transient cannot produce an anticipatory swell on first playback.
- Source changes discovered during playback invalidate dependent swells; IR/DSP changes enqueue relevant rebuilds. Apply timeline placement, trims, gain and eligible envelope edits at playback time without unnecessary reconvolution.
- Distinguish automatically suggested markers from explicit/pinned user edits; don't let re-analysis silently discard intentional edits.
- Background processing must be bounded, cancelable and real-time safe. Display status instead of primary Generate/Analyze/Regenerate buttons; reserve manual refresh/rebuild operations for recovery or troubleshooting.
- The **approved visual direction** remains the large two-lane waveform editor, contextual inspector and full-song overview. The updated automatic-workflow visual is a design vision only; implementation details will be reconciled with screenshots and OpenSpec changes after the core engine is ready.

**Detailed backlog and acceptance tests:** see `Preverb_Post_Engine_Automatic_Workflow_Backlog.md` (AUTO-01 through AUTO-06; legacy filename not yet migrated).

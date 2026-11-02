# Preverb — Functional UI & Automatic Processing Handoff (v1.2)

**Audience:** Codex / OpenSpec implementation.  **Status:** target product behavior, to be implemented *after* verification of the current audio engine.  **Design reference:** clean, single-screen `Preverb_Lean_Functional_UI_Wireframe_v1.1.png` (visual reference only).  **Precedence:** this handoff defines required behavior; the wireframe does not authorize additional features.

## 1. Objective and boundaries

Refactor the current debug-oriented plugin into a usable reverse-reverb FX VST3. The core experience is **insert the plugin, press Play in FL Studio, and Preverb progressively learns the incoming song audio, suggests target transients, creates default swells, and renders them automatically**. The user edits the result, not a sequence of Capture → Detect → Generate buttons.

Preserve the existing sample-timeline capture, convolution/rendering, per-swell IR assignment, wet scheduling, manual edit controls, and audition path wherever they work. **Audit before replacing engine code.** Build the automatic processing coordinator on those existing primitives; avoid a DSP rewrite as part of UI cleanup.

**Explicitly out of scope:** high/low-cut EQ, stereo width, IR-decay DSP, global dry/wet mixer mix, preset management, Reverb/Detection tabs, host-initiated DAW transport, CPU meter, another visual envelope editor, and new reverb algorithms. No prominent Generate, Suggest Onsets, or Regenerate buttons in the main UI. Retain developer tools in a collapsible Diagnostics panel until verified obsolete.

**Honesty rule:** statuses reflect real engine events. Do not show “Automatic Processing” as working, or “Ready” for swells, before the corresponding functions actually execute and a playable result has been committed.

## 2. User-facing happy path (normative)

1. **Insert:** On instance creation, load an **actually bundled and valid default IR** and initialize the song map, transient detector, renderer queue and empty swell list. No initial modal, wizard, manual IR selection, capture arm or Generate action. If default IR is unavailable, surface an actionable error; never pretend rendering succeeds.
2. **Play somewhere:** User presses Play in **FL Studio Song mode**, perhaps at bar 86 rather than bar 1. While playing, the plugin records only the **incoming dry audio** into a sparse map at the host-reported absolute sample positions. The timeline shows coverage as it becomes available. Captured silence is distinct from an unmapped gap.
3. **Discover targets:** A background detector examines newly complete/updated captured source windows and proposes **automatic transient markers**, with a documented default sensitivity and minimum separation. Stable markers are drawn on the source lane; manual markers/edits are protected from being overwritten.
4. **Create swells:** For each newly accepted automatic target, create a swell model with its **own IR reference** (initially the valid default IR) and conservative default source window, audible start/end and envelope. One target should not produce duplicate swells on repeated playback.
5. **Render:** When the necessary source window is available, queue a convolution job automatically. On completion, validate and **atomically commit** the result. The corresponding swell displays a wet waveform and becomes ready for scheduled playback. Rendering never blocks the audio callback.
6. **Replay:** On a subsequent pass, Preverb plays committed swells at their song positions, including **before the original transient**. The dry signal continues through the plugin unmodified apart from previously existing intentional routing behavior. The first pass cannot retroactively play a swell before a transient it has only just discovered.
7. **Refine:** Selecting a swell exposes its per-swell IR and numerical timing/gain/fade controls. Dragging handles and typing values edit the same model. Changes trigger the **minimum necessary** automatic update; no Generate button.
8. **Audition:** The selected swell can be auditioned while the DAW is stopped, either wet only or blended with its *captured* dry source, with a typeable 0–100% Wet value. Audition mix never modifies normal mixer output or rendered assets.

**Non-contiguous example:** If playback visits bars 86–90 and later bars 1–4, both chunks are mapped to their own positions. Bars 4–86 remain **unmapped**, not silent. The user can add swells inside either captured region without recording the whole song.

## 3. Precise state and ownership model

### 3.1 Capture map

Maintain sparse coverage on the **sample timeline** (integer sample positions) and source audio stored on disk off the real-time callback. Capture must use the raw input buffer **before adding wet swells or audition output**. Each interval carries:

- `unmapped` — no source samples received; cannot be interpreted as silence;
- `captured` — source samples received, including legitimate silence;
- `updated` or a monotonically increasing source-region revision — new playback overwrote prior samples;
- `invalid/error` — storage failed or source data is inaccessible.

Record the sample rate, channel count, transport epoch and capture provenance so the system can detect incompatible state. Map every process block using the host's actual position, not elapsed wall time. On backward jumps/loops, write to the new absolute location instead of appending. Never allocate unbounded storage or perform disk I/O in `processBlock`.

Revisiting a captured region replaces/refreshes that region. Compare or checksum updated samples in worker code where feasible; unchanged source should *not* provoke expensive re-renders. If the host does not report a usable position, pause mapping and show `Waiting for song position` rather than invent an absolute bar. Song mode is a user/host requirement: do not assert the standard VST3 API identifies individual FL Studio Playlist clips or Pattern IDs.

### 3.2 Markers and source windows

Differentiate **detector suggestions**, **accepted automatic targets**, and **user-created/modified targets**. The detector may refine unedited suggestions as capture grows, but may **not silently delete or move user-edited targets**. Automatic markers require stable IDs and a deterministic duplicate/merge policy (e.g. proximity tolerance around the same transient) across repeated playback. Detection sensitivity and spacing are internal/default settings for this milestone; they do **not** require a new Detection tab.

A target at time `T` requires source audio from a clearly defined extraction window associated with that transient. The wet render may also depend on IR content, IR version, source-window settings, render parameters and the sample rate. Do **not** assume simply observing a transient guarantees the whole source tail has been captured: until its input extraction window is complete, mark it `Waiting for source` and queue later. A capture at the start of a mapped region may still support a wet swell *earlier* than that region once convolution completes; pre-roll audio need not have been captured there.

### 3.3 Each swell owns its own IR

Each swell stores an independent `irAssetId` (plus content identity/revision), not just a pointer to a mutable global 'current IR'. A new swell inherits the default IR **at creation**. Changing the selected swell's IR updates **that swell only**, queues its render, and does not alter its neighbors. Distinct swells may legitimately use different IR lengths and channels. On missing IR, keep the swell edit state, mark it `Missing IR`, and request relinking; do not silently substitute a different IR for an existing swell.

### 3.4 Render status

Expose honest per-swell states: `Waiting for source`, `Queued`, `Rendering`, `Ready`, `Stale / updating`, `Missing IR`, `Failed`, and optionally `Disabled`. Show a restrained aggregate status in the header, e.g. `Mapping · 2 updating` or `Ready · 12 swells`. A completed worker result becomes visible/audible only after validation and a safe buffer swap. On new versions, an existing ready buffer may continue to play while updating **only if explicitly marked stale**; otherwise mute it rather than implying it matches the current settings. Never let an obsolete job overwrite a newer result.

## 4. Automatic processing coordinator — required sequence

The implementation should be event/dependency driven, not a timer that continuously rerenders all swells. This describes logical events; audio-thread communication should use real-time-safe techniques.

```text
On plugin instance initialization:
    validate/load bundled default IR (worker as needed)
    restore or create stable source-map / swell state
    enable Song-mode progressive mapping and auto-processing
    show empty/waiting state until source blocks arrive

On each host audio process block:
    read absolute host transport sample position (if available)
    pass through incoming dry audio
    enqueue dry input samples + timestamp into bounded capture pipeline
      only while host transport is playing and mapping is enabled
    mix already committed scheduled wet buffers using absolute sample offsets
    never run transient scans, IR parsing, convolution, file writes or UI work here

On completed/updated capture interval [a,b):
    persist source and coverage metadata
    compare with previous content, increment revision if changed
    enqueue detection for affected source region (+ context around boundaries)
    invalidate only swell jobs that depend on changed source

On detection result:
    reconcile suggestions with existing stable marker identities
    preserve manually edited targets
    create default swell models only for new eligible automatic targets
    for each affected swell, determine readiness of required source extraction
    enqueue a render only when required source and IR are available

On render job:
    snapshot source, settings, IR identity and revision
    perform convolution and required DSP away from audio callback
    prepare committed buffer and waveform overview
    if dependency revision still matches, atomically publish
    else discard obsolete result and schedule the latest version

On parameter edit:
    update canonical swell model immediately
    classify edit as playback-only or render-affecting
    schedule minimum work; coalesce/debounce rapid render-affecting edits

On stop/seek/loop:
    stop adding capture blocks when not playing
    continue/finish safe background jobs as appropriate
    schedule wet playback based on new absolute song position
    do not continue free-running the old playback cursor
```

**Required first-pass behavior:** Swells found during first playback can only be heard in their entirety on replay, or when the DAW later visits their pre-roll positions. Do not claim zero-latency predictive playback of unseen notes.

**Transport gaps:** A process block may straddle the boundary of a scheduled swell; the mixer must add only the overlapping samples, sample accurately. Playback started in the middle of an existing swell must start at the corresponding offset within its committed audio. Song seek/loop must not leak audio from a previous timeline position. Test whether FL Studio processes an FX insert during source silence and on an entirely unused mixer insert; unsupported cases must be reported.

## 5. Edit-to-work classification (prevent unnecessary convolution)

| User or engine event | Required processing |
|---|---|
| New mapped audio interval | Capture → source revision → detect eligible targets → render new/affected swells |
| Revisit identical captured samples | Preserve markers, swells and ready renders; no gratuitous regeneration |
| Changed captured samples in a swell's source window | Mark dependent swell stale → re-render automatically |
| New accepted automatic transient | Create swell with default IR → render when source complete |
| User adds/deletes a target | Create/delete corresponding swell and scheduling entry; render new only |
| User manually moves target / changes source window | Update dependencies/placement; re-render **if** extracted source or render inputs change |
| Change selected swell's IR | Re-render **selected swell only** |
| Change fade-in/out, curve, audible trim, level, mute or enabled state | Apply playback/envelope updates immediately **where supported without convolution**; regenerate shaped playback asset only if current engine architecture absolutely requires it, never recompute convolution needlessly |
| Change audition dry/wet mix or audition playback state | Audition only, **zero convolution jobs**, no change to normal FX output |
| Change default IR for *future* swells | Future swell creation only; existing per-swell assignments remain unchanged unless user explicitly requests otherwise |
| Rapid drag of render-affecting control | Debounce/coalesce; latest revision wins; no runaway worker queue |
| Missing capture or missing IR | Show waiting/error, retain edits, do not commit misleading output |

**Dependency identity:** At minimum include source window IDs/revisions, target and source selection, IR asset content/revision, render-specific settings, channel layout and sample rate. Use an immutable job snapshot or generation counter. Make render commits atomic and cancel/supersede stale pending jobs. Limit worker concurrency and cache memory usage; prioritize work near the playhead if appropriate but do not starve older jobs.

## 6. Single-screen layout — UI controls only

**Header:** Preverb title and real aggregate status (waiting, mapping, rendering, ready, error). Avoid promotional content, CPU gauges, and buttons for generation steps.

**Left swell list:** One row per swell: selected state, enabled checkbox, optional name, short per-swell IR name and actual ready/updating/waiting/error indicator. Selecting a row updates the waveform and inspector; adding/deleting is available if backed by engine functionality. A list row must not imply all swells have the same IR.

**Central waveform:** Song bar ruler; captured dry waveform; rendered wet waveforms; accepted/suggested transient/arrival markers; selected swell's draggable start/end and envelope handles. Unmapped spans are visually distinct from captured silence. Zoom/pan/snap and song-position navigation, without showing raw sample addresses by default. Numeric inspector values and on-canvas handles bind bidirectionally to the same model. If a proposed control does not correspond to the real engine model, omit it rather than simulate an effect.

**Right Selected Swell inspector (no tabs):** Per-swell IR selection/load; name and enabled; editable timing fields for arrival/source selection/audible start and end as supported by actual engine; fade-in/out, curve types and swell gain. Use compact knobs **plus typeable values** where parameter domains are confirmed; preserve precision for sample/time input. Do not invent a separate Peak Position DSP parameter if the actual envelope model cannot express it. Avoid duplicate derived length/pre-roll readouts.

**Audition strip:** Play/Stop of *selected* swell, `Wet Only`, `Dry + Wet`, a 0–100% Wet slider and typeable percentage. Shows missing dry-data status if needed. This is **separate from normal FL Studio mixer output**.

**Song overview:** Display mapped/unmapped regions, targets and viewport; click/drag to navigate. It may be delivered after the core editor if it requires substantial underlying work, but its displayed coverage must match the real map.

**Diagnostics foldout (keep):** 440 Hz spike test, raw sample addresses, host PPQ/sample positions, capture logs, peak levels, IR/load errors, force-recapture/re-render and manual suggest/regenerate recovery actions. Do not place these on the default interface. No new transport buttons; FL Studio remains the transport authority unless capability has been explicitly tested.

**UI semantics:** A transient suggestion isn't automatically a ready swell. The interface must distinguish each stage. Actions should use one stable selected-swell identity so repeated capture/detection cannot change the user's selection unexpectedly.

## 7. Audition behavior contract

- Audition uses the **committed shaped wet result** for the currently selected swell. It works while FL Studio is stopped, via the existing audition route, if the host supports it. Verify real audible output before claiming success.
- `Wet Only`: selected wet swell in isolation, regardless of stored blend value.
- `Dry + Wet`: fetches corresponding **captured dry timeline** and aligns it against the wet swell by absolute sample position. Do not reverse or time-shift the dry. The wet pre-roll can precede any dry signal.
- Mix is a typeable **0–100% Wet** value; 0 = dry only, 100 = wet only, 50 = reference blend. Choose and document linear or equal-power gain law; do not silently mix gain laws. Switching to Wet Only and back preserves the last mixed value.
- If dry coverage is incomplete, indicate `Dry source not captured for this span`; do not label an unmapped region as silence. Wet Only remains available.
- Modifying audition mix or starting/stopping audition **never** changes regular VST output, render dependencies, exported assets, or the default per-swell IR. Switching selection must stop or cleanly switch the audition without leaking old audio.
- Keep disk reads outside the audio callback; use prepared RAM buffers and safe handoff. Audition state may be persisted, but not used to affect song playback on restoration.

## 8. Failure and restoration cases

- On project reopen: restore stable IDs, capture coverage, saved per-swell IR references, edits and committed cache references. If assets are missing, show `Missing source` or `Missing IR` and keep marker edits; do not fabricate silent/ready content.
- If source audio changes upstream in FL Studio while transport is stopped, the plugin **cannot know until that area plays again**. Call it *last captured*, not *confirmed current*. After recapture, update only affected swells.
- Do not trust waveform display alone as proof the plugin outputs wet audio. Verify host FX output and the independent audition path.
- During playback with no input, committed wet material must still sound at scheduled positions **if FL Studio calls the insert**. Verify source-silent routing and document FL Studio-specific behavior. Never promise host behavior that tests have not established.
- On unsupported song-position information, suspend mapping with clear status; do not mix Pattern-mode phase with absolute Song-mode coordinates.
- On queue overload, file failure or invalid IR, keep plugin responsive, preserve previous valid audio when safe and expose actionable errors rather than dropping data silently.

## 9. Acceptance criteria (Codex: demonstrate, not merely implement)

**Progressive mapping and first-use automation**

1. Insert a new instance, select Song mode in FL Studio, provide input and press Play. **Without pressing any plugin button**, source waveform and map coverage appear at correct bars; transient markers then appear; qualifying swells get queued and eventually show real wet waveforms and `Ready` state.
2. Start at bar 86, stop, then start at bar 1. Both regions are captured in place; the gap remains unmapped and does not create false silent-source transients.
3. Play the same region twice with unchanged audio. The same marker/swell IDs remain, no duplicates are created and no unnecessary convolution jobs run.
4. First pass does **not** claim it played a reverse swell before a newly discovered transient. On second pass, the completed swell sounds before arrival at the correct song position.
5. Begin transport inside an already rendered swell and loop/seek across it. Playback uses sample-position offsets without drift, wrong placement or stale output.

**Independent IR and regeneration**

6. Assign IR A to Swell 1 and IR B to Swell 2; both play correctly. Change Swell 1 to IR C: only Swell 1 regenerates, and Swell 2 keeps IR B and its rendered audio.
7. Change source samples feeding Swell 1, recapture via normal Song playback, and verify Swell 1 becomes stale then updates automatically; unrelated swells are untouched. Repeat unchanged recapture and verify no unnecessary renders.
8. Rapidly change an IR or render-affecting field: final value wins, obsolete render commits are prevented and the real-time audio callback remains responsive.
9. Drag a fade/trim/gain control, then type a precise value. Handle, numeric field and playback agree. Neither action triggers unnecessary *convolution* if the engine can apply the change as playback shaping.
10. Remove default IR asset in a controlled test: new render is blocked with an explicit error, not fake `Ready` status. Existing per-swell missing IR is retained for relinking.

**Audition and separation**

11. With DAW stopped, `Wet Only` auditions selected committed swell; `Dry + Wet` overlays captured original source with proper pre-roll and transient alignment.
12. Type 0%, 50%, and 100% Wet; verify endpoints and blend. Switching modes retains the previous blend. These changes trigger zero render jobs and leave normal mixer output untouched.
13. Audition missing dry coverage: show unavailable indication, allow Wet Only, and don't pass uncaptured gaps off as recorded silence.

**UI and resilience**

14. Main view is usable without any raw sample addresses, Generate, Suggest or Regenerate buttons. Current engineering diagnostics remain reachable from a dedicated foldout.
15. Plugin saves/reopens with its map, markers, per-swell IRs, edits and valid committed assets. Missing cache/IR errors are visible and recoverable.
16. During silence before the original note, test wet playback on (a) an insert routed from a silent instrument and (b) a completely unused mixer insert; document each actual FL Studio outcome instead of assuming both run.
17. Verify no blocking disk I/O, convolution, dynamic asset load or heavy detection on the real-time audio callback.

## 10. Delivery plan / OpenSpec decomposition

**Phase A — verify baseline (blocking):** Establish a reproducible test of wet audio audible in FL Studio at the intended song position; confirm sample mapping, worker rendering, per-swell IR state and audition independently. Keep developer controls accessible. Log defects separately before polishing.

**Phase B — automatic pipeline (core behavioral change):** Implement progressive capture coordinator; detection reconciliation and stable marker identity; automatic swell creation; default IR inheritance; dependency invalidation; background render queue and atomic commit. Prove acceptance tests 1–10. This phase is **required for the eventual automatic-by-default UX**, even if scheduled *after* the current engine task. Do not claim it is already implemented.

**Phase C — lean UI refactor:** Left swell list, dual-waveform editor, inspector with per-swell IR and bidirectional fields, honest status, overview and diagnostics foldout. No additional DSP. Prove acceptance tests 9, 14–15.

**Phase D — audition enhancement:** Wet-only and captured-dry-plus-wet options, adjustable persisted mix (audition only), missing coverage handling and output isolation. Prove tests 11–13.

**Phase E — stability:** Cross-check FL Studio seeks, loops, silent inserts, project restore and audio-thread safety. Test full happy path after fresh insert without clicking a Generate button.

**Codex execution rule:** First audit current code and OpenSpec changes against this document. Report which states and tests are already supported, which need engine work, and which are UI-only. Implement in small reviewable OpenSpec changes; do not silently remove working diagnostics or introduce speculative controls.

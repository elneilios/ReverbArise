# ReverbArise — Three-Mode Adaptive Workflow (Design Proposal)

**Status:** Proposed · **Scope:** Workflow, UX, state, capture, scheduling, export, persistence · **Visual authority:** `ReverbArise_UI_Layout_and_Style_Guide.md` and its reference images.  
**Related:** [Pattern playback/capture integrity issue #1](https://github.com/elneilios/ReverbArise/issues/1).

## Product principle

**Capture once. Shape and audition live. Optionally finalise into audio.** The large three-position mode control communicates the entire operating model: **Capture — Learn**, **Live — Edit & play**, **Finalise — Export**. Modes control both DSP behaviour and which workspace regions are visible.

## 1. Mode model

| Behaviour | Capture | Live | Finalise |
|---|---|---|---|
| New instance default | Yes | No | No |
| Capture previously unmapped dry source | Yes, on transport playback | No | No |
| Overwrite existing mapped source | Only with explicit Recapture selection | No | No |
| Schedule existing wet swells with DAW | Yes, if ready | Yes | **No** |
| Source dry passthrough | Yes | Yes | Yes, unaltered |
| Per-swell markers/IR/envelopes | Locked | Editable | Locked |
| Global controls | Available as appropriate | Available | Locked |
| Plugin audition | Locked during capture | Available | Unavailable |
| Render/export a timeline-aligned wet WAV | No | Initiated by Finalise | Available |
| Retain captures, edits and rendered assets | Yes | Yes | Yes |

Mode switch is **prominent, three-segment, mutually exclusive**, directly below existing product header. Each segment contains mode name and sublabel; a short, live status line below describes the active operating state. Highlight the active segment aqua; inactive enabled controls are neutral; unavailable modes visibly disabled.

### Initial availability

New instance with **zero valid captured source**: **Capture active; Live and Finalise disabled**. The two modes enable only once sufficient valid mapped material exists. If no useful capture occurs, remain in Capture. A user can return from Live to Capture to add more unmapped song regions. Return from Finalise to Live is supported and does not discard the existing export.

### Automatic transition

1. Insert plugin: Capture ready.
2. FL Studio starts playback: capture eligible, uncovered song positions; display coverage as it progresses.
3. DAW transport stops: finish/commit valid capture blocks, enqueue candidate marker analysis and swell generation, then **automatically enter Live** once valid captured content exists. Rendering may continue asynchronously with a subtle progress indication; do not hang the UI.
4. In Live: subsequent host playback only auditions previously generated wet events; **never starts capture**.
5. User may select Capture to add more material, or an explicit scoped Recapture action to replace existing capture.
6. User may stay in Live indefinitely as a conventional live insert effect. **Finalise is entirely optional.**
7. Selecting Finalise initiates preparation of an export revision; enter export-ready state only upon verified successful preparation. If preparation fails, preserve Live playback and a valid previous render.

Use separate notions of **operating mode**, **capture coverage**, **render generation**, and **export revision**; do not conflate them.

## 2. Capture workspace — Learn

**Adaptive layout:** Collapse/hide the Swells list and Selected Swell inspector, widening the central source-waveform canvas across most of the plugin. Retain header, mode switch/status, relevant global controls, timeline ruler, real capture waveform and full-song **Overview** with coverage. Preserve unmapped gaps distinctly; never display fake recorded audio for missing regions. Capture status and current region should be obvious but unobtrusive. Do not draw stale wet-envelope controls as editable.

**Lock editing controls:** individual swell selection/editing, IR, fades, draggable handles, manual marker operations and plugin audition controls are disabled/inaccessible. Relevant global controls remain enabled, e.g. Auto-marker and sensitivity; a sensitivity change during capture must not mutate committed, user-pinned markers. Existing wet swells may continue playing according to the agreed Capture behaviour, but capture always records **dry input only**, never plugin wet output.

**Coverage policy:** Captures are non-contiguous and keyed to absolute DAW song positions. A Capture pass **fills uncovered positions only**, including actual captured silence; previous covered intervals are untouched. Explicit **Recapture Selection** is the sole way to replace already covered audio. Implement interval-level coverage and write/commit protection, not just UI button disabling. Partial or aborted capture retains verified coverage without damaging earlier captures.

**Accepted Pattern-mode trade-off:** Pattern playback may map previously unmapped host positions; this is a documented user-training risk. It **must not overwrite prior captured Song coverage** or invalidate untouched render dependencies. Do not implement blanket Pattern-mode suppression. Avoid unreliable Song/Pattern inference from bars, looping or seek behaviour. Update #1 to reflect revised policy.

## 3. Live workspace — Edit & play

Restore approved **three-column work area** (Swells list, largest waveform editor, Selected Swell inspector); show global controls, audition Dry↔Wet mixer, **Overview** and transport information. Editing controls are enabled.

Live schedules wet swells at absolute host positions during DAW playback and passes the current dry audio through; Live **never captures dry source**, including during Pattern playback. The plugin can remain in Live forever if the user does not need rendered audio.

### Automatic marker ownership

Store marker provenance independently:
- **Auto markers:** generated by transient/onset analysis. Changing sensitivity re-evaluates/reconciles automatic suggestions against the existing recorded source. Turning Auto-marker **OFF removes automatic suggestions** and leaves manual/pinned markers untouched.
- **Manual/pinned markers:** added manually or promoted by explicit user movement/edit; they are immutable under automatic reanalysis, sensitivity changes or Auto-marker OFF.
- Switching Auto-marker back ON generates automatic suggestions while respecting pinned/manual markers; avoid near-duplicate candidates and distinguish their source visually if needed.
- Editing a generated marker (position or intentional parameters) should pin/promote it, never allow the next sensitivity change to discard the user's work.

IR changes and source/render-invalidating edits queue bounded background re-renders. Envelope timing, gain and fades update nondestructively where feasible. Preserve last-good wet renders until replacements are ready and clearly indicate stale/pending state.

### Audition

Plugin Play/Stop and one Dry↔Wet audition mix slider are valid in Live. The audition mix is not the plugin's permanent dry/wet output setting. Host transport sync and wet scheduling remain independent of capture. Do not add speculative EQ, duplicated envelope panels or redundant wet-only controls.

## 4. Finalise workspace — Export

Finalise is an **optional, reversible, dry-passthrough state** that retains every piece of editable session information. The plugin remains loaded but produces **no wet audio**, performs no capture and does not silently update renders. Dry passthrough remains unchanged (no duplicate dry or added processing latency if avoidable).

**Adaptive layout:** Keep product header and mode switch visible. Dim the prior working surface and place a **large central export overlay over the control surface**, clearly making export the sole task. Prominent draggable WAV file tile; **Save WAV…** alternative; concise format/alignment/status; **Return to Live** action. Existing editor contents remain conceptually behind the overlay, not deleted. All edit controls inactive in Finalise.

**Export:** prepare a real persistent **100% wet** WAV from enabled, completed swells. Default **song-aligned** export: time starts at chosen absolute song origin (normally beginning of song), includes leading silence needed for Playlist alignment, handles multiple non-contiguous swells/overlaps and negative pre-roll policy explicitly. Optional trimmed-range export may be considered separately. Default lossless 32-bit float WAV subject to engine support; no unrequested normalisation. Use exact sample timing and preserve tail endings.

Drag/drop: write a verified file before starting JUCE external file drag; test FL Studio VST3 host embedding and Windows external drop. Conventional Save WAV is mandatory fallback. **Exported WAVs are project-critical assets**, not ephemeral disposable temp files; keep a stable path/manifest or let users choose save location. On edits after returning Live, previous exports remain intact and an export-revision mismatch can be reported; never overwrite a file referenced by the DAW without explicit user intent.

**Transition safety:** Avoid silencing wet playback before export is prepared successfully. Returning from Finalise to Live restores wet scheduling and editing. If the WAV remains in the Playlist, warn briefly of potential **double wet playback**. Returning to Capture should be available through Live and should not implicitly overwrite coverage.

## 5. UI styling and layout requirements

Follow the approved design guide strictly: flat charcoal background; aqua active controls; yellow timing targets; thin horizontal/vertical rules, no widespread card panels or gradients; stylised waveform A within Arise; Inter-style typography; **Overview** title (not “Song Overview”); only overview viewport gets the major rounded enclosing border. New mode-switch control should harmonise with those tokens; avoid a huge card around the entire header or altering the waveform lane styling.

- **Capture:** visually widened waveform and capture coverage; editing columns hidden, not just greyed.
- **Live:** full approved eight-region layout with three-column editor.
- **Finalise:** full-surface dimmed editor with central export overlay; mode selector remains navigable.
- Responsive at JUCE plugin dimensions/DPI scaling; preserve readable mode status and waveform editing space.

### Proposed text

- Capture — *Learn*: “Capturing unmapped audio. Swell editing is locked.” / when idle: “Press play to map your song.”
- Live — *Edit & play*: “Swells play with your DAW. Edit and audition without recapturing.”
- Finalise — *Export*: “Your swells are ready. Drag the WAV into your DAW or save it.”

## 6. Persistence and architecture

Persist active mode, mapped coverage intervals, manually confirmed/pinned marker identities, automatic-detection configuration and provenance, IR settings, source revision/dependencies, wet render revisions, export metadata and associated stable assets in project state + disk-backed session store.

**No file I/O, blocking, expensive DSP, or unsafe allocation inside the real-time audio callback.** Capture and rendering use background queues/workers. Atomic commit/snapshot handoff protects active playback. A stopped transport must not race with incomplete capture worker flush; switching modes must be deterministic and testable. Do not silently discard sole copies of recorded source or wet assets. On project reopen Finalise must stay finalised/dry-only until user changes mode.

## 7. Acceptance criteria

- [ ] New plugin shows Capture active, Live and Finalise disabled; no fictional waveforms.
- [ ] Capture presents a widened real waveform canvas and Overview; editing panels/controls are hidden or locked.
- [ ] Capture fills new host positions only and never silently overwrites covered positions.
- [ ] An explicit scoped Recapture permits deliberate replacement, preserving unrelated regions and renders.
- [ ] DAW Stop completes/commits capture and automatically transitions to Live when valid material exists; pending rendering never stalls audio.
- [ ] Live restores the full approved three-column edit UI; editing and audition work.
- [ ] Live DAW playback schedules wet events but never records additional source, including in Pattern mode.
- [ ] Auto-marker sensitivity recalculates only automatic suggestions; Auto OFF removes auto suggestions; manual/pinned markers always survive.
- [ ] User-edited auto markers become pinned/manual and survive re-analysis.
- [ ] Finalise produces a verified, correctly aligned 100% wet WAV, with drag/drop and Save WAV fallback.
- [ ] Finalise shows prominent export overlay and visually disables editor; dry passes unchanged and **wet scheduled output is zero**.
- [ ] Finalise is optional; Live can be permanent. Live↔Finalise roundtrip preserves all project data.
- [ ] Existing exported files are protected from unintentional overwrites; re-export revision tracking exists.
- [ ] Project save/reopen restores modes, captures, swells and export references correctly.
- [ ] Tests cover DAW stops, seeks, loops, noncontiguous captured regions, silence, Pattern coverage of unmapped timeline, multiple instances, partial capture, stale render, missing assets and failed export.
- [ ] Layout/color/typography agree with approved UI guide at tested plugin dimensions.

## 8. Suggested implementation breakdown

1. **State-machine OpenSpec proposal** (mode, eligibility, automatic DAW Stop transition, persistence).
2. **Capture interval guard and explicit recapture** (non-destructive sparse coverage, revision dependencies).
3. **Marker provenance and reanalysis** (user-pinned protection, sensitivity live regeneration).
4. **Adaptive JUCE layouts** (expanded capture, full Live, export-overlay Finalise).
5. **Wet export and drag/drop** (sample alignment, file durability, Save WAV fallback).
6. **Regression suites and FL Studio validation** (Song/Pattern observations, audio safety, reopen).

## 9. Open engineering decisions

- Exact threshold for “valid capture” that enables Live/Finalise; behaviour with captured silence and no detectable onsets.
- Whether mode can enter Finalise with no enabled/generated swells; preferably disable until an exportable wet result exists.
- Capturing paused vs transport stopped, discontinuities, host-reported playback status and background worker completion.
- Specific UI placement/sizing of the prominent mode bar, final overlay, and Recapture controls.
- Windows VST3 external drag/drop reliability; WAV alignment metadata and paths.
- When to bypass wet playback during export preparation; protect live audio if export fails.
- Optional export of selected swell/range, any later offline composite export.
- Preserve pre-existing JUCE IDs and session state formats unless deliberate migration is planned.

**Authoritative context:** ReverbArise Product Vision and UI Layout/Style Guide. The workflow decisions above supersede older “continuous recapture/manual Generate” assumptions where inconsistent. Do not silently broaden the DSP feature set merely to match illustrative mockups.

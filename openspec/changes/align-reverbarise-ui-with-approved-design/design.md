# Design

## Context

See `proposal.md` for the problem and scope. This is a native UI alignment over an implemented capture/prepare/replay engine, not a new audio milestone. `openspec context --json` resolves this repository and its spec-driven schema; configuration has no additional context/rules. No `AGENTS.md` was found outside generated/dependency output. There were no active changes before creating this one; predecessors, including automatic workspace and Song playback, are archived. No implementation or host acceptance is claimed by this plan.

### Reference inventory and authority

| Input actually inspected | Use and authority |
| --- | --- |
| User's attached `Pasted text.txt` | Detailed proposal brief; subsequent chat decisions refine placement, source visibility, control parity, reset and names. |
| `docs/ReverbArise_Product_Vision_and_Design_Specification.md`, v0.7 | Behaviour/scope. Its final Automatic by default section supersedes manual-first vision instructions; it does not override accepted engine constraints. |
| `docs/style/ReverbArise_UI_Layout_and_Style_Guide.md`, v1.0 | Written visual authority, read in full. Numerical tokens are proposed baselines, adopted consistently here rather than represented as extracted production measurements. |
| `docs/style/ReverbArise_Layout_Structure_Reference.png` | Inspected region/border reference; the image's outer enclosing frame is not a requirement for extra panels. |
| `docs/style/ReverbArise_Visual_Style_Reference.png` | Inspected mark, colour/type/control reference. Its conceptual square icon and textured background do not override the written rules. |
| `docs/prototypes/ReverbArise_Init.png` | Inspected current empty composition. |
| `docs/prototypes/ReverbArise_SamplesLoaded.png` | Inspected current loaded/selected composition, including the corrected 1.35 s example. |
| `docs/a_clean_dark_gray_charcoal_ui_wireframe_for_a_plug.png` and older handoff | Historical context only; not mixed into the current visual target. |
| `resources/logo/README.md`, `source/ARTWORK_NOTES.md`, `licenses/Inter-OFL.txt`, SVG masters and preview/wordmark PNGs | Added during planning and inspected: production outlined wordmark and font-source provenance; reuse the dark SVG, not the preview contact sheet. |
| `Source/Plugin/PluginEditor.{h,cpp}`, `Source/UI/*`, processor/model/engine and tests | Evidence of implemented behaviour, not proof of visual compliance. |
| `out/transport-ui/envelope-1400.png`, `empty.png` | Existing native software-rendered evidence, inspected. The file named empty still contains captured waveform/export-warning context, so it is not a clean fresh-instance empty acceptance fixture. |

The prototype pair is the only named Init/SamplesLoaded pair currently in `docs/prototypes`; matching composition and timestamps on 2026-11-01 support using it together. The user subsequently supplied `resources/logo`: production path-only SVG wordmarks/tagline lockups, transparent PNG variants, source fonts, Inter OFL notice, build source and export manifest. The inspected dark wordmark uses only #F5F7F9/#00EFFF with viewBox -14 -91 636.8359 109 and no live text, raster image or filters; preserve that aspect ratio/clear space. This resolves the original missing-artwork dependency. `resources/PreverbHall.wav` remains the only bundled IR. The vision's referenced `Preverb_Post_Engine_Automatic_Workflow_Backlog.md` was not found in the inspected docs; do not invent its contents or block the already implemented automatic workflow on it.

Authority decisions: uniform flat surfaces override raster texture/vignette; the 65% slider must be mathematically positioned even where imagery is inaccurate; actual convolution energy need not peak at target. Omit nonexistent presets/undo controls instead of inventing systems. Keep durable Preverb host/state/cache names. Later explicit user decisions authorize two global sensitivity knobs, visible Freeze/Solo, real bundled/file IR choices, inspector knob parity for all canvas edits, clearer source selection, two reset scopes and bar-based names. These deliberate deviations from the prototype are functional requirements, not aesthetic improvisation.

### Observed implementation

- `CMakeLists.txt` builds a Windows x64 C++20 JUCE 9.0.4 VST3 effect using MSVC/CMake and pinned WavPack. Host product remains `Preverb Spike`; bundle ID `com.k7ma.preverb`, manufacturer `K7Ma`, plugin code `Prv1`. No new dependency is needed.
- `PluginEditor` is a large native component with a ListBox, SwellTimeline, SwellOverview, inspector and diagnostics Viewports; controls use default JUCE styling plus scattered literal colours. There is no project LookAndFeel or UndoManager; the newly supplied logo/fonts are not yet integrated into this editor.
- Default size is 1400x900, limits 1100x850 through 2000x1400. `resized()` uses fixed strips and adaptive inspector width; opening inline diagnostics consumes another 190 logical pixels. `paint()` boxes header/list/editor/inspector. A 10 Hz timer refreshes snapshots, fields and waveform views even with diagnostics closed.
- List names/enabled/own IR/status are real. Source/wet bounds, source handles, fade handles, tensions, gain, whole-body placement, per-swell mute/solo/freeze, precise timing, marker and regeneration sensitivity all have working model paths. The source selection currently has small bottom-edge grips without a clear shaded selected interval or equivalent independent full-height boundary lines.
- `SwellEngine::snapshot()` returns selected raw/shaped/source peak summaries, coverage/session state, live/offline readiness, monitoring members, dry span and an audition cursor. UI commands are model edits and background jobs, not host parameters. `SwellViewport`, `MusicalPosition`, `SongBarRuler` and `EnvelopeHandles` provide reusable coordinate/gesture laws.
- `PluginProcessor` has no registered host-automatable parameters/APVTS in the inspected source; program access is a one-program stub, not a preset system. Keep this parameter surface unchanged. XML `PreverbState` and JSON session are version 7 with legacy parsing; capture/IR/render UUIDs, immutable references and preferences already persist.
- Automatic capture/detection/rendering, per-swell freeze, tolerance, live scheduling and explicit offline bounce exist. Current host validation records precede this UI revision; do not infer new appearance/DPI acceptance from them. Standalone plugin file export and Pack/Relink remain future scope.
- `tests/swell_ui_test.cpp` constructs the real editor without a desktop peer, drives bindings/gestures, and uses JUCE SoftwareImageType/paintEntireComponent through `PREVERB_UI_M3_OUTPUT`. Native offscreen rendering is feasible on this Windows environment. It does not establish FL Studio window embedding, physical DPI or host behaviour; those checks remain manual. Native computer-control APIs are unavailable here.

### Requirement-to-implementation gap matrix

Classes: V = visual/layout; B = interaction/binding; E = focused engine/model dependency. Status describes inspected baseline, not task completion.

| ID / requirement | Authority | Current evidence | Status | Proposed action / class | Verification |
| --- | --- | --- | --- | --- | --- |
| G01 Eight stable regions, globals above inspector | Guide I.1-4; brief 4 | Editor::resized, inline diagnostic allocation | Partial | Shared layout, fixed bands/columns independent of state; V | Region assertions, matched state PNGs |
| G02 Uniform background and border discipline | Guide I.2, II.6 | Editor::paint and Timeline::paint literal fills/rounded boxes | Gap | Theme tokens, rules only, one major overview outline; V | Surface samples + border inventory review |
| G03 Integrated wordmark, type and icons | Guide II.5/7/9 | Plain Preverb title; new resources/logo pack not bound | Integration gap | Reuse dark SVG, supplied Inter with tested system fallback, stroke icons; V | Asset provenance and native appearance review |
| G04 Two independent global sensitivities | User follow-up; marker/tolerance specs | markerKnob/regenerationKnob work but cramped | Partial | Global band: Auto-marker plus both labelled exact-entry knobs; V/B | Resize, restore and independence tests |
| G05 Scrollable list and meaningful names | Guide I.3; user naming | paintListBoxItem, Core::add uses collection count | Partial | Thin selection, own IR, stable ID, new bar-derived names; V/B/E | Long lists/names, naming timing fixtures |
| G06 Real per-swell IR choices | User follow-up | loadIR(file,true), only bundled Preverb Hall, custom default | Partial | Bundled/current dropdown plus file button, explicit target IDs; B/E | Two IR assignments, chooser race/cancel/freeze |
| G07 Shared source/wet time and target | Guide II.10; accepted specs | SwellViewport/Timeline, target currently pale solid | Partial | Reuse transforms, yellow dashed target, real grey/aqua waves; V | 120 BPM sample/pixel oracle, negative time |
| G08 Clear source start/end and shaded span | User follow-up | Small source grips; no selected range overlay | Gap | Both full-height boundaries, subtle solid selection overlay; V/B | Source knob/drag parity, gaps remain distinct |
| G09 Every canvas edit has knob/exact entry | User follow-up | Several fields hidden under advanced; only some knobs | Gap | Eleven inspector controls, grouped scrollable two-column grid; V/B | Each handle/body/target roundtrip, precision tests |
| G10 Curve/endpoint gesture safety | User; transport time-alignment spec | EnvelopeHandles and regression fixtures already work | Preserve/restyle | Outline circles, preserve centre-line/z-order/coincidence rules; V/B | Zero/full/extreme/far/near zoom tests |
| G11 Wet placement and real support | reverse-convolution and workspace | moveWet/requestedShapeBounds/setShape | Present | Bind relative controls without changing laws; B | No reconvolution for movement, pending extension tests |
| G12 One audition slider, independent live settings | Guide I.3/II.8; brief 6 | Two mode buttons plus blend; hidden latent percentage | Gap | Effective continuous percentage adapter, real Wet Only at 100%; B | 0/50/65/100 pixel and audio fixtures, restore |
| G13 Preview playhead/isolation/tail/trim | Accepted monitoring, user praise | EditorSnapshot cursor and existing controls | Present | Preserve cursor/span; Settings options, visible nonzero trim/mode summary; V/B | Cursor lifecycle, selected-dry and tail tests |
| G14 One honest Overview with ruler | Guide I.2-3 | Song Overview heading, outer frame includes title, schematic swells | Gap | Heading/ruler outside sole waveform frame, actual support/capture; V/B | Sparse fixtures, viewport/navigation oracle |
| G15 Informational host footer | Guide region 8 | Diagnostic string has host flags; no normal footer | Gap | Availability-aware position/BPM/meter, no host controls; V/B | Unknown/stopped/seek fixtures |
| G16 Truthful automatic/ready/error UX | Vision final section; processing specs | Real automatic engine, status/warnings | Present, cluttered | Concise status and actionable Settings recovery; V/B | Fresh empty + mapped unselected + stale/error |
| G17 Dedicated purposeful diagnostics | Brief 8 | Inline viewport, all spike controls retained | Gap | Separate owned window; disposition table below; V/B | Lifecycle/no side-effect tests during audio |
| G18 Reset to actual creation defaults | User follow-up | resetShape uses Shape{}; initialShape adds 20 ms release | Gap | Wet reset split action and explicit all-swell defaults transaction; B/E | Precise defaults, scope and failure fixtures |
| G19 State/automation compatibility | Brief preservation | Version-7 XML/JSON, no actual parameter registrations | Preserve | Retain schema/IDs; project UI values over existing fields; B | State roundtrip v1-v7, host identity compare |
| G20 Minimum/default/DPI usability | Guide I.4, brief 9 | Three-size snapshots, no documented physical DPI matrix | Partial | Explicit logical layout policy and separate host scale evidence; V/B | Native scaled PNGs plus FL Studio 100/125/150/200% |

## Goals / Non-Goals

**Goals:** Restyle and compose the actual JUCE components around one layout/theme and one edit-binding layer. Keep existing engine contracts and canonical values; make every additional control earn its space through a stated user requirement. Complete a small, purposeful diagnostics cleanup with evidence.

**Non-Goals:** General GUI framework replacement, web mockup implementation, new plugin/parameter identities, cache migration, automatic anticipation on first play, new DSP/IR library content, a host automation architecture, undo history, preset engine, telemetry or plugin-initiated DAW transport. Restyling controls does not authorize removing silent-output or offline-render regression coverage.

## Decisions

### 1. Native component boundaries and centralized presentation

Keep `PreverbAudioProcessorEditor` as owner/coordinator. Extract focused `WorkspaceTheme`/LookAndFeel, `WorkspaceLayout`, reusable `KnobValueControl`, branded header, selected inspector and `DiagnosticsWindow` under `Source/UI`; exact filenames can follow existing conventions during apply. Reuse native ListBox virtualization, FileChooser, Slider/TextEditor and Viewport. Restyle `SwellTimeline`/`SwellOverview`, keeping tested transforms and gestures. Avoid wrapping each row in a heavyweight independent engine/controller.

Baseline tokens: background #0F1418, primary #00EFFF, hover #00C2CC, timing #FFC700, text #F5F7F9/#C7CDD4, separator #4A5058, outline #6B737C. Neutral dry derives from secondary text; translucent range/selection fills are controlled overlays, never a replacement large panel background. Use single 1-logical-pixel separators/control strokes, consistent 4/8/12/16 spacing, 20 outer padding (16 at minimum), 4 control radius and 6 overview radius as proposed implementation metrics. Aqua is selective. Any token deviation must be recorded and reviewed, not silently sampled from raster textures.

Use the supplied `resources/logo/source/InterVariable.ttf` for native control typography where JUCE loading/style rendering verifies correctly; retain `resources/logo/licenses/Inter-OFL.txt` with any packaged font. The logo is already outlined and needs no runtime font. Define roles: headings 17-20 bold, emphasis 14-15 semibold, controls 13-14 medium, secondary 12-13 regular and italic tagline. Validate actual native weight/italic support rather than assuming variable-TTF/WOFF2 compatibility. Installed system Segoe UI (regular/bold/italic verified) is the documented fallback for unavailable faces/styles; record any fallback in visual evidence. Do not add a font downloader or require the logo export script/Inkscape in the plugin build. Package only needed runtime assets/notices, not the entire source/export pack.

Embed `resources/logo/svg/reverbarise-wordmark-dark.svg` through the existing JUCE binary-resource approach and render it as a Drawable, keeping its integrated waveform-A, transparent clear space, viewBox and aspect ratio. Prefer the plain wordmark at plugin-header sizes and render the italic tagline separately for the prototype composition; larger supplied tagline lockups remain available, not required runtime assets. Do not redraw the mark, stretch the A, use the standalone mark as an extra left logo or embed preview.png/prototype imagery. Verify the supplied vector in the actual native header and at supported scales; the former missing-logo refinement task is replaced by integration. Header Settings is real; omit absent undo/redo/presets rather than adding dummy controls. Keep the host wrapper name unchanged despite editor branding.

### 2. Stable layout and responsive policy

Proposed default 1400x1000 logical pixels; retain 1100x850 minimum and existing support through 2000x1400, including 1400x900/1600x1000. This is a coherent native layout choice, not a copy of raster pixel size. Six bands reserve roughly 64 header, 56 globals, flexible workspace, 72 audition, 132 overview and 40 info, with consistent padding/rules. At minimum, budget about 420 logical pixels for the workspace; exact metrics must be asserted during apply rather than squeezed by runtime state.

Left width about 180-220, inspector 320-360, centre receives the remainder (about 510+ at minimum after padding/separators). Column headings align at a fixed top baseline; global controls are full width above all three. The editor owns its own ruler, two useful lanes, footer readouts and zoom/navigation rows; footer text cannot flow into other columns. Narrow states may use two fixed readout rows, never state-driven column movement. At default, prioritise the large central editor over gratuitous blank footer space.

Eleven inspector knob/value cells require six rows, arranged in logical groups; the inspector alone scrolls when necessary. Freeze/Solo/enabled and IR are at its top, Reset remains visibly discoverable in its header/action row. Source/target/placement controls remain in that panel, not hidden in Settings. List scroll remains independent. Menus and diagnostics float above the editor and do not reserve new bands. Empty/mapped/selected states use identical region geometry; placeholders replace content only.

Render in logical coordinates and let JUCE/host scaling handle device pixels. Offscreen captures at 1/1.25/1.5/2 scale are useful but not proof of actual Windows DPI embedding. Do not promise an 1100x850 logical window fits a physically smaller work area at 200%; record monitor/host constraints and scrolling/window usability in the host matrix. Do not silently lower text size to force a screenshot to fit.

### 3. Binding and precision contract

Use stable selected IDs, message-thread snapshot application and a shared binding adapter for canvas/knobs/fields. Pin edit ID and initial values on gesture/focus start; cancel if that ID disappears or selection changes, rather than committing on focus loss into another swell. `updating`/dontSendNotification-style suppression remains necessary. Return commits a validated edit, Escape cancels, focus loss commits only a valid still-owned edit; invalid input restores the previous canonical value with concise reason. Coalesce continuous edit notifications/preparation and avoid duplicate callbacks for the same canonical value. There is no existing undo history; do not display unsupported undo buttons or implement a new general history in this change.

| Inspector control | Canonical binding and units | Important constraint |
| --- | --- | --- |
| Target / Arrival | `moveArrival`, absolute musical position or exact samples | Moves arrival/source-relative definitions under existing rules, unlike wet-body placement. |
| Source Start / Source End | `setSource`, absolute musical position/sample | Arrival remains inside valid source interval; exclusive end; gaps remain unavailable, not zeros. |
| Start / Pre-roll | Display `(arrival - positionedWetStart)/rate`; set inverse through checked crop/placement conversion | Automatic start uses raw start plus placement. Preserve valid signed values for a start after target, not an arbitrary nonnegative cap. |
| End Offset | `(positionedWetEnd-arrival)/rate`, signed seconds/ms | Invert placement before editing logical end; genuine extra support may become pending. |
| Fade In / Fade Out | Integer samples, readable seconds/ms | Existing clamp to interval length minus one; zero/full/overlapping fades keep their established law. |
| Level | `shape.gainDb`, -60..+12 dB | No upward normalization or raw modification. |
| Curve In / Curve Out | `shape.inTension/outTension`, -1..+1 | Preserve independent linear/smooth/exponential presets in Settings. |
| Move Wet | `placement`, signed ms with exact sample alternative | Translate wet PCM and envelope together; keep dry target/source unchanged. |

Absolute sample arithmetic must remain checked int64. A rotary GUI's floating value must not round a large absolute sample through double: use a relative-delta gesture anchored to the exact integer and a separately parsed exact field, or a checked local-range proxy. Range comes from current valid context/support, not an invented whole-song bound. Step/fine adjustment follows existing sample and Shift conventions; numeric entry is not grid rounded. Snap changes canvas policy, not typed precision.

At 48000 Hz, 120 BPM and arrival 96000, 1.00 s pre-roll/+0.35 s end means [48000,112800), 64800 samples/1.35 s, two beats before and 0.7 beat after target. Assert model and pixels, not just strings. Repeat with nonzero placement and negative offset. Unknown beat mapping uses truthful unavailable musical readouts; exact compatible sample controls may remain available.

### 4. Source, wet, handles and overview drawing

Retain the `SwellViewport` sample/pixel transform for both lanes and convert overview events through the same global-bound function used in paint. Introduce explicit named rectangles for ruler/source/wet/footer and overview ruler/content so no hardcoded mouse offset differs from painting.

Both source boundaries become matching thin vertical lines across the source lane, with independently grabbable outlined circles and subtle uniform shading between their exact samples. Use restrained neutral selection overlay, not a large teal panel. Keep coverage distinctions, dry waveform and target readable; a selected range crossing a gap shows missing coverage underneath/through it. Do not equate source start with arrival when they happen to coincide. Preserve deterministic hit priority plus paired exact fields for genuine coincidences; lines/hover labels must identify Source Start, Source End and Target.

Target is narrow dashed yellow across both lanes. Dry is neutral, wet aqua; stale/previous state is labelled honestly. Retain actual peak summaries, disclosure of display-only scale and source-extension context. Do not warp samples or move an energy peak to a drawn target. Preview playhead remains distinct by stroke/label from the host playhead; preserve the selected dry-span indication without introducing extra decorative colour semantics.

Make endpoint/fade/tension glyphs small outlined circles while preserving `EnvelopeHandles` anchors, centre-line endpoints, reverse paint-order hit selection and genuine coincidence offsets. Keep larger invisible hit targets (current approximately 11-pixel radius) and mouse/keyboard feedback. No zoom-driven fan-out; distinct times separate on zoom-in. No speculative independent peak control: the displayed peak is data; every actual existing editable point has its paired inspector control.

Overview has title exactly Overview and a real ruler above its one rounded waveform viewport, not inside an additional enclosing panel. Keep full captured extent and any real wet lead-in visible; identify capture extent separately from display lead-in. Replace generic quadratic swell icons, where they imply shape/timing, with positions/bounds/envelope summaries derived from actual swell state. Retain existing bounded overview caches; do not load whole-song PCM for painting. Unmapped and unavailable are not captured silence. One detail-viewport highlight agrees mathematically with the actual detail range. Navigation changes only editor viewport, never DAW position.

### 5. IR selection, Settings and global controls

Global band has Auto-marker and both typeable sensitivity knobs; Marker sensitivity governs event selectivity, Regeneration sensitivity governs retained-capture variation tolerance. Neither is a mapping-arm switch. Display master-paused/Off honestly and keep stored sensitivity editable as existing specs require.

Selected Swell shows own IR dropdown and adjacent Load file icon/button. Current bundled catalogue is exactly Preverb Hall; no Ethereal Hall/Plate entries are fabricated. Include the current custom assignment as a labelled entry; choose a bundled response independently of a custom future-default IR. `DefaultImpulse::installBundledImpulse` already verifies/writes the actual asset on a worker, but no public per-ID bundled assignment command exists. Add a narrow engine command using that existing preparation/import infrastructure. Pin ID/session/generation at dropdown/file invocation, reject deletion/freeze/supersession, and never apply a delayed chooser result to the then-current selection. Both paths preserve assignment to other swells and future defaults. No new IR DSP/library is required.

Settings gear has scoped sections: Playback (live enable/routing/wet level); Processing (mapping/master automatic policy, future IR, minimum separation); Selected Swell (rename/delete, mute, independent curve presets, precise unit choice/Full response and relevant recovery); Audition (wet/dry isolation, listening tail, monitoring trim/reset); Recovery/Diagnostics. Do not relocate source/target/placement knobs out of the inspector after the user's explicit parity request. Freeze and Solo remain visible, frozen IR/source actions stay gated. Use submenus or bounded native settings content for exact entries, not a new permanent dashboard. Nondefault routing/live-off/trim and critical missing/export warnings remain visible in concise normal-state text; menu placement must not hide an inaudible setting.

### 6. Continuous audition control without a new mixer

Retain model `auditionMode` plus `wetBlend`; project the visible value as 100% for wetOnly, otherwise blend*100. Do not rewrite an old stored wetOnly/latent-50% session merely by opening the editor. For explicit input below 100, call the existing dryWet monitoring path with that percentage; at 100 use genuine wetOnly so no dry dependency/listening tail is manufactured. Preserve latent blend at the wet-only endpoint; new instances retain effective wet-only behaviour unless the user changes it. This intentionally replaces separate mode-button UX, not live routing or the underlying safety law.

Thumb centre is `left + radius + fraction*(width-2*radius)` for its usable track; fill, hit testing and typed percentage share the same normalized value. Test 0/50/65/100 at supported sizes/scales. Play/Stop remain manual preview actions. Capture precedes replay; keep -6 dBFS branch/sum protection, saved trim, short ramps, scope, actual audition cursor and Song playback priority. Settings changes must not start audition or run convolution solely for a monitoring adjustment.

### 7. Defaults, reset scopes and automatic names

Visible split button: main Reset wet shape restores `initialShape(compatibleRate)` and zero placement: automatic raw start, zero fade-in/neutral in curve, 20 ms positive endpoint and 20 ms Smooth fade-out, neutral tensions and 0 dB gain. Use creation's integer rounding/clamping, never the older `resetShape()` assignment of `Shape{}` which loses the gentle release. Preserve source/arrival/name/IR/IDs/activity/freeze and unrelated preferences. Retain valid immutable raw assets and request actual support if needed; no forced unfreeze or invented padding.

Dropdown explicitly labelled Reset all swell settings resets only the selected definition at its current arrival: restore the existing creation-policy source interval from current compatible committed coverage/next arrival, assign the current valid future-default IR, restore wet defaults/placement, enabled=true, mute/solo/frozen=false. Preserve UUID/name/current target and all other definitions/global settings; do not clear capture. Build and validate a candidate transaction first; if coverage/default IR/timing is unavailable, leave the entire prior swell unchanged with a reason. Input changes use existing generation invalidation/background render. This is not an undo operation or a whole-plugin factory reset. Full response remains a distinct Settings action.

New default names become `Swell <one-based Playlist bar>` from actual arrival and fixed musical origin/meter, via a shared checked naming helper at creation. The shared `Core::add` path currently uses collection size and is a focused model change. Do not use wet placement or create speculative bar data when timing is unknown. Keep existing/saved/custom names and names of retained targets through reanalysis/restore/movement; no new persisted auto-name flag is required for creation-only naming. Multiple targets in a bar may share the bar name; secondary arrival text/tooltip distinguishes them, stable UUID remains authoritative. Reset does not rename.

### 8. Diagnostic inventory and disposition

References below use `PE` for PluginEditor.cpp constructor/resized/refresh, `PP` for PluginProcessor, `SE` for SwellEngine. Original purposes are established only where named spike docs/tests support them. No host seek/play/stop probe buttons or implementation were found; do not invent or retire nonexistent controls.

| Item | Current purpose / original spike purpose | Implementation references | Current consumers/dependencies | Disposition | Rationale and verification |
| --- | --- | --- | --- | --- | --- |
| Host position/PPQ/BPM/meter/rate/block | Host timing diagnosis / M0 observability | PE status, PP getHostPositionDiagnostics | spike/transport guides; processor timing tests | Consolidate/rename | Compact Timing read-only group; footer uses same availability flags; unknown/seek tests. |
| Capture status/revision/drops/discontinuities | Commit/queue/timeline health / M0-M1 sparse experiment | PE status, CaptureWorker snapshots | persistence, loop/capture tests, troubleshooting | Retain | Capture/assets group; safe snapshot refresh, no clearing on open. |
| Offline block count/status; live readiness/guard/warnings | Diagnose bounce omissions/cold preparation; newer production work | PE status/helper, EditorSnapshot | MultipleExport/PerformanceExport/LiveProcessor cases | Consolidate/rename | Playback/export group; keep important warnings in normal UI too. |
| Selected IR/source/raw bounds/frame counts/progress | Verify active dependencies / M2 numerical preview | PE detail, SE snapshot | M2/M3/transport validation | Consolidate/rename | Selection/render group bound to stable ID, concise units. |
| Raw/shaped/monitor peaks and guard attenuation | Explain loud IR/monitoring levels / M2 safety | PE detail, SwellMonitoring | audition mix and peak tests | Retain | Separate wet/sum/live protections; no misleading combined output ceiling. |
| Regenerate selected / all | Explicit recovery from captures / M2-M3 manual generation | PE generateButton/allButton, SE generate | engine/UI tests and legacy manual mode | Retain | Explicit recovery group with honest eligibility; no auto invocation on window lifecycle. |
| Suggest onsets, candidate chooser, Confirm | Manual/diagnostic marker acceptance / M3 analysis | PE analyze/confirm/candidateBox, SE analyze | UI analysis fixtures; manual mode spec | Retain | Keep bounded candidate controls in developer recovery; do not reintroduce primary Generate flow. |
| Duplicate diagnostic Sensitivity | Same saved field as main marker knob | PE sensitivitySlider/setPreferences | tests currently find controls by name/value | Consolidate/rename | Remove duplicate writable slider/callback; main value is authoritative, optional diagnostic readout. Update handlers to read canonical session, never a stale hidden slider. |
| Minimum separation | Detection aid, not raw spike | PE separationSlider -> SE setPreferences | onset fixtures and saved preferences | Move to normal UI | Settings Processing; no new main knob required. |
| Automatic processing master and legacy auto-refresh | Manual/auto recovery policy | PE automaticButton/autoButton | automatic/manual migration tests | Consolidate/rename | Settings Processing master, explicitly labelled legacy diagnostic option only where necessary. Retain serialized autoRegenerate compatibility and manual tests. |
| Map while playing | Actual capture preference / M0 mapping experiment | PE mappingButton -> PP setMappingEnabled | capture/migration/automatic tests | Move to normal UI | Settings Processing with visible paused state; marker switch does not replace it. |
| Clear map | Explicit destructive capture reset / M0 map reset | PE clearButton -> PP requestClearCapture | capture Clear/session recovery tests | Retain | Recovery action labelled Clear captured map, preserving existing safeguards/asset retention; no implicit invocation. |
| Recapture/regenerate, Cancel, fresh range status | Exact scoped source refresh / M3 | PE armSelected/recaptureStatus, PP armRecapture | scoped_recapture tests and guide | Retain | Explicit developer recovery, range/instructions/coverage visible, no transport request. |
| Exact sample position toggle | Display-unit preference | PE sampleUnitsButton, MusicalPosition | UI precision fixtures | Move to normal UI | Settings units, panel fields still directly editable; no raw sample flood by default. |
| Source bar start/end and Apply | Convenience source selection, not tone bounds | PE applyBarRange -> setSource | reverse-convolution bar requirement; UI tests | Move to normal UI | Settings source-unit convenience alongside inspector's exact source knobs; no silent whole-bar rounding of canvas edits. |
| Copy asset info | Existing paths for backup/recovery | PE assetInfoButton, SE assetDirectory | persistence/missing-asset manual guides | Retain | Selection/assets group; clipboard only on explicit click, no fictional Pack/Relink/export logs. |
| Scheduled tone enable/start/end | 440 Hz silent-output/phase/loop proof / M0 | PE toneButton/sliders, PP, ScheduledTestTone | scheduled_tone_test, processor state/capture isolation, spike validation | Retain | Test signal group in normal builds; active indication and explicit Off. Preserve tone fields/callback and headless tests. |
| Tone peak | Actual generated-block tone observation | PP getGeneratedTonePeak | processor tone-off regression | Retain | Add existing readout to active test group if useful; no new audio generator. |
| Audition trim and reset | Musician monitoring level | PE auditionSlider/trimResetButton | audition protection/state tests | Move to normal UI | Settings Audition; nonzero trim indication/reset remains in band. |
| Future default IR | Real default assignment | PE defaultIRButton, AutomaticCoordinator | default IR/migration tests | Move to normal UI | Settings Processing; independent of selected IR dropdown. |
| Reset deleted targets | Clear manual-deletion suppression | PE suppressionButton -> SE resetSuppression | protected automatic target tests | Retain | Explicit recovery action with stated effect, not implied by Reset wet/all. |
| Curve presets, Full response | Musical shaping alternatives | PE in/outCurveBox/fullButton | envelope numerical/UI tests | Move to normal UI | Settings Selected Swell; no hidden engine removal. |
| Inline diagnostics toggle/viewport | Current layout mechanism | PE diagnosticsOpen/diagnosticsViewport | current snapshot/layout tests | Retire | Replace with dedicated settings-opened window; remove inline height allocation, polling and obsolete visibility logic after updating tests. |
| Hidden curveInLabel/curveOutLabel, unused fieldBounds | Redundant labels and unused layout helper | PE initialization, setVisible(false), fieldBounds declaration/definition | No calls to fieldBounds found; presets themselves still used | Retire | Remove exclusively unused UI members/helper after final reference check; preserve real dropdown bindings. |
| Preverb title override / duplicate status construction | Historical UI copy and duplicate strings | PE constructor/refresh | screenshots, no durable identity | Consolidate/rename | One branded header and shared status formatter; retain internal plugin names/IDs. |

Dedicated window groups: Timing; Capture/assets; Selection/render; Playback/export; explicit Recovery and Test signal actions. Read-only observations are distinct from state-changing commands. Available through Settings > Diagnostics in Debug and normal Release builds; no global shortcut that accidentally toggles test audio. Editor owns one window with SafePointers/cancelled async callbacks. Closing stops diagnostic timer and destroys content/listeners; reopening constructs current snapshot data, not cached stale text. A visible window refreshes at at most 5 Hz; hidden/closed window performs no periodic work. Closing the editor tears it down before LookAndFeel/model bindings, clears ListBox model and detaches callbacks. It never stops engine workers or toggles test mode just because the window closes; an active tone remains visibly disclosed in the main status with explicit Off access.

### 9. Threading, compatibility and verification

All new layout, bindings, typography and waveform painting stay on message thread. Snapshot reads may use existing engine locking on that thread; never call UI code or perform new allocation/lock/file I/O in the real-time callback. Reuse shared immutable waveform summaries and retain bounded visible-peak work; cache paths by revision/shape/placement/view where useful. Main status/model refresh remains bounded at 10 Hz; an active cursor can repaint only affected regions at a capped 30 Hz using the existing cursor snapshot. Hidden diagnostics must not continue their heavy string construction. Retire UI-owned snapshots on message thread and retain engine's worker ownership policy.

Keep all durable plugin/capture IDs, state tags, cache path namespace, render math and parameter set unchanged. New relative controls and mode projection use existing properties; no schema bump is needed unless implementation establishes a genuine new persistent field, which must then have a reviewed migration and fixture. New names are ordinary saved names. New reset actions are explicit edits, not restore migrations. Preserve old tone/legacy preferences even if a duplicate widget retires. Do not delete PreviewEngine, scheduling or compatibility APIs merely because a UI control moved.

Use meaningful geometry and binding tests plus native visual review, not full-image equality alone. Extend the actual editor harness for fresh/no-capture, mapped/unselected, ready/selected, missing IR/source, pending support, stale/error, long names, 500 rows, near/far zoom and diagnostics open/closed. Assert uniform unoccupied surface colours, fixed region rectangles, outside-overview absence of major boxes, text/value fit and selection clearing. Screenshots must come from JUCE components, not HTML/image generation.

Retain regression oracles for convolution alignment/negative lead-in; source coverage and capture isolation; shape/placement identity; short monitoring ramps and dry isolation; looping/final-source boundary/tolerance; frozen swells; multi-instance offline export including host tail resets; versioned state. Do not claim they pass at planning time or rerun them as evidence for nonexistent implementation.

### Requirement/task/evidence traceability

Task prefixes correspond to `tasks.md`; artifacts below are expected during apply, not created/passed by this proposal.

| Requirements | Task groups | Expected evidence |
| --- | --- | --- |
| G01/G02 stable bands and visual rules | 1, 2, 3, 9 | Native matched PNGs + region/token/border assertions |
| G03 mark/type/icons | 2, 9, 10 | Asset provenance + faithful mark review + native text fit |
| G04 globals; G16 honest automatic state | 3, 4, 8, 9 | Two-knob saved-state/independence tests + fresh-use/state screenshots |
| G05 names/list; G06 IR selection | 4, 6, 8, 9 | Bar/origin/meter fixtures + custom-name preservation + chooser race tests |
| G07-G11 source/handles/relative bindings | 5, 6, 9, 10 | 120 BPM sample/pixel fixture, each control roundtrip, extreme gestures and raw identity |
| G12/G13 preview/live independence | 7, 8, 9, 10 | Slider geometry/audio samples + cursor/isolation/tail/state tests |
| G14/G15 overview/footer | 5, 9, 10 | Sparse coverage/navigation oracle and unknown-host screenshots |
| G17 diagnostics | 8, 9, 10 | Item disposition checklist, lifecycle/no-side-effect fixture, retained tone regression |
| G18 reset scopes | 6, 9, 10 | Direct default values, all-swell transaction/failure and scope tests |
| G19 compatibility | 1, 4-9 | Before/after identities/parameters, legacy/current reopen and audio regression results |
| G20 sizes/scales | 3, 9, 10 | Minimum/default PNG matrix and separately recorded FL Studio DPI/interaction results |

## Risks / Trade-offs

- [Eleven paired controls exceed the prototype's six] -> this is explicitly required; preserve groups/inspector-only scroll and central lane size, not extra global toolbars or shrinking text.
- [Supplied logo rendering or variable-font support differs in native JUCE] -> use the provided path-only dark SVG without redrawing, test weights/scales, retain the OFL notice and document any system-font fallback; do not substitute bitmap mockups.
- [A generic rotary double loses large sample precision] -> anchored integer delta/precise parser and overflow/roundtrip tests.
- [IR chooser resolves after selection/freeze changes] -> pin ID/session/generation and reject obsolete completions; no current-selection lookup at completion.
- [Reset-all changes meaningful source/IR/freeze] -> explicit labelled dropdown action, transactional validation, preserve current target/identity/name and other swells; no partial destructive reset.
- [Moving musical options to Settings hides an inaudible state] -> concise visible nondefault/live/trim/readiness warnings and predictable scoped menu names.
- [Legacy monitoring has latent percentage distinct from effective mode] -> projection without restore rewrite; test old wetOnly and mixed modes separately.
- [Screenshot harness lacks actual host DPI proof] -> preserve unchecked FL Studio tasks; record actual monitor/scale and limitations, not fabricated evidence.
- [Cleanup accidentally weakens spike regression] -> disposition/consumer table, retain tone and persisted fields, remove only exclusive unused UI work after reference checks.

## Migration Plan

Apply in staged UI/model slices against the current archived-spec baseline, recording tasks as each slice is verified. Preserve test fixtures and an existing saved project before changing bindings. First establish native baseline and constraints, then theme/brand/layout, controls/bindings, reset/names/IR commands, audition/overview and diagnostics replacement. Remove old widgets only once their replacement and tests exist. Update current usage/host checklist and leave historical validation evidence intact, clearly labelled as historical.

No automatic asset conversion, cache move or naming migration. Keep the prior VST3 build for rollback; identical plugin IDs and existing version-7 state allow loading saved sessions without installation identity churn. New bar names and explicit reset edits are normal saved values; old binaries may not expose the new UI but must not be forced to reinterpret audio assets. Do not archive until native visual compliance, regression evidence, native header/style acceptance and explicit host acceptance are recorded; unavailable validation stays open.

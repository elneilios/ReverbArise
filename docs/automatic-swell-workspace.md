> Song playback is now being added in `play-swells-with-song-transport`. See [transport playback](transport-swell-playback.md) and [FL Studio checks](fl-studio-transport-playback-validation.md) for current status. Historical references below to deferred transport apply to the earlier workspace change; its host acceptance remains separate.
# Automatic swell workspace

This change extends M3. It prepares swells automatically; Song-synchronized swell
playback remains a separate follow-up. Prepared swells are heard with manual
Audition. FL Studio must continue calling the effect for audition to advance.

## Saved model and defaults

Fresh sessions enable mapping and automatic processing. The default audition mode
is Wet Only, with a stored 50% linear dry/wet blend and neutral 0 dB monitoring
trim. A saved trim such as -12 or -24 dB is never raised automatically.

Plugin state version 4 contains typed session metadata version 2, with no PCM.
It saves mapping/processing preferences, per-swell ownership and detector anchors,
deleted-target suppression, default IR identity and monitoring preferences.
The existing 4096-swell/8 MiB limits remain; suppression also has a 4096-record
limit. Invalid metadata is rejected transactionally.

Automatic processing owns discovery and refresh when enabled. The old optional
Auto-refresh after recapture switch controls the retained manual mode; leaving
that switch off does not disable the new automatic pipeline. Diagnostic manual
suggestions remain unconfirmed until explicitly accepted. This scopes the older
M3 manual-only suggestion/refresh scenarios to the retained manual workflow.

v1/v2 plugin states retain their legacy capture/tone behavior and empty swell
collections, using neutral trim where no trim was saved. M3/v3 sessions retain
saved levels, IRs and edits, migrate existing swells as protected manual edits,
and reopen in manual mapping/processing mode. Enable automatic processing in
Diagnostics when ready to discover new targets. No restoration starts audition.

## Default IR and ownership

Preverb Hall is an original deterministic 1.8-second stereo generated IR embedded
in the binary. [Resource provenance](../resources/README.md) lists its generator,
seed and checksums. Resource checksum/format validation precedes managed import.
Each new swell owns its immutable managed IR reference; changing one assignment
does not affect neighbors. Changing the future default affects only new swells.
An invalid/missing assigned IR is not silently replaced by the bundled default.
External originals may move after successful managed import. Assets remain local
to the existing instance cache; this is not a Pack Project feature.

## Detection and source windows

Only committed dry input drives detection, never tone or audition output.
The current detector uses spectral flux, sensitivity 0.5 and minimum separation
80 ms by default. Its power-of-two FFT window is at least 21 ms (minimum 256,
maximum 8192 samples), with quarter-window hops. Matching tolerance is one window
capped at half the separation. It is an onset suggestion heuristic, not a
musical-note or Playlist-clip detector. The standard host API does not identify
FL Studio Pattern IDs or confirm Song mode for us.

Automatic acceptance preserves matched identities. Editing an automatic swell
protects it from detector movement/deletion. Deletion saves a suppression anchor;
Diagnostics can reset suppression. Changed source can retire unedited automatic
targets only after valid affected-region analysis. Failed scans retain edits.

Automatic source starts at the detected arrival and extends one quarter-note or
to the next known arrival, whichever is earlier. Unlike the manual M3 convenience
default, the source end is not clamped to current coverage. A partially captured
tail remains Waiting for source. Wet pre-roll can precede captured audio and can
be negative at song start; that does not require capturing dry pre-roll.

Capture updates are merged into bounded dirty work and analyzed in cells of at
most eight seconds plus detector history. Real gaps reset history. Incomplete
trailing context is retried; detection and rendering alternate worker turns.
Identical decoded source preserves renders even when storage revisions differ.
Source/IR edits coalesce for 150 ms; fades, shape, gain and monitoring never
recompute convolution. A 256 MiB shared working budget and bounded metadata
remain in force. Failure/backlog states are explicit; audio is never truncated.

Captured audio is *last captured*: edits upstream while stopped cannot be known
until that area is played through the effect again. Invalid timing or sample-rate
changes pause admission instead of moving or reinterpreting saved samples.

## Monitoring gain and alignment

Wet Only plays the committed shaped interval. Dry + Wet plays the union of that
interval and the selected captured source interval, aligned by absolute samples.
The dry attack is not reversed or shifted. Zeros outside an event's support are
intentional absence; missing data inside the dry source is an error, not silence.

The blend is linear: dry gain is `1 - wet`, wet gain is `wet`. At 0%, 50% and 100%
the pre-protection sums are dry only, half of each branch, and wet only. Wet Only
overrides but preserves the mixed percentage. Quiet renders are not boosted.

The wet branch is attenuated only when its peak exceeds -6 dBFS. The prepared sum
receives further attenuation only if it exceeds -6 dBFS, then the saved monitoring
trim applies. At +7.2 dBFS shaped wet, branch attenuation is -13.2 dB and Wet Only
peaks at -6 dBFS with neutral trim. These gains never change IR/render asset PCM.
The ceiling describes the audition contribution; live input plus tone plus
audition can exceed it. Diagnostics identifies each gain stage separately.

Missing dry disables Dry + Wet. Independently verified cached wet can be heard
only through explicitly labeled last-committed Wet Only recovery; it is not
presented as a current Ready render. Corrupt wet, incompatible rate or unverified
IR cannot use that recovery path. Reopen never starts either branch.

## Workspace controls

The left list shows each swell's own IR and processing state. Click its checkbox
to enable/disable it; select a row to bind the central handles and right inspector
to that stable ID. The main inspector has editable name/arrival, per-swell Load IR,
fade knobs plus typed milliseconds and curve selectors, gain, Full response and
Reset shape. **Source and audible timing...** reveals precise extraction and crop
bounds without filling the normal view with engineering fields. Multi-selection
is retained, but inspector Load IR assigns only the primary selected swell.

Musical positions use `bar.beat` or `bar.beat.fraction`, with up to nine digits of
fractional beat. `1.1` is song start, `1.2` the second beat, `1.1.5` half a beat
after the start. Negative lead-in can precede bar 1. Conversion rounds once to an
integer sample; exact sample-entry mode remains in Diagnostics. Fade values are
milliseconds. Snap affects timeline dragging, not precise typed entries.

The full-width Song Overview shows real sparse coverage, targets and the teal
detail viewport. Click/drag to navigate; navigation does not alter audio inputs.
The detail lanes share a bar ruler, show muted raw wet context plus selected
shaped wet, and disclose display-only peak scaling. The envelope overlay follows
the existing fade curves; it adds no Peak Position parameter or new DSP.

The audition strip provides selected Audition/Stop, Wet Only, Dry + Wet and a
typeable percentage. A saved nonzero trim is visible there and clicking it resets
to 0 dB. Full trim adjustment and peak diagnostics remain in Diagnostics.

Diagnostics is a scrollable foldout containing manual Suggest/Confirm,
Regenerate selected/all, scoped recapture/cancel, mapping/processing switches,
future-default IR, suppression reset, exact sample mode, raw host/asset details
and the 440 Hz test. These tools remain available for recovery. At small sizes
the inspector also scrolls. Supported size range is 1100x850 through 2000x1400;
default is 1400x900. Undo, presets, host transport, EQ and other unsupported
wireframe controls are omitted.

| State | Meaning |
|---|---|
| Waiting for source | Required extraction tail or gap has not been committed |
| Queued / Checking | Background dependency verification is pending |
| Rendering | Source/IR preparation or convolution is in progress |
| Ready | Complete verified render is committed |
| Stale / updating | Inputs changed; old audio is not current |
| Missing IR / Failed | Actionable input, resource or storage error; edits remain |
| Missing source, last-committed Wet Only | Explicit recovery of verified cached wet |

## Verification status

On 2026-11-01 the Windows MSVC Debug VST3 and all test targets built successfully.
The complete CTest suite passed **22/22** in **25.37 seconds**; targeted UI and
processor checks also passed after layout refinements. Strict validation passed
for both active changes and both main specs. Main specifications were not synced
or edited. M3 and this successor remain unarchived pending actual host acceptance.

Automated coverage includes:

- Session/resource: typed state, v1/v2/v3 migration, default-on fresh policy,
  protected old edits, suppression limits, embedded IR checksum/decoding,
  resampling, independent managed imports and corrupt-resource rejection.
  Concurrent directory publication is covered by 20 fresh-cache trials with four
  capture/IR writers each; existing directories created by another worker are
  accepted, while real storage failures remain explicit. Fresh UI startup also
  asserts successful default-IR initialization.
- Automatic coordinator: repeated/shifted/tied detections, whole versus split
  capture, real source-tail waiting/completion, stable ownership/deletion,
  unchanged-source reuse, rapid IR/source coalescing, changed recapture, 600 dirty
  notifications, bounded draining and default-store failure. The race regression
  passed three consecutive runs after pairing dirty ownership with a fresh map.
- Monitoring/processor: independent sample oracle for 0/50/100% linear blend,
  negative pre-roll/dry tails, wet/sum guards, 35% mode memory, cursor-preserving
  ramped replacement, saved trim/mode, missing-dry cached-wet recovery, dry input
  passthrough, allocation/deallocation guards and dry/wet feedback exclusion.
- UI: software renders and interactions at 1100x850, 1400x900 and 1600x1000,
  Diagnostics open/closed and scrolled, precise fields/handles, real waiting and
  updating states, stale/error/empty states, hundreds of rows, overview navigation,
  monitoring controls and visible trim reset. Reviewed PNGs are in `out/lean-ui`.
- Existing capture, immutable persistence, WAV/WavPack, reverse-convolution,
  scoped-recapture, timeline/ruler and diagnostic-tone regressions remain passing.

The full build is `cmake --build --preset windows-msvc-debug`; tests are
`ctest --preset windows-msvc-debug`. Set `PREVERB_UI_M3_OUTPUT` to an output folder
to retain offscreen editor PNGs. Host acceptance follows the separate
[FL Studio guide](fl-studio-automatic-workspace-validation.md); no host audibility,
user UI signoff or deferred transport scheduling result is inferred from CTest.

Implementation and acceptance are tracked in
[the change checklist](../openspec/changes/automatic-lean-swell-workspace/tasks.md).
Completed checkboxes require their listed verification; FL Studio checks remain
separate from headless tests. M3 acceptance remains open until explicitly recorded.


## Transition feedback revision (2026-11-01)

The first-feedback session JSON was version 3 inside the version-4 plugin wrapper; the second-feedback revision below advances session JSON to version 4. Versions 1/2 migrate with neutral per-fade tension (0), a two-second listening tail, and all saved timing, fade presets/durations, names, IR references, ownership, suppression, selection and monitoring trim retained. Reopening never starts audition. Existing raw assets retain their engine/cache format and are reused when their genuine input range and fingerprint match.

Attack policy 2 keeps Sensitivity 0.50 and minimum separation 80 ms in Diagnostics; the new energy confirmation makes acceptance less aggressive without overwriting saved preferences. Positive spectral flux must coincide with short-time energy growth against four preceding hops, a decaying energy peak/floor, and rearming hysteresis. Confirmation can look ahead within the FFT window, but reported positions remain the original flux-peak positions. The FFT is the next power of two at or above 21 ms (minimum 256), with quarter-window hops. Automatic scans include 100 windows of preceding context; gaps reset evidence. Cache identity includes policy version and both detector preferences. Changing those preferences reanalyzes covered material while preserving manually edited targets and deletion suppression.

Deterministic fixtures reproduce 15 false markers for one modulated decaying pluck in the former detector. Revised tests require exactly one for that pluck, a noisy decay, a 12.5 ms soft attack and a sustained modulated note with release, plus three distinct-pitch notes 200 ms apart. At 48 kHz measured initial fixtures were at 4864/5120 samples for an attack at 4800, and 4864/14336/24064 for attacks at 4800/14400/24000. Tests also cover 44.1/48/96 kHz, sparse gaps, split commits, cancellation, incremental automatic reconciliation and minimum spacing. These are onset heuristics, not note transcription: phase-cancelling overlapping tones, extremely slow swells, tiny attacks over a loud sustained bed, or attacks below configured separation may merge. Raising sensitivity does not disable attack confirmation.

Wet start/end square handles, fade-duration circles and tension diamonds lie on the actual envelope; painting and hit testing share geometry with 22-pixel targets. Hover identifies each point, nearest-point ties give endpoints deterministic priority, and zero-duration fades omit their redundant duration/tension points (use the precise fade value to open a segment). Drag diamonds vertically for continuous tension; each inspector tension value is typeable from -1 to +1. Shift-drag gives one-tenth movement. Envelope gestures ignore Song Snap; source/arrival edits still honor it. Arrow keys nudge the focused handle, and the inspector provides exact values. Accepted metadata paints immediately without preparing audio on the UI thread. Viewport and target identity remain stable during shaping.

Tension warps u in [0,1] by expm1(6*t*u)/expm1(6*t), with the exact identity at neutral tension, before applying Linear/Smooth/Exponential. Fade segments remain monotone and retain exact endpoints; overlapping fades multiply. All shaping is nondestructive and leaves the committed raw render identity unchanged.

Wet end is exclusive and independent of the original dry arrival. Drag it before the arrival for a gap or beyond it for overlap. Fade-out remains a separate duration, clamped only when the entire audible interval becomes shorter. The inspector reports gap/overlap in milliseconds. No dry fade, ducking or timing change is introduced. Monitoring blend, trim and peak guards remain separate controls.

If wet end exceeds raw support, the requested endpoint remains visible while the worker prepares genuine extended input [base start, max(base end, wet end)). The base extraction fields remain unchanged. The inspector discloses extra captured input and the source lane highlights its extent in amber; later notes inside that input can contribute to the wet convolution. Missing coverage reports Waiting for source; capture through the requested endpoint to continue. No zeros, stretching or old endpoint clamp substitute for that audio. The original N+M-1 convolution and alignment are unchanged. Extended input participates in the content fingerprint, so editing it invalidates the render. Shortening the wet end retains already prepared extended support for inexpensive shaping; changing the base extraction resets that retained support. The 256 MiB working budget still applies; failures retain requested edits with an explicit status.

Captured Dry + Wet now reads [base source start,D), where D=max(base source end,max(dry arrival,wet end)+tail). The typeable Listen after preference ranges from 0 to 5 seconds and defaults to 2. Original stereo dry samples and song positions are unchanged, including through a wet fade or intentional gap. Wet Only uses only wet bounds and adds no dry tail. Missing any required context disables Dry + Wet and displays the exact needed sample interval; capture that range or select verified Wet Only. Newly committed tail coverage retries monitoring without reconvolving. Changing only tail, blend or trim does not rewrite assets or rerun FFT. A selected source longer than the listening tail is never truncated. Normal live dry passthrough and future transport scheduling are unchanged.

New regression target: preverb_transition_feedback_test checks exact extended convolution samples/length/alignment, retained base bounds and fade, missing extension then capture recovery, extension-only dependency changes, unchanged stereo dry samples through the full tail, tail-only no-FFT edits and saved transition preferences. Updated software UI images are written to out/transition-ui. FL Studio acceptance for this revision remains pending.


### Feedback revision verification evidence

This is new verification, separate from the historical 22-test baseline above. On 2026-11-01 the complete Windows MSVC Debug build, including the VST3 bundle, succeeded. All **23/23 CTest targets passed in 28.55 seconds** with software UI snapshots enabled. `openspec validate --all --strict` passed all four active change/main-spec items. No FL Studio acceptance is inferred from these results.

| Added scenario coverage | Automated evidence |
| --- | --- |
| One decaying event; sustained modulation/release/noisy tail; genuine repeated/soft attacks | swell_analysis_test: 1/1/1/3/1 targets, independent expected attack positions, 44.1/48/96 kHz, sparse-gap fixture |
| Capture boundary and policy/preference reanalysis; unchanged repeat; manual ownership/deletion suppression | swell_analysis_test split commits/cancellation; automatic_coordinator_test whole/incremental convergence, no-work repeat, preference cache invalidation; automatic_targets_test protected reconciliation |
| Grab a visible fade; zero/co-located controls; precise input and stable viewport | swell_ui_test shared geometry/hit assertions, zero-fade endpoints, unsnapped endpoint at 2003 samples, immediate inspector update, keyboard one-sample nudge, no FFT/raw identity change |
| Bend each fade; restore neutral legacy curves | swell_shape_test independent exp/monotonicity/endpoint oracle across all presets and tensions; swell_session_test typed version-1/2/3 migration, preservation and malformed fields |
| Intentional gap; independent overlap/fade duration; unchanged dry | transition_feedback_test gap and overlap stereo sample oracles through the listening context; swell_ui_test endpoint/fade/tension controls and fixed arrival; swell_session_test independent durations/clamping |
| Endpoint fits existing raw support | transition_feedback_test shorter endpoint retains committed checksum and render count; swell_ui_test shape/keyboard/tension edits leave raw assets unchanged |
| Endpoint requires more support; source extension missing; cancellation | transition_feedback_test complete N+M-1 sample/alignment oracle, base selection retained, missing extension waits, returning inside old support cancels request; swell_ui_test amber extension, requested end and explicit Fit in pending-support screenshot |
| Extended dependency changes and obsolete work isolation | transition_feedback_test changed sample outside base interval refreshes raw dependency; automatic_coordinator_test coalescing/latest generation and unrelated assets; swell_engine_test cancellation/deletion/restore/resource pressure |
| Saved transition/support intent and reusable assets | swell_session_test typed gap/overlap/tension/support/tail round trips and overflow/range rejection; transition_feedback_test restore reuses exact committed raw with no autoplay; existing engine/processor missing source/IR/render recovery cases |
| Wet pre-roll, original dry placement, longer base context | transition_feedback_test absolute stereo dry oracle includes uncaptured pre-source wet span, gap and overlap tails; swell_session_test longer source dominates tail formula; audition_mix_test exact union/sample order |
| Wet Only, blends 0/50/100%, trim and protection separation | audition_mix_test independent branch/sum sample and loudness oracles; transition_feedback_test Wet Only remains ready without context; processor/voice tests allocation, feedback, mode changes and safe buffer handoff |
| Missing post-roll and saved monitoring preferences | transition_feedback_test needed-range error, retained tail, newly captured context retries with no FFT, saved tail/tension restoration; swell_ui_test typeable tail; processor_test state migration/no autoplay |
| Realistic editor sizes and Diagnostics | swell_ui_test generates envelope-1100/1400/1600.png and envelope-diagnostics-1100/1400/1600.png under out/transition-ui; all six inspected, with scrollable inspector/Diagnostics at smaller sizes |

The final VST3 binary is `out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3/Contents/x86_64-win/Preverb Spike.vst3` (32,148,992 bytes), SHA-256 `475e73566529f126214a62c47d163da4b06b8da37ea0fba753ab84dcb3b22868`. Use the whole .vst3 bundle including Contents/Resources. The final CTest log is under out/build/windows-msvc/Testing/Temporary/LastTest.log.

Only host acceptance remains: original checks 8.1–8.5 plus feedback checks 13.2–13.3. Follow docs/fl-studio-automatic-workspace-validation.md; transport-synchronized swell playback remains deferred.


## Second feedback revision: movement, listening and marker controls

Session JSON version 4 now saves signed wet placement, Isolate selected (default off) and Auto markers (default on), retaining the existing version-4 plugin wrapper. Versions 1-3 migrate with zero placement and the new preference defaults while preserving saved master/mapping, sensitivity, envelopes, IRs, suppression and trim. A legacy manual master remains off. Reopen never starts audition. The earlier 23-test verification belongs to the first feedback revision; updated results are recorded separately below.

The main toolbar exposes one Marker sensitivity rotary knob with a typeable percentage and adjacent Auto markers switch. These are per-instance settings independent of selected swell. Lower values favour stronger attacks. At 48 kHz, the deterministic busy fixture measured low/default/high counts of 3/4/10 at 0/50/100%, retaining all three prominent fixture attacks at low. Detector policy 3 compares attack energy with a preceding-window decaying peak; selectivity changes this ratio plus the existing rise/flux thresholds, without increasing the 80 ms default separation. These are onset heuristics, not key/chord recognition or a guarantee of transcription on arbitrary dense audio. Existing isolated/modulated/noisy/soft/repeated/gap tests remain required. Save explicit sensitivity values rather than resetting them on upgrade.

Auto markers Off cancels/suppresses automatic target reconciliation, including superseded jobs, and retains targets and deleted-marker suppression. Capture, manual Add/Delete/Suggest/Confirm and the existing automatic render-refresh policy continue. An explicitly disabled automatic-processing master still prevents marking; the main control shows Master paused rather than silently enabling it. Sensitivity is editable/saved while marking is off. Re-enable catches up from committed coverage with the latest value. Knob edits debounce analysis for 200 ms; no PCM scan runs on the UI/audio thread. Complete valid reanalysis may remove unedited automatic targets, but not protected user edits. Changed settings/enable generations invalidate obsolete completions.

Zero fade-in/out durations now have their own grabbable circles. When points collide, labelled grab targets fan out with leaders to actual curve anchors; square endpoints remain separately accessible. Fade dragging uses the starting duration plus pointer delta, so a displaced target does not introduce a jump. Tension diamonds appear only for nonzero fades. Precise values and keyboard alternatives remain available. Full source handles and dry arrival remain separate.

Drag the selected wet body (away from its precise handles) to move wet audio and the entire envelope together. Move wet (ms) and one-sample arrows in the inspector offer precise placement. Length, raw sample order, crop, fades, presets, tensions and gain are preserved; the dry arrival, base extraction and detector anchor stay fixed. This is a saved signed offset, not stretching or re-extracting source. Movement alone leaves raw checksum and convolution count unchanged, including negative-time moves. Raw summaries and envelope points move immediately, while prepared playback updates on the worker. Movement keeps the existing fine/snap-independent envelope policy; Snap still governs source/arrival edits and does not start transport-synchronized playback.

A positioned crop [A,B) maps back to logical raw coordinates by subtracting placement L. Only an independent endpoint extension beyond genuine positioned support can require input through max(base end,B-L); pure movement changes B and L together and needs no new input. Existing extension disclosure/waiting/cancellation remain. Invalid arithmetic is rejected transactionally, preserving relative spacing.

New manual/automatic swells begin with a 20 ms overlap after dry arrival and a 20 ms Smooth fade-out at neutral tension. Existing saved shapes, including explicit zero fades, survive restore, reanalysis and regeneration. Reset shape retains its documented neutral crop action; it does not reinterpret a saved fade on reopen. Extra support for the new endpoint still requires genuine captured input.

Audition defaults to eligible overlapping wet inside the selected anchor's listening window. Wet Only uses that anchor's positioned wet bounds. Dry + Wet uses its existing captured-dry context/tail union. Neighbours are clipped to this fixed window, so overlap chains do not expand into the whole song. Enabled/mute/global-solo gates apply; muting the anchor does not mute eligible neighbours. Isolate selected is saved and independent of mode/blend/tail/trim. The strip shows scope and prepared wet-member count. Required missing/stale/corrupt/incompatible members block the group with an identifying diagnostic; excluded members do not block it, and explicit isolation can recover a separately verified selection.

Members retain their shaped relative gain; sum their aligned wet first, then apply aggregate wet peak protection (-6 dBFS), linear wet/dry blend, combined-sum protection and saved trim. Captured dry is read and mixed once, never per swell. Monitoring hydrates one needed member at a time under the shared 256 MiB budget; scope changes reuse raw assets. Member/generation/source changes invalidate the prepared group. Selection changes stop playback and preparation never autoplays.

Monitoring includes disclosed 2 ms wet-edge safety ramps and 5 ms outer audition start/end ramps, shortened for tiny intervals with zero first/last samples. The wet guard avoids an internal wet cut while dry context continues; the outer ramp affects only replay at the listening-window boundaries. Live dry, captured PCM and immutable renders remain unchanged. These guards are separate from editable musical fades and preserve interval length. Tests compare original dry interior exactly and account separately for monitoring boundaries. Stop/replay and replacement retain cursor/ownership safety; unsafe rate/dependency invalidation can still silence immediately. The callback remains allocation/lock/file-I/O free.

Updated offscreen images are under out/second-feedback-ui, including the zero-fade, envelope/moved-body and pending-support views at 1100x850, 1400x900 and 1600x1000 with Diagnostics open/closed. Small-size inspector/diagnostic panes scroll; main marker and audition controls remain directly available. FL Studio acceptance remains open, including new tasks 18.3 and 19.6.


### Second feedback verification evidence (2026-11-01)

The complete Windows MSVC Debug build, including the VST3 bundle, succeeded. The expanded existing **23/23 CTest targets passed in 37.40 seconds**, with the software UI snapshots enabled. Strict validation passed all four active change/main-spec items. This is new evidence for the second feedback implementation, not a claim that the earlier 23-test pass covered it. No FL Studio host acceptance is inferred.

| New scenarios/tasks | Automated evidence |
| --- | --- |
| Zero/initial fades and zoom collisions; no initial drag jump | swell_ui_test checks shared hit/paint targets, four separately reachable coincident zero-fade controls and direct opening of both zero fades with fixed bounds. The six zero-fades[-diagnostics]-1100/1400/1600 images were inspected after label-spacing fixes. |
| Whole body and precise signed placement; fixed shape/dry/source | swell_ui_test body drag, immediate bound/inspector feedback and numeric return to zero; swell_shape_test identical shaped samples with translated start; swell_session_test signed arithmetic, migration and overflow rejection. |
| Pure translation cache reuse; inverse endpoint extension; persistence | WorkspaceFeedbackCases.h inside transition_feedback_test checks negative placement with unchanged raw checksum/count, later logical extension to source end 500 with placement -1000 and positioned end -500, preserved base end 350, reuse on restore and no autoplay. automatic_coordinator_test protects moved automatic targets/anchors against recapture. |
| Boundary clicks, zero user fades, tiny buffers and Stop/replay | audition_voice_test reproduced old nonzero start/end (0.25/0.25), then verifies zero boundaries, exact ramp oracle for 1/2/3/4/9/40 frames, Stop/replay/replacement, cursor continuity, allocation/deallocation guards and retirement. audition_mix_test independently checks wet safety and protected branch/sum samples. |
| Gentle new defaults and old edits | swell_session_test confirms 20 ms creation defaults and v1/v2/v3/v4 migration, retained fades/tensions and new preferences; automatic target/coordinator/processor fixtures exercise automatic creation with the shared defaults. Existing saved zero fades and raw assets remain unchanged. |
| Overlapping listening window, isolation, boundary touch and no overlap chain | WorkspaceFeedbackCases.h verifies two Wet Only members, excludes the third chain-only member, includes the three actually intersecting Dry + Wet context members, clips to the anchor window, tests exact boundary exclusion and rapid membership edits. |
| One dry branch, linear blend and relative wet levels | The group sample oracle checks full positioned wet sums and one original stereo dry context at 0/50/100%, with unchanged live passthrough baselines; audition_mix_test checks loud correlated branch/sum ceilings without asset normalization. |
| Required/excluded bad members and mute/solo | Group fixtures check stale/missing and mixed-rate required-member diagnostics, explicit isolation recovery, muted anchor with audible neighbour, global solo excluding all wet and bounded resident memory. Engine/processor tests retain verified cached-wet, missing IR/render/source, and larger residency-pressure cases. |
| Latest-generation group/monitor work and realtime safety | Rapid placement/membership changes cannot publish the old group. Included-member/captured-context changes invalidate monitoring; group decoding is sequential and bounded. Voice/processor tests verify no callback allocation/free and no feedback into dry capture. |
| Prominent knob, separate switch and saved values | swell_ui_test exercises named main controls and typeable sensitivity, shared saved state, selected-row independence and scope toggle across all layouts. Session/processor tests cover new defaults, explicit Off, malformed types, old master-off migration and no autoplay. |
| Busy selectivity and preserved meaningful attacks | swell_analysis_test measures 3/4/10 markers at 0/50/100% with all three strong attacks present at low and unchanged separation; isolated, noisy/modulated, sustained, soft/repeated, gap and split-commit cases remain passing. |
| Marker Off, debounce and catch-up | automatic_coordinator_test covers repeated setting/Off/On generations, no late publication after Off, saved sensitivity while off, latest catch-up, stable IDs/manual ownership/suppression and unchanged-source reuse. AutomaticProcessorCases starts with marking off while mapped musical capture proceeds, verifies no automatic targets, then enables catch-up without a new pass. |

The tested binary is `out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3/Contents/x86_64-win/Preverb Spike.vst3` (32,218,624 bytes), SHA-256 `1fe46a11528e9cd550f6cfaccd5a3e1e69ef7e57000093780db1028fdd464dcc`. Load/copy the complete .vst3 bundle, including Resources. The final CTest log is out/build/windows-msvc/Testing/Temporary/LastTest.log; UI artifacts are under out/second-feedback-ui.

Implementation tracking is 80/89 after verified second-feedback tasks are checked. The nine remaining tasks are host-only: 8.1-8.5, 13.2-13.3, 18.3 and 19.6. Use the separate host guide and record actual FL Studio results. M3 signoff and both archive workflows remain separate; transport-synchronized swell playback is still deferred.

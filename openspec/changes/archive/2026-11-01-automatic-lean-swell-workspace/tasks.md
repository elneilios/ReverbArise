# Tasks

Update each checkbox only after its implementation and stated verification pass. Keep FL Studio acceptance incomplete until actually performed; this checklist does not sign off or archive M3.

Tasks 1–7 retain the completed automatic-workspace baseline and its historical verification. The approved FL Studio feedback revision is tracked separately in tasks 9–13 below; none of that new work is implemented or validated by the existing checkmarks. The user's confirmation of automatic generation does not imply that all tasks in group 8 passed. Dry transition shaping is explicitly excluded: endpoint/fade/tension controls affect wet only.

## 1. Session Policy and Default IR

- [x] 1.1 Extend the canonical model with target provenance/anchors, suppression, mapping/automation preferences, default IR reference and monitoring mode/blend; verify validation, stable identity, defensive limits and transactional malformed-state rejection in session tests.
- [x] 1.2 Add versioned encoding and v1/v2/v3 migration fixtures; verify protected legacy swells, preserved stored trim, neutral absent trim, manual-mode migration and new-session default-on policy without automatic audition.
- [x] 1.3 Add a deterministic generated stereo default IR, reproducible generator/provenance and embedded build resource; verify decoded checksum, nonempty finite decay, supported-rate preparation and independent managed imports without distributing user IRs.
- [x] 1.4 Connect future-default inheritance and explicit missing-default/relink handling; verify selected IR replacement affects only intended swells, default changes affect future swells only, and missing assigned IR is never silently substituted.
- [x] 1.5 Document defaults, ownership, migration and IR provenance in the feature guide; register model/resource tests and verify examples against saved fixtures and built resources.

## 2. Progressive Capture and Incremental Analysis

- [x] 2.1 Enable mapping for fresh sessions with valid playing host position and preserve saved policy after restoration; verify stopped/invalid-position/rate-mismatch blocks do not map and capture remains strictly before tone/audition output.
- [x] 2.2 Publish committed dirty intervals to bounded worker coordination with overlap merging and retained retry summaries; verify queue pressure, cancellation, capture Clear/restore and disjoint bars 86–90 then 1–4 without fabricated gap coverage.
- [x] 2.3 Implement incremental context-aware detection with trailing-boundary retries and gap resets; verify commits splitting a transient, silence, dense signals, stop with partial context and later context completion against whole-run fixtures.
- [x] 2.4 Use exact source fingerprints to bypass unchanged expensive work; verify sample-identical recapture across different chunk boundaries preserves detector acceptance, IDs and render counts while changed overlap dirties only affected intervals.
- [x] 2.5 Register incremental capture/analysis integration tests and document context, latency, source limits and last-captured semantics; verify analysis never runs in the callback or requires a UI timer to initiate it.

## 3. Stable Automatic Targets and Source Readiness

- [x] 3.1 Implement deterministic one-to-one candidate reconciliation with the documented tolerance and stable anchors; verify repeated passes, shifted candidates, ties, adjacent commit boundaries and selected-ID stability without duplicates.
- [x] 3.2 Protect manual-created/edited targets and persist deletion suppression/reset; verify moved/renamed/shaped/IR-edited targets survive reanalysis, deleted targets stay absent after reopen, and suppression limits fail explicitly without losing edits.
- [x] 3.3 Retire unsupported unedited automatic targets only after complete valid affected-region analysis; verify incomplete, failed or superseded scans cannot delete targets or publish stale jobs and unrelated definitions remain unchanged.
- [x] 3.4 Create full quarter-note/next-arrival automatic source windows without coverage clamping; verify Waiting for source across incomplete tails/gaps, later readiness, manual source protection and negative wet lead-in from a complete source at zero.
- [x] 3.5 Register reconciliation/readiness tests and document automatic versus manual source defaults, provenance and deletion recovery; verify documented boundary examples with deterministic fixtures.

## 4. Automatic Rendering and Honest State

- [x] 4.1 Dispatch complete ready dependencies automatically through existing render jobs; verify a headless fresh instance reaches real committed Ready swells from musical input with no Analyze/Generate commands and no automatic wet output.
- [x] 4.2 Automatically coalesce IR/source/arrival edits and changed recapture per swell; verify latest revision wins, selected/unrelated isolation, identical-source reuse and no obsolete commit after deletion, Clear or restore.
- [x] 4.3 Keep shape/curve/gain preparation and activity updates separate from convolution; verify model and playback changes agree while raw render count/checksum remain unchanged for non-render edits.
- [x] 4.4 Add fair bounded analysis/render scheduling and account for waveform, blend, pinned and scratch memory under the existing budget; stress continuous capture, many targets and pressure, verifying rendering progresses and queued work drains after input stops without truncation.
- [x] 4.5 Expose complete waiting/queued/rendering/ready/stale/missing-IR/failure state and manual recovery through snapshots; verify failure retains edits and valid prior references without describing stale audio as current.
- [x] 4.6 Register coordinator tests and document state transitions, preference switches, debounce and resource behavior; verify disabled automatic processing leaves manual recovery usable and no timer continuously re-renders all swells.

## 5. Aligned Audition and Gain

- [x] 5.1 Prepare captured dry and shaped wet branches off-thread on their absolute interval union; verify negative wet start, dry tail beyond wet end, stereo order, exact first/last frames and support zeros against an independent sample oracle.
- [x] 5.2 Add linear Dry + Wet blend, Wet Only override and saved mixed percentage; verify 0/50/100%, 35% mode round trip, continuous cursor with ramped updates and zero convolution or raw/shaped asset writes.
- [x] 5.3 Set absent trim to 0 dB, retain wet branch protection and add prepared-sum peak guarding; verify +7.2 dBFS wet becomes -6 dBFS at neutral trim, quiet audio is not boosted, hot dry/blends remain bounded and saved -24/-12 dB values remain intact.
- [x] 5.4 Handle unavailable dry with explicit disabled Dry + Wet and independently verified last-committed Wet Only recovery; verify missing source is not Ready/current, gaps inside required dry never become silence, and corrupt wet/IR or incompatible rate cannot bypass validation.
- [x] 5.5 Extend pinned voice handoff/retirement, Stop and selection/dependency cancellation; verify no callback allocations/deallocations, blocking or file reads, no old-selection leakage and no capture of either audition branch.
- [x] 5.6 Register monitoring/processor tests and document linear law, interval support, gain stages, saved preferences and stopped-host limitations; verify reopen restores settings without playback and normal dry passthrough is unchanged.

## 6. Lean Workspace

- [x] 6.1 Refactor layout into actual-state header, virtualized IR-labeled swell list, central editor, right inspector, audition strip and lower overview; verify empty/waiting/rendering/ready/stale/error states and selection binding through asynchronous updates in offscreen UI tests.
- [x] 6.2 Bind musical timing fields and supported scalar knobs with typeable values to canonical edits; verify bidirectional handles, exact off-grid sample round trips, invalid entries, fixed timing limitations and source controls reflecting the selected interval.
- [x] 6.3 Present visible source/wet summaries, arrivals, selected handles and shared bar ruler; verify unflattened high-peak envelopes, disclosed display scaling, other-swell context and exact alignment without loading all stored PCM.
- [x] 6.4 Move tone/raw timing/peaks, analysis, forced generation, scoped recapture, preferences and asset recovery into a collapsible Diagnostics area; verify every retained tool works and fresh automatic capture/edit/audition needs no diagnostic button.
- [x] 6.5 Add selected manual Play/Stop, mode, percentage and visible nonzero trim/reset controls; verify availability, saved blend, missing-dry message, no fake DAW transport controls and explicit distinction from deferred Song playback.
- [x] 6.6 Integrate overview click/drag, pan/zoom/snap and list navigation; verify distant sparse regions, unchanged render dependencies during navigation, hundreds of rows and preserved selected identity.
- [x] 6.7 Render and inspect representative states at 1100x850, default and 1600x1000 with Diagnostics open/closed; fix clipping/overlap and verify documented supported size limits, keyboard/numeric access and scroll reachability in UI tests.
- [x] 6.8 Update user editing/automatic-processing documentation and add the FL Studio validation guide, mapping handoff acceptance to delivered versus deferred behavior; verify instructions match controls and omit unsupported wireframe features.

## 7. Persistence and Integrated Verification

- [x] 7.1 Complete processor save/restore for automation/provenance/suppression/default IR and monitoring preferences; verify capture, edits, independent IRs, selection and settings round trips, saved snapshots after Clear, fresh timing gates and no startup audition or stale jobs.
- [x] 7.2 Exercise missing/corrupt managed source/IR/render assets, default-resource failure, rebuild and manual-mode migration; verify explicit per-swell diagnostics, unaffected valid swells, immutable asset preservation and documented recovery paths.
- [x] 7.3 Build Windows MSVC Debug VST3 and run all CTest targets; verify existing M1/M2/M3 capture/convolution/WavPack regressions, new automatic/blend/UI integration, callback safety and bundled resource presence, recording actual results.
- [x] 7.4 Validate this change strictly and review successor behavior against active M3 deltas; verify complete scenario coverage, no accidental main-spec edits and an explicit M3-first sync/archive order in the guide.

## 8. FL Studio Acceptance — Leave Open Until Performed

- [x] 8.1 In a fresh instance with Diagnostics closed, play musical input in Song mode without plugin actions; verify positioned capture, automatic targets/default-IR renders, complete-source waiting and audible selected Wet Only at neutral trim, recording host/audio settings and observed results.
- [x] 8.2 Capture bars 86–90 then 1–4, replay unchanged and changed source, manually edit/delete targets and use distinct WAV/WavPack IRs; verify sparse gaps, stable IDs/suppression/selection, independent automatic refresh, no needless renders and responsive continuous capture.
- [x] 8.3 With host stopped where processing is supported, audition selected Wet Only and 0/50/100% Dry + Wet; verify alignment, usable protected levels, nonzero saved-trim indication/reset, mode memory, selection stopping and missing-dry behavior without capture feedback.
- [x] 8.4 Exercise the main layout and Diagnostics at supported sizes, precise typed/drag edits, overview navigation, many swells and manual recovery; verify usability with no main Generate/Suggest/Regenerate or raw sample-address clutter and record user UI acceptance.
- [x] 8.5 Save/reopen an edited automatic session and a legacy M3 fixture; verify identities/edits/IRs/suppression/preferences, changed or missing assets, compatible timing gates and no auto-audition; record remaining M3 acceptance separately without assuming all earlier checks passed.

## 9. Attack-aware Detection Feedback

- [x] 9.1 Add deterministic single-event pluck/decay, sustained modulation, release/noisy-tail, soft-attack and nearby repeated-note fixtures; verify current failure is reproducible and define onset/count expectations for whole-run, split-commit and gap cases before changing acceptance.
- [x] 9.2 Add attack-confidence/energy-rise confirmation, adaptive floor and hysteresis with less aggressive defaults; verify one isolated event yields one accepted target while supported distinct/soft attacks remain detectable without shifting timestamps or relying only on a longer dead time.
- [x] 9.3 Version analysis policy/cache identity and reconcile valid reanalysis under the existing ownership rules; verify unchanged repeat capture, saved sensitivity preservation, manual/protected targets, deletion suppression, stable matched IDs and incomplete/superseded analysis retaining edits.
- [x] 9.4 Register detection/coordinator regressions and document measured defaults, supported spacing, onset tolerance and musical limitations; verify fixture results and diagnostic controls match the documented policy.

## 10. Wet Transition Model, Tension and Render Support

- [x] 10.1 Extend saved metadata for per-fade continuous tension and explicit wet endpoint/support intent; verify signed gap/overlap arithmetic, independent fade duration, exclusive bounds, malformed/range rejection and migration preserving all old edits with neutral tension.
- [x] 10.2 Implement the shared monotone tension law over existing fade presets and nondestructive wet shaping; verify endpoint values, neutral legacy equivalence, continuous curvature, overlapping fades, duration clamping and an independent sample oracle without changing dry samples or raw render identity.
- [x] 10.3 Prepare effective render input through a requested wet endpoint beyond existing support while preserving the base source selection; verify exact convolution length/alignment, visible source-extension bounds, genuine committed coverage, changed-extension fingerprints and no padding/stretching or silent endpoint clamp.
- [x] 10.4 Integrate requested/pending support with generation guards and bounded automatic coordination; verify missing extension waits, resource failure retains intent, obsolete completions cannot become current, unrelated swells stay unchanged and in-support endpoint/fade/tension edits do not reconvolve.
- [x] 10.5 Add model/render/shape persistence tests and document source-extension consequences, limits and cache migration; verify gap/overlap/tension round trips, reusable compatible renders and explicit recovery for missing source/IR/render assets.

## 11. Direct Envelope Manipulation

- [x] 11.1 Share on-curve endpoint/fade/tension geometry between painting and hit testing with distinct visible glyphs, generous hit areas and hover/active feedback; verify target selection, co-located points, zero-length fades and dry-arrival separation in offscreen interaction tests.
- [x] 11.2 Update accepted selected-envelope visuals immediately during dragging with stable gesture ownership and viewport; verify fine/snap-independent movement, numeric/keyboard alternatives, smooth intermediate values and bounded background preparation without UI-thread PCM or FFT work.
- [x] 11.3 Bind independent wet endpoint, fade duration and tension controls to handles and precise inspector values; verify gap/overlap readouts, dry timing/level remaining untouched, pending-support ghost geometry, source-extension disclosure and actionable missing coverage.
- [x] 11.4 Render and inspect realistic pluck/IR fixtures at 1100x850, 1400x900 and 1600x1000 with Diagnostics open/closed; verify readable grab points, consistent curve/audio values and no layout regression, then document gestures and updated screenshots.

## 12. Transition Listening Context

- [x] 12.1 Extend captured Dry + Wet preparation through D=max(base source end,max(dry arrival,wet end)+post-roll), defaulting post-roll to two seconds; verify gap/overlap examples, long selected sources, stereo order and unchanged dry samples/positions against a sample oracle while Wet Only retains wet bounds.
- [x] 12.2 Add a saved typeable zero-to-five-second tail preference and safe monitoring handoff; verify 0/50/100% blend, trim/protection separation, missing context disabling Dry + Wet with a needed-range message, verified Wet Only recovery, no tail-only convolution and allocation/feedback-safe callback behavior.
- [x] 12.3 Document and test migration/restoration of tail and transition preferences; verify prior fades/endpoints/IRs/provenance/trim are preserved, no dry fade-in or ducking is introduced, extended context is not silently shortened and reopen never starts audition.

## 13. Feedback Integration and FL Studio Acceptance

- [x] 13.1 Build the updated Windows Debug VST3, run all existing/new CTest targets and strict OpenSpec validation, and cross-check every added scenario; verify detector/envelope/extended-support/context integration and record new results separately from the historical 22-test baseline.
- [x] 13.2 In FL Studio, test isolated decaying events, sustained modulation, soft attacks and genuine repeated notes with revised defaults; verify marker counts, onset placement, manual ownership and repeated-capture stability, recording actual settings/results before acceptance.
- [x] 13.3 In FL Studio, drag on-curve fade/tension/end controls into gaps and overlaps, adjust endpoint and fade independently, and audition through one/two-second tails; verify untouched dry timing/level, real extended wet support, missing-context guidance, usability and save/reopen, then record explicit user acceptance.

## Second Feedback Revision � Pending Implementation

Tasks 1-13 retain their recorded first-feedback scope and 56 completed checkboxes; historical automated results do not validate the following additions. The new host acceptance task supplements, rather than completes, existing open checks 8.1-8.5 and 13.2-13.3.

## 14. Initial and Coincident Fade Controls

- [x] 14.1 Add shared true-anchor/collision-target geometry with labelled connectors and generous hit areas; verify zero fade-in/out and endpoints remain separately grabbable, zoom collisions resolve deterministically and zero segments do not present fictitious active tension.
- [x] 14.2 Bind fade dragging to the initial value and pointer delta, preserving bound/arrival positions; verify opening zero fades without numeric setup, no displacement jump, immediate coherent feedback and fine/keyboard/numeric alternatives without UI-thread audio preparation.
- [x] 14.3 Extend offscreen interaction fixtures at 1100x850, 1400x900 and 1600x1000 with Diagnostics open/closed; inspect initial/zero/colliding controls and document discoverable gestures and screenshots.

## 15. Whole Wet Audio and Envelope Placement

- [x] 15.1 Add versioned signed wet placement and checked logical/positioned coordinate transforms; verify zero-offset migration, negative positions, exact duration preservation, inverse endpoint support coordinates, overflow rejection and round trips retaining all prior edits.
- [x] 15.2 Add a clear selected-body move gesture plus precise placement/nudge controls; verify wet waveform and every envelope point translate together, fades/tensions/presets/gain remain unchanged, handles take precedence and selection/viewport stay stable.
- [x] 15.3 Integrate positioned shaping/waveforms/audition and inverse support admission without reconvolution for pure moves; verify unchanged raw sample/checksum/IR/base-source/dry-arrival identity, genuine support only for independent endpoint extension, cancellation and bounded work.
- [x] 15.4 Add placement sample-oracle, ownership/reanalysis, cache-reuse and persistence regressions; verify protected automatic edits, stable detector anchors/suppression, unrelated swells unchanged, missing-resource intent retained and reopen without playback, then document placement versus arrival/crop editing.

## 16. Click Prevention and Gentle New-Swell Defaults

- [x] 16.1 Add deterministic click-prone fixtures with nonzero/DC boundary samples, zero/short fades, natural completion, Stop/restart/replacement and one/very-short buffers; reproduce old discontinuities and define endpoint/sample-jump expectations before changing monitoring.
- [x] 16.2 Prepare disclosed 2 ms per-wet-contribution monitoring safety ramps without changing stored PCM, saved shape edits, interval placement or dry transition gain; verify zero boundary samples, proportional short-interval handling and correct ordering before aggregate protection.
- [x] 16.3 Add 5 ms outer manual-audition start/end and smooth Stop ramps with exact interval length/cursor continuity; verify natural completion, replay/replacement, short buffers, unsafe-state immediate silence, no callback allocation/locks/file I/O and safe pin retirement.
- [x] 16.4 Apply shared 20 ms overlap and 20 ms Smooth wet fade-out defaults once to new manual/automatic swells at valid sample rate; verify zero placement, real support/waiting, valid duration clamping and no reset of saved/edited shapes during restore, reanalysis or regeneration.
- [x] 16.5 Extend independent monitoring/sample and migration oracles and diagnostics/docs; verify original dry interior/PCM/live passthrough unchanged, safety versus user fade clearly disclosed, raw identities preserved and no autoplay or guard-only convolution.

## 17. Overlapping-Swell Audition With Optional Isolation

- [x] 17.1 Persist Isolate selected defaulting off and derive a finite anchor listening window and eligible half-open intersecting membership; verify Wet Only versus Dry + Wet tail, placement, enabled/mute/global-solo policy, muted-anchor neighbours, empty membership and no recursive overlap expansion.
- [x] 17.2 Prepare a bounded aligned group with one captured dry context, per-member saved wet shaping/gain and safety, aggregate wet protection and existing blend/sum/trim law; verify 0/50/100% sample oracles, correct clipping/relative levels, no duplicate dry, unrelated swells excluded and unchanged raw assets.
- [x] 17.3 Version group job dependencies across anchor/window/scope, member IDs/assets/placements/generations/activity and capture revisions; verify missing required members block with identity/reason, excluded members do not block, obsolete groups cannot publish and explicit isolation can recover an independently valid selection.
- [x] 17.4 Bind scope/count/status controls and safe monitoring handoff; verify scope/mode/blend/tail/trim remain independent, selection stops old playback, rapid edits stay responsive and scope/membership controls cause no convolution, feedback or callback resource work.
- [x] 17.5 Add overlap-chain, boundary-touch, mixed-rate/missing/stale-member, summed-peak, pressure and save/reopen regressions; verify bounded shared-budget residency, preference migration, verified cached-wet rules and no automatic playback, then document audition scope/recovery.

## 18. Second Feedback Integration and Host Acceptance

- [x] 18.1 Update interaction and monitoring guides/screenshots and map every new scenario to meaningful automated coverage; verify defaults, placement/support semantics, collision gestures, disclosed ramps and overlap-window/isolation behavior remain coherent with deferred Song scheduling.
- [x] 18.2 Build the updated Windows MSVC Debug VST3, run the complete existing/new CTest suite and strict OpenSpec validation, and record actual new results separately from the historical 23-test pass; verify migration, callback safety and all second-feedback scenarios before marking implementation complete.
- [x] 18.3 In FL Studio, test zero-fade grabbing, whole wet-body movement, independent crop editing, new gentle defaults, click-free wet and audition endings, overlapping/isolate listening with mute/solo, dry continuity and save/reopen at actual host settings; record explicit user acceptance and leave earlier unperformed host checks open.

## 19. Main-View Marker Sensitivity and Independent Enable

These approved controls extend the second-feedback revision; previous implementation/test results do not complete these tasks. Preserve all earlier checkboxes and host acceptance status.

- [x] 19.1 Add a validated saved per-instance Auto markers preference defaulting on, using the existing canonical sensitivity without resetting saved values/master/mapping settings; verify saved On/Off, legacy manual-mode migration, invalid values and reopen without autoplay.
- [x] 19.2 Add one prominent main-view rotary Marker sensitivity control with typeable 0-100% value and adjacent Auto markers switch; verify shared diagnostic binding, independent per-instance versus per-swell state, effective Off/master-paused/analyzing status and readable controls at all supported sizes with Diagnostics open/closed.
- [x] 19.3 Calibrate/test low/default/high sensitivity on deterministic busy fixtures with strong attacks plus small transients; verify lowering sensitivity reduces minor markers while documented prominent attacks remain detectable at unchanged separation, retaining isolated/soft/repeated/gap/incremental regressions and documenting measured limits rather than claiming key-change recognition.
- [x] 19.4 Coalesce sensitivity reanalysis off-thread and gate analysis admission/publication by marker enable/control generation; verify disable during a scan, rapid Off/On/knob changes, saved sensitivity while off, bounded catch-up on enable, stable matched IDs/protected edits/suppression and incomplete analysis preserving work without forcing unchanged renders.
- [x] 19.5 Extend model/coordinator/editor/processor tests and guides for the new controls; verify capture/manual Add/Delete/Suggest/Confirm and permitted existing-render refresh continue with marking off, callback safety and no implicit master changes, then build/run the full automated suite and strict validation with new results recorded separately.
- [x] 19.6 In FL Studio, adjust low/default/high sensitivity on actual busy source, switch Auto markers off/on during capture and save/reopen; verify useful prominent-event selectivity, responsive feedback, retained edits/suppression and continued capture/manual/render workflow, recording actual settings and explicit user acceptance.

## Workflow follow-up

- Review M3 and this change with actual host evidence; sync/archive M3 first, then this successor through explicit workflows after signoff.
- Propose transport-synchronized swell playback separately, including second-pass sound, start-in-middle offsets, overlap mixing, seeks/loops and routed-silent/unused-insert host behavior. None of those scheduler validations are claimed by this checklist.
- Carry wet transition metadata into that future playback work; do not introduce a complementary dry fade or change live dry passthrough as part of this feedback revision.

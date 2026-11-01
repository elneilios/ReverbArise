# Tasks

Unchecked work below remains pending. Check tasks only after their stated verification succeeds; preserve predecessor acceptance checkboxes. Implementation should finish automated work and stop for FL Studio validation.

## 1. Live state and coherent transport

- [x] 1.1 Add typed, independently saved live enable/routing/wet level preferences and state-version migration; verify older v1-v4 fixtures, malformed values and roundtrip preservation of existing edits/audition settings in session and processor tests.
- [x] 1.2 Pass a coherent playing/position/mapping block snapshot to the engine and validate sample-to-PPQ origin with documented rounding tolerance; verify missing fields, nonzero origins, overflow, rate/tempo/meter changes and discontinuities in timing tests.
- [x] 1.3 Document live defaults, routing and fixed-mapping restrictions in a new transport playback guide; verify examples match migration and timing fixtures.

## 2. Prepared schedule and bounded pages

- [x] 2.1 Build the worker-side eligible positioned schedule from verified current assets, honoring enabled/mute/global solo, shape and placement independently of selection; verify independent interval and shaped PCM oracles, gap/overlap edits and exclusion of cached-source recovery.
- [x] 2.2 Calculate fixed revision-level wet protection from overlap peak bounds; verify loud stereo overlaps, quiet no-boost, relative gain preservation, nonoverlapping peaks and invariance across page boundaries/residency shifts.
- [x] 2.3 Prepare independent immutable per-swell timeline pages outside the callback with bounded forward/loop-destination prefetch and stopped-position preparation; verify seam-exact PCM against a whole-interval reference, selective invalidation, warm/cold seeks and explicit refusal above 64 simultaneous contributions.
- [x] 2.4 Integrate page, scratch, hydration and retired-pin reservations with the shared 256 MiB admission budget; verify a logical long-song pressure fixture, concurrent render/audition work, refusal diagnostics and no pinned eviction.
- [x] 2.5 Document page coverage, prefetch bounds, maximum supported callback spans/block handling and resource-miss recovery; verify each stated limit against automated admission fixtures.

## 3. Real-time renderer and safe transitions

- [x] 3.1 Add bounded pinned publication and external retirement for transport pages, with generation cancellation; verify allocation/deallocation guards and concurrent publication, queue saturation, restore and destruction lifetime stress tests.
- [x] 3.2 Implement absolute block intersection mixing and variable-block slicing without callback allocation, locking or disk I/O; verify exact independent sample oracles for starts/ends, negative lead-in, large blocks, random partitions and int64 boundary rejection.
- [x] 3.3 Implement start/stop/seek/loop, missing-page and revision transitions of at most 5 ms, applying crop safety only once per event; verify nonzero/DC edges, short events, seamless page boundaries, warm mid-swell joins and no old-cursor continuation.
- [x] 3.4 Apply independent live gain and Dry + Wet/Wet Only routing after original-input capture; verify unity dry in the default mode, neutral live gain, routing ramps, dry return on stop/disable and independence from audition blend/trim/tail/isolation.
- [x] 3.5 Document disclosed crop/handoff/routing ramps, wet-only peak ceiling and combined-output limits; verify documented numeric examples against renderer tests.

## 4. Engine lifecycle and processor integration

- [x] 4.1 Connect playback preparation to existing render/shape/activity generations and actual dependency fingerprints; verify identical/unrelated recapture continuity, affected-source invalidation, IR edits, placement edits and no late cancelled publication.
- [x] 4.2 Integrate priority between enabled Song playback and manual audition; verify audition stops within 5 ms on host Play, is inhibited while live Song playback is active, remains independent with live disabled, and never resumes automatically on Stop.
- [x] 4.3 Gate playback across clear, restore, missing/corrupt assets and incompatible timing; verify affected exclusion, unaffected availability, fresh-host-snapshot requirements and no automatic transport request or stale wet recovery shortcut.
- [x] 4.4 Verify dry-only mapping and scoped recapture before every output branch using processor sample oracles with wet/tone/preview over zero input and Wet Only over nonzero input; confirm playback cannot feed detection or convolution through internal capture.
- [x] 4.5 Exercise bounded cold-seek recovery and continuous callback processing under worker delay, repeated edits and capture churn; verify correct current-offset admission, no retriggered preroll, explicit misses and stable resource bounds.
- [x] 4.6 Extend the playback guide with recapture validity, missing-asset recovery and stopped/disabled/manual behavior; verify instructions against integration fixtures.

## 5. Workspace and extreme handle regressions

- [x] 5.1 Reproduce extreme handle issues with event-driven tests for coincident/zero/full fades, crossing endpoints, maximum tensions, extreme signed placement and viewport edges; record the actual failing geometry or gesture sequence before changing it.
- [x] 5.2 Fix reproduced handle bounds/identity/packing failures while preserving on-curve controls, separate zero-fade targets and numeric editing; verify gesture ownership, finite clamped state, unchanged dry anchor and fade/tension preservation at supported window sizes with Snap both on and off.
- [x] 5.3 Add prominent independent live enable, routing and typeable level controls plus actual readiness, miss and attenuation status; verify UI-to-model behavior and resized screenshots while retaining manual Audition controls.
- [x] 5.4 Draw the host playhead in detail/overview and distinguish live, stopped, preparing and inhibited-audition states; verify seeks/loops and offscreen rendering, with audio operating identically when the editor is closed.
- [x] 5.5 Update workspace usage and diagnostics documentation for the implemented live path and extreme-gesture fixes; verify examples and screenshots reflect actual implemented behavior rather than the previous 'coming later' placeholder.

## 6. Automated integration and handoff

- [x] 6.1 Run the complete Debug build followed by the full CTest suite and strict OpenSpec validation; record commands and results, requiring the existing 23-target baseline and new meaningful transport regressions to pass without unreviewed behavioral weakening.
- [x] 6.2 Deliver a transport-specific FL Studio checklist, exact VST3 path/hash, prepared fixture and warm/cold readiness instructions; verify the checklist covers each host task below and clearly states that host acceptance is pending.

## 7. FL Studio acceptance — user validation required

- [x] 7.1 Confirm normal Song playback hears all eligible prepared swells alongside unchanged live dry at correct positions, including wet gap/overlap edits; compare Wet Only and disabled modes and record the tested build, host version and audio settings.
- [x] 7.2 Confirm start inside a swell, forward/backward warm seeks, cold seek reporting/recovery, and looping through an overlap; verify no wrong-offset retrigger or sustained old-position audio.
- [x] 7.3 Confirm edits, mute/solo, identical versus changed recapture, stopping and re-enabling are musically usable without obvious clicks or duplicate audition output; verify extreme handle drags remain stable in the user's project.
- [x] 7.4 Confirm reopen, missing asset, incompatible timing and editor-closed playback behavior; verify saved preferences restore and no stale wet is represented as current.
- [x] 7.5 Test routed-silent input, fully idle inserts and smart-disable behavior, plus offline bounce with two mixer instances and unavailable-swell warnings; record actual callback/coverage limitations, audible results and any misses rather than assuming successful export.

## 8. Loop capture feedback

- [x] 8.1 Preserve capture identities and suppress commit notifications for sample-identical covered audio; verify repeated full/partial normal loop captures, silence, changed PCM and scoped-recapture completion.
- [x] 8.2 Keep ready assets and prepared audio stable across transient stopped/playing timing-read gaps; verify a delayed host query and repeated unavailable worker snapshots without dependency requeue or render churn.
- [x] 8.3 Fit untouched automatic source/default wet tails to explicit loop ends or observed closed backward passes, using only genuine coverage and preserving manual edits; verify final-attack rendering, growing-pass waiting, later coverage extension and persistence of the resulting edits.
- [x] 8.4 Verify repeated automatic loop playback keeps target IDs, render assets and prepared contributions stable while actual source changes still refresh only their dependants; add a stopped-settle regression and boundary-marker coverage.
- [x] 8.5 Run the full build, CTest suite and strict OpenSpec checks; record the replacement VST3 hash and a focused FL Studio loop/stop/final-marker retest without marking host acceptance passed.

## 9. Performance variation and offline bounce

- [x] 9.1 Add a saved, separately adjustable change-tolerance policy against retained captured baselines; verify small level/phase/noise variations are ignored, larger level/spectral/transient changes refresh, drift does not accumulate, exact mode and scoped recapture remain available. Add saved per-swell Freeze render: protect committed wet against source refresh, preserve shape/gain/placement edits, require unfreeze for source/IR/support changes, verify missing-source restore and unfreeze.
- [x] 9.2 Implement an explicit non-real-time bounce path with frozen prepared definitions and synchronous bounded pages, excluding export input from capture; verify cold start, large/irregular blocks, long timelines, overlaps, routing, missing assets and realtime return without dropped wet or live callback waits.
- [x] 9.3 Add processor/UI/state regressions for separate regeneration sensitivity, frozen renders and offline output against independent positioned references; verify the export path does not depend on background scheduling or host Playing being true.
- [x] 9.4 Run the full build/tests and strict spec validation, record the new VST3 identity, and provide FL Studio variation/export retests with host acceptance left pending.

## 10. Final-loop and multiple-instance export regressions

- [x] 10.1 Reproduce fractional-tempo/one-sample loop-end overrun and final-marker waiting; fix completed-pass fitting without inventing coverage or changing manual/frozen edits, verifying advertised bounds, inferred wrap, stable repeats and later genuine extension.
- [x] 10.2 Exercise two independent instances exporting on separate mixer paths, including a waiting final marker, serial/parallel fast callbacks, repeated bounces and isolated failures; export all available contributions even when another swell is unavailable, preserve dry routing and report skipped swells prominently. Verify complete positioned output after repair and per-instance callback diagnostics.
- [x] 10.3 Run the full build, all tests and strict validation; record the replacement binary and focused FL Studio final-marker/two-track export checks, leaving host acceptance open.

## 11. Offline completion diagnostics

- [x] 11.1 Reproduce PPQ reset/freeze during host offline tail processing after all wet has ended; preserve successful/skip-warning status and exact selected dry routing there, while retaining mapping/rate/tempo/meter failures when relevant wet remains. Add explicit timing context to real failures and verify both routing modes and real mismatch regressions.
- [x] 11.2 Run the full build/tests and strict validation, record the new bundle identity and explain the distinction between a tail-time reset and an actual incomplete wet export; leave FL Studio confirmation pending.

## 12. Envelope and audition acceptance feedback

- [x] 12.1 Reproduce misleading displaced/crossed envelope handles and replace arbitrary horizontal fan-out with bounded, time-aligned collision controls; verify full/zero/overlapping fades, tension, extreme drags and supported sizes with stable hit identity.
- [x] 12.2 Publish an allocation-free coherent audition sample cursor and display a distinct moving playhead in detail and overview, independent of DAW transport; verify start/advance/replacement/stop/completion and actual UI pixels.
- [x] 12.3 Add an optional saved Isolate dry source audition control, limiting replayed dry to the selected source interval while retaining contextual mode, wet membership, live dry and listening-window semantics; verify independent sample boundaries, migration and UI roundtrips and show the replayed dry span.
- [x] 12.4 Run full build/tests and strict validation, inspect updated UI screenshots, record the new binary and request focused checks 3/4 again without closing their mixed host tasks.

## 13. True-position stacking refinement

- [x] 13.1 Replace zoom-dependent handle packing with true-position z stacking; distinguish genuine sample-coordinate coincidence from visual proximity, offset only genuinely coincident controls, match hit priority to paint order and verify zoom separation plus zero-fade grabbing and extreme gestures.
- [x] 13.2 Build/test the replacement, inspect close/far-zoom screenshots, validate specs strictly and record its binary identity; retain the outstanding envelope/audition host checks.

## 14. Waveform-centred endpoint handles

- [x] 14.1 Anchor Start/End handles permanently at the waveform centre line, leaving fade/tension handles on the curve; require both sample and amplitude coincidence for exceptional offsets and verify zero/full/edited fades, direct grabbing, zoom and unchanged timing/audio.
- [x] 14.2 Build/test and inspect zero/edited-fade screenshots, validate specs and record the new bundle; keep envelope host acceptance pending.
- [x] 14.3 In FL Studio, confirm Start/End stay on the waveform centre line with zero/nonzero/full fades, fade controls naturally remain separate, and endpoint timing/dragging is usable; record acceptance before archiving this refinement.

## Workflow follow-up

- M3 has 42/42 completed tasks; the automatic workspace is already synced and archived. Preserve their recorded acceptance and do not infer transport check 14.3 from those results.
- Reconcile and sync/archive `edit-and-persist-target-swells` against the already synced workspace specs, then sync/archive transport after 14.3 passes. Preserve contextual/overlapping audition, neutral absent-trim migration and all recovery scenarios when applying reverse-convolution deltas.
- Archive only after implementation checks and explicit host results are recorded.

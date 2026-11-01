# Tasks

## 1. Editable Session and Coordinate Model

- [x] 1.1 Add stable session/target/swell/source/shape/IR-reference records and primary/multiple selection under `Source/Core`; verify independent add/confirm/delete/rename and enable/mute/solo behavior, UUID uniqueness, and selection cleanup with headless model tests.
- [x] 1.2 Implement absolute-sample arrival/source moves and initial source defaults, including containment of arrival, contiguous coverage limits, next-arrival bounds, fixed-tempo quarter-note length and checked arithmetic; verify sub-bar precision, negative lead-in, first/last sample and overflow cases without altering unrelated swells.
- [x] 1.3 Implement nondestructive crop/fade/gain validation, default automatic raw start and arrival end, explicit Full Render/Reset Shape, and invalid-crop state after an IR-length change; verify defaults, restoration of raw bounds, finite/range rejection, fade clamping and render-identity preservation.
- [x] 1.4 Add versioned metadata encoding/decoding for the model and saved preferences with 8 MiB/4096-swell defensive limits; verify full round trips, duplicate IDs, malformed versions/paths/bounds/non-finite values and transactional rejection retaining the prior valid model.
- [x] 1.5 Document model defaults, arrival/source coupling, sample/bar rounding, shape semantics and parsing limits in `docs/m3-editing.md`; verify documented examples against the model tests and add the new headless target(s) to CTest.

## 2. Immutable Managed IR and Render Assets

- [x] 2.1 Add a managed asset store using the existing project/instance root and safe relative references; verify canonical mono/stereo float IR and full stereo render round trips, sample order/gain/rate/length, metadata/checksum validation, and traversal rejection in a temporary store.
- [x] 2.2 Import through existing WAV/WavPack decoders and publish canonical managed IR assets with stable decoded-content identity; verify equivalent WAV/WavPack assignments, leading silence/above-unity floats, Unicode paths, original-file immutability, and successful reload after moving the external original.
- [x] 2.3 Publish IR/render files and metadata atomically on a worker before Ready/reference publication; verify interrupted writes, blocked publication, checksum mismatch, and failure retaining earlier committed references without partial files becoming current.
- [x] 2.4 Preserve immutable assets across definition deletion, Clear and independent saved snapshots; verify earlier project references remain readable after later edits and that shared content is not overwritten or automatically removed.
- [x] 2.5 Document `irs/`/`renders/` formats, content keys, engine version, retention and same-PC portability limits in `docs/m3-persistence.md`; register asset tests and verify documented fixtures match the saved metadata.

## 3. Offline Onset Suggestions

- [x] 3.1 Add bounded streaming contiguous-run analysis over committed source, reusing verified asset reading without loading a whole song; verify adjacent chunk continuity, silence, unavailable assets, negative positions and history reset at genuine gaps.
- [x] 3.2 Implement deterministic spectral-flux/local-peak suggestions with sensitivity/minimum-separation controls; verify onset proximity within one window for synthetic impulses/plucks/soft attacks, separation/tie behavior, dense/noisy input, silence and multiple sample rates.
- [x] 3.3 Add cancellable analysis requests and candidate refresh separate from confirmed definitions; verify stale completions are discarded and sensitivity/reanalysis does not move, delete, reset or duplicate manually confirmed targets.
- [x] 3.4 Register analysis tests and document window/hop, control defaults/ranges, tolerance and musical limitations in `docs/m3-editing.md`; verify fixture expectations and no automatic swell generation from unconfirmed suggestions.

## 4. Envelope Preparation and Protected Selected Audition

- [x] 4.1 Add pure shape evaluation and background cropped audition-buffer preparation over immutable full renders; verify exact sample placement/end exclusivity, linear/smooth/exponential factors, zero/overlapping fades, singleton intervals, negative bounds and finite output against an independent numerical oracle.
- [x] 4.2 Build selected raw/shaped waveform summaries and edited peak without reconvolution; verify crop/gain/curve changes preserve the raw checksum/render identity, coalesce obsolete shape requests and expose clipping-free labeled display scaling.
- [x] 4.3 Integrate prepared selection with the pinned `AuditionVoice`, shaped-peak protection and saved-level control; verify complete edited playback, -6 dBFS ceiling/default extra -12 dB, no quiet boost, mute/enable/solo exclusion, Stop and edit/selection-triggered stopping without automatic restart.
- [x] 4.4 Extend callback allocation/deallocation, dry passthrough and capture-feedback tests through shaped audition and retired buffers; register envelope tests and document raw/shaped/audition gain distinctions and manual transport-independent playback.

## 5. Multi-Swell Render Coordination

- [x] 5.1 Add canonical interval content fingerprints over exact decoded source samples with revision-based dirty hints; verify identical recapture, changed overlap, zero-audio versus covered silence, chunk splitting, sub-chunk boundaries and unrelated changes without per-sample file reads.
- [x] 5.2 Generalize the single-preview worker into per-swell request/state coordination with one render worker and session/swell/input generations; verify independent completions, deletion/restore/Clear cancellation, checking-to-ready/stale transitions and no obsolete publication.
- [x] 5.3 Implement Regenerate Selected and Regenerate All with committed source/managed IR snapshots and per-swell progress/errors; verify selected-set isolation, missing/stale batch membership including muted/disabled swells, current-result reuse, and continued valid work when another swell fails.
- [x] 5.4 Add shared 256 MiB render/analysis memory admission, worker eviction of unpinned inactive audio, disk-backed inactive render references and one selected shaped buffer; verify many stored swells, pinned-buffer pressure, scratch/analysis accounting, overflow/rejection and no truncation or callback retirement.
- [x] 5.5 Implement optional persisted auto-regeneration, off by default, with commit-only triggering/coalescing and retry; verify unchanged content does not reconvolve, rapid commits supersede obsolete work, failure retains an inspectable stale result, and document transitions/resource accounting in `docs/m3-regeneration.md`.

## 6. Scoped Recapture and Manual Transport Fallback

- [x] 6.1 Add explicit armed-range admission distinct from passive mapping, intersecting dry blocks before queueing and tagging their arm generation; verify exact samples at both boundaries, blocks larger than the range, unrelated coverage preservation, cancel, and unchanged tone/dry behavior.
- [x] 6.2 Track fresh committed coverage for an arm independently of old coverage, including queue drops, failed commits, seeks and repeated passes; verify partial input never completes a refresh and only fully committed fresh coverage disarms/queues dependent changed swells.
- [x] 6.3 Connect Recapture & Regenerate completion/cancel to coordinator requests, separate from ordinary auto-regeneration preference; verify content-identical completion does not render, unrelated swells remain current, cancellation never fabricates coverage, and no host transport requests are issued.
- [x] 6.4 Extend processor allocation/deallocation and feedback tests through armed capture/audition; document manual move-playhead/play instructions, fresh-coverage progress, commit wait and cancellation in `docs/m3-regeneration.md`, verifying the guide matches observable states.

## 7. Plugin State and Background Restoration

- [x] 7.1 Write/read v3 compact editing metadata and committed asset references using immutable snapshots in plugin state; verify a multi-swell round trip with fades, IRs, source ranges, names, selection, timing, analysis/auto-regeneration preferences and -24 dB level, without PCM or save-time disk/render waits.
- [x] 7.2 Keep explicit v1/v2 migration fixtures and reject unsupported/malformed v3 state transactionally; verify legacy capture UUIDs/manifests/tone survive into an empty collection at -12 dB, prior valid state survives rejection, and restore always stops audition, mapping, arms and old jobs.
- [x] 7.3 Restore assets asynchronously after capture and timing validation, loading active audio on demand; verify awaiting timing, matching cached render reuse, missing/corrupt/old-engine render rebuild, missing IR/source diagnostics with unaffected swells retained, and incompatible rate/tempo/meter without reinterpretation.
- [x] 7.4 Test save during import/render/recapture, reopen after moving external IR, earlier-snapshot recovery after Clear, and separate saved copies sharing immutable assets; verify only committed dependencies become ready and document v3 migration, retention and M2 rollback limitations in `docs/m3-persistence.md`.

## 8. Timeline and Editing Controls

- [x] 8.1 Add shared sample/pixel/grid transforms and overview/detail viewport with 1/2/4/8/16-bar paging, scroll/zoom, overview jump and host playhead; verify round-trip coordinates, snap on/off, negative time, distant sparse sections, and no edits/render invalidation from navigation.
- [x] 8.2 Add distinct suggestion/confirmed target lanes and a virtualized swell list with confirm/add/move/delete, names, enable/mute/solo and primary/multiple selection; verify offscreen hit testing, keyboard/numeric alternatives, stable IDs and unrelated-swell preservation.
- [x] 8.3 Add selected-swell IR assignment, precise source handles/fields alongside bar controls, full raw context and shaped lane, crop/fade handles, curve/gain and Full Render/Reset Shape; verify offscreen interaction changes only the intended model fields and envelope edits do not request convolution.
- [x] 8.4 Add analysis controls, selected/all regeneration, scoped recapture/cancel/fresh-coverage progress, automatic-refresh preference and per-swell asset/job states; verify control availability for empty/checking/loading/rendering/ready/stale/error and invalid-crop states, including safe restore synchronization.
- [x] 8.5 Integrate Audition/Stop, saved audition level, raw/shaped/predicted peaks and disclosed display scaling; verify edited readiness/activity rules, readable labels, high-peak shapes without flattening and no autoplay after restoration or background preparation.
- [x] 8.6 Visually inspect software-rendered editor states at supported sizes and test layout/interaction with many and distant swells; update `docs/m3-editing.md` and write `docs/fl-studio-m3-validation.md` with reproducible media, exact expectations and separate host checks below.

## 9. Automated Integration

- [x] 9.1 Build the Windows MSVC Debug VST3 and run every existing/new CTest target; verify unchanged M1/M2 DSP/capture/WavPack regressions, updated v3 behavior with retained legacy fixtures, worker shutdown races and allocation-free callback integration, recording results in the M3 guide.
- [x] 9.2 Run strict OpenSpec validation and cross-check delivered editing/regeneration/persistence behavior against all delta scenarios; verify no main-spec edits during implementation and leave unperformed FL Studio checks visibly incomplete.

## 10. FL Studio Acceptance

- [x] 10.1 In the recorded FL Studio setup, capture separated musical sections, analyze/confirm/manual-edit several targets, page/zoom/snap, assign mono/stereo WAV and WavPack IRs, shape and audition selected swells; verify responsiveness, precise shared alignment/full context, no unintended reconvolution or transport-synchronized swell playback, level protection and independent activity controls, recording settings/results separately from automated tests.
- [x] 10.2 In FL Studio, exercise selected/all regeneration, changed/identical/unrelated recapture, partial armed coverage, cancellation, auto-refresh off/on, edits during rendering, silent input and generation during playback; verify only dependent swells change, capture excludes audition, zero dropped blocks in the tested setup, manual transport fallback and rate/tempo/meter safeguards.
- [x] 10.3 Save/reopen a multi-swell edited project with nondefault audition level, move the original external IR, temporarily remove a render cache in the test copy, and exercise missing managed input plus earlier-project recovery after Clear; verify preserved edits/settings, verified restoration/rebuild, explicit missing-asset state, no automatic audition/capture, unaffected swells retained and legacy-project compatibility, recording user acceptance.

## Workflow follow-up

- Review the M3 implementation and recorded host results, then sync/ archive through their explicit OpenSpec workflows.
- Continue transport-synchronized prepared-swell playback in M4; keep export, Pack Project/Relink and portability/recovery polish in M5.

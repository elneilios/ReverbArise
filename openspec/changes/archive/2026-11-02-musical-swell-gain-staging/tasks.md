# Tasks

Planning does not satisfy these boxes. Implement only after a new apply request. Record evidence with each completed task and keep listening/host approvals explicit across usage resets.

## 1. Compatibility and gain policy

- [x] 1.1 Capture numerical legacy output and version-1..7 state fixtures for default, edited, imported, frozen, overlapping and two-instance cases; record original IR/raw hashes and verify the fixtures before changing gain code.
- [x] 1.2 Add strictly validated version-8 gain policy/revision state and per-swell measured calibration metadata with old-version legacy migration and fresh-instance musical selection; verify transactional handling of unknown/nonfinite/invalid or mismatched calibration data, save/reopen, positive musical live gain and no autoplay or repeated baseline multiplication.
- [x] 1.3 Implement the policy baseline outside raw convolution and retain neutral 0 dB Level/reset semantics; verify -6/0/+6 dB ratios below protection, silence/near-silence, unchanged IR/raw hashes and convolution count, reset idempotence and gain nonaccumulation; document fixed versus source-referenced calibration and compatibility.
- [x] 1.4 Implement the local source-peak-matching candidate against creation-default wet at neutral Level; verify a numerical D/W oracle at 44.1/48/96 kHz, bounded target windows, next-target exclusion, source/wet floors, maximum boost, stereo-linked peaks, incomplete coverage and silent input without NaN/Inf, fabricated matching or noise amplification; document and report all provisional tuning values.
- [x] 1.5 Persist and reuse measured baselines through Level/fade/tension/crop/placement edits, reset, freeze and reopen; verify no inverse envelope compensation, identical recapture/reanalysis stability, atomic recalibration for genuine source/IR/extraction/target changes, accepted-old/pending-new races, missing-source frozen restore and explicit unavailable calibration on adoption without a reference.

### Additional source-energy calibration work (approved plan refinement)

- [x] 1.6 Implement bounded stereo-linked maximum-window RMS dry/default-wet measurement with provisional 100 ms attack, 20 ms RMS window, 1 ms hop, common short-interval window, -60 dBFS RMS floors, +24 dB boost cap and 0 dB target offset; verify independent 44.1/48/96 kHz oracles, next-target/coverage bounds, IR-scale compensation, source-amplitude tracking, silent-tail invariance, sparse/dense/polarity/stereo cases, floors/caps and finite failure states without raw/IR changes.
- [x] 1.7 Persist an immutable energy-method revision and dry/wet RMS, sample peaks and effective window/hop metadata; preserve supported peak-prototype version-8 output without reinterpretation, require explicit energy adoption, and verify fixed baselines across Level/fade/tension/crop/placement/reset/freeze/reopen, genuine dependency recalibration, pending accepted-version handover, live/audition/export equality and useful -60..+12 dB Level ratios below protection. Reverify affected completed integration tasks for the new revision.

## 2. Actual aggregate protection

- [x] 2.1 Implement bounded streaming measurement and immutable gain-plan keys including saved per-swell calibration over full accepted support; verify independent sample-reference oracles for noncoincident member peaks, correlated/anticorrelated overlaps, stereo asymmetry, negative starts, placement and cropped/faded intervals at 44.1/48/96 kHz.
- [x] 2.2 Integrate musical plans into live publication with -60..+12 dB wet level and <=5 ms transitions; verify exact gain with headroom, explicit ceiling reduction, stable gain across pages/seeks/loops/block partitions, unchanged dry and the original legacy-protection oracle.
- [x] 2.3 Integrate cancellation, shared-budget reservations, caching and prefetch servicing; verify rapid gain/IR edits, failed preparation, stale plan rejection and accepted-version handover with overlapping neighbours, callback allocation/lock/I/O guards and <=256 MiB accounting; record long-support preparation latency and cache reuse.

## 3. Audition and offline consistency

- [x] 3.1 Share musical wet gain/protection with audition while retaining blend, trim, isolation and safety ramps; verify unity Wet Only interior samples against live, 0/50/100% mixed oracles, single captured-dry contribution, stable measured baselines across monitoring scope/tail changes, unchanged live preferences and exact legacy trim/protection behavior; document legitimate scope/routing differences.
- [x] 3.2 Snapshot the accepted gain policy/plan for offline export alongside accepted IR audio; verify numerical equality to the live reference under irregular blocks, pending gain/IR replacements, cold export, final events and independently interleaved two-instance callbacks without missing/duplicated output.

## 4. Musical calibration checkpoint

- [x] 4.1 Generate reproducible comparisons of legacy, +3/+6/+9 dB fixed baselines and local source-peak matching initially at zero target offset with the candidate -1 dBFS ceiling using production preparation, all six factory IRs and representative imported responses; deliver Wet Only and matched-dry musical comparisons, reference-window/dry/default-wet peaks, computed/capped calibration, RMS/wet-to-dry/protection/ceiling occupancy, quiet/hot/busy/overlap cases and unchanged asset hashes.
- [x] 4.1a Generate a revised production-path listening comparison of source-energy matching at documented target offsets against fixed +6 dB and the peak prototype across all six factory IRs, sparse/dense imported responses and differently scaled copies, quiet/strong attacks, sustained/busy sources and overlaps; report actual reference/window/hop, RMS/peaks, baseline/caps, wet-to-dry energy, protection and ceiling occupancy, deterministic fixture identities and unchanged asset hashes. Correctly align dry/wet events and use bounded duplicate overlaps without recursive accumulation or sample clipping in review-file writing.
- [x] 4.2 Obtain explicit user listening approval of the source-energy method/tuning or a named comparison candidate and ceiling; record exact approved measurement window/hop/channel linking/short-interval treatment/floors/boost cap/target offset, fixture identities, intensity trade-offs and useful Level adjustment rather than treating generated files or equal measurements as approval.
- [x] 4.3 Pin the approved immutable musical method/revision and fresh-instance default; verify a clean reproduction matches its gain results, timbre/decay/stereo characters and PCM identities remain intact, approved calibration survives reset/freeze/reopen and legacy fixtures still match; publish the calibration report and numerical tolerances.

## 5. Controls and user guidance

- [x] 5.1 Add explicit legacy/musical adoption in Settings and policy-dependent live wet bounds; verify numerical edits/assets remain intact, positive-gain legacy switching is rejected without clamping, Level entry/drag/reset and frozen controls work, and legacy/musical state round trips preserve their output.
- [x] 5.2 Show requested/calibration/protection gain in existing status/Diagnostics with understandable matching-unavailable/capped and ceiling-constrained indications; verify protected/unprotected/zero/missing/pending states at the approved minimum editor size, preserve the layout, and obtain visual approval if presentation materially changes.
- [x] 5.3 Document neutral Level, approved baseline method, peak versus perceived loudness, fixed-baseline editing behavior, protection limits, explicit existing-project adoption, version-8 rollback implications and audition/live routing differences; verify documented actions against native UI tests and the recorded listening build.

## 6. Integrated acceptance

- [x] 6.1 Run full configured Debug build/CTest and focused Release gain/state/UI/library/handover/streaming/transport/export checks; record results, final binary/asset identities and preparation-memory/timing measurements without weakening legacy tests or raising budgets, then pass strict OpenSpec validation and diff checks.
- [x] 6.2 Obtain FL Studio acceptance of default impact, useful down/up Level travel, protection feedback and explicit old-project adoption on musical sources; record build, host/rate/buffer, routing/trim, long overlaps, seeks/loops and two-instance WAV export/save/reopen results before final acceptance.

## 7. Host-feedback UI corrections

- [x] 7.4 Trial a shared actual-amplitude scale for captured source and calibrated shaped wet, with a typeable display-only Wave zoom applying equally to both; verify equal amplitudes have equal relative heights, Level remains visible, hot raw references do not affect scaling, and provide a native preview and Release host build. This supersedes independent wet scaling in 7.1/7.3.

- [x] 7.3 Anchor audible waveform display scaling at neutral Level, leaving visual headroom so Level changes grow/shrink the waveform; verify -60/-6/+6 dB and preserve independence from hot raw IR references.

- [x] 7.1 Make the audible shaped wet waveform determine its own visual scale, with separately scaled subdued raw references and clear display-only labelling; verify hot raw IR amplitudes cannot shrink the aqua waveform and preserve envelope/handle geometry and audio settings.
- [x] 7.2 Fix shared knob text ownership so Level remains typeable after Return without refocusing and parameter values update continuously during knob drag; retain gesture anchors, fine adjustment, selection-change cancellation, exact entry and +/- Level values; verify native Debug/Release UI checks and provide an updated host build.

## Workflow follow-up

- The user approved source-energy +6 listening, the shared-amplitude display trial, and all three host-validation question groups on 2026-11-02. See `docs/gain-staging-evidence.md` for the accepted results and explicitly unspecified host configuration fields. Earlier prototype evidence remains historical.

- Update each task at its CLI-returned source location immediately after its implementation and verification pass.
- Do not finalize numerical defaults before task 4.2 or substitute automated tests for listening/FL Studio acceptance.
- Reconcile overlapping transport/audition deltas with the library change before syncing. Preserve accepted IR handover and its independent remaining host evidence; do not mark that change complete by inference.
- Sync and archive only after required evidence and approvals are complete. No commit/push, GitHub comment or issue closure is implied by this proposal.

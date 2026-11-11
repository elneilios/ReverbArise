# Tasks

## 1. Finite composition and shared wrap authority

- [x] 1.1 Add immutable Loop export plan metadata including the independent Dry/Wet pair and checked capture-local origin/range/repeat identity, preserving Song identity behavior; extend export plan tests for the minus-2.4-bar/four-bar example, zero lead-in, dry-only origin, missing grid authority and rejected overflow/rate/count/level values.
- [x] 1.2 Extract and reuse the existing Loop cycle contribution/absolute-boundary authority in bounded export layer reads, repeating captured dry once per scheduled pass with no dry in initial lead-in or final wet tails; verify independent dry/wet sample-sum oracles for single/multiple passes, long negative wet and tails, overlap, final-tail preservation and absence of an unscheduled next pass, plus existing live compositor regressions.
- [x] 1.3 Cover exact Duration, End on Signal and End on repeat extents, including non-whole-bar and fractional-period cases at 44.1/48 kHz and nonintegral tempo; verify many-repeat spacing does not accumulate rounding drift and document coordinate rules in the living export technical guide.

## 2. Worker delivery, gain and readiness

- [x] 2.1 Integrate Loop plans with existing waveform measurement, preview, atomic file preparation, Drag and Save; verify decoded WAV/steady preview sample parity at Dry/Wet 0/100, 100/0, 100/100 and 35/65, zero-branch dependency behavior, warned missing dry, silent 0/0 readiness, transparent calibrated gain, truthful overload warnings and bounded memory/zero callback allocation tests without applying the live peak guard.
- [x] 2.2 Add narrow validated optional Loop preference/revision metadata including the authoritative Dry/Wet pair while preserving Song values and immutable export history; verify fresh Wet Layer defaults, legacy/new round trips, invalid metadata, stale session/pass/generation completion, delayed replacement, Save chooser cancellation, disk failure and partial-publication rejection with focused export engine tests.
- [x] 2.3 Admit Export from accepted Loop capture-local material independently of live clock/Phrase/Cue; add workflow and engine tests for Clock-only accepted takes, unavailable live sync, compatible file delivery with unavailable preview monitoring, no automatic playback and unchanged Song eligibility; document the two timing authorities.

## 3. Native Export UI approval and integration

- [x] 3.1 Revise and launch a single native Loop Export preview using existing shared controls and current production tokens, reconciling the epic Export mockup with the owner's confirmed Dry/Wet revision; show Repeat, Lead-in grid, shared Mix Character and Dry/Wet faders, WAV start, Loop Bar 1 and adjacent placement/dry-doubling help at 900x600 and an expanded size, with component tests for layout, preset/custom pair bindings, disabled explanations, focus and stale intent.
- [x] 3.2 Obtain explicit owner approval of the native Loop Export appearance and interaction before rollout; record approval and any scoped family/variant exception with rationale and evidence, updating applicable standards/tests together if needed.
- [x] 3.3 Integrate the approved Loop presentation in the existing Export workspace, leaving Song controls unchanged; verify component/editor mode switching and current-versus-retained revision behavior, and run scripts/check-ui-standards.ps1 without regenerating the debt inventory.
- [x] 3.4 Use maintain-user-guide to document Loop export with Mix Character and independent Dry/Wet levels, Wet Layer defaults, dry-doubling guidance, grid placement, repeat/tail behavior and clear timing/readiness distinctions, including host appendices; verify offline guide links and currency record and update relevant living design/export guidance.

## 4. Integrated acceptance and review delivery

- [ ] 4.1 Validate a proven Clock-only Rack capture through finite export with live recurrence unavailable, then import Drag and Save results into FL Studio and compare placement, repeat spacing, initial lead-in, final tail, and Dry/Wet balance against preview for wet-layer and dry-inclusive mixes; use install-test-vst3 for any host build installation and record exact revision, host versions, results and remaining limitations in dated docs/validation evidence, keeping physical DPI qualification in #130.
- [x] 4.2 Run scripts/build.ps1 -Configuration Release -Test and required OpenSpec/UI checks on the final implementation; record the full native result separately from fixture/host evidence and resolve any regression before marking delivery ready.
- [ ] 4.3 Open a review-ready PR referencing #128 against feature/loop-mode with scope, automated/native/host evidence and known limitations; verify base/head and required CI checks and leave the PR open for owner review without merging.

## Workflow follow-up

- Sync and archive the implemented OpenSpec change only after validation and the requested review workflow; merge only with explicit owner authorization.
- Close #128 only after its reviewed PR is merged. Keep #116 open until integrated acceptance succeeds; #129 and #130 remain separate work.
- After completed delivery, apply task-owned regenerable-output cleanup while preserving durable assets, evidence, active reviews, host fixtures and rollback bundles.

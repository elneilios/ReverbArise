# Issue #108: IR Level range investigation

2026-11-06. Planning only. Inspected baseline: `717c890c4121cdfc84e20c988dedf7a6da0bc811` (merged #80 / PR #109), equal to fetched `origin/main` at investigation. [Issue #108](https://github.com/elneilios/ReverbArise/issues/108) has no discussion or existing issue PR. Only the unrelated completed waveform change was active in this checkout.

## Recommendation

Use **-12 to +12 dB for the main dial sweep**, linear in dB, neutral/reset **0 dB**, coarse **1 dB** and fine **0.1 dB** steps. Retain exact numeric and absolute accessibility entry over **-60 to +12 dB**, including arbitrary valid saved float values. Call it per-swell **Level**, as the current UI does. It is not an IR asset trim.

A hard -12 dB semantic minimum is not justified. At the approved energy +6 dB baseline, -12 dB gives approximately -6 dB wet/dry short-window intensity on matched fixtures; -24 dB gives -18 dB for a substantially subtler transition. That latter musical preference is a use case, not a new listening result. Uncalibrated hot imports also provide a concrete technical reason to retain deeper attenuation. Existing mute/Enabled controls exclude a swell; they do not substitute for a quiet audible transition. -60 dB remains finite, not digital mute. No new mute control is needed.

The proposed sweep offers three times the angular travel per dB (24 versus 72 dB span), with 0 dB centered instead of at 5/6 of the sweep. Existing step sizes and exact numeric entry already provide precision; no skew, default boost or coarser numeric quantization is needed. Interaction approval remains pending.

## Verified current semantics

| Concern | Finding and code location |
| --- | --- |
| Definition | `Source/Core/SwellSession.h:43`: `Shape::gainDb`, float default 0. `SwellSession.cpp:152` validates finite [-60,12]; no interval/step constraint on stored values. |
| Creation/reset | `SwellSession.cpp:86` creates `initialShape`; `PluginInspector.cpp:68` resets only Level to that shape's 0 dB. Reset wet shape also resets timing/fades/placement; it is a separate broader action. |
| Main UI | `PluginEditor.cpp:233` names `Level (dB)` / `swellLevel`. Perform's selected Sound section places it after the shared IR field (`:500`). It is not a child of `ConvolutionIrEditor`. |
| Mapping/edit binding | `PluginInspector.cpp:145,189,229,263`: relative clamp [-60,12], finite signed numeric validation, `(gain+60)/72`, domain span 72, accessible range [-60,12], coarse step 1. `:106` applies Shift factor 0.1. |
| Shared interaction | `Source/UI/KnobValue.h`: normalized linear dial [0,1], relative pinned gesture, Shift velocity mode, wheel/key steps, exact text entry, double-click/context reset and accessible numeric/string writes. Compact text displays one decimal for Level, but the exact entry string preserves extra precision. Current accessibility numeric writes go through relative deltas. |
| Learn / #80 | `Source/UI/ConvolutionIrBinding.h` distinguishes future-default IR and selected-swell IR. `Session::defaultIR/defaultIRName` contain an IR identity, not Level. New swells start at 0 dB independently; editing a Perform Level does not alter Learn defaults or existing neighbours. |
| Gain stage | `Source/DSP/SwellShape.cpp:15-25`: envelope applies `10^(gainDb/20)` and saved calibration separately, linking stereo. This occurs after raw reverse convolution. Level does not alter the IR or raw samples. |
| Import | `Source/DSP/ReverseConvolution.cpp:18` decodes finite mono/stereo WAV (or forwards WavPack), and preparation resamples without runtime gain normalization. `SwellAssetStore.cpp:108` writes the decoded IR. Authored factory gain and import recording level remain intact. |
| Calibration | `GainStaging.cpp:35` measures neutral default wet and local dry; approved strongest-channel 20 ms RMS, 1 ms hops, up to 100 ms local dry, +6 dB target, -60 dBFS floors, baseline bounds [-60,+24]. Quiet/unavailable/capped references are disclosed. Editing Level does not remeasure. |
| Output controls | `Session::liveWetDb` is a separate monitoring gain; Musical range [-60,+12], Legacy [-60,0]. Legacy audition trim/blend are separate. Export Dry/Wet are independent linear [0,1] final-composition gains (`SwellSession.h:111`), not this Level range. |
| Protection | Musical positioned wet sum uses -1 dBFS guard; Legacy retains -6 dBFS semantics. Audition can also guard its dry+wet sum. Protection can mask further boosts and is not IR normalization or destructive clipping of raw renders. |
| Render/cache | `SwellEngine.cpp:226-260`: Level is a shape change, increments shape generation, stops audition and invalidates eligible live shaped output, not source/IR input generation. `SwellTransport.cpp:19` member/wave keys include actual dB, calibration and raw identity. Reuse current workers and accepted/superseded publication rules. |
| Persistence | `SwellSession.cpp:312,348,452`: current session v11, shape `gain` stores actual dB and parse casts to float before validation. Existing plugin wrapper remains unchanged. Calibration, frozen raw references and Level are independent durable values. |
| Host automation / presets | No `addParameter`, AudioParameter or APVTS registration is present in the inspected `Source` tree. Processor reports non-parameter state changes (`PluginProcessor.h:104`). `PluginProcessor.cpp:464` exposes one no-op program. Thus this Level has no host parameter ID or normalized automation mapping. Host project/plugin state still saves actual Level. |
| Undo / bulk | No Level undo/redo stack exists in inspected source. #9 is future bulk interaction/history; require it to preserve actual before/after dB values and share validation, without implementing it under #108. |

The current canonical `swell-gain-staging` spec explicitly promises typeable [-60,+12] and nondestructive gain. The proposed main sweep preserves that numerical contract. No changes to calibration, audible defaults, raw assets or other gain controls are proposed.

## Measurement and listening evidence

Reuse the bounded production-generated comparison from 2026-11-02 rather than rebuild an unchanged engine in this new worktree. The retained `C:/git/k7ma-preverb/out/gain-staging-listening-v2/metrics.json` has SHA-256 `050b931fa9f4698568a9fc4db256fca3ba98e072aefce36619a7352bb7ceeb34`, exactly matching [the recorded evidence](gain-staging-evidence.md#source-energy-extension--2026-11-02). It covers six factory IRs plus imported chamber at original/quiet/hot recording gains and a sparse imported response, with quiet/strong chords and other synthetic music. The original tool is `tests/gain_listening_review.cpp`, using production importer/convolution/calibration/shaping/protection. Factory hashes and source PCM identities are recorded there. The source file is warm retained evidence, not a newly built current binary.

Today reanalysed 14 rows: Hall, Monumental, Whisper, imported chamber, its +20/-20 dB recording variants, and imported sparse, each with quiet and strong chords. [Retained selected measurements and derived values](2026-11-06-issue-108-measurements.json) include source hashes and original candidate rows, so the report remains reviewable without the machine-local full metrics. [Reanalysis script](2026-11-06-issue-108-measure.py) checks the source hash and applies scalar Level offsets analytically; it does not render audio or rerun current C++.

| IR | Baseline, quiet / strong (dB) | Unprotected Musical peak at Level -12, quiet / strong (dBFS) |
| --- | --- | --- |
| Hall | 7.319 / 7.319 | -38.978 / -10.034 |
| Monumental | -1.096 / -1.096 | -38.427 / -9.484 |
| Whisper | 9.763 / 9.763 | -36.239 / -7.296 |
| Imported chamber | 8.487 / 8.487 | -39.616 / -10.672 |
| Imported chamber +20 dB | -11.513 / -11.513 | -39.616 / -10.672 |
| Imported sparse | 13.863 / 13.863 | -35.951 / -7.008 |

All twelve matched rows give wet/dry attack intensity approximately -6 dB at Level -12 (error below 0.001 dB); none needs isolated wet protection at that setting. At -24 they give approximately -18 dB. Quiet and strong source levels remain distinct. The quiet-recording chamber rows deliberately fail/cap exact matching: at -12 their relative intensities are -34.487/-10.487 dB. Do not generalize the matched result to floors, caps, edited envelopes or arbitrary source material.

For the hot chamber/strong chord, raw default wet peak is **+12.840 dBFS**. With Legacy/unavailable calibration, Level -12 leaves **+0.840 dBFS** before protection, still **6.840 dB above the Legacy -6 dBFS ceiling**. Level -24 instead gives -11.160 dBFS, below that ceiling. This is an analytical Legacy projection from the measured raw peak, not a newly executed Legacy playback test. It demonstrates why import normalization must not be assumed and why -12 is not a universal lower bound. Floors, caps and correlated overlaps further limit universal conclusions.

Listening evidence is the owner's recorded 2026-11-02 selection of energy-plus-6 across the v2 comparison, including the quiet/strong/imported cases ([approval](gain-staging-evidence.md#listening-approval--2026-11-02)). That approval supports retaining the present baseline. It does **not** approve this new sweep, establish -12 as a universal musical minimum or constitute listening to today's derived -12/-24 values. No fresh subjective listening is claimed. Use one short native interaction/listening checkpoint during Apply for those values; do not repeat the entire accepted calibration review.

## Compatibility and coordinated scope

- Existing values below -12 are valid and exercised (for example -60 in `tests/gain_staging_test.cpp:31`); the entire finite [-60,-12) interval is supported persisted state. No owner DAW project/library scan was performed, so prevalence is unknown. Do not infer that no existing sessions use it.
- A naive load clamp raises -30 to -12 by 18 dB (7.943 times amplitude), or -60 to -12 by 48 dB (251.189 times), before protection. Protection would not preserve the old relative balance. Retain actual floats without a migration or automatic write on editor refresh. Frozen and Legacy sessions retain their policy, measurements and assets.
- Main sweep saturation must never change actual value. Exact text and accessible value remain true; extended numeric writes, cancellation, stale intents and save/reopen need explicit checks. Do not persist dial fractions or round loaded values to coarse/fine steps.
- #80 is merged at the inspected baseline. #79 is a tracker for incremental DSP; #9 owns future bulk/history. Neither authorizes adding Level defaults, effects or a second DSP panel. Open PR search returned #31 (asset/shared-engine planning) and #102 (timing diagnostics), with no competing Level implementation. Local #107 control-system planning is an integration risk to reconcile before Apply; no assumption of its approval or implementation is made.
- No host Level automation migration exists today. Add a processor-interface assertion during implementation; if concurrent work adds an exposed parameter, stop and design sound-preserving mapping rather than changing normalized interpretation implicitly.

## Plan and validation status

[OpenSpec proposal](../../openspec/changes/archive/2026-11-07-focus-swell-level-adjustment/proposal.md), [design](../../openspec/changes/archive/2026-11-07-focus-swell-level-adjustment/design.md), [tasks](../../openspec/changes/archive/2026-11-07-focus-swell-level-adjustment/tasks.md). The delta addresses `swell-gain-staging`; canonical specs remain current behavior. All implementation tasks are unchecked and require a new Apply request.

Executed: source/spec/docs audit, issue/related PR discovery, retained-metrics SHA-256 verification and 14-row analytical reanalysis. No production source or user guide semantics changed. No new native build, MSVC/CTest, FL Studio test, new listening session or DPI approval occurred. Implementation checks cover endpoints/midpoint/reset, precise and invalid entry, extended legacy round trips/frozen restoration, accessibility, unchanged raw/cache dependencies, Learn/Perform/instance isolation and equivalent admitted audition/live/plugin WAV/offline gains; actual future results must be recorded separately.

Planning verification passed: `openspec validate focus-swell-level-adjustment --strict`; `openspec status --change focus-swell-level-adjustment` reports 4/4 planning artifacts, with all 8 future implementation tasks unchecked. `git diff --cached --check` passed. A local link audit verified all 10 links in the investigation/change documents; numerical assertions verified all 12 matched rows are within 0.001 dB of -6 dB relative attack at Level -12 and need no isolated wet guard there.

Reproduce the bounded analysis from the retained historical full report:

```powershell
python docs/validation/2026-11-06-issue-108-measure.py C:/git/k7ma-preverb/out/gain-staging-listening-v2/metrics.json docs/validation/2026-11-06-issue-108-measurements.json
```

The default nvm shim initially failed because it had no active Node version. Reused installed Node `v26.13.0` by prepending its existing install directory to the command-local PATH; no downloads or global configuration changes. Git branch operations required normal sandbox escalation because worktree metadata lives in the main checkout; refresh and issue branch creation subsequently passed. No native build cache was created or cleared, and no speedup is claimed.

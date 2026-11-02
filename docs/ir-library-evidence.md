# Bundled IR library implementation evidence

Change: `expand-bundled-ir-library`; issue #7. Prototype listening and final
library-specific FL Studio acceptance are approved; see the latest results below.
Earlier checkpoint statuses are retained as history.

## Compatibility baseline (2026-11-02)

Before implementation, Debug compatibility CTest passed 4/4 in 44.47 s:
`preverb_swell_ui_test`, `preverb_swell_engine_test`,
`preverb_swell_session_test`, `preverb_processor_test`. The engine/session fixtures
exercise imported managed IRs, exact edited shapes/placement, frozen definitions,
version-7 serialization/restore, independent defaults and missing assets. Processor
fixtures also cover legacy state and multi-instance export. These are native
automated fixtures; no new FL Studio test has been claimed.

Baseline commit: `489bc0ad2f0c6921800a9287f2459fc2c8ae46e4` (includes UI commit `1f69712`). Unchanged host identity: Preverb Spike,
`com.k7ma.preverb`, K7Ma/Prv1, zero registered host parameters, version-7 state.
Hall WAV SHA-256: `b7275a27fc7629a9a1a7191abda4fbb3c8a46372dc9071e36dc1112e4839b458`.
Hall PCM SHA-256: `52b531d5bd0d77e94770b7796a034798349b05c3d80a71466c070e49cdc91648`.
Hall format: 48000 Hz, stereo, 16-bit PCM, 86400 frames (1.8 s).

Exact binary identities and rollback copies are in ignored
`out/ir-library-baseline/baseline.json` and its adjacent files. This is a local
baseline, not an installed build or GitHub release.

Reproduce using Visual Studio 18 Community's CMake/CTest executables under
`Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin`:

```powershell
& $ctest --preset windows-msvc-debug -R '^preverb_(processor|swell_engine|swell_session|swell_ui)_test$' --output-on-failure
```

## Progress and acceptance

Prototype audio must be approved before shipping content revisions are locked.

### Catalogue implementation

The descriptor-driven decoder and ID-based import are implemented with legacy
Hall adapters. `preverb_swell_asset_test` verifies original decoded content,
wrong WAV checksum, wrong format, wrong decoded checksum, insufficient budget
and unknown IDs without replacing output. Engine coverage includes explicit
catalogue success and pinned generation/session/freeze rejection, existing
WAV/WavPack imports and custom future-default isolation. Initial default install
also uses actual available memory, not the former single-Hall assumption.

After the catalogue edits, native UI/engine/assets passed 3/3 (11.67 s).
After explicit preset assignment and available-budget default installation,
automatic coordinator/engine/assets passed 3/3 (17.24 s). Debug builds of these
targets and the review utility passed. These are focused tests, not a new full
Debug/Release suite. No installed VST3 has been replaced.

Descriptor/provenance/compatibility guidance and field-to-test mapping:
`docs/ir-library.md`. Header declarations and legacy entry points retain the
existing Hall/default API behavior; source state has not changed format.

### Prototype v2 generation and measurements

Python 3.10.6 stdlib-only generator, explicit LCG32 seeds and all construction
parameters are recorded in `docs/ir-library-prototype-manifest.json`. The new
IRs remain ignored `out/ir-library-prototypes/*.wav` imports, not shipped assets.
The manifest records exact packaged/PCM/canonical hashes, 48 kHz stereo PCM24,
duration, peak/RMS/energy/DC, first-order spectral energy indicators, cumulative
energy crossing times, L/R correlation and mono fold-down. This is a provisional
revision, not a listening-approved release identity. Original procedural audio
and original synthetic fixtures are CC0, as declared in `scripts/IR_AUDIO_LICENSE.md`.

`python tests/ir_generator_test.py`: passed (13.312 s). Generates all five twice,
compares full WAV/manifest identities, independently decodes PCM24, verifies
measurements, endpoints, valid samples, duration ranges, mono compatibility and
unchanged original Hall. It uses temporary directories and no numerical package.

`preverb_ir_review` successfully used actual production convolution, shaping and
audition protection for 6 responses x 4 synthetic sources. Full output length
N+M-1 and absolute start are asserted; full/raw, initial 20 ms Smooth release and
common 250 ms pre-roll measurements are recorded. Both wet-only and 50% dry/wet
examples and separate forward references are exported. All 102 review WAVs were
independently checked for complete stereo 48 kHz PCM24 RIFF payloads without
stale append bytes. No non-finite/clipped export sample was admitted by the tool.

Calibration report: `docs/ir-library-calibration.json`; complete report and WAVs:
`out/ir-library-listening/`. Authored gains changed only generator content;
runtime importer/resampler/monitoring gains are unchanged. Median active RMS
differences from Hall across piano/pad/voice-like/pluck are +0.04, +0.04, +0.03,
-0.04 and -0.05 dB (Monumental/Plate/Dark/Shimmer/Whisper respectively).
Source-specific differences remain substantial: pad deltas are approximately
+11.1/-10.5/+8.8/+5.1/-25.8 dB. These are disclosed candidates for listening,
not an assertion of equal perceived loudness or acceptance of these outliers.

Local review: `out/ir-library-listening/index.html`, built with
`python scripts/build_ir_review_page.py out/ir-library-listening`. The page
offers source/mode selection and individual players, plus fixed-origin 60 s
comparison files with dry events at 12/27/42/57 s. Shimmer is labelled static.

**Pending gate: task 2.4, user listening approval.** No release checksum locking,
five-preset packaging, six-choice UI or accepted-audio handover is claimed yet.
Current work stops at this gate. Final listening/host/performance tasks remain
unchecked; no issue comment/close, commit, push or installation was performed.

### Listening feedback and spatial revision 3 (2026-11-02)

The user accepted the length variation but rejected the similar character of
the first candidates, asking that larger responses such as Monumental have more
size, width and presence. **This is not approval of task 2.4.** Revision 2 is
retained under the original ignored output folders for comparison. No factory
assets were locked or packaged.

Revision 3 changes the offline synthesis structure: Monumental is dominated by
long fixed damped delay networks/allpass diffusion plus separated lateral
reflections; Plate uses much shorter networks and a narrow image; Dark Hall
uses damped room modes; Shimmer emphasizes static resonances with a broad image;
Whisper stays narrow/airy. This authors static WAV responses only: no runtime
effect/control or real-time DSP was added. Durations remain 10/1/4/7/0.5 s.

After calibration, median full-response active RMS deltas relative to Hall are
+1.23/+0.03/+0.02/-0.31/-0.04 dB. Source-specific level differences are still
reported in `docs/ir-library-calibration.json`; these are not listening passes.
Piano wet side/mid energy for Monumental is approximately +3.2 dB versus Plate
at -15.1 dB, a measured spatial contrast rather than a claim of subjective size.
All 24 protected wet fixture fold-downs lose less than 6 dB relative to mean
channel energy. Shimmer width was reduced after the initial pad fixture showed
excessive side dominance. Mono fold-down still changes stereo level; no claim
of mono/stereo loudness equivalence is made.

The generator's updated repeated-generation/PCM/measurement/mono tests passed
(17.916 s), including a >10 dB wide-versus-compact IR side/mid contrast oracle.
The unchanged production review utility rendered 6 responses x 4 sources and
all 102 exported PCM24 WAVs have valid complete RIFF payloads. No native runtime
source changed in this feedback round; previous catalogue tests are separate.

Current prototype manifest: `docs/ir-library-prototype-manifest.json`, generator
`ir-library-prototype-v3`. Current review is **out/ir-library-listening-v3/index.html**
with manually loadable IRs in **out/ir-library-prototypes-v3**. The unversioned
folders are historical revision 2. Task progress stays **7/28**, paused at the
same listening checkpoint; subjective character/width approval remains pending.

### Partial listening approval and targeted revision 4 (2026-11-02)

User feedback: Monumental is long enough but too gentle, and needs more swell,
width, volume and intensity; Vintage Hall remains too similar to original Hall.
The user says the others are good and Shimmer is interesting. Record **Plate,
Whisper and Shimmer accepted at their exact revision-3 WAV hashes**, while
Monumental and Vintage Dark Hall remain under review. Task 2.4 stays unchecked;
progress is still 7/28. This is partial sound acceptance, not release packaging.

Only those two candidate sounds were revised. Monumental retains 10 s but uses
more diffusion stages, fewer dominant isolated reflection peaks, denser body,
an authored onset-energy emphasis that becomes a reverse climax, higher gain
and broader stereo. Vintage Hall retains 4 s but uses longer damped room modes,
low resonant body and a two-pole 850 Hz final filter to darken even its early
reflections. It is calibrated near Hall's median level so tone can be compared.

The prior +/-3 dB median authoring target is deliberately exceeded for
Monumental in response to the user's explicit louder/more intense creative
request: its raw/shaped median active level is +6.04 dB over Hall, before
monitoring protection; final approval of this intentional difference is pending.
Existing runtime monitoring protection and live gain rules are unchanged.
Monumental's actual protected final-second RMS changes against revision 3 are
[4.0, 1.5, 5.9, 4.4] dB for piano/pad/voice-like/pluck respectively. These numbers are
objective evidence, not a claim that the user has accepted the dramatic effect.

Generator tests passed (23.568 s): repeatable PCM/manifest generation, independent
PCM24 measurements, endpoints, duration, width/mono checks, unchanged default,
and exact hash assertions for all three accepted responses. The actual renderer
again produced 102 complete PCM24 files. All 24 wet fixture mono fold-down losses
remain below 6 dB (Monumental worst approximately 5.53 dB); no clipped/nonfinite
export sample was admitted. No native runtime or VST3 build changed this round.

Current generator: `ir-library-prototype-v4`. Current listening page:
`out/ir-library-listening-v4/index.html`; current IRs:
`out/ir-library-prototypes-v4/`. Previous revision audio players are included for
the two revised sounds. Older entries above are historical. Current manifests
and calibration JSON are refreshed; no factory content is locked, and no commit,
push, issue mutation or installation was performed.

## Full collection listening approval (2026-11-02)

The user explicitly selected "Approve both revised sounds; continue" for revision
4 Monumental and Vintage Dark Hall. Together with the recorded revision-3
Plate/Shimmer/Whisper approval, all five sounds and levels are accepted.
Task 2.4 is complete. Exact approved WAV hashes are pinned in the packaging
tool; full-rate, resource, native UI, live handover and FL Studio integration
acceptance remain separate, unchecked work.

## Approved revision-1 packaging

All five accepted WAVs, measurements and redistribution declaration are now in
`resources/irs/`, with generated catalogue entries in
`Source/DSP/BundledImpulseEntries.inc`. The package tool pins exact user-approved
hashes and refuses to overwrite an existing release with differing bytes.
`python scripts/generate_ir_library.py --output out/ir-release-reproduction`
followed by `python scripts/package_ir_library.py --source out/ir-release-reproduction --check`
passed: all WAV/PCM identities and package metadata match; original Hall remains
unchanged. `scripts/IR_AUDIO_LICENSE.md` is included with the installed resource
manifest; full native/runtime/rate tests follow separately.

## Native integration evidence

The six-entry inspector preserves the existing widget/layout. An actual native
selection of Whisper switches only the selected swell and then restores the
prior session; custom assignment remains a seventh entry when appropriate.
The pending label shows retained/playing sound first and the requested sound,
with complete names/error in its tooltip. Native images under
`out/ui-ir-library-final/` include `ir-replacement-pending.png`; the 1100x850
pending view was visually inspected, and the snapshot run passed (17.63 s).
No new effect controls or host parameters were introduced.

`preverb_ir_handover_test` verifies accepted A survives failed and superseded
requests, all six assignments preserve unrelated IRs/defaults/source/shape,
managed restore, and a shorter response cannot silently clamp an explicit crop.
Reset wet shape and explicit Full response recover genuine new-IR bounds.
Activity, solo, placement, envelope, source, deletion and restore revoke
retention. Oversized computational requests report budget failure without
consuming accepted output. Frozen assignment remains rejected.

`preverb_transport_playback_test` includes an independent five-sample handover
oracle at 1 kHz: new PCM joins the actual Song offset through the existing
<=5 ms transition, with unchanged neighbouring samples and zero callback
allocation/deallocation. Existing random-block, seek, loop, retirement-pressure
and protection oracles remain intact.

`preverb_ir_streaming_test` paces 512-frame callbacks at 96 kHz during actual
long response resampling/FFT work with an overlapping neighbour. It requires
continuous prepared pages and zero callback allocations, verifies unchanged
neighbour assets and no reconvolution on identical content, and starts a cold
bounce during real pending work to compare the accepted snapshot numerically.
The initial test exposed repeated full-wave peak scanning; cached verified
peaks are now reused, while file integrity is checked. Long jobs service pages
between processing stages with workspace reservations and earlier headroom;
resource pressure aborts replacement explicitly, preserving accepted audio.

Library same-content installation now reuses only bounded, parsed, verified
managed PCM, including sample identity and exact decoded comparison. New or
corrupt assets still use immutable unique-file publication. No old cache file
is overwritten or silently repaired in place.

One legacy in-flight-save fixture expected identical IR content to become stale;
it now requires Ready without increased render count. A legacy live fixture
expected mute during identical IR import; it now checks the exact retained
samples and unchanged render count. The Release bounce verifier also now clears
its dry input instead of reusing a previous live-output buffer. No audio, source
integrity, state or ownership oracle was relaxed to accept arbitrary output.
Final full-suite and build identities are recorded below after completion.

## Final Debug integration and measured costs

The complete Debug build and **28/28 CTest cases passed (91.63 s)** after the
retained-output and identical-PCM fixture updates. This includes the existing
processor's recapture/tolerance, live seek/loop/final-event, silent input,
callback-allocation, legacy/current state and two-instance export regressions.
The added source-commit race and missing retained-IR cases pass without reviving
old source. No fixture is being accepted merely because its timeout was raised.

Actual Debug metrics are committed as `docs/ir-library-metrics-debug.json` and
`docs/ir-streaming-metrics-debug.json`. Library rows cover all six responses at
44.1/48/96 kHz using a 32-frame sparse/asymmetric source and a full O(M) numerical
oracle, including all final samples; mono mapping has an independent reference.
Installation timings refer to original-rate managed assets (48 kHz), while
preparation/render timings are at each project rate. They are not whole-plugin
host CPU guarantees.

For Monumental, decode took about 56 ms; preparation/render took approximately
753/737 ms at 44.1 kHz, 9/840 ms at 48 kHz and 1532/1736 ms at 96 kHz. Conservative
workspace estimates were about 98/99/130 MiB. Reported process peak working set
is cumulative within the test executable, not an isolated allocator measurement.
The complete 96 kHz paced engine test with overlapping long swells took about
5.29 s from first assignment to playable versus 350 ms for repeated identical
content, with no additional convolution for repetition. It processed 990 paced
blocks without missing pages; sampled engine accounting peaked at 146487008
bytes and process peak working set was 92377088 bytes. Process measurements
include the test harness and reference buffers; engine accounting includes
conservative working reservations. Both are labelled explicitly.

The paced test's cache grew from 15670235 to 35640190 bytes. This includes two
separate Monumental render preparations (the second after restoring the old
snapshot to test a pending bounce), not a claim of 20 MB growth per identical
assignment. The Monumental managed IR itself is 3840104 bytes. Actual Release
measurements and final bundle identities follow after verification.

Native/persistence/resource test traceability: catalogue and ownership requirements
map to `ir_library_test`, `ir_handover_test`, `ir_streaming_test`, the actual
selector interaction in `swell_ui_test`, and the existing processor/export/state
fixtures. Approved sound/provenance hashes map to generator tests and immutable
packaging checks. Subjective listening is recorded above; final FL Studio plugin
integration (tasks 6.2/6.3) and final all-evidence acceptance (6.4) remain pending.

### Final build and packaging verification — 2026-11-02

Final configured Debug build and full CTest: **28/28 passed, 91.63 s**.
Release library, retained handover, paced streaming and native UI tests:
**4/4 passed, 11.43 s**. The separate optimized Release transport playback
numerical/callback test also passed (**1/1, 0.05 s**). Commands used:

```powershell
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug --output-on-failure
cmake --build out/build/windows-msvc --config Release --target preverb_plugin preverb_ir_library_test preverb_ir_handover_test preverb_ir_streaming_test preverb_swell_ui_test preverb_transport_playback_test
ctest --test-dir out/build/windows-msvc -C Release -R '^preverb_(ir_library|ir_handover|ir_streaming|swell_ui)_test$' --output-on-failure
ctest --test-dir out/build/windows-msvc -C Release -R '^preverb_transport_playback_test$' --output-on-failure
python scripts/generate_ir_library.py --output out/ir-release-reproduction
python scripts/package_ir_library.py --source out/ir-release-reproduction --check
python scripts/package_ir_library.py --check
```

Clean reproduction and immutable packaging checks passed. Both built bundles'
IR manifest and license match the checked-in resource files. Existing WavPack
and font licenses remain packaged. Managed asset tests verify reopen without
the original external file and verify immutable same-content reuse.
No system plugin installation was performed.

Exact final binary identities are in `ir-library-build-validation.json`:

| Build | Binary bytes | Growth versus recorded baseline | SHA-256 |
| --- | ---: | ---: | --- |
| Debug | 41658880 | 7877632 | `267d51f9ad52bb47c516826d71525e29d7a49f42e36519cce48dd30d5426d24c` |
| Release | 16982528 | 6500352 | `0c8ca6b71e2898eb40ad22b39dc479d4fa285a075dfa41910738f7d9f55e22a6` |

The five added WAVs total 6480220 bytes. Per-asset identities remain pinned in
`resources/irs/manifest.json`; the original Hall hashes are unchanged. Plugin
identity, zero host parameters and version-7 state are preserved; no migration.

Release paced 96 kHz retained-plus-pending measurements are in
`ir-streaming-metrics-release.json`: first assignment to playable 1411.39 ms,
repeat identical assignment 153.95 ms with no reconvolution, 263 paced blocks
without lost pages, peak sampled engine accounting 144758520 bytes and process
peak working set 87015424 bytes. Cache sizes match the Debug fixture above.
These are fixture measurements, not FL Studio performance guarantees.

### Remaining host acceptance

Use the whole Release bundle at
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/Preverb Spike.vst3`.
Record the host version, sample rate/buffer, trim/routing and this build identity.

- Task 6.2: hear all five approved characters in the plugin; switch IRs during
  playback, check retained/requested status and failure recovery, overlapping
  long swells and the documented negative-time lead-in limit.
- Task 6.3: use two instances with different bundled/imported IRs, frozen and
  shaped swells; check seeks, loops/final events, WAV export during a pending
  choice, and save/reopen with both contributions intact and no duplication.
- Task 6.4: complete the final requirement/evidence review with those actual
  host reports. Automated success does not mark these host results complete.

Implementation and automated verification satisfy tasks 5.5/6.1. Final host
acceptance remains outstanding; do not archive on automated evidence alone.

## Final evidence review — 2026-11-02

The integrated build includes the separately approved musical gain policy and
shared-amplitude display. Its Release SHA-256 is
`9ef65e3de3ccbb09276933bf2f385bfe8af309c5b841c721afa06a3d024ffb91`.
Earlier binary sizes/hashes and version-7 compatibility statements describe the
library-only checkpoint; version-8 state and runtime musical calibration are
documented in `gain-staging-evidence.md`. The library does not introduce that
state migration, alter the approved WAVs, or reinterpret immutable assignments.
Authored IR measurement numbers remain unchanged; their stale pending-listening
metadata has been corrected to reference actual approvals.

Requirement/scenario evidence audit:

| Requirement area | Objective and acceptance evidence |
| --- | --- |
| Distinct musical uses | Revision-3/4 feedback and approvals above; duration/format oracles in `ir_generator_test.py` and `ir_library_test.cpp` |
| Reproducible provenance | Checked generator/parameter manifest, CC0 declaration, repeated-generation/independent PCM measurements and immutable package check |
| Immutable saved identity | Descriptor checksum/format failures in `swell_asset_test.cpp`; managed restore, defaults, independent assignments and frozen gating in `ir_handover_test.cpp` and engine/UI fixtures |
| Authored levels and untouched PCM | `ir-library-calibration.json`, approved listening fixtures/hashes, rate/channel convolution oracles; subsequent runtime policy covered separately by gain-staging evidence |
| Full linear response and resource limits | `ir_library_test.cpp` all six responses at 44.1/48/96 kHz, N+M-1 and mono/asymmetric reference samples; streaming memory/cache reports and explicit rejection tests |
| Real catalogue choices and async UI | Native six-choice/custom selector, pinned chooser/deletion/restore/freeze tests in `swell_ui_test.cpp`; requested/retained status snapshots and engine handover errors |
| Playback dependency validity | Retention revocation on source/shape/placement/mapping/activity/restore and missing-asset tests in `ir_handover_test.cpp`; existing processor recapture/export regressions |
| One bounded accepted contribution | Superseded B/C, failed requests, budget rejection, independent bounce snapshots and pending-version numerical tests in handover/streaming fixtures |
| Safe absolute-position handover | Independent <=5 ms transition sample oracle, unchanged neighbour, seek/loop and allocation/lifetime tests in `transport_playback_test.cpp`; paced 96 kHz streaming |
| Musical/operational readiness | Approved sound hashes, Debug/Release cost/build reports and prior full-suite evidence; library-specific host questions 6.2/6.3 remain explicitly pending |

Strict change and all-spec validation passed (11 items, zero failures).
`python scripts/package_ir_library.py --check` verifies all five approved
revision-1 assets and unchanged Hall. No new sound generation or DSP change
was needed for this final review. Recent gain-stage host approval is not silently
substituted for pending IR-switch/failure/export-specific questions.

Integrated Release library/handover/streaming/UI/transport CTest passed **5/5,
12.38 s** during this review. Whitespace checks passed. Task 6.4's review is
complete; tasks 6.2 and 6.3 stay unchecked pending the requested host results.

## User library host results — 2026-11-02

The user reports **Pass—all tested** for all six sounds at useful levels and
live IR switching including Monumental, old audio retained during preparation,
smooth handover and unaffected overlapping neighbours. They report
**Pass—including pending-change export** for two instances with different
bundled/custom assignments, shaped/frozen swells, save/reopen, seeks/loops/final
events and accepted-version consistency during a pending-choice bounce. Task
6.3 is complete. These are user-reported results for the delivered integrated
Release build identified above, not agent-operated host tests.

The user confirms the setup/version is unchanged from prior screenshots:
FL Studio 2026 v26.1.7 build 5653, Windows 64-bit, FL Studio ASIO at 48 kHz and
512-sample buffer. Routing/trim were not individually restated; no change was
reported. No independently checked installed-binary hash was provided.
Failed-replacement check passed: the user's screenshot shows a decode error,
`Retained Shimmer Cloud | Requested fail-IR.wav`, and the user confirms the
previous swell continues during DAW playback. The user also explicitly reports
**Pass** for long wet lead-in before Song sample zero: playback starts with the
available portion, without delaying dry or replaying the missing lead-in.
Task 6.2 is complete; all 28 library tasks are now checked. No remaining host
acceptance is inferred solely from automation. No archive, commit, push or issue
closure is performed by recording these results.

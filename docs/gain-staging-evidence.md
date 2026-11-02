# Musical gain-staging evidence

## Legacy baseline — 2026-11-02

Captured before version-8 or musical-gain implementation. The repository was
already carrying the completed `expand-bundled-ir-library` implementation; this
baseline deliberately includes its accepted-IR handover and export behavior.

Focused Debug tests:

```powershell
cmake --build --preset windows-msvc-debug --target preverb_swell_session_test preverb_transport_playback_test preverb_audition_mix_test preverb_processor_test
ctest --test-dir out/build/windows-msvc -C Debug -R '^preverb_(swell_session|transport_playback|audition_mix|processor)_test$' --output-on-failure
```

Result: **4/4 passed in 25.47 s**. These fixtures cover version 1–7 restore,
default and edited shape/gain, imported and frozen assets, overlapping live wet,
accepted/pending IR handover, irregular offline blocks and independent multiple
processor instances.

The numerical legacy playback oracle remains:

- aggregate wet ceiling: `10^(-6/20)` at 0 dB user wet gain;
- overlapping member bound: sum of whole-member peaks (2 + 3 produces ceiling/5);
- non-overlapping member bound: largest peak (2 and 3 produces ceiling/3);
- live user wet range: -60 to 0 dB, ramped over 5 ms;
- audition wet and final sum guards: -6 dBFS before -60 to 0 dB trim;
- live dry remains unchanged in Dry + Wet, while Wet Only suppresses it only
  during enabled playing transport.

State fixtures verify exact numerical Level, fades/tensions, placement,
activity, routing/trim and managed asset references. Versions 1–4 restore the
then-absent live controls to enabled Dry + Wet at 0 dB. Versions 5–7 retain
their saved live values. Restore never starts audition or transport playback.

Asset WAV SHA-256 identities before gain work:

| Asset | SHA-256 |
| --- | --- |
| Preverb Hall | `b7275a27fc7629a9a1a7191abda4fbb3c8a46372dc9071e36dc1112e4839b458` |
| Monumental | `0799b724acba5b66f3e933109a79454673da944d022f2ac13a8d6cb2a0532148` |
| Intimate Plate | `fad9e920013101794af9050fbf88d784f65ccb7fd65523e963998a2853b10c1c` |
| Vintage Dark Hall | `1f86a72cbf8fcdae256ea8a8765705fd227ae1adaf94594908c80fd2f370d65c` |
| Shimmer Cloud | `16d1ea58c5db5ae1bf110ceea30da6a88276092a40bb34f25745f72f7d9eee7f` |
| Whisper | `59f013118f8daccb87b93c9b125c385d620f50e72e710bede6eedcd93d5387c7` |

Raw render identity is dependency-addressed and remains unchanged by shape or
monitoring gain in the existing asset/session tests. Subsequent musical-policy
tests must keep these legacy fixtures and hashes unchanged.

## Provisional musical calibration candidate

The candidate is deliberately not the shipping fresh-instance default until
the listening gate is approved. It measures the stereo-linked sample peak of
verified captured dry audio from the target for at most 100 ms, shortened by
source coverage, the selected source end, or the next confirmed target. It
compares that with the creation-default shaped wet peak at neutral Level and
zero placement. Both floors are -60 dBFS, the target offset is 0 dB, upward
calibration is capped at +24 dB, and the provisional wet aggregate ceiling is
-1 dBFS.

The measured baseline is saved separately from the user's Level value and raw
convolution. Fixed candidates use the same post-convolution baseline slot.
Level therefore remains a neutral 0 dB relative control. Fixed candidates
preserve authored inter-IR level differences; source matching intentionally
reduces peak differences without changing PCM, decay, stereo character, or
perceived-loudness dynamics. Versions 1-7 remain on legacy gain/protection;
version 8 carries a strict policy, revision, and per-swell measurement identity.

Musical aggregate preparation streams 4096-frame stereo chunks across the full
accepted support, is cancelled on stale generations, and uses the existing
256 MiB availability accounting. The immutable key includes accepted render,
shape, placement, calibration revision/amount, activity/solo membership and
path wet level. Page changes reuse that result; tests report build count, cache
hits, last preparation milliseconds and measured peak while retaining the
audio-callback allocation/lock/file-I/O guards.

Debug two-instance export measurement (2026-11-02): instance A built 3 plans,
reused 3 cached plans, and its last scan took 1.410 ms for a 0.125 peak;
instance B built 3, reused 2, and took 0.873 ms for a 0.250 peak. The existing
dense live publication fixture used 61,020,080 bytes and the sparse fixture
9,332,848 bytes, both below the unchanged 268,435,456-byte shared budget.

## Listening comparison v1

`preverb_gain_review` generated `out/gain-staging-listening-v1/index.html` and
`metrics.json` through production reverse convolution, creation-default shaping
and audition protection. The 282-file, 251,615,997-byte review covers four
original CC0 synthetic sources (transient chord, sustained pad, hot voice-like
attack and busy overlapping plucks), all six approved factory responses and a
representative imported stereo chamber response. Every response/source exposes
Wet Only and 50% Dry + Wet for legacy, fixed +3/+6/+9 dB and source matching.

Across 28 response/source cases, fixed +3/+6/+9 reached the provisional -1 dBFS
ceiling in 2/8/13 cases respectively. Source matching requested baselines from
-35.07 to +12.44 dB, never reached the +24 dB cap in this fixture, and did not
need aggregate protection; busy duplicate swells remain an explicit overlap
case. Metrics include reference/default-wet peaks, calibration state, prepared
peak/RMS, wet-to-dry peak ratio, branch/sum attenuation and ceiling occupancy.
All six approved factory WAV SHA-256 identities still match the legacy table.
No candidate is approved merely because these files exist or peaks match.

## Source-energy extension — 2026-11-02

Implemented `candidate-source-energy-v1` separately from the existing immutable
peak prototype. The strongest per-channel RMS window supplies one gain for
both stereo channels. Initial tuning: dry reference up to 100 ms, 20 ms RMS
window, 1 ms hop including the final valid window, common shorter window when
necessary, -60 dBFS RMS floors, -60 to +24 dB calibration, initial target 0 dB,
and provisional -1 dBFS sample-peak protection. Wet measurement includes the
creation-default envelope and production 2 ms edge ramps. Rolling energy uses
constant scratch memory; cancellation is checked every 4096 frames.

New version-8 energy metadata stores dry/wet RMS, independent sample peaks,
effective window/hop and measurement identities. Existing peak metadata keeps
its original revision and semantics. Explicit musical adoption prepares the
energy candidate; fresh-instance defaults remain pending listening approval.
Legacy processing explicitly excludes saved musical calibration even when
that metadata is retained for a later return to musical mode.

Automated evidence: energy and peak numerical oracles at 44.1/48/96 kHz;
IR-scale compensation, source-level tracking, short-window stereo/reference
bounds, silence/floors, boost caps, cancellation, polarity and silent padding;
exact -60/-6/0/+6/+12 dB Level ratios before protection. State tests cover
energy roundtrip and transactional rejection of missing measurement fields.
Engine lifecycle verifies adoption without reconvolution, baseline retention
through edits/reset, freeze/save/reopen and policy switches. Processor tests
pass musical two-instance irregular offline bounces and exact return to the
original legacy output. Focused state/energy/engine tests pass 3/3 (8.28 s),
with handover, audition and processor tests passing in the preceding run.

The revised review uses six factory responses and four imported fixtures:
dense chamber at original and +/-20 dB recording scales, plus a sparse stereo
response. Six sources include quiet/strong versions of the same chord and a
two-member positioned overlap. It compares fixed +6 dB, peak matching and
energy targets 0/+3/+6 dB in Wet Only and 50% Dry + Wet. Imported responses
are written as float WAVs and loaded through the production importer; output
writing rejects nonfinite/over-full-scale samples instead of clipping them.
Source PCM hashes and factory hashes appear in metrics.json. Source filtering
and per-row cap/unavailable/protection labels support listening comparisons.

The zero-target chamber case at -20/0/+20 dB recording levels produced wet
peaks -13.61565684/-13.61565746/-13.61565684 dBFS respectively. Its quiet IR
copy reaches the +24 dB baseline cap at energy +3/+6, which is disclosed rather
than claimed to be an exact match. All factory WAV hashes remain identical.

Final review: `out/gain-staging-listening-v2/index.html`, 10 IR cases x 6
sources x 5 methods x 2 modes = 600 verified audio links. The final metrics
SHA-256 is `050b931fa9f4698568a9fc4db256fca3ba98e072aefce36619a7352bb7ceeb34`.
The additional neighbour-reference lifecycle test passes: inserting/removing a
nearby target changes/restores reference bounds and calibration with no new
convolution. Strict OpenSpec validation and whitespace diff checks pass.
Listening approval (task 4.2), default pinning and final host acceptance remain
pending. No generated comparison is recorded as subjective approval.

## Listening approval — 2026-11-02

The user explicitly selected `energy-plus-6`: "energy-plus-6 sounds good in
all playbacks to my ears". This approves the v2 comparison's +6 dB energy
target and -1 dBFS wet ceiling. Pin release revision
`musical-source-energy-plus6-v1`: 100 ms maximum local attack, stereo-linked
maximum per-channel 20 ms rectangular-window RMS, 1 ms hop including the last
valid window, common shorter window where needed, -60 dBFS RMS floors, -60 to
+24 dB baseline bounds, and +6 dB target offset. The source fixture identities
and final metrics hash are recorded above. The reference is short-window
intensity; spectral balance and duration can still affect perceived loudness.
Level stays at 0 dB relative to this baseline with -60 to +12 dB adjustment;
cap/ceiling limits remain disclosed. This is listening approval of the review
audio, with FL Studio plugin acceptance still pending.

## Final implementation verification — 2026-11-02

The approved revision is pinned and fresh plugin construction selects it.
Legacy migration and supported prototype revisions preserve their prior output;
the old numerical processor fixtures explicitly select Legacy. The approved
calibration produces bit-identical baseline/RMS results to the reviewed +6 dB
candidate in the independent 44.1/48/96 kHz oracle (sample ratio tolerance
1e-5; state metadata roundtrips exactly). All six factory WAV hashes still
match the baseline table. Neutral Level, freeze/reopen, explicit adoption and
positive-gain legacy rejection pass native engine/UI checks.

Full configured Debug build and CTest: **29/29 passed, 103.20 s**. Focused
Release gain/state/UI/engine/library/handover/streaming/transport/audition/export
build and CTest: **10/10 passed, 26.49 s**. A corrupt incremental Debug COFF
object was resolved by rebuilding its generated DSP library before the final
successful build. No test deadline, callback guard or memory budget was raised.

The fresh-default resource regression now exercises musical preparation:
gain scans merge occupied support, skip empty Song gaps and share verified
reads for identical shapes. An isolated translated waveform's exact peak is
reused; real overlaps still scan aligned samples. Restore coalesces expensive
gain scans while reporting capacity errors promptly. Gain keys include actual
saved shape values and calibration/asset identities rather than relying on
runtime generation counters alone.

Release resource accounting: dense 60,819,840 bytes, sparse 10,108,504 bytes,
budget 268,435,456 bytes. Two-instance gain scans last took 0.5333/0.5182 ms,
with three cache hits each and measured peaks 0.198642/0.397284. At 96 kHz,
the long-IR streaming test prepared first/repeat assignments in 1217.61/84.10 ms,
with 229 paced blocks free of missing pages and sampled accounted peak memory
144,758,520 bytes. The retained Release test log is
`out/gain-staging-release-ctest.log`; native final screenshots are under
`out/gain-staging-approved-ui/`. Main workspace geometry remains unchanged;
Diagnostics fits the separate requested/calibration/protection measurements.

Release VST3 bundle:
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/Preverb Spike.vst3`.
The binary inside `Contents/x86_64-win/Preverb Spike.vst3` has SHA-256
`b38fcf2c786638ca88ee618c1a1900361b307e0643473a068963171756a8d24a`.
Strict OpenSpec validation and final diff checks pass. FL Studio acceptance
(task 6.2) is the remaining gate; follow `docs/musical-gain-staging.md` and
record host, rate, buffer, routing/trim, Level travel, loops/seeks, two-instance
WAV export and save/reopen results. No host acceptance is inferred from tests.

## Waveform and knob feedback verification — 2026-11-02

The audible shaped wet now determines its own display scale. Raw convolution
references are independently scaled and subdued; a raw peak increase from 100
to 10,000 leaves the aqua waveform's pixel height unchanged. Display scaling
does not change audio gain or envelope geometry. Native previews are retained
under `out/audible-wave-knob-fix/` (`ready.png` and `audible-wet-hot-ir.png`).

Shared knob value boxes update during dragging without resetting the gesture
anchor. Text entry restarts after Return without requiring a focus change,
and the Level parser accepts explicit positive values such as `+6`.
Debug and Release native UI checks pass, including continuous drag feedback,
repeated typed entry, exact entry, fine adjustment and cancellation checks.
The focused Release CTest passed in 2.03 s.

The updated Release VST3 at the bundle path above supersedes the earlier binary:
17,029,632 bytes; SHA-256
`79a22ad72d649491bc2bd3862f1b35b9ef422269202441e60ba4a77bb25e6ea5`.
FL Studio acceptance remains open.

Follow-up: neutral-Level display anchoring replaces per-Level peak normalisation,
which previously hid Level changes. The aqua wave now has 6 dB of visual headroom
at Level 0 dB. Release UI pixel regressions verify shrinking at -6/-60 dB and
growth at +6 dB, alongside the unchanged hot-raw-IR independence check. Release
UI CTest passed (2.05 s); Release VST3 rebuilt. Latest binary SHA-256:
`af343d29c169b213ca491d4b2b1a90fe16e7da253373378dab1db7ab35d79a52`.

Shared-amplitude trial supersedes both preceding independent wet scales. Source
detail/overview data in the source lane and shaped wet now use identical gain,
default 1, controlled by display-only Wave zoom (-24..+48 dB). The separate Song
Overview and subdued raw reference retain their contextual scales. Native pixel
oracles compare equal 0.5 source/wet amplitudes, shared +6 dB zoom, -60/-6/+6 dB
Level with stationary source height, and hot raw peak independence. Release UI
CTest passes (1.97 s); native previews generated and compact ready layout reviewed
under `out/shared-wave-scale/`. Audio processing and gain policy are unchanged.
Latest Release binary SHA-256:
`9ef65e3de3ccbb09276933bf2f385bfe8af309c5b841c721afa06a3d024ffb91`.
User trial/FL Studio acceptance remains pending.

## User host acceptance — 2026-11-02

Following delivery of the shared-amplitude trial build above, the user approved
the display and reported **Pass** for each of the three explicit check groups:

- Default musical impact in a fresh instance using factory/custom IRs and
  quiet/strong transients; useful down/up Level adjustment subject to protection.
- Playback through loops/seeks and overlapping/final events, without glitches or
  continual regeneration; WAV export with two plugin instances.
- Persistence of edited Levels/frozen swells through save/reopen and the grouped
  Legacy compatibility/explicit musical-adoption check. No untested exception was
  reported in the reply.

Host: FL Studio. Exact version, sample rate, buffer, audition trim and Live wet
level/routing for this acceptance run were requested but not supplied. Earlier
screenshots are not treated as confirmation of this run's configuration.
The referenced delivery is the shared-amplitude Release build with SHA-256
`9ef65e3de3ccbb09276933bf2f385bfe8af309c5b841c721afa06a3d024ffb91`;
the user did not independently report a binary hash. These are user-reported
results, not a claim of agent-operated FL Studio tests. Task 6.2 is accepted;
the prior pending statuses above describe earlier checkpoints.

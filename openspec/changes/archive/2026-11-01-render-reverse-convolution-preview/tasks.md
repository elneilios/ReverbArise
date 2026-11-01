# Tasks

## 1. Revision-Bound Source Reading and Selection

- [x] 1.1 Add a committed-manifest interval reader that decodes each intersecting chunk once, copies only selected frames, and retains explicit silence; verify exact stereo values across partial chunks, adjacent chunks, silence, negative starts, and non-contiguous ranges without per-sample WAV reads.
- [x] 1.2 Reject incomplete coverage, missing/corrupt assets, non-finite samples, invalid intervals, and position/length overflow before publishing render input; verify failures name the affected interval and leave committed capture data unchanged.
- [x] 1.3 Add fixed-tempo/meter bar-to-sample selection using one host timing snapshot, independent of tone controls; verify exclusive end, late-playhead/early-selection alignment, non-4/4 meter, values beyond bar 512, unavailable timing, and rate mismatch. Document rounding and fixed-tempo limitations in `docs/development.md`.

## 2. IR Loading and Preparation

- [x] 2.1 Add background mono/stereo WAV IR decoding with finite-sample and format validation and immutable IR identity; verify valid PCM/float WAVs, mono duplication policy, stereo isolation, invalid/empty/multichannel files, and file-dialog cancellation preserving prior selection.
- [x] 2.2 Implement offline IR sample-rate conversion with compensated origin and the documented prepared-length convention; verify equal-rate identity, 44.1/48/96 kHz conversions, duration rounding, impulse alignment, preserved leading silence, and downsampling rejection.
- [x] 2.3 Add checked render-memory estimates including retained output and scratch, enforce the 256 MiB M2 budget without truncation, and document supported IR/channel behavior; verify oversized and overflow inputs fail before large allocations and without replacing the previous completed preview.

## 2a. User-Requested WavPack Import

- [x] 2.4 Add pinned static libwavpack decoding and content-based `.wv`/`.wav` dispatch with existing channel, gain, finite-sample, integrity, and memory safeguards; verify integer/float and Unicode-path fixtures, equivalent WAV decoding, corruption/truncation, resource rejection, and shipped decoder license.
- [x] 2.5 Extend the chooser and M2 documentation/specs for WavPack, validate the supplied large-hall IR read-only through decode and convolution, and rerun affected tests; record actual file metadata/results without redistributing its audio.

## 3. Headless Reverse-Convolution Engine

- [x] 3.1 Add the `preverb_dsp` CMake target and full zero-padded FFT convolution of source with reversed prepared IR; verify exact N + M - 1 length, P - (M - 1) origin, P + N exclusive end, singleton inputs, and negative lead-in.
- [x] 3.2 Add an independent double-precision direct-convolution oracle and numerical comparisons within the design tolerance; verify asymmetric signals, impulses at both ends, silence, seeded noise, non-power-of-two lengths, stereo isolation, and equivalence to reverse-source/convolve/reverse-output.
- [x] 3.3 Exercise long IRs, FFT-padding boundaries, finite output, inverse scaling, and cancellation checkpoints; verify complete final samples and no latency shift, and document engine input/output, tolerance, and resource behavior alongside the DSP tests.

## 4. Render Coordination and Validity

- [x] 4.1 Add a dedicated render worker with immutable source/IR requests, request IDs, phase progress, and complete-result publication; verify the selected committed revision stays consistent during recapture and that errors/superseded jobs never install partial or obsolete output.
- [x] 4.2 Track the selected source intersections and input/timing identity; verify overlapping recapture, selection/IR changes, capture Clear/restore, and rate/tempo/meter changes mark affected previews stale and stop audition, while unrelated capture commits preserve a current result.
- [x] 4.3 Implement cancellation and worker/control-thread result retirement, keeping the last completed preview visibly distinct on failure; verify rapid Generate/replace/restore/shutdown sequences and document progress, error, and stale-state transitions in the development guide.

## 5. Manual Wet-Only Audition

- [x] 5.1 Add a pinned-buffer audition voice with a sequential cursor and nonblocking command/acknowledgement handoff, including reliable Stop intent under command pressure; verify start-at-zero, complete natural playback, restart, bounded Stop ramp, and buffer retirement outside the audio callback.
- [x] 5.2 Mix the wet contribution only after dry capture, preserving live dry input and diagnostic-tone behavior; verify sample-exact dry passthrough, silent dry capture while audition is audible, block-size seams, no host seek/loop retrigger, and silence after completion/Stop.
- [x] 5.3 Verify invalidation/rate changes stop preview safely and old buffers cannot restart after Clear or restore; add a callback-allocation/deallocation guard for audition-path tests and document that stopped-transport audition requires the host to process the insert.

- [x] 5.4 Address the reported loud large-hall preview with audition-only peak attenuation to -6 dBFS, no quiet-render boost, a -60 to 0 dB level slider defaulting/resetting to -12 dB, smoothed live adjustment, and separate render/audition peak display. Verify high-peak playback, default/clamped levels, immutable render, unchanged dry/capture, allocation-free smoothing, editor layout, and all regressions; document host retesting.

## 6. Plugin Controls and Aligned Preview

- [x] 6.1 Add Load IR, IR identity/format display, separate source start/end bars, Generate, progress/error status, Audition, and Stop; verify control availability for empty/loading/rendering/ready/stale/error states and UI synchronization after host state restore.
- [x] 6.2 Add a wet waveform lane sharing the source overview's absolute sample/bar scale, extending for the entire negative-time lead-in; verify peak positions and view bounds headlessly and expose source/result bounds, full frame count, rate, and unclipped peak warning for host inspection.
- [x] 6.3 Keep M2 IR/range/result state session-local while preserving existing version-1 migration and version-2 capture/tone restoration; verify reopen clears jobs and audition without embedding PCM or changing saved capture identities, and document regeneration after reopen.
- [x] 6.4 Write `docs/fl-studio-m2-validation.md` with a reproducible custom/synthetic IR workflow and separate source/tone controls; verify the instructions name the current build path, fixed-tempo scope, protected audition level, silent/stopped-host caveat, and all host checks below.

- [x] 6.5 Fix the reported flattened wet waveform by fitting over-full-scale peaks with a labeled display-only scale, preserving raw PCM and audition gain; build and run regression checks.

## 7. Integration and FL Studio Acceptance

- [x] 7.1 Build the Windows Debug VST3 and run all existing and new CTest targets; verify M1 capture/persistence/tone/ruler regressions remain green and strict OpenSpec validation passes, recording the automated result in the M2 report.
- [x] 7.2 In FL Studio Song mode at fixed tempo/meter, select a fully captured region, load mono and stereo WAV IRs, Generate, and inspect source/wet bar alignment including a result before song start; verify rejected gaps/invalid IRs and record host settings, visual results, and render progress responsiveness.
- [x] 7.3 In FL Studio, verify Audition/Stop and full wet-only playback with transport stopped and routed-silent input; test replay, seek/loop independence, live dry passthrough, mapping without preview feedback, and zero dropped capture blocks while generating. Record whether the host processes the stopped insert and any manual fallback.
- [x] 7.4 In FL Studio, recapture an overlapping and an unrelated interval, change IR/selection, and save/reopen; verify stale handling, regeneration, preserved source assets, no automatic audition, and session-local preview reset, recording user acceptance separately from automated checks.

## Workflow follow-up

- Review the M2 host results and accepted limitations before syncing specs and archiving this change.
- Continue marker/envelope editing in M3 and Song-position swell playback in M4 as separate changes.




# Issue #71 diagnostic investigation and owner checkpoint

Scope: draft [PR #102](https://github.com/elneilios/ReverbArise/pull/102),
`support-existing-tempo-maps`, tasks 1.1–1.2 only. The owner explicitly requested
artifact revisions and a bounded diagnostic/host investigation. Production
timing policy, canonical specs and plugin/state identities remain unchanged.
Tasks 2 onward require the separate task 1.3 checkpoint; no merge/closure is
authorized.

## Rubber-duck revisions

The proposal/design/specs/tasks now explicitly preserve fresh fixed-tempo affine
editing, distinguish callback-partition comparison from ramp precision, define
independent affine-calibrated run offsets without translating PCM, persist scoped
conflict fences and explicit recovery, cover upstream tempo edits and seeks into
wet lead-in/tails, and define mixed legacy/new authority joins. Capacity loss
aborts the pending source/evidence transaction; missing host fields are ordinary
unavailable records. The design expresses the proposed 1,048,576-record cap as
recording durations, including just 11 min 39 s at 32 frames/48 kHz and 1 h 33 min
12 s at 256 frames. These resource/admission rules are proposed, not implemented
or owner approved.

## Implemented diagnostic harness

The opt-in `REVERBARISE_TIMING_TRACE_DIR` path creates a per-instance raw CSV.
Its fixed 65,536-record prefix and SPSC worker ownership are independent of
source/evidence acceptance. It records the actual `getPosition()` result before
existing admission, with optional-field flags and raw PPQ/BPM/bar/cycle values,
sample/rate/frames and diagnostic lifecycle IDs. CSV formatting/file access stay
on a worker. It contains no audio or project names. Normal launches remain
untraced. No mathematical tempo map or replacement timing policy is implemented.

See the [collection protocol](../development/tempo-timing-trace.md) and scripts
for scratch-host launch and descriptive trace summaries. A closed trace is
complete only with a final `omitted=0 io_failed=0` summary. The diagnostic prefix
cap does not claim production evidence capacity or full-map coverage.

## Native verification

Fresh native MSVC Debug build through `scripts/build.ps1 -Configuration Debug`
passed using existing dependency/build trees; changed objects were rebuilt.
The first build found a test-only invalid StringArray method, corrected before
verification; no production guard was changed. Passing build logs include
`out/build-logs/Debug/20261106-023657-09eb8eac/` and final error/lifecycle checks at
`out/build-logs/Debug/20261106-024408-d4139a13/`.

- `preverb_timing_trace_test`, `preverb_host_grid_test`, `preverb_processor_test`:
  **3/3**, 45.07 s (`out/issue-71-focused-debug.log`). The new trace test took
  1.88 s and covers SPSC pressure/concurrent transfer, exact raw step/ramp tuples,
  missing PositionInfo/PPQ, seek/reset/offline/Playing states, signed range
  overflow, 65,536-row saturation, CSV/summary integrity and zero callback
  allocations/deallocations. Instrumented/uninstrumented processor instances
  produce identical dry samples and retain the same existing tempo rejection.
- `preverb_transport_time_test`, `preverb_transport_playback_test`: **2/2**,
  0.32 s (`out/issue-71-transport-debug.log`), preserving #73 clock/audio guards.
- After diagnostic resource-error handling and stronger lifecycle assertions,
  all five affected tests passed again: **5/5**, 58.38 s
  (`out/issue-71-final-debug.log`); trace fixture 1.93 s. File I/O/lock separation
  was checked by source review; allocation/deallocation assertions run on the
  recording caller, independently of worker formatting allocations.
- Strict change validation, UI standards and read-only user-guide currency
  passed after explicit semantic review. No new UI control family/token or
  production timing behavior was introduced; musician guide prose is unchanged.

These are synthetic/native results, not FL Studio measurements. Full Debug and
Release CTest suites were not rerun for this diagnostic-only stage; task 7.1
remains incomplete. Release VST3 configure/build passed through the local-test
installer (`out/build-logs/Release/20261106-024005-e0d618de/`); initial system
installation was denied by Windows ACLs and retried through the prescribed
elevated installer with retained logs/exit status. The first UAC prompt was not
visible to the owner and Windows reported cancellation; one informed retry
succeeded. Final Release configure/build/install logs:
`out/build-logs/Release/20261106-024616-a20ee3a9/` and
`out/issue-71-elevated-install.log` (exit 0).

Installed system bundle: `C:\Program Files\Common Files\VST3\ReverbArise.vst3`.
Its x64 DLL SHA256 is
`5559EAB4EFBD495E05CCFDA2E34BF24ADFD4F4B712F7EDBE2C06DE27DA8C54C7`.
Retained previous bundle:
`out/vst3-install-backups/20261106-024634-e2783e707e274583bf6fd4b9bb335b7c/ReverbArise.vst3`.
This local build reports base `beb1492` modified; the
[metrics/source hashes](2026-11-06-issue-71-host-metrics.json) identify the exact
diagnostic source inputs and tested DLL rather than claiming a clean build.
FL process module inspection verified the loaded system DLL path. The installed
diagnostic build remains in place, untraced on normal launches.

## Host measurements

FL Studio **26.1.7.5654 x64**, pinned JUCE **9.0.4 VST3**, **48 kHz**, **4/4**,
FL Studio ASIO V1.05, default input/output. The copied installed Basic template
under ignored `out/issue-71-host/` used an empty eight-bar pattern and Master-slot
ReverbArise. There was **zero audio input**, no prepared wet and no added
latency-producing effect. This measures timing exposure, not wet playback or
tempo-map feature acceptance. Actual capture's fixed-tempo checks remain active.

The device was initially **512 frames**, changed to **256** for the second
curved/live and step cases, then restored to **512** before normal host exit.
Audio settings showed output + plugins latency **3017 samples / 63 ms** at 512,
**2880 samples / 60 ms** at 256; these are displayed device/chain totals, not an
independent measurement of plugin PDC. Multithreaded generator/mixer processing
and Smart disable were enabled; Align tick lengths and Reset plugins on
transport were off. WAV bounces used Full song, Leave remainder, 16-bit stereo,
512-point sinc, HQ plugins and master/insert effects enabled, Trim PDC silence
enabled. Upload was off. No deliberate nonzero-plugin-PDC variation was tested.

The [compressed raw CSV](2026-11-06-issue-71-host-trace.csv.gz) retains **51,013
records**, with `omitted=0 io_failed=0 queue_bytes=7340048`. Raw SHA256:
`7DB9B2114A692DA8B0AA59E76A53091142710F66E815FA4B62EFA0E4ED35043C`.
The summary script also verified the closing count equals the parsed row count.
Scratch FLPs/WAVs remain local; the committed trace contains numeric observations
only. Run IDs below are diagnostic IDs, not engine admission decisions.

| Case | Run | Observations | Observed BPM | Actual callback frames |
| --- | ---: | ---: | --- | --- |
| Constant Song baseline, 512 device frames | 2 | 7,258 playing | 130 only | 1–231; 231 distinct sizes |
| Linear up/down ramps, live at 512 | 3 | 9,301 playing | 130–161.25; 2,999 distinct | 1–231 |
| Same linear arrangement, offline | 4 | 3,074 offline | 130–161.25; 2,999 distinct | 186–231; 46 distinct sizes |
| Curved incoming segment, live seek/loops at 512 | 6 | 9,878 playing | 130–161.25; 2,985 distinct | 1–231 |
| Start at PPQ 12 inside curve, live at 256 | 8 | 5,489 playing | 130–161.25 | 1–231 |
| Held 130 to 161.25 step then falling ramp, live at 256 | 9 | 9,854 playing | 130–161.25; 1,537 distinct | 1–231 |
| Same held-step arrangement, offline | 10 | 3,074 offline | 130–161.25; 1,537 distinct | 186–231 |
| Curved arrangement, offline | 12 | 3,074 offline | 130–161.25; 2,985 distinct | 186–231 |

Other records are initial/parked/Stop/reset observations. All playing/offline
rows had sample/PPQ/BPM/meter availability flags in this experiment. Each bounce
had 3,072 Playing callbacks plus two nonplaying offline tail callbacks. Seeks,
automatic Song wraps and offline lifecycle resets were visible; origin/start and
mid-ramp start were both exercised. The 130 baseline's inferred affine origin
ranged approximately **-0.00000066 to +0.076923 samples**, consistent with the
existing one-sample guard in this trace. This does not establish universal host
rounding behavior or a nonzero offset for automated maps.

### What the measurements establish

- The host supplies varying boundary BPM/PPQ under real automation, with raw bar
  and musical cycle fields present. Device-buffer length is not the effect's
  callback size: realtime partitions subdivided further than offline partitions.
- Comparing **exact common absolute sample positions**, including repeated loop
  visits, gave 5,925 matched live rows for linear, 6,622 for curved and 5,149 for
  step. Respectively 3,376 / 3,256 / 4,705 live rows had no exact-sample offline
  counterpart. Maximum raw PPQ differences at matched samples were about
  **1.6621e-5 / 1.1971e-5 / 3.6900e-6** quarter notes; BPM matched at those
  points. These are descriptive differences, not approved ramp error bounds.
- At the held step, offline callbacks included
  `sample 354000 / PPQ 15.97916666666657 / BPM 130 / 231 frames`, then
  `354231 / 15.989596254006289 / 161.25 / 231`, then
  `354462 / 16.000025838460555 / 161.23199462890625 / 186`.
  Live further subdivided this region (for example 73 and 158 frames).
  A changed reported BPM thus precedes the configured PPQ-16 step in these raw
  observations: **do not identify the exact map boundary from a BPM field alone**.
  Its sample 354231 is inside a nominal 256-frame device period; the effect sees
  a separate callback there. Exact events inside an individual effect callback
  remain unobservable through this API and were not established.
- Linear offline timing reached
  `sample 631572 / PPQ 31.989576463136711 / 130 BPM / 231 frames`, then
  `sample 631803 / PPQ 0 / 130 BPM`; a further callback repeated sample 631803
  before the host reset processing. This is a real tail-origin reset observation,
  not evidence of an early wet failure or successful automated-map audio.

No complete future map, exact intra-block ramp law, general offset calibration,
later-map conflict algorithm or wet-audio parity has been proved by these
observations. Synthetic tests remain separate. No owner FL Studio action is
required to reproduce this **diagnostic matrix**. A deliberately latency-producing
plugin chain/other device and exact intra-callback automation would require
additional targeted host evidence if the owner wants those supported guarantees.
The future feature's audible capture/Generate/play/bounce/reopen smoke test is
still task 7.2 and cannot pass before implementation.

## Task 1.3 checkpoint

Tasks 1.1–1.2 are complete; **task 1.3 remains unapproved**.
The concrete decisions for owner review are:

1. Preserve today's fresh fixed-tempo affine Snap/ruler/entry contract. Automated
   regions use only independently justified local conversion precision; missing
   precision disables the affected musical operation while sample/time edits
   remain. Do not treat a reported BPM change alone as an exact region boundary.
2. Compare different partitions through supported authority, not interpolation.
   For a map run, calibrate a comparison-only sample offset on two accepted
   affine observations; do not extrapolate it from a ramp's current BPM.
   Unknown/uncalibrated ramp comparisons remain unverified, allowing independently
   safe sample audio only when no known fence exists. No new arbitrary tolerance
   is justified by the small measured PPQ differences.
   Where host uncertainty lacks a justified bound, even matched sample anchors
   do not prove a conflict from those PPQ differences. Detection of every later
   edit in an arbitrary ramp is therefore not promised; that limitation is part
   of this checkpoint, not silently waived acceptance.
3. Persist scoped conflict fences through seeks/Stop/bounce/reopen, conservatively
   including downstream effects of upstream edits and the affected swell's whole
   wet contribution. Clear only through explicit complete validated recovery.
   Mixed legacy/new source needs compatible join evidence, not an implicit upgrade.
4. Review the proposed **1,048,576 records AND 64 MiB** production snapshot cap.
   A future packed record must fit 64 bytes to achieve the design's durations:
   11 min 39 s at 32 frames, 1 h 33 min 12 s at 256, 3 h 6 min 25 s at 512
   (48 kHz, worst case). Larger records or existing coverage reduce that capacity;
   total old/new/index/staging memory still needs accounting before enablement.
   Overflow aborts the pending source/evidence transaction. The implemented
   diagnostic's measured **112-byte record, 7,340,048-byte queue, 65,536-row cap**
   is separate and retains a marked incomplete prefix on overflow.

These are reviewable proposals, not production changes or approved precision
guarantees. No later tasks will be implemented until this checkpoint is approved.

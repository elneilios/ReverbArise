# Tasks

All tasks are implementation or verification work; planning does not satisfy a checkbox. Use the three delta specs and design decisions for scope. Record objective evidence and actual listening approvals as work completes.

## 1. Compatibility baseline and catalogue

- [x] 1.1 Record baseline Hall WAV/PCM hashes, host/parameter identity, version-7 Hall/imported/frozen/edited session fixtures and current build/test commands; verify the fixtures reopen and render unchanged before library edits.
- [x] 1.2 Introduce the shared immutable catalogue descriptor and legacy Hall adapter with identity/revision/resource/format/checksum fields; verify real resource decoding, incorrect checksum/format rejection and unchanged Hall bytes/default installation.
- [x] 1.3 Replace single-Hall bundled import assumptions with explicit catalogue IDs and descriptor-based budget admission; verify ID/session/generation/freeze guards, file WAV/WavPack parity, unrelated/future-default preservation and no menu-index persistence.
- [x] 1.4 Document catalogue identity, provenance fields and compatibility policy alongside the baseline evidence; verify every descriptor field and preserved legacy path has a test/evidence reference.

## 2. Deterministic prototypes and musical calibration

- [x] 2.1 Add deterministic generation and measurement tooling for the five documented static designs, including seeds/algorithm/environment/rounding and stereo 48 kHz/24-bit output; verify repeated generation checksums, finite samples, endpoint behavior and declared duration ranges.
- [x] 2.2 Measure peak/RMS/energy/DC/spectrum/decay and stereo/mono compatibility for each prototype, and document original authorship with CC0 redistribution terms; verify manifests match decoded WAV measurements and contain no third-party source dependency.
- [x] 2.3 Generate licensed deterministic piano-like chord, pad, voice-like and pluck examples with dry/reference/reverse-wet variants using actual convolution; verify length/alignment and measured active-window calibration against Hall, reporting the +/-3 dB median objective and all outliers/protection gains without runtime normalization.
- [x] 2.4 Obtain user listening approval of five distinct reverse-swell characters and sensible levels, including controlled Plate/Whisper and long Monumental/static Shimmer; record actual approvals or revise prototypes and repeat measurements before claiming this task complete.
- [x] 2.5 Lock approved asset revisions/checksums, package their WAV resources and manifests/licenses, and document the reproduction command; verify a clean documented generator run reproduces shipped PCM/WAV hashes and the unchanged Hall asset.

## 3. Selection, persistence and support

- [x] 3.1 Populate the existing per-swell dropdown from verified catalogue descriptors with descriptions and the current imported entry; verify all six real choices, custom label, file icon, pinned selection/cancel/restore/deletion races and frozen gating without changing the approved minimum-size layout.
- [x] 3.2 Install selections through immutable managed assets while preserving saved source/arrival/shape/placement/name/activity and future defaults; verify independent two-swell assignments, same-content reuse, old/current reopen and label/order/content upgrades without silent substitution or a schema bump.
- [x] 3.3 Verify automatic full raw start, negative lead-in, genuine endpoint support and final-loop extraction behavior with long IRs; test a shorter response conflicting with an explicit crop and retain the edit with a clear error instead of silently clamping/resetting it.
- [x] 3.4 Add user guidance for preset character/static limits, authored gain, crops/Full response, old assignments and unchanged default/future-default scope; verify the documented actions against native selected/unselected/custom/missing/frozen states and request user review if layout must change.

## 4. Bounded accepted-audio retention and handover

- [x] 4.1 Separate one accepted verified audible snapshot from a requested IR-only generation; verify A remains eligible during B/C work, outdated B cannot publish, import/render failure retains A, and unrelated swells and manual preview behavior remain unchanged.
- [x] 4.2 Add the narrow retention ownership fence and revocation rules; verify source/shape/placement/mapping changes, invalid assets, deletion, disabled/mute/solo/live routing, freeze and session restoration cannot leak stale or ghost output or revive missing-source recovery.
- [x] 4.3 Prepare admitted old/new pages and swap at current absolute Song position using <=5 ms transitions/protection; verify numerical position, no lead-in replay, no steady duplicate sum, untouched overlapping neighbours, stopped behavior and callback allocation/lock/lifetime safety.
- [x] 4.4 Bound accepted-plus-pending preparation and page memory, coalesce rapid requests and retire ownership off the live callback; verify repeated long-IR choices and budget pressure preserve eligible old output with explicit diagnostics rather than truncation, unbounded retention or raised limits.
- [x] 4.5 Snapshot the accepted audible version for host offline bounce during pending IR work; verify no mid-bounce version switch, correct available contribution/warnings and independent two-instance export under irregular non-real-time blocks.
- [x] 4.6 Expose concise requested-versus-audible/loading/failure feedback in existing status/tooltip and document the IR-only exception; verify truthful native states and unchanged ordinary dependency invalidation through engine/UI tests.

## 5. Rates, levels and operational costs

- [x] 5.1 Run all new responses through 44.1/48/96 kHz preparation, mono-IR/stereo-source and asymmetric stereo channel cases; verify finite PCM, nearest-sample duration, preserved gain/origin and independent direct/reference convolution oracles including N+M-1 length.
- [x] 5.2 Exercise long-tail overlaps, negative start, covered silence/chunk boundaries, missing source and completed song-loop boundaries; verify exact placement, no fabricated dry pre-roll, no lost raw tails and no unintended dry duplication or discontinuity beyond documented numerical/ramp behavior.
- [x] 5.3 Compare authored and rendered levels against the approved fixtures and existing protection; verify stored PCM is not clipped/normalized/limited, no NaN/Inf or click-producing boundaries, disclosed raw/shaped/monitoring peaks and no unsafe preset-specific routine gain jump.
- [x] 5.4 Measure the longest stereo option at supported rates for resampling/render latency, first/repeated assignment, peak accounted/observed memory and cache growth, including retained-plus-pending stress; publish reproducible results and explicit resource rejection limits rather than treating planning estimates as measured performance.
- [x] 5.5 Measure actual Debug/Release bundle/resource growth and verify clean resource/license installation and managed reopen after the original external file is removed; record the exact asset and binary hashes and run documented reproduction/measurement commands.

## 6. Integrated acceptance

- [x] 6.1 Run configured Debug build/full CTest and focused Release library/UI/transport tests, resolving failures without weakening existing source/shape/state/freeze/export oracles; record commands/results and confirm unchanged plugin parameters/identity and no asset migration.
- [x] 6.2 In FL Studio, obtain user acceptance of actual library sounds, normal-level switching, last-good/pending/failure feedback, long overlaps and negative-lead-in limits on real musical sources; record build, host/rate/buffer, trim/routing and reported results, leaving unavailable checks open.
- [x] 6.3 In FL Studio, verify two instances with different catalogue/imported responses, frozen and shaped swells, seek/loop/final events, pending-choice WAV bounce and save/reopen; record heard/exported identity and the absence of lost or duplicated contributions.
- [x] 6.4 Review all requirement/scenario evidence, provenance/calibration/performance reports and actual listening approvals, then run strict change/all-spec validation and diff checks; leave any unverified musical or host result unchecked.

## Workflow follow-up

- Start implementation only after a new apply request; this proposal creates no implementation audio or code.
- Update exact task boxes as their implementation and verification pass. Keep prototype listening and final host checks explicit across usage resets.
- Keep #3/#6 and additional experimental responses outside this change. Do not post GitHub comments or close the issue without explicit authorization.
- Sync and archive only after required objective and user acceptance evidence is complete.

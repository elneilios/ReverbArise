# Design

## Context

See proposal.md for motivation and specs/reverse-convolution/spec.md for the behavior contract. This cross-module change needs a design because it combines disk-backed captures, offline DSP, UI commands, and real-time playback ownership.

M1 stores immutable stereo WAV chunks and manifests, with explicit silence and absent coverage. `CaptureWorker` owns its mutable `CaptureSession`; processor/UI access copied metadata snapshots. `CaptureSession::readSample()` decodes a complete chunk for each requested frame, so it is unsuitable for render input. `processBlock()` copies dry input to its SPSC capture queue before adding the diagnostic tone. `SongBarRuler` already uses host sample/PPQ anchors at fixed tempo/meter. There is no DSP convolution library or audition voice yet. CTest covers capture, state, tone, and ruler behavior.

## Goals / Non-Goals

**Goals:** Provide a reusable headless renderer, revision-consistent source extraction, explicit ownership of immutable preview audio, and plugin controls for the agreed single-result workflow. Preserve integer sample anchors and the existing capture pipeline's throughput.

**Non-Goals:** This change does not turn the capture worker into a general job runner, load full-song PCM, add a persistent render-asset format, implement host transport control, or replace the future Song-position scheduler with the audition cursor. Proposal.md lists the product exclusions.

## Decisions

### Separate rendering from capture

Add a dedicated background render coordinator rather than performing convolution in `CaptureWorker` or the message/audio threads. A job contains an immutable manifest/reference, source interval, timing snapshot, prepared IR identity, and monotonically increasing request ID. The reader uses a separate asset-store instance; mutable validation caches are not shared between threads.

Read only intersecting manifest ranges, verify their assets, and decode each relevant audio chunk once. Copy the intersecting frames into a contiguous, bounded source buffer; represent explicit silence with zeros. Require complete coverage of `[start, end)` and report the first gap or unavailable interval. Do not call `readSample()` in a per-frame loop or flush the capture worker synchronously from Generate: the UI explicitly selects the currently committed revision, with pending capture distinguished in its status. Immutable old assets keep the accepted job stable during recapture.

### Use sample-domain DSP with an independent oracle

Add a `preverb_dsp` library under `Source/DSP`. The engine consumes stereo source frames, a prepared mono/stereo IR, source start sample, and rate, and returns full stereo wet PCM plus timeline metadata. Reverse only the prepared IR, then compute ordinary full linear convolution. For source length N and prepared IR length M, return exactly N + M - 1 frames anchored at P - (M - 1); the exclusive result end is P + N. Check all length arithmetic and signed position overflow before allocating.

Use zero-padded FFT convolution with a transform length at least N + M - 1, reusing scratch buffers across channels. Pinned JUCE FFT facilities can supply the transforms; verify inverse scaling rather than assuming it. Process channels independently, with a mono IR reused for both. Full-buffer FFT avoids the latency/tail-flushing ambiguity of adapting a real-time convolution processor. Naive direct convolution remains a small, independent double-precision test oracle, not the production long-IR algorithm. No bundled IR assets are needed; WavPack input uses the pinned library described below.

Check impulse, asymmetric, singleton, zero, seeded-noise, stereo-isolation, and non-power-of-two cases. For direct-reference comparisons use a per-sample absolute error bound of `2e-5 * max(1, L1)`, where L1 is the sum of absolute convolution products contributing to that output sample; report maximum error and verify exact frame count and anchors separately. Include long-IR and FFT-padding cases, plus an equivalence test against reverse(source) -> convolve(normal IR) -> reverse(full output).

### Prepare imported IRs without changing their meaning

Decode mono/stereo WAV and lossless WavPack files off the UI/audio threads. Reject empty, unsupported-channel, invalid-rate, and non-finite input before rendering; cancel file selection without changing the previous IR. Apply mono IR independently to source L/R, or stereo IR L->L and R->R. Four-channel true-stereo IRs are outside M2 and are rejected explicitly. Preserve initial silence and gain; no normalization, tail trimming, fades, or predelay removal.

When rates differ, use an offline band-limited resampler with compensated filter delay and a defined origin at input sample zero. Set prepared M to `max(1, round(originalFrames * captureRate / originalRate))`, with zero-extension at boundaries. This length defines subsequent alignment. Test unity-ratio identity, converted duration, impulse origin, and downsampling rejection with synthetic signals. Record the resampler convention alongside the tests so a future engine change cannot silently shift swell anchors.

### WavPack import extension

The user requested native WavPack IR loading after the original WAV-only M2 build.
Statically link official libwavpack 5.9.0, pinned to commit
`5803634a030e2a11dba602ba057b89cc34486c67`, through CMake FetchContent. Disable upstream
programs, tests, extra decoder threads, assembly and DSD support in a scoped configuration
that does not change this project's CTest setting. Ship upstream COPYING in VST3 Resources.

Dispatch a `wvpk` header to libwavpack irrespective of `.wv` or `.wav` filename.
Open read-only with UTF-8 Windows paths and correction-file support. Decode in bounded
4096-frame blocks, converting signed PCM by its stored byte width or bit-preserving
32-bit float samples. No float normalization is requested. Check mono/stereo channels,
known positive length/rate, the existing memory budget, finite samples, complete frame
count and decoder integrity errors before publishing the IR. Reject DSD, unknown-length
inputs, and hybrid files lacking lossless correction data explicitly. Keep WAV decoding
and all downstream IR preparation/convolution unchanged. Test synthetic library-encoded
PCM/float fixtures and the supplied large-hall file read-only without redistributing it.

### Bound the single-preview workload

For waveform drawing, use display-only gain `1 / max(1, renderPeak)` and label the
scale in dB. Over-full-scale raw renders must fit the lane without a flattened
envelope. This is independent of audition attenuation and never changes PCM.

Use a proposed M2 working-memory budget of 256 MiB per instance for source, IR, output, retained previous result, and FFT/resampling scratch. Estimate with checked arithmetic before loading large inputs or allocating transforms. Show an actionable error when a request exceeds the budget; never shorten the source or IR to fit. Read waveform peaks as compact metadata and never copy the full song. The budget is an internal M2 limit to document, not a promise of full-song convolution.

Expose phase-based progress (source read, IR preparation, convolution, preview build) and allow supersession between phases/channels. Shutdown cancels and joins the worker outside the callback. Cancellation during a transform completes that bounded transform before returning; it never installs a partial result.

### Resolve source bars using a frozen timing snapshot

Add Source start bar and Source end bar (exclusive), initially 1 and 2, separate from the existing tone interval. Permit positive integer bar values without inheriting the tone sliders' 512-bar restriction; resolve to checked 64-bit sample bounds. At Generate, use the current valid host sample/PPQ anchor, capture rate, tempo, and meter to calculate both boundaries with one nearest-sample rounding convention. The host may be stopped and located anywhere in the Song. Display the resolved sample interval and source revision.

This is explicitly fixed-tempo/fixed-meter selection, matching the M1 ruler limitation. Monitor changes to the known tempo, meter, or rate and invalidate accepted jobs/results rather than silently stretching or relabeling them. A full host tempo map is unavailable in the current project; the UI must not claim support for it. Invalid timing/rate/ranges fail with a visible explanation.

### Publish by request identity and source dependency

Use states Empty -> Loading IR -> Ready to generate -> Rendering -> Ready, with explicit Error and Stale outcomes. Any change to selection or IR supersedes the active request. Capture dependencies record the intersections actually read: absolute bounds, coverage kind, asset checksum/path, and relevant revisions. Compare those intersections after a commit, so unrelated capture changes do not invalidate the preview. Overlapping replacement, Clear, state restore, host rate changes, or musical-mapping changes invalidate it. Conservatively invalidate on any overlapping revision even if its samples happen to match; content-diff regeneration is later work.

Retain the last completed buffer for inspection on failure, visibly labeled with its original dependency identity. If it does not match current inputs it is stale and cannot be auditioned. Check request ID and dependencies again before publishing a worker completion. Old completions never overwrite newer inputs. Retain buffers until the audio thread has acknowledged retirement.

### Audition is an independent playback voice

Audition starts from frame zero and advances with callback frames, even when host transport is stopped, provided FL Studio processes the insert. Host seek/loop never changes its cursor. A repeated Audition restarts the current buffer; Stop ends preview with at most a 5 ms ramp plus one host block, leaving dry unchanged. Natural completion plays the entire buffer; stop ramps never modify the stored render or its preview. Preserve unity render gain. For audition only, apply min(1, gain(-6 dB)/renderPeak), then the -60 to 0 dB level control, default -12 dB. Display raw and predicted wet peaks and automatic attenuation. Smooth live control changes over 5 ms; keep the level session-local and reset it on restore. This attenuates without limiting or boosting quiet renders, and does not bound the dry-plus-wet sum.

The worker owns the immutable audio. Transfer fixed-size audition commands/handles through a bounded nonblocking channel; the callback reads pinned buffers and returns acknowledgements. Free retired buffers on the worker/control thread, never through a final shared-pointer release in `processBlock()`. Handle full command queues without losing Stop/invalidation intent (for example, a separate atomic stop generation). No callback-side allocation, mutex, disk read, or blocking wait is allowed.

Retain the existing order: capture incoming dry first, then add tone and audition. Audition adds only wet PCM to current live input, never an old captured dry recording. Tone remains an independent diagnostic control; the audition guide asks users to turn it off when evaluating the swell. Report that audition requires host processing and does not command the DAW to start. Sample-rate change or input invalidation stops the voice at the next processing boundary and retires its handle safely.

### Extend the editor with a second aligned lane

Provide Load IR, its name/rate/channels, source range fields, Generate, progress/status, and Audition/Stop. Show only the current single result, with separate source and wet lanes sharing a scale spanning both source coverage and full wet output, including negative positions. Reuse bar-label calculation, but extend the visible range rather than clipping the leading swell. Show source and wet bounds, frame count, render rate, and peak. Disable audition until a current result is ready; keep Stop available while playing. Accommodate the added controls by enlarging/layouting the editor without hiding capture diagnostics or the bar ruler. No marker interaction is introduced.

### Keep new preview state session-local

As a proposed M2 default, IR selection, source controls, and render buffers are transient; reopening requires selecting/loading and generating again. Preserve the existing version-1 migration and version-2 tone/capture state behavior, without modifying capture manifests or serializing PCM into host state. State restore cancels jobs and stops audition. Persisting editing/render dependencies belongs with the later marker/session model; adding a premature asset schema here would widen this DSP proof unnecessarily.

## Risks / Trade-offs

- [Long IR/source combinations exceed RAM] -> checked 256 MiB budget and clear rejection; no unbounded full-song reads or truncation.
- [Rendering starves capture through disk or CPU contention] -> separate workers, read each source chunk once, reuse prepared data, and validate sustained mapping while generating in the host.
- [FFT padding, inverse scaling, or resampling shifts output] -> independent reference tests, fixed origin/length contracts, and exact anchor assertions.
- [Audition survives invalidation or frees memory on the audio thread] -> generation IDs, pinned lifetime with acknowledgement, separate stop intent, and repeated replacement/restore tests.
- [Unity convolution can exceed full scale] -> preserve raw output, apply disclosed audition-only attenuation and level control, and test high-peak playback.
- [FL Studio suspends the effect while stopped] -> verify routed-silent and stopped-transport audition, report limitations and manual fallback; no guaranteed output without callbacks.
- [Session-local previews are lost on restart] -> label the workflow in documentation; M1 source assets continue restoring normally.

## Migration Plan

Add DSP/range-reader tests and integrate controls without altering existing capture formats. Run all existing CTest targets plus the new render/coordinator/audition tests. Build the Windows Debug VST3 and document a separate M2 FL Studio acceptance procedure. Preserve M1 state compatibility and verify restored projects start with no preview voice. Rollback removes the M2 build without changing existing source caches; the new preview owns no sole persistent project assets.


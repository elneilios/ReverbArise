# M2 Rendering Internals

The render input is one committed stereo interval `[start, end)` and one non-empty
mono/stereo WAV or lossless WavPack IR. Source reading verifies complete coverage and assets, reads each
intersecting chunk once, and never fills an uncaptured gap with silence. Manifest
revisions provide a stable source snapshot while the capture worker continues.

WavPack import uses pinned static libwavpack 5.9.0 on the background worker. The
loader detects `wvpk` content, so `.wv` files and WavPack files named `.wav` work.
Integer PCM is scaled by its stored byte width; floating-point values retain their
original gain, including finite values above full scale. Decoding checks the complete
declared length and integrity errors before publishing an IR. Hybrid files require
their lossless correction data; DSD, unknown-length streams, and multichannel IRs
are outside the supported scope. Temporary decode buffers count against the budget.
The decoder's BSD notice is shipped as `Contents/Resources/WavPack-LICENSE.txt`.

IR preparation preserves input gain and origin, without trimming or normalization.
Equal rates are an exact copy. Other rates use a symmetric Hann-windowed sinc with
32 zero crossings on either side and cutoff `min(1, outputRate/inputRate)`, evaluated
at input coordinate `outputFrame * inputRate/outputRate` with zero extension. There
is no added causal delay. Output length is `max(1, round(inputFrames * outputRate/inputRate))`.
Band-limited resampling can introduce ringing near an onset; it never removes the
leading interval or intentionally relocates the IR. Extreme rate ratios requiring
more than 4096 input samples of kernel radius are rejected explicitly.

Mono IR applies independently to both source channels. Stereo IR maps L to L and R
to R; true-stereo four-channel responses are rejected. Prepared IR reversal followed
by full zero-padded FFT convolution returns exactly `N + M - 1` stereo frames. For
source origin P, wet origin is `P - (M - 1)` and exclusive end is `P + N`. No output
fade, limiter, normalization, tail trimming, or hidden latency correction is applied.

The numerical oracle uses direct double-precision convolution. Per-sample tolerance
is `2e-5 * max(1, L1)`, where L1 sums the absolute products contributing to that
reference sample. Tests assert integer placement and length independently, and cover
singletons, asymmetric signals, seeded noise, channel isolation, padding boundaries,
long IRs, negative origins, and cancellation preserving an earlier output.

The preview working budget is 256 MiB per instance. Checked estimates include retained
audio, source, prepared IR, complete wet output, FFT arrays/plan scratch, and a bounded
64 MiB capture-decoding allowance. Oversized requests fail without shortening input.
IR decoding checks its own temporary buffer cost before allocating. All these operations
run outside `processBlock()`; real-time playback only reads a pinned completed buffer.

## Coordination and audition

Each request freezes one capture manifest, source sample interval, IR, and fixed
tempo/meter snapshot. A separate render worker reports source-read, IR-preparation,
convolution, and waveform-build progress. New input selections supersede older jobs;
only the current request may publish a ready result. Errors keep the previous waveform
visible but disable audition until a current result is generated. Dependency signatures
normalize revision/coverage intersections, so recapture outside a selected interval
does not invalidate it even when an immutable chunk is split into new files or an
unchanged all-zero remainder is represented as covered silence instead of audio.

Audition is sequential and independent of Song seeks/loops. It adds only wet samples,
after input is queued for capture, while live dry input continues. Stop uses at most a
5 ms ramp plus a processing block. Sample-rate or known tempo/meter changes stop
audition and mark the result stale; Clear and project restore invalidate it too.
The callback reads raw pointers pinned by control-thread shared ownership. Fixed-size
commands and audio acknowledgements let the worker collect retired buffers; no final
buffer release occurs on the callback. A separate stop epoch cannot be lost when the
Start queue is full. Callback tests guard C++ allocation and deallocation.

Audition with transport stopped requires FL Studio to keep processing the insert.
No host play/stop/seek requests are made. Turn off the independent diagnostic test
tone while judging the swell. Raw rendering retains unity gain. Audition applies
`min(1, gain(-6 dB)/renderPeak)` followed by the Audition level control (-60 to 0 dB,
default -12 dB). This is constant attenuation, without limiting, and never boosts a
quiet render. The UI displays raw and predicted wet audition peaks plus automatic
attenuation. Live slider changes ramp over 5 ms of processed audio. The wet ceiling
does not bound the sum with live dry input or diagnostic tone. Stored samples and
waveform stay at their original gain; changing audition level needs no regeneration.
The control resets to -12 dB on project restore.
Wet waveform drawing fits peaks above full scale by applying a display-only scale
of `1 / max(1, renderPeak)`, labeled in dB above the lane. This preserves the envelope
instead of flattening values outside +/-1. It changes neither raw render nor audition.

M2's IR selection, source-bar fields, and generated preview are session-local.
Saving/reopening retains M1 capture IDs/assets and tone settings, but clears preview
work and never starts audition. Reload the IR and Generate explicitly after reopen.
See [FL Studio M2 validation](fl-studio-m2-validation.md) for the host workflow.

# First four swells: source-boundary investigation — 5 November 2026

Historical precursor to the [implemented source-edge fix](2026-11-05-source-edge-fix.md).
That later report records native regression results, installation, the owner's
substantial listening improvement and the source-only PR integration scope.

This is offline analysis and a listening prototype, not a plugin DSP change.
Installed export-clock code `1532f37` remains unchanged. Keep a future source
conditioning change separate from the focused export repair and issue #71.

## Inputs and scope

Owner supplied the isolated ReverbArise track `ReverbAriseBreakup.wav`, a separate
disabled-plugin bounce `ReverbAriseBreakup_Dry.wav`, direct plugin export
`Swells-100pct-wet-ec4aa6d029b8482fa56aa3d65f93d01f.wav`, and the original
`Large Wide Echo Hall.wav`. The screenshot identifies the first four overlapping
swells. The original IR decodes sample-for-sample identically to managed IR
`074407e1dd15fb9933543cf3522e4d54b481dcf470f162817804753cbf5df8b4`.

The DAW bounce is stereo 24-bit / 48 kHz, 1,822,623 frames (37.9713125 s).
Its peak is **-6.730565 dBFS**, with zero full-scale samples; an independent
FFmpeg 8x float reconstruction estimates **-6.720317 dBFS**. This is a finite
measurement, not certified true-peak conformance. The direct plugin wet export
is 32-bit float / 48 kHz, starts at absolute Song sample zero, and peaks at
**-4.322205 dBFS**. No file-output ceiling overload explains the targeted spikes.
Raw render peaks above unity are retained in float assets and are not proof of
clipping. Earlier overload/distortion cannot be excluded from peaks alone.

## Confirmed signal path and candidate cause

The four matching managed raw renders were read without modifying any asset.
An independent double-precision FFT convolution of the captured source windows
and reversed IR matches each full native raw render to **1.80–1.83e-7 relative
RMS error**, maximum absolute error **3.02e-7**. IR preparation reproduces the
origin-preserving 64-lobe Hann sinc from 44.1 to 48 kHz; final IR samples are
rounded to float32 before the independent convolution. This verifies ordinary
linear convolution, rather than a numerical overload at this material level.

The decoded source windows have these boundary amplitudes (maximum absolute
channel value, linear full scale):

| Source sample interval | First sample | Last sample |
| --- | ---: | ---: |
| [3729072, 3752679) | 0 | 0.053903 |
| [3752880, 3776487) | 0.103810 | 0.094186 |
| [3777456, 3793328) | 0.108303 | 0.109320 |
| [3793328, 3816935) | 0.115483 | 0.009271 |

Three windows therefore introduce substantial zero-to-waveform steps at their
starts; their end cuts are nonzero too. Existing wet-output envelope/edge fades
occur **after convolution** and cannot remove the excitation introduced by those
input cuts. The strongest output curvature events at **7.355208, 7.869125 and
8.199792 seconds** align with strong early IR features reversed to approximately
10.7 / 8.8 / 8.8 ms before their source starts. This supplies a specific mechanism
for click-like transients that become conspicuous in overlapping notes/swells.

The reconstructed four-swell wet sum matches the entire first group in the
direct plugin export to **7.11e-8 relative RMS error**. Registration against the
DAW wet lead-in requires a common **28-sample alignment** and gain **0.761428934**;
remaining relative RMS error is **1.88e-5**. These are comparison-registration
parameters: the DAW file has no absolute Song timestamp, so this does **not**
establish 28 samples of plugin latency or identify a particular DAW gain control.

The dry-only export did not yield a reliable sample-null in an interval where
the plugin wet is absent; a direct dry subtraction was therefore not used as
proof. The reason for the bounce difference is undetermined. Its second-difference
peaks in ±4 ms windows around the three events are 0.000491 / 0.000689 / 0.001066,
versus 0.020378 / 0.034747 / 0.033173 in the processed DAW export. Direct wet/cache
agreement independently avoids relying on the dry bounce's phase consistency.

## Offline conditioning experiment

Apply a shared stereo raised-cosine ramp to **copies of each extracted source
window**, before convolution: 0-to-1 over the first 48 samples and 1-to-0 over
the last 48 samples at 48 kHz (**1 ms** per boundary). Preserve original source
positions, IR, output envelopes and fitted per-swell gains. No limiter, lookahead,
dynamic ducking or audio-path EQ is added. A static **+0.000541 dB** wet match is
measured over the pre-dry body, 3.0–6.8 s.

- Wet energy above 8 kHz in the affected **6.8–8.25 s** region, with a Hann
  analysis window, falls **42.57 dB**. This is a band-energy metric, not a claim
  that overall perceived loudness or all noise falls by that amount.
- Maximum second differences at the three targeted events in the mixed export
  fall **15.90 / 23.38 / 21.83 dB**. Those are impulse/curvature measurements,
  not a perceptual click-loudness scale.
- Wet-only seven-second excerpt RMS changes **-0.02136 dB**; the corresponding
  mixed excerpt changes **-0.01671 dB**. The wet excerpt peak is -5.63879 dBFS
  originally and -5.46086 dBFS after conditioning. Improvement is not explained
  by turning down the output.
- 0.5 / 1 / 2 / 5 ms were tested. Targeted high-band reductions were about
  39.93 / 42.57 / 42.79 / 43.01 dB respectively. Start with the shorter 1 ms
  candidate for listening; longer fades are not yet justified by musical review.

The paired wet-only files are under ignored `out/breakup-analysis/`:
`wet-only-original-first-four.wav` and `wet-only-1ms-edges-first-four.wav`.
Each is seven seconds, corresponding to 2.5–9.5 s of the DAW excerpt after
alignment, and is derived from the actual provided direct wet export. The
candidate replaces only the first four modeled wet contributions. The separate
mixed pair retains the original DAW residual rather than subtracting a different
dry performance. No supplied file or managed asset was overwritten.

## Verification limits and proposed implementation boundary

Tools: existing bundled NumPy/Python and local FFmpeg; no packages downloaded.
Disposable scripts and intermediate metrics are in `out/breakup-analysis/`.
These are offline data experiments, not new native MSVC/CTest results. Input
hashes and focused results are retained in the accompanying JSON record.

Evidence strongly identifies source cuts as the cause of the targeted first-group
spikes; it does not diagnose every subjective artifact elsewhere in the track.
Listening acceptance of the prototype is pending. A production fix should
condition only the wet render input, preserve captured PCM, dry playback, stored
coordinates and frozen/accepted revisions, and version render dependencies so
new wet renders are explicitly generated. Short-window/rate handling, calibration,
cache invalidation and persistence need native regressions before installation.
This report does not authorize silently replacing existing or frozen assets.

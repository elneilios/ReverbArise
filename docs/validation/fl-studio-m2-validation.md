# FL Studio M2 Reverse-Convolution Preview Validation

Status: implementation, automated verification, and guided host acceptance complete.
Host results below are user-reported passes, supported by supplied screenshots;
they are separate from the automated checks.

## Tested environment

- FL Studio 2026, v26.1.7, build 5653, Windows 64-bit (About screenshot).
- Windows 11 Pro, version 26H2, OS build 26300.9807 (System About screenshot).
- AMD Ryzen 5 7600X, 32 GB RAM.
- Main guided setup: Song mode, fixed 120 BPM, 4/4, 48000 Hz; changes to 121 BPM,
  3/4, and 44100 Hz were tested and restored.
- Audio settings screenshot: FL Studio ASIO, 48000 Hz, configured buffer length
  512 samples (11 ms), zero reported underruns. Smart disable is enabled. Plugin
  screenshots show varying callback sizes despite the configured driver buffer size.
- Media: generated `m2-source.wav`, mono/stereo diagnostic WAV IRs, the user's
  `IMP large_hall.wav` WavPack IR, and deliberately invalid `invalid-ir.wav`.
- Acceptance recorded 1 November 2026. No host-specific fallback was needed for
  stopped-transport audition in the tested setup.

## Reported IR format issue

On 1 November 2026, the supplied `IMP large_hall.wav` (122630 bytes) was inspected
read-only. Its header begins `wvpk`, identifying WavPack-compressed audio rather than
a standard WAV container. The initial WAV-only build rejected this input. Following
the user's request, native lossless WavPack import now detects content independently
of the `.wv` or `.wav` extension. A read-only automated check decoded and convolved
the supplied file: stereo, 48000 Hz, 210372 frames, unit-impulse render peak 0.860687.
Its SHA-256 remained unchanged; the supplied audio is not redistributed.

The user confirmed the generated WAV IRs work. Their screenshot shows Ready status,
48 kHz, 82 BPM, 4/4, source bars 4–5, and zero dropped capture blocks. Later guided
acceptance below includes native WavPack loading, generation, and audition.
The user subsequently confirmed the large-hall IR loaded and worked, and confirmed
the audition gain fix worked. Their latest screenshot shows raw +7.2 dBFS and wet
audition -18.0 dBFS. A reported flat-topped waveform was traced to drawing raw samples
clamped at full scale. The updated build uses labeled display-only peak fitting,
confirmed by the user during the guided render checks. M2 remains manual audition; transport-
synchronized playback belongs to M4.

## Build and reproducible media

Use the MSVC build/test commands in [development.md](development.md). Close existing
instances and FL Studio before replacing the installed plugin with this bundle:

`out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3`

From the repository root, generate redistributable synthetic test media once:

```powershell
& './out/build/windows-msvc/Debug/preverb_create_m2_test_media.exe'
```

This writes `m2-ir-mono.wav`, `m2-ir-stereo.wav`, and `m2-source.wav` to
`out/test-media/m2`. It refuses to overwrite existing files; keep using those files
for repeated tests or supply a new output directory as its first argument. The source
is a short decaying tone in a two-second stereo file. The synthetic IRs are 0.5 seconds
at 48 kHz with fixed-seed decaying noise; stereo channels intentionally differ.
These are diagnostic fixtures, not a supplied musical IR library.

## Single swell workflow

1. Set Song mode, fixed 120 BPM, 4/4, 48 kHz. Place `m2-source.wav` at Playlist bar 1
   without stretching it, and route it to the mixer insert containing Preverb.
2. Turn **Enable 440 Hz test tone** off. Enable Map while playing, start at bar 1,
   and play beyond bar 2 to cover the entire first bar. Stop, disable mapping, and
   wait for the capture revision to commit. Check dropped blocks remain zero.
3. Click **Load IR**, select `m2-ir-mono.wav`, set **Source start bar = 1** and
   **Source end bar (exclusive) = 2**, then click **Generate**. These controls are
   separate from the diagnostic tone interval. Generation needs valid host timing;
   briefly play/stop if timing is unavailable immediately after plugin loading.
4. Check progress and Ready status. Source interval should be `[0, 96000)` frames.
   The prepared IR has 24000 frames, so output is 119999 frames, starting at -23999
   and ending at 96000 exclusively. The source and wet lanes must share this scale
   and show the leading result before bar 1. Audible energy need not peak at bar 1.
5. With transport stopped, press **Audition wet**. It starts at the rendered buffer's
   first sample, independent of the Playlist cursor. Stop ends preview; Audition
   replays from its beginning. Live input still passes through. This is wet-only
   preview contribution, not a global mute of the current live dry input.
6. Repeat using the stereo IR, then an IR with a different supported sample rate.
   Confirm channel behavior and stable full output bounds after IR preparation.

Rendering retains unity gain. Audition now attenuates peaks above -6 dBFS, then applies
the Audition level slider (-60 to 0 dB, default -12 dB). There is no boost for quiet
renders and no limiter. Raw render and predicted wet audition peaks are displayed.
Live dry input and diagnostic tone are separate contributions. The per-instance render budget is 256 MiB;
oversized regions/IRs report an error instead of being shortened.

M2 bar selection/ruler assumes fixed tempo and meter. Tempo-map editing is unsupported.
Audition requires host callbacks even while stopped: record whether the routed-silent
insert is processed. If FL Studio suspends it, manual playback on a silent routed
section is a fallback; the plugin never commands host transport.

## Task 7.2: Rendering and timeline inspection

- Verify mono/stereo IR selection, file-dialog cancellation, generation progress, and
  source/wet label readability and alignment, including negative-time output.
- Choose an interval containing an uncaptured gap, then an invalid WAV/unsupported IR:
  verify explicit errors, preserved capture, and no partial current result.
- Record host build, Windows version, rate, tempo, meter, block size, test media,
  displayed frame counts/origins, UI responsiveness, and observations.

Result: passed by user confirmation, 1 November 2026. The guided render
checks passed: full bar-1 capture at 120 BPM / 4/4 / 48 kHz with zero dropped blocks;
large-hall IR source [0, 96000), wet [-210371, 96000), 306371 frames; complete
peak-fitted waveform and responsive UI; chooser cancellation preserving the ready
result; synthetic mono/stereo WAV IR generation with wet [-23999, 96000), 119999
frames. The user subsequently confirmed all guided error checks passed: an uncaptured
source interval reports a coverage error without replacing the current result;
the deliberately invalid `invalid-ir.wav` reports a loading error without a crash
or capture loss; reloading the large-hall IR and source bars 1–2 recovers to Ready
with unchanged bounds/waveform. Host environment and configured driver buffer size
are recorded above from the supplied settings screenshots.

## Task 7.3: Audition, processing, and capture isolation

- Retest the large-hall IR after the reported +7.2 dBFS wet render: raw peak remains
  unchanged but default wet audition peak should be -18 dBFS (automatic -13.2 dB,
  slider -12 dB). Adjust the slider during playback; confirm smooth level change,
  unchanged dry input, and reset to -12 dB after project reopen.

- Verify complete preview while transport is stopped and input is silent; record
  whether the insert runs. Test Stop, repeated Audition, and natural completion.
- Seek and loop during audition; it must continue sequentially rather than retrigger
  or become Song-synchronized. Confirm current live dry input still passes through.
- Enable mapping while audition is audible with zero dry input; captured samples
  must represent dry silence, not the generated preview.
- Generate while capturing/playing and confirm zero dropped capture blocks and no
  playback interruption in the tested setup. Record any stopped-host fallback.

Result: passed by user confirmation, 1 November 2026. All guided audition
checks passed: full natural playback with FL Studio transport stopped; prompt Stop
and replay from the beginning; uninterrupted sequential audition through host seeks
and looping in an empty section; live dry passthrough with preview-only Stop; smooth
live level adjustment without regeneration. The user subsequently confirmed the
capture checks passed: mapping empty bars 20–24 during audible audition captured
grey silence; rendering that committed interval produced a flat, silent preview;
generating source bars 1–2 while mapping unrelated empty bars 28–32 reached Ready,
kept the UI responsive, retained current validity, and left dropped blocks at zero.
The user also confirmed the guided reopen check reset audition level to -12 dB.
Task 7.3's guided host behavior checks have passed. This confirms stopped-host insert
processing in the user's tested setup, without establishing it for other host settings.

## Task 7.4: Stale results and project reopen

- Recapture an overlapping source interval: preview becomes stale and cannot audition
  until regenerated. Recapture an unrelated interval: current result remains valid.
- Change source selection/IR during rendering: old completion must not replace the
  newly selected request. Verify Clear stops audition and invalidates old work.
- Change sample rate/tempo/meter: old output must stop and become stale, not be
  silently reinterpreted. M1's rate-mismatch capture safeguards remain in force.
- Save and reopen: source capture/tone state survives; M2 IR selection, source controls,
  and preview are reset. No audition starts automatically. Reload and regenerate.

Result: passed by user confirmation, 1 November 2026. The recapture/reopen
walkthrough passed as written: recapturing selected bars 1–2 invalidates the result
and stops active audition until regeneration; source-range and IR changes invalidate
the old result; changing 120 to 121 BPM stops audition and marks the preview stale;
restoring 120 BPM allows regeneration. Save/reopen retains captured source and tone
settings, clears the IR/render, resets source controls to 1–2 and audition level to
-12 dB, and never starts audition. Reloading the large-hall IR regenerates the same
result from saved capture without recapturing. The user then confirmed all final
checks passed: changing 4/4 to 3/4 stops audition and stales the result; changing
48000 to 44100 Hz stops/disables the old preview and reports the capture rate
mismatch, with regeneration working after return to 48000 Hz; changing source
selection/IR during generation prevents obsolete completion from becoming current;
Clear stops audition, clears coverage and disables the old result; reopening the
saved test copy without saving Clear restores its capture.

## Automated verification

The suite checks exact source extraction, gaps/corruption/non-finite failures, bar
resolution, mono/stereo IR decoding, resampling duration/origin/alias rejection,
FFT output against an independent direct-convolution oracle, complete length and
negative origin, resource rejection, request supersession, invalidation, pinned-buffer
lifetime, callback allocation/deallocation, audition seams/replay/Stop, capture feedback
exclusion, source state restoration, and offscreen editor control/layout smoke checks.
The level-control regression also checks an over-full-scale render, the -18 dBFS
default wet ceiling, the -6 dBFS ceiling at maximum slider level, clamped/invalid
control values, unchanged raw samples/dry input, and allocation-free live gain ramps.
The updated editor's level slider and peak labels were visually checked offscreen.

Final automated result (1 November 2026): Windows MSVC Debug VST3 built successfully;
11/11 CTest executables passed, including all M1 regressions and WavPack integer/float,
Unicode-path, disguised-extension, corruption, truncation, missing-correction, channel,
gain, and memory-budget checks; strict validation of
`render-reverse-convolution-preview` passed. Offscreen software-rendered editor layout
was visually inspected at 1020 x 880 and its ready/empty control availability checked.
Synthetic media were generated successfully under `out/test-media/m2`.

Automated results alone do not establish audible quality or host processing behavior.
The separate user-reported guided results above confirm the three FL Studio acceptance
tasks in the recorded setup; they do not establish behavior for all host configurations.

# Proposal

## Why

M1 provides durable, precisely positioned source captures, but Preverb cannot yet generate reverse reverb. M2 must prove the sound, full convolution length, and timeline alignment while providing enough plugin controls to audition the result in FL Studio.

## What Changes

- Load one custom mono or stereo WAV or lossless WavPack impulse response, preparing it at the capture sample rate with a defined channel policy. Detect WavPack content even when named `.wav`.
- Select one committed source interval using separate start/end Playlist-bar controls, with the end exclusive and fixed tempo/meter only.
- Read the selected interval efficiently across capture chunks, rejecting uncaptured or unavailable samples while retaining captured silence.
- Generate one full-length, wet-only reverse-convolution result in the background, reporting progress and errors without blocking capture or audio processing.
- Show the wet waveform on the source timeline, including negative-time lead-in and exact source/result bounds.
- Provide Audition and Stop for sequential wet-only preview, independent of Song position, while preserving live dry passthrough. Generated preview audio never enters capture.
- Verify the engine headlessly against reference convolution and retain all M1 behavior.
- Provide protected audition gain and a session-local level slider after a user-reported +7.2 dBFS large-hall render: preserve raw DSP output, attenuate wet playback to a -6 dBFS ceiling before additional -12 dB default trim, and show both peaks.

Out of scope: Song-synchronized swell playback (M4), onset detection, arrival-marker editing, fades, draggable boundaries, multiple swells (M3), production export and project packing (M5), tone/IR shaping, bundled IR libraries, and tempo-map support. The M2 preview is session-local; existing captured-source persistence remains intact.

## Capabilities

### New Capabilities

- `reverse-convolution`: Generate, locate, display, and manually audition a single reverse-reverb result from committed source audio and an imported WAV/WavPack IR.

### Modified Capabilities

None. Existing `song-timeline-audio` capture, sparse coverage, state restoration, and diagnostic tone contracts remain unchanged.

## Impact

- New headless DSP library under `Source/DSP` and associated CTest coverage; reuse pinned JUCE facilities and statically link pinned libwavpack 5.9.0 for WavPack input. Include its BSD license with the VST3 bundle.
- `Source/Capture`: revision-bound range reading, using each intersecting chunk once rather than the existing per-sample WAV reader.
- `Source/Plugin`, `Source/UI`, and `Source/Playback`: render-job coordination, plugin controls, aligned preview, and real-time-safe audition buffer handoff.
- `CMakeLists.txt`, existing processor/persistence tests, and M2 development/FL Studio validation documentation.
- No migration of M1 capture manifests, no automatic transport control, and no change to saved capture identities.

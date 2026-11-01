# Tasks

## 1. Project and Plugin Foundation

- [x] 1.1 Select and document a JUCE version and license approach, then configure the Windows x64 C++20/CMake/MSVC build; verify a clean configure and build using the documented commands.
- [x] 1.2 Add the minimal VST3 stereo effect with transparent passthrough and diagnostic UI shell; verify it loads in FL Studio 2026 and passes stereo audio unchanged.

## 2. Host Position and Sparse Capture

- [x] 2.1 Query and display available FL Studio playhead, transport, tempo, meter, sample/PPQ position, and block-size data; verify fields update during Song playback and unavailable values are reported accurately.
- [x] 2.2 Stage capture blocks through a bounded real-time-safe queue and build a sparse position map with discontinuity detection, explicit coverage, and overlap-only replacement; verify unit tests cover seeks, captured silence, gaps, recapture boundaries, and queue overflow.
- [x] 2.3 Verify mapped samples against a known synthetic signal after capturing bars 86-90 and then bars 1-4 in FL Studio; confirm both remain aligned and the gap remains uncaptured, and record the result.

## 3. Scheduled Tone Probe

- [x] 3.1 Add configurable absolute bar interval controls and a 440 Hz, -18 dBFS peak tone with boundary fades, mixed with passthrough; verify automated tests check tone frequency, peak, interval boundaries, and unchanged passthrough outside the event.
- [x] 3.2 Derive tone phase from absolute sample position and serialize schedule settings; verify tests cover mid-event positioning and deterministic replay, then verify project save/reopen restores the schedule without duplicate output.

## 4. FL Studio Feasibility Gate

- [x] 4.1 Test the scheduled tone on a routed mixer channel with zero-valued input before any source note; measure its frequency, level, boundaries, and audibility, and treat failure as an architecture blocker.
- [x] 4.2 Separately test a completely unused mixer insert and document whether FL Studio invokes the effect; classify this result as non-blocking when the routed-silent case passes.
- [x] 4.3 Verify transport start/stop, seeking, loop wrap, replay, mid-tone start, pre-roll, buffer-size changes, and offline render where supported; document host version, routing, fixed tempo/meter, observed discrepancies, and go/no-go recommendation before any editor or convolution work.
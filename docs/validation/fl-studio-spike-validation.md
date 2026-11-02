# FL Studio M0 Spike Validation

## Environment

- Host: FL Studio 2026, Windows x64
- Plugin: Preverb Spike VST3, JUCE 9.0.4, Visual Studio 2026/MSVC 14.51
- Observed project settings: 48 kHz, 150 BPM, 4/4, 200-sample host blocks

## Confirmed

- The VST3 loads in FL Studio and passes stereo audio through.
- In Song mode, the plugin receives advancing sample and PPQ positions, transport state, tempo, meter, block size, and sample rate.
- Capturing bars 86-90 and then bars 1-4 produced two captured regions, one discontinuity, and zero dropped blocks. The gap remained uncaptured.
- With the routed mixer input silent and no source notes, the scheduled tone played in its configured interval.
- With the effect on a completely unused mixer insert, the scheduled tone also played. Seeking into and out of the interval retained the expected tone behavior.
- Transport stop/start and seeking into/out of the tone interval retained the expected behavior.
- Tone enable and bar interval settings survived saving and reopening the FL Studio project.
- Headless tests verify synthetic sample placement, tone frequency/level/boundaries, deterministic phase at a mid-event offset, and repeated rendering at the same absolute positions.

## Remaining Host Checks

- [x] Check loop wrap and replay for correct sample offset and no missing or duplicate audio.
- [x] Start playback before the first event and verify the configured interval's lead-in behavior.
- [x] Change the host buffer size and verify diagnostics update and playback remains stable.
- [x] Try an offline render and record whether FL Studio processes the scheduled tone.

## Outcome

All M0 feasibility checks passed in FL Studio 2026. The plugin generated scheduled audio with both routed-silent input and an unused mixer insert, and the tested transport, recapture, persistence, and offline-render behavior matched expectations. No blocking discrepancy was reported. This result is limited to the tested fixed-tempo/fixed-meter Song-mode setup; it does not establish behavior for tempo or meter changes.
# Design

## Context

See proposal.md for motivation and scope. The repository currently has OpenSpec/package scaffolding but no plugin source, build system, or tests. The spike targets Windows x64, C++20, JUCE, CMake, MSVC, and FL Studio 2026 Song mode. The behavior contract is in specs/song-timeline-audio/spec.md.

## Goals / Non-Goals

**Goals:**
- Establish a buildable VST3 stereo effect that can report host timing, pass audio through, capture sparse song-positioned input, and emit the scheduled test tone.
- Keep capture, diagnostics, and storage work off the real-time audio callback.
- Produce objective host observations that decide whether the proposed capture/replay architecture can proceed.

**Non-Goals:**
- Production-quality asset storage or portable capture-map persistence.
- Convolution, onset analysis, a full waveform editor, MIDI triggering, Pattern mode, or support claims for hosts other than FL Studio 2026.
- Correct behavior across tempo or meter changes; the spike verifies a fixed tempo and meter and records limitations.

## Decisions

### Host position is sampled per audio block

Query JUCE's host playhead once per block and retain every available field, including transport state, absolute sample position, PPQ position, tempo, meter, and block size. Report absent fields as unavailable. Use host absolute sample position as the capture and scheduling anchor when supplied; use PPQ and the fixed test tempo/meter to derive bar boundaries and, only when necessary, a sample anchor. Compare each block's position with the expected continuation so seeks, stops, and loop wraps begin distinct map segments rather than stitching unrelated audio together.

This favors the host's timeline over a local playback counter, which cannot remain correct through seeks or loops. The spike must verify what FL Studio actually supplies and whether positions agree with the visible Playlist; it must not treat JUCE's field availability as proof of host accuracy.

### Capture is staged to a background worker

The audio callback performs passthrough and tone synthesis, snapshots compact timing metadata, and copies input into a preallocated bounded single-producer/single-consumer queue. A worker consumes blocks to update the sparse map and diagnostics. The callback performs no allocation, file I/O, logging, locking, or blocking operation. Queue overflow is counted and exposed as missing coverage; dropped audio must never be represented as captured silence.

For the spike, the map may live in memory or temporary storage. It stores covered sample intervals separately from sample values, so zero-valued audio remains distinguishable from an uncaptured gap. A recaptured interval replaces only its overlap; existing portions before and after that interval remain. Capture reads the incoming dry input before the plugin adds its own scheduled tone, preventing the probe from recording its own generated output.

### Tone timing is derived from absolute position

Use the FL Studio bar/PPQ position and meter to define a half-open interval from the beginning of bar 8 through, but not including, bar 10. Verify the initial schedule at a fixed tempo and meter. For each output sample, derive oscillator phase from its absolute sample offset from the interval start; do not advance a free-running phase accumulator as the source of truth. This makes seeking into the event and replaying the same position deterministic.

Generate a 440 Hz sine at -18 dBFS peak, with a short fixed raised-cosine fade at both interval boundaries. The diagnostic UI controls tone enable and interval start/end, plus mapping enable and clear-map. The test tone is added to the current input, which remains pass-through; no automatic limiter or normalization is added. A zero-input host test isolates the tone level and proves output is not dependent on source content.

### Diagnostic state is separate from captured audio

Present host-position and input-peak snapshots on the UI using non-blocking handoff from the audio/worker side. Keep verbose logs on a non-audio thread. Serialize the test schedule and diagnostic control state so the project-reopen test can verify deterministic scheduling. Captured map persistence and portability are deferred to M1; this spike does not claim that temporary or in-memory capture survives project moves or reopen.

### Validation combines headless and host tests

Use a small automated test target for timeline conversion, discontinuity handling, sparse coverage, overlap replacement, and deterministic tone samples. Exercise capture with a known synthetic signal and verify its stored samples against the expected absolute positions. Then test in FL Studio 2026 Song mode at a fixed tempo/meter, including non-contiguous bars 86-90 followed by bars 1-4; transport start/stop, seeks, loop wraps, mid-tone starts, pre-roll, buffer-size changes, and save/reopen.

Test a routed mixer channel whose input is silent separately from a completely unused insert. The routed-silent case is an architecture acceptance gate. The unused-insert result must be recorded but is not a blocker if the routed-silent case works. Record host version, routing, project settings, measured discrepancies, and whether offline render is supported. A host that does not invoke the plugin cannot receive plugin-generated audio; if the routed-silent gate fails, stop and investigate routing or an always-running source/companion architecture before proceeding.

## Risks / Trade-offs

- [FL Studio does not report stable absolute position or suspends the effect on a routed silent channel] -> Treat host measurements as the first architecture gate; do not proceed to editor or DSP work if required behavior fails.
- [Playhead position fields are absent or inconsistent during seek, loop, or pre-roll] -> Report unavailable data, compare against visible Playlist position, and document unsupported cases rather than fabricating a continuous timeline.
- [Capture staging queue overflows under load] -> Bound memory, expose dropped intervals as uncovered, and test with changing host block sizes.
- [Temporary capture data is lost on reopen or project move] -> Persist only diagnostic schedule state in this spike and defer production capture asset management to M1.
- [A tone mixed with non-silent dry input exceeds full scale] -> Do not normalize or limit implicitly; use zero-input measurement for the tone acceptance check and document summed-output headroom.

## Migration Plan

There is no existing plugin project or user state to migrate. Create the minimal build and test setup as a new project foundation. If the feasibility gate fails, retain the recorded results and do not extend the spike into the waveform editor or convolution engine; revise the architecture in a later change.
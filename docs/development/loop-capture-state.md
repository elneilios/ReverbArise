# Loop capture state

This is the saved-source contract for [#124](https://github.com/elneilios/ReverbArise/issues/124) and the handoff to [#129](https://github.com/elneilios/ReverbArise/issues/129). See [Loop host timing and admission](loop-host-admission.md) for the separate live timing contract.

## Capture in Learn

Loop Mode selects the next capture path. The Audio canvas identifies the accepted source and its extent independently. With Duration selected, switching Arm on confirms that audio and MIDI share a 4/4 clock, reset and aligned origin. Switch Arm off to cancel a pending attempt.

Route stereo audio and MIDI channel 1 into the same plug-in instance. Use notes 21 for beat, 19 for each bar, 12 for the four-bar phrase and 22 for reset. A phrase marker coincides with its bar and beat. When the host declares tempo and 4/4 meter, the plug-in checks the actual marker offsets against that cadence. The measured external clock profiles remain available at 48 kHz with 137 BPM and 128 frames or 280 BPM and 256 frames per callback. Host routing and integration evidence determine which hosts are supported.

An arm reserves one capture and pins its generation configuration. Recording starts at a fresh verified phrase marker and uses the observed musical end for the selected Bars or Beats. The ending sample is excluded. End signal requires a separately verified request and closes on the first verified bar at or after that request. The current Rack route exposes Duration because a raw gate release can also occur on disconnect.

Automatic Swells keeps Detect events and Event sensitivity available in Loop Learn. Markers are analysed from the accepted phrase and remain tied to its exact managed revision while a replacement is pending. Generate Swells and Generate on Stop require Song audio.

Recording uses the shared bounded queue with a limit of two minutes or 5,760,000 frames, whichever comes first. Clock or callback interruptions, queue overflow, invalid audio and storage failures discard the replacement. The previous source and arrangement remain available until the worker verifies and accepts a new revision. The worker writes and checks immutable assets before its short publication step. Cancel and restore invalidate the attempt through the same publication fence.

## Save and reopen

- The accepted source is identified by its project ID, instance ID, immutable managed manifest reference and revision. Loop uses capture manifest schema 3. The accepted map starts at sample zero and ends at the exclusive phrase length. It records the native sample rate, verified tempo, meter, length in meter beats and observed run-local origin. The origin is capture evidence and cannot grant live phase after reopening a project.
- The processor's existing version 8 state can carry two independent version 1 records. `AcceptedLoopSource` identifies the kind of the referenced manifest even if that reference cannot be opened. `NextCapture` stores the next attempt's Loop selection, method, unit and count. Neither record grants live timing or an arm. An older state with neither record restores as Song with default next-capture settings.
- A working-folder save wraps those same records in the version 9 envelope. Their validated XML stays part of the working snapshot digest.
- Reopen verifies the identity, revision and kind against the managed record. Missing or invalid material remains unavailable under its saved identity. A missing Loop manifest does not turn the source into Song, and metadata alone cannot make the source readable.
- Save uses the last accepted snapshot and the current durable preference. Callback evidence, pending replacement audio, queue state and arm status do not belong to project state. Reopen requires a new arm and fresh timing observations.

Use the existing managed references and asset leases in #129. Preserve the accepted kind and next-capture preference through relocation and recovery, and verify the actual audio before making it available. Check exact identity and revision before reusing source-dependent edits or renders. Recovery must never substitute an absolute Song map for a Loop phrase at coincident sample positions.

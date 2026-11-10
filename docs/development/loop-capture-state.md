# Loop capture state

This is the saved-source contract for [#124](https://github.com/elneilios/ReverbArise/issues/124) and the handoff to [#129](https://github.com/elneilios/ReverbArise/issues/129). See [Loop host timing and admission](loop-host-admission.md) for the separate live timing contract.

## Capture in Learn

Loop Mode selects the next capture path. The Audio canvas identifies the accepted source and its extent independently. Recapture Audio off protects an accepted Loop from both Learn Arm and external Start/Arm. Turn it on to allow a fresh arm to replace the Loop after successful settlement.

Route stereo audio and aligned clock pulses to the same plug-in instance. The simplified route uses explicit 4/4 with one pulse per beat on MIDI channel 1 note 23. Optional notes 24 and 25 request Start/Arm and Stop. The existing marker-based Duration route remains available. See the [timing contract](loop-host-admission.md) for its mapping and measured configurations.

Learn shows setup details in the footer when you point at Loop Mode, the capture method or Arm. The empty Loop view explains manual and sequencer starts. The status below Arm appears for capture progress, a result or a problem that needs attention.

An arm reserves one capture and pins its configuration. The simplified route records from the first observed beat at or after the arm, with that beat as the local downbeat. Duration uses the selected Bars or Beats. Arm off or Stop cancels a waiting attempt or a recording Duration take. During End Signal recording, it requests a finish at the first observed downbeat strictly after the request. The ending sample is excluded. Loss of timing before that sample cancels the take.

The Learn Arm toggle can cancel a waiting attempt or a recording Duration take even while the host is idle. Cancellation invalidates the pending pass before the next audio callback can start or publish it.

Automatic Swells keeps Detect events and Event sensitivity available in Loop Learn. Markers are analysed from the accepted phrase and remain tied to its exact managed revision while a replacement is pending. Explicit Generate Swells uses the current accepted Loop markers and bounds its automatic source windows at the exclusive phrase end. Generate on Stop remains a Song capture option.

Recording uses the shared bounded queue with a limit of two minutes or 5,760,000 frames, whichever comes first. Clock or callback interruptions, queue overflow, invalid audio and storage failures discard the replacement. The previous source and arrangement remain available until the worker verifies and accepts a new revision. The worker writes and checks immutable assets before its short publication step. Cancel and restore invalidate the attempt through the same publication fence.

## Save and reopen

- The accepted source is identified by its project ID, instance ID, immutable managed manifest reference and revision. Loop uses capture manifest schema 3. The accepted map starts at sample zero and ends at the exclusive phrase length. It records the native sample rate, verified tempo, meter, length in meter beats and observed run-local origin. The origin is capture evidence and cannot grant live phase after reopening a project.
- The processor's existing version 8 state can carry two independent version 1 records. `AcceptedLoopSource` identifies the kind of the referenced manifest even if that reference cannot be opened. `NextCapture` stores the next attempt's Loop selection, method, unit and count. Neither record grants live timing or an arm. An older state with neither record restores as Song with default next-capture settings.
- A working-folder save wraps those same records in the version 9 envelope. Their validated XML stays part of the working snapshot digest.
- Reopen verifies the identity, revision and kind against the managed record. Missing or invalid material remains unavailable under its saved identity. A missing Loop manifest does not turn the source into Song, and metadata alone cannot make the source readable.
- Save uses the last accepted snapshot and the current durable preference. Callback evidence, pending replacement audio, queue state, arm status and Loop Recapture Audio permission do not belong to project state. Reopen starts with Recapture Audio off and requires a new arm and fresh timing observations.

Use the existing managed references and asset leases in #129. Preserve the accepted kind and next-capture preference through relocation and recovery, and verify the actual audio before making it available. Check exact identity and revision before reusing source-dependent edits or renders. Recovery must never substitute an absolute Song map for a Loop phrase at coincident sample positions.

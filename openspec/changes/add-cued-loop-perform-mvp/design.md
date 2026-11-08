# Design

## Context

Learn captures a verified stereo float WAV between sample-positioned Host-FX Note Gate edges. Its accepted-take metadata includes the rate, exact frame count and content hash. Existing reverse convolution and shape preparation accept sample intervals and can retain negative positions. The Song job surrounding those operations also requires a sparse map, absolute positions and validated host timing.

The offline spike replayed the prior 48 kHz Rack trace and checked it against the 382,975-frame synth take. All five phrase markers arrived one sample after their corresponding gate rise. Recurring markers were 764,275 or 764,276 samples apart. That fixture proves capture and event delivery. Its gate high span covers only part of the marker period. The same spike compared a cached cycle with three independently placed swell sets and checked a cue one sample after the lead-in deadline.

## Goals / Non-Goals

**Goals:** Prove a one-part, fixed-clock Rack workflow from accepted audio through a manual editable swell, Standby and sample-aligned wet playback.

**Non-goals:** Timing inference from DAW data missing in Rack, correcting a drifting or changed tempo, producing a playback clock from incoming audio, changing host transport, or adding host latency.

## Decisions

1. **Make the patch the phrase authority.** The first live spike should patch a common clock and reset into the source sequencer, a one-cycle capture gate and a recurring phrase-start pulse. Use distinct Host-FX Note Gate mappings. Test D0 (14) for Capture Gate, C0 (12) for Phrase Start, E0 (16) for Cue and F0 (17) for Stop. The latter two are provisional fixture assignments. Measure both phrase phase and pulse delivery at 48 kHz and the two previously tested callback sizes. Require at least 32 complete reference cycles at each size, a gate span equal to the marker interval and zero sample error against the patch audio after any measured fixed wiring offset is removed in the fixture. Only proceed to production synchronization once the owner has reviewed measured limits. The current LFO fixture's period and drift are evidence against treating a captured length alone as the patch clock.

2. **Preserve separate source and edit identities.** Add a narrow reader for the verified Loop WAV with zero at its first captured frame. Use its take ID, content hash, rate and frame count as preparation dependencies. Reuse existing worker-side edge conditioning, IR preparation, reverse convolution, shape and Audition. Keep the Song job's sparse map, host mapping checks and frozen recovery as-is. Persist the Loop edit separately against its accepted take. A missing file or superseded generation cannot unlock wet. Keep the current project identifiers and validate rollback with a disposable older Song project.

3. **Separate edit readiness from synchronization.** A verified Loop at the current processing rate opens editing and explicit Audition. Standby passes live dry and tracks the external phase. Validate at least two consecutive matching phrase intervals before scheduling. For a ready cue, compute the first future phrase start with sufficient frames for the longest lead-in. Keep tracking marker deadlines. A mismatch or known reset cancels pending launch or ends active wet with the existing bounded fade policy. Treat sample-rate changes and missing markers as loss of sync. Request a new explicit Cue after recovery. Captured PCM is never stretched.

4. **Prepare the mix outside realtime processing.** Pin the exact take/edit/render revision for launch. Preserve a separate signed lead-in for initial playback, then add negative material to the corresponding end of the cycle during worker preparation. Keep the first milestone within one cycle of lead-in and within the accepted phrase end. Explain unsupported placement or budget failure before Cue. Verify the wrap sum against independently placed source contributions. Callback work observes bounded timestamped events, mixes prepared samples and ramps wet on Stop. Simultaneous Stop wins. Keep the dry path active and retire prepared assets outside the callback.

5. **Build the production UI from approved components.** Follow the accepted-take Perform mockup and current production style guide. Reuse the existing Audition row, shared command buttons and context help for Cue Loop and Stop Loop. Show reference, queued, lead-in and repeating states in the adjacent status area. Reuse the overview and waveform interactions for the local range with a truthful zero and exclusive Loop end. Only show bar or beat labels after a verified musical reference defines them. Avoid adding new shared control variants for this milestone. If a genuine gap requires one, present a native isolated component review and update the design standard and tests together before using it in production. Review the integrated production editor at 900×600 and the supported scale choices.

## Risks / Trade-offs

- The host can suspend callbacks silently or deliver cable removal as an ordinary note-off. The live spike records those behaviors. Known lifecycle changes clear the arm and reference. The patch instructions require an explicit Stop before rewiring or suspending Host-FX.
- A reference mismatch discovered after pre-roll begins cannot undo audio already heard. End wet promptly, surface the loss of sync and require a fresh cue.
- Long IRs and signed placements increase working memory and wrap overlap. The first milestone admits bounded complete prepared spans and reports limits before playing.

## Migration Plan

Keep the Learn WAV identity, accepted Song data and plugin IDs. Restore old Song projects with their existing workflow rules. Restore a Loop edit only with its verified take and leave the instance in silent Standby awaiting a fresh reference. Run the local Rack tests before considering external delivery. Retain a prior VST3 bundle and disposable project copies for rollback.

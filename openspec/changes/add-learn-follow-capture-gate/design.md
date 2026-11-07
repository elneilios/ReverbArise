# Design

## Context

The existing `PluginProcessor::processBlock` admits Song passes through validated host transport, then writes absolute sample positions through `AudioCaptureQueue` and `CaptureWorker`. Session and portable-asset state tie those commits to Song mapping and render identities. The Rack probe instead copied a local stereo span between sample-positioned MIDI events, including partial callbacks, and verified every sample of the boundary fixture. On patch load, Host-FX could send a note-on for a gate that was already high.

## Goals / Non-Goals

**Goals:** Deliver one explicit Learn capture from a live VCV patch, preserve Song compatibility, and let Stuart inspect the exact completed WAV.

**Non-Goals:** Fixed-length capture, configurable MIDI bindings, Loop playback or export rendering, beat and bar inference, and migration of Loop WAVs into the Song map.

## Decisions

1. **Route the gate through the VST3 MIDI event bus.** Enable note input in CMake and the processor while preserving the existing plugin identity, parameters and stereo audio buses. Use MIDI note 14, displayed as D0 in Rack. Handle note-off and note-on with velocity zero as gate falls. Keep the source explicit in Learn and document the tested Host-FX mapping. Ordinary automation lost short edges and exposed held edges at callback boundaries in the comparison. Use no inferred clock, meter or host origin for Loop.

2. **Give each capture type its own admission.** Leave the existing Song mapping and settlement logic in place. The next capture preference selects which path can start in idle Learn. Reject preference changes during recording or settlement, and prevent a Loop arm while a Song pass is active. Track the gate on the processor, including while idle, to require an observed low before a fresh rise. Pin the arm to its session generation and prepare epoch. On reset, release, rate change or state replacement, invalidate pending or active work with a reason. Leave previously accepted source intact.

3. **Adapt the proven bounded recorder.** The audio callback copies between sorted MIDI sample offsets before any audio mutation, using prepared stereo storage with a 15-second cap for the candidate. Reject unsupported rate, invalid offsets, discontinuity, missing channels and overflow. Preserve multiple events within one callback and ignore duplicates. Give an independent worker exclusive access to a completed immutable take until settlement finishes. This keeps phrase samples and timing separate from the sparse Song manifest and keeps file operations, verification and UI updates outside the callback.

4. **Publish a verified take under the existing project asset root.** Use a separate Loop asset directory and unique relative take ID. Validate finite input and useful signal, write a temporary stereo float WAV, verify frame count, rate and integrity, and publish with atomic replacement of the accepted reference. Associate it with the current session generation and active root. A separate versioned reference and capture preference supplement the existing host state. On restore verify the referenced file asynchronously and report unavailable audio with its prior identity intact. Existing snapshots with no Loop metadata resolve to Song. The candidate makes a verified WAV available locally. Extending cross-PC portable asset inventory to Loop WAVs belongs with later Loop delivery.

5. **Keep the capture UI in Learn.** Reuse the existing Learn Source Capture layout and shared Switch, ActionButton, text roles and context help. Use explicit status for waiting for low, waiting for the rise, recording, settling and failure. Show the accepted take type separately from the next capture controls. For a verified Loop take show the duration in seconds and provide **Show captured WAV**. The candidate uses the accepted take type to gate Perform and Export and explains access to the raw WAV in Learn. Song remains the accepted type through an abandoned or failed Loop attempt. Changing the next capture type back to Song prepares the existing Song path. A later accepted Song pass makes Song active again. Check the 900×600 editor and existing scale settings in a focused native preview.

## Risks / Trade-offs

- The new MIDI input changes what hosts can discover. Verify the original VST3 identifiers, existing parameters, buses and an older FL Studio project. Account for JUCE's MIDI CC emulation when checking the VST3 parameter inventory.
- Host-FX can report an initial held gate on load. An unknown or high gate waits for low, and the arm is ephemeral. Startup, disconnect, bypass/reset and post-cancel edges need direct tests. Host-FX may report cable removal as an ordinary fall. Record the observed behavior in the candidate notes if the host cannot distinguish it.
- A missing asset or a delayed worker must not accept a partial take. Use verified references, generation checks and immutable file names, and exercise restore and worker failures before packaging.
- The 15-second bound keeps the native candidate close to the proven prototype. Show the bound and give clear feedback on an overlong gate.

## Migration Plan

Preserve Song and VST3 identities. Verify a pre-change saved Song state on the candidate and verify rollback on an expendable copy. Build from a committed issue branch using the existing Windows candidate packager and the last build actually shared with Stuart as the release-notes baseline. Ask Stuart to keep a copy of his patch and previous bundle, map the D0 gate, arm one take, and report whether a playable WAV appears. Record exact binary, package hash and tested host versions.

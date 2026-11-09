# Design

## Context

Song enqueues absolute-position audio in the processor and uses `CaptureWorker` and `CaptureSession` to stage, prepare and accept a managed snapshot. The worker fences publication with the pass, generation and base revision. Processor saves and the shared swell engine already consume accepted capture and creative state. The VST3 currently declares no MIDI input. The spike demonstrated clocked stereo Loop capture, with its own recorder and state. Reuse its boundary cases and measured inputs through the approved [host admission contract](https://github.com/elneilios/ReverbArise/blob/2c3322a/openspec/specs/loop-host-admission/spec.md).

## Goals / Non-Goals

**Goals:** Reuse managed source ownership and asynchronous settlement, keep one authoritative accepted source per instance, and give Learn a complete capture and replacement flow.

**Non-Goals:** Loop playback scheduling, phrase-local Perform editing and export composition, a take browser, new VST3 IDs or host parameters, and broader relocation/recovery workflows. The downstream work can consume the accepted source record and distinguish unsupported actions.

## Decisions

1. **Make source kind part of the accepted snapshot.** Pair the existing accepted capture identity and managed reference with an explicit Song or Loop kind. Loop carries its native sample rate, exclusive phrase extent, stable tempo, explicit meter, duration in meter beats and verified phrase origin. Store Loop as contiguous phrase-local coverage beginning at zero. Stage a replacement separately from the accepted map. Reuse managed asset verification and bounded source reads, so a capture can feed the shared path without creating a second authoritative WAV/session store. Any operation that requires absolute Song coordinates must check the kind before using them. Leave existing Song mapping, recovery and Stop generation on the Song path.

2. **Pin capture intent before the first sample.** In Learn, Loop Mode selects the capture path for the next attempt. Arm captures one method, duration, route, timing epoch, session and pass, and the applicable generation context. Duration waits for the next verified phrase origin and counts observed beats/bar boundaries using the declared meter. For End signal, verify low before a fresh start rise and accept only an independently verified end request. Route both through one recording and settlement path. Give reset and invalidation priority, preserve distinct roles at the same offset, and exclude the end sample even when it shares a block with capture audio. The approved Rack route withholds raw gate release as an end. Exercise independent-request semantics with fixtures until a real route is verified.

3. **Keep the callback bounded and publish on the worker.** Enable VST3 MIDI input for the approved channel and role map while keeping Song admission tied to the existing host playhead. Normalize offsets without clamping and enforce the approved event limit. Enqueue only the admitted stereo subranges into bounded, preallocated storage. Preflight maximum duration against the bounded recording budget before arming. Discard a candidate after queue pressure, a discontinuity, cancellation, storage failure or invalid content. Reuse the prepare/accept split and recheck the instance, session epoch, attempt, pinned dependencies and base revision at publication. Publish kind, managed reference, revision and timing as one accepted snapshot. Reject stale render, edit and generation work by source identity and revision so identical sample numbers across domains cannot alias. A validated replacement can change the accepted kind while Learn remains visible. Downstream creative actions require compatible source and readiness.

4. **Reuse Learn controls and preserve review boundaries.** Use the existing Source Capture layout, shared latching buttons, choice, numeric field, automatic settings switches and status text. Show accepted source kind and extent separately from next capture selection. Show Duration inputs only where relevant, plus the Arm toggle and the actual waiting, recording or settlement state. Explain missing capabilities in place. Use the approved #116 mockups for placement, the production style tokens for geometry, and the existing keyboard/accessibility focus policy for the reused controls. Obtain a native 900×600 review at 100% and 125% user scale and test real bindings. Any new family or styling exception requires owner approval before adoption.

## Risks / Trade-offs

The owner approved both toggles and asked for host-neutral operation and help. Arm confirms a shared 4/4 clock, reset and aligned origin. Arm stays on while the current take can be cancelled and switches off when the attempt ends. The processor verifies the same MIDI markers against declared host tempo and meter when available. It also checks the measured reference profiles independently for an external clock without matching host timing. The selected reference remains fixed until loss or reset. The Rack validation patch verifies the dry and MIDI paths.

The owner review keeps Automatic Swells in the same Capture & Generate column for Loop. Detect events and Event sensitivity feed the existing onset scanner with the accepted phrase's managed revision and phrase-local coordinates. The review shows the Song generation options with their availability explained. The column uses the existing focus viewport at minimum size. Its painted control edges align with the labels using the existing focus margin.

- A type change can expose stale Song commands or associate an edit with the wrong source. Gate commands on accepted kind and revision, including when no editor is open.
- Interrupted callbacks may leave no completion event. Revoke affected timing on observed interruption and require an observed boundary before settlement. Missing evidence is never filled from elapsed wall time.
- Enabling a MIDI input can affect host wrappers. Validate an actual supported Rack Duration take and retain native Song regressions. Keep FL Studio Loop routing in #127.

## Migration Plan

Decode supported legacy state as Song with its existing references and identities. Version the next-capture preference separately from the accepted source record. Save only the coherent accepted record and durable dependency references. Restore Loop disarmed and report an invalid asset as unavailable. Preserve prior durable assets and source-dependent edits on disk when a new revision takes over. Verify minimum save/reopen and older Song compatibility here, with broader restore, bypass, relocation and recovery acceptance under #129.

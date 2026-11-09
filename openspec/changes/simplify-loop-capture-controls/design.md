# Design

## Context

`ClockAdmission` verifies supplied marker roles against either the host-declared tempo or a measured Rack profile keyed to sample rate and callback size. `Take` already owns start-inclusive and end-exclusive audio bounds. The worker transaction owns publication of the accepted source. The existing Learn latch currently requests cancellation when switched off.

## Goals / Non-Goals

**Goals**

- Use sample-timed Clock, Start/Arm and Stop for a one-take workflow that a Rack musician can patch without separate bar, phrase, or reset cables.
- Keep an explicit 4/4 capture-local grid and maintain sample-accurate boundaries, bounded processing, and atomic publication.
- Demonstrate both End Signal settlement and incomplete-take cancellation in Rack with an audible patch.

**Non-Goals**

- Inferring meter or shared playback phase from an unaccented clock.
- Implementing Perform timing, Export, a native Rack module, or host-specific detection.
- Changing the saved manifest format or adding a Learn screen, control family, visual token, or styling exception.

## Decisions

### Separate simple capture from marker-based timing

Give the simplified Clock and its external Start/Arm and Stop commands dedicated roles on the sample-offset MIDI route. Keep the existing note assignments and marker path for current projects, including the independent Perform Cue/Stop roles. Verify candidate note mappings and positive trigger delivery in Host-FX before documenting them. An input note-off never finishes a take. A separately supplied reset still invalidates capture.

Use explicit 4/4 in the simplified setup contract and show it beside the existing Learn controls. Arm can wait before either clock is ready. Select the route from received evidence and pin it to the attempt before recording. A distinct Clock role keeps the marker path's beat from being mistaken for simple capture. Define deterministic rejection for conflicting timing routes and do not switch routes mid-take. A capture-local downbeat does not establish a recurring external playback reference.

### Verify cadence without predicting boundaries

Measure consecutive Clock offsets at the actual sample rate. Maintain a bounded feasible interval for a stable beat period using both adjacent and cumulative sample distances, allowing only sample rounding. Reject a missing or out-of-cadence pulse, an unsupported tempo, and discontinuities. Apply the existing supported range and capture limits unless measured evidence requires a narrower Rack claim. Stage from the first observed beat after an arm. If a short cold-clock take ends before there is enough evidence, hold the candidate provisionally for bounded corroboration without adding audio beyond the selected boundary. Reject if evidence fails or times out.

Use the observed start and end samples and counted beat span. Reuse the existing Loop timing metadata. After cadence verification, choose a supported tempo consistent with both the verified pulse intervals and the exact sample extent. Deriving tempo from the extent alone would allow irregular beats to appear stable.

### Keep one state machine for UI and external intent

Normalize UI and host requests to one attempt-scoped state machine. Host triggers retain sample offsets and UI intent takes effect at the audio-side observation boundary. Pin readiness, settings, pass and storage generation before accepting the start. Reuse the bounded workflow reservation and worker publication fence. Establish callback-safe storage readiness outside the callback and invalidate it on restore or storage changes. Do not call the UI arm path or acquire its storage lock from MIDI processing.

Before recording, Stop withdraws the arm. During Duration, Stop cancels, including on the configured end sample. During End Signal, the first Stop queues the closing downbeat strictly after the request. Further Start/Arm and Stop commands do not postpone or duplicate the pending finish. Process invalidation first, then Stop or Start/Arm, then the beat boundary for a same-sample group. Stop wins over a coincident Start/Arm. This gives a coincident Start and beat an inclusive start and a coincident first Stop and downbeat the full bar. Only a successful closing boundary enters normal validation. Clock loss, a reset, invalid evidence or interruption before that boundary discards the take and preserves the accepted revision.

### Preserve Learn presentation

Reuse the existing shared Arm latch, method selector, setup help and status text. Describe the method-dependent Arm-off behavior before the owner arms, then distinguish waiting for the clock, recording, finishing at the end of the bar and automatic cancellation. Keep the controls and layout within the approved Learn surface. Present a native preview for owner approval of the changed interaction and status before final delivery.

Reuse the existing Recapture Audio switch row with Loop-specific help. Maintain Loop overwrite permission separately from Song recapture behavior. It starts off on load and is pinned when either UI or MIDI arms. An accepted Loop and Recapture off prevents another arm. Recapture on permits each subsequent fresh arm to replace the Loop only after successful settlement.

## Risks / Trade-offs

- Rack audio and control paths may incur different sample delays. Measure the sequencer and dry input offsets at the hosted plug-in and use a fixture with matched paths. Document the shared-origin setup in the patch.
- A lost callback cannot carry a closing beat or immediate loss message. Publish nothing while processing is suspended and reject the pending attempt when the interruption becomes observable.
- Rack may produce edge behavior on cable connect or disconnect. Verify the actual mapped inputs and exclude any event pattern that can falsely publish a take.

## Migration Plan

Keep the existing marker-based Duration path and its note assignments. Use the new mappings in the #133 Rack patch. Old Song and Loop projects continue to restore with the existing accepted source format and disarmed capture. Record the observed Rack versions, rates and callback behavior in dated evidence. Keep DAW verification under #127.

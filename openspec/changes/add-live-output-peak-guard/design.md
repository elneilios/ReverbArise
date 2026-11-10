# Design

## Context

See the proposal for #144. `TransportPlayback` and `LoopPlayback` each form a final stereo mix after applying their prepared wet gain. The Song offline path also uses a separate `TransportPlayback` instance. Loop Audition is mixed through a separate path.

## Goals / Non-Goals

**Goals**

- Share a small stateful DSP component between the two realtime mix points.
- Keep dry sample timing and normal gain predictable in FL Studio and Rack.

**Non-Goals**

- A true-peak or mastering limiter, latency compensation, or changes to monitoring and export gain.
- A new UI control, setting, automation parameter, or persisted state.

## Decisions

1. Apply a shared stereo gain to the final Dry + Wet sum, after existing wet protection and live routing. Enable it explicitly at the realtime Song and Loop mix points. Keep the Song offline player out of this path even though it uses the same mixer.
2. Start with an immediate attack and bounded, sample-rate-aware release at a −1 dBFS sample-peak ceiling. The current peak constrains gain on each sample, and a 50 ms linear release returns it to exactly unity. No delay or host latency is needed for this candidate. Treat inter-sample peaks as outside the guarantee.
3. Keep guard state on the audio thread and independent per playback instance. Apply it only in realtime Dry + Wet when a wet contribution or its recovery is active. Include an emitted wet stop or handover fade. Reset to unity when the path returns to dry-only passthrough, changes routing, or starts an unrelated run. Keep block and publication boundaries from resetting active protection.
4. Verify the ceiling and linked gain with deterministic stereo inputs, including coherent overlaps, and qualify the sound on sharp attacks in both hosts. Track gain reduction during validation using the test harness or existing diagnostics. If fast attack audibly flattens normal swells or introduces artifacts, pause for a design decision before adding latency or changing the approved wet target.

## Risks / Trade-offs

- A sudden gain reduction can reshape a coincident transient. Host listening and transient fixtures determine whether this candidate is acceptable.
- Audition has a known captured window and uses fixed mix protection. The live envelope will differ when it engages. Host offline bounces also keep their existing behavior under this live-only scope. Document the potential difference when the guard is active.

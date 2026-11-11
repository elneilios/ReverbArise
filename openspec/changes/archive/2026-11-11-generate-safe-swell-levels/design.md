# Design

## Context

The approved source-energy calibration is measured against a local dry attack. The worker currently measures or bounds the positioned wet mix after user gain and publishes a shared attenuation. Song, Loop, manual Audition and Perform-matched Loop export have separate consumers of that behavior. Realtime Dry + Wet also has an independent linked guard on the sum of live dry and wet.

## Goals / Non-Goals

Keep the musical starting balance and make 0 dB a safe wet default. Keep gain edits independent across playback and export. Retain the existing source, raw convolution, project identities, and real-time resource bounds. The guarantee is a sample peak for the generated wet composition at its default settings. It makes no claim about true peaks, arbitrary subsequent edits, downstream DAW gain, or arbitrary dry input.

## Decisions

1. **Separate calibration, generated headroom and user gain.** Store headroom attenuation with the accepted swell revision and apply it once alongside the accepted source-energy gain and Level. Compute it from the neutral Level and unity wet reference rather than the current knobs. Save it with the accepted revision, including frozen swells. A Level edit, reset, solo, seek or export must never recompute it. Keep each gain separate so repeated generation cannot compound attenuation.
2. **Prepare generated headroom before publishing new wet audio.** Generate can still create definitions and enter Perform while renders are pending. Complete the cancellable scan on the worker after the relevant renders are verified. Publish the wet revision and starting gains together. A new batch with hot overlap receives the same reduction for its new members. If generation adds or replaces members in a composition, retained members keep their accepted starting gains and edits. Allocate available headroom to the new members against the retained neutral reference. If retained material leaves no usable headroom, report that the new mix could not be admitted and keep the last coherent accepted wet revision.
3. **Reuse positioned peak measurement.** Scan the actual aligned Song sum and fixed-grid Loop composition, including the first and last passes where cancellation can differ from steady repeat. Use a conservative bound for a supported phase route whose alignment cannot be measured in advance. Include float accumulation margin. Preserve scheduling, crossfades, and callback resource limits. Peak analysis happens before publication, so a failed, cancelled or stale analysis cannot expose unverified default gain.
4. **Make playback consume the accepted gains directly.** Keep the existing intentional fades and Live Wet controls. Remove the wet ceiling calculations from Song live/offline playback, Loop playback, Audition wet preparation and the Loop export wet reference. Keep ordinary export fader behavior and derive Loop export Wet 100% from saved swell gains and Live Wet. Track output overload with a bounded peak observation, reusing the existing diagnostic text or shared notice roles and export peak reporting. No new control family, token or visual variant is needed.
5. **Dry + Wet uses a direct float sum.** The owner approved removing the combined guard. Live dry stays at unity during steady Dry + Wet playback, and Audition uses its unity captured dry. Report the measured output overload and let the DAW handle deliberately hot signals.

## Risks / Trade-offs

- A default Song or Loop revision could sound quieter than an unchecked source-matched mix. The reduction is measured once and never increases a quiet generated mix.
- Separate render and publication stages can expose a mix with incomplete headroom calculation. Gate use of new wet on the matching completed scan and preserve the existing cancellation and publication checks.
- A later boost can overload the host output or an export. Preserve finite float samples and indicate measured overload. Keep the distinction between generated wet headroom and mixed output clear.
- Compare a +12 dB boost against the same accepted schedule with an isolated second swell. Cover hot overlap, Loop wrap and partial overlap, adding or replacing swells, save/reopen, and agreement across the render paths.

## Migration Plan

Use the new policy for this pre-release build and for sessions saved by it. No conversion or audible compatibility for earlier development snapshots is required.

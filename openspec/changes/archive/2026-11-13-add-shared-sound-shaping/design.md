# Design

## Context

`SoundPresetEditor` and `SoundPresetBinding` provide the shared Learn and Perform surface. Perform already hosts the inspector's Level control. Learn has a separate persisted future Level. `SwellEngine` prepares a worker-side copy of the source and IR, convolves them in reverse-IR order and hands off the accepted render. The existing asset hashes and #183 handover guard saved and audible state.

## Goals / Non-Goals

**Goals:** Reuse the shared editor and worker transactions, preserve neutral PCM and accepted assets, and reproduce the approved response-shape change independently of wet level.

**Non-Goals:** Host automation expansion, transport or Envelope redesign, and changes to the installed catalogue.

## Decisions

### Preserve assignment ownership

Persist typed shared Damping and cutoff settings on the accepted future assignment and on each swell. Keep Level in its existing fields and setters. Store each cut as Off or a canonical frequency in Hz. Carry the settings in the guarded Learn creation snapshot. Existing swells retain their shaping when Learn changes. When an explicit Generate matches an existing swell under the current automatic replacement policy, retain its existing shared shaping and Level while following the established preset replacement rules.

Keep one effective Damping value per assignment. When integrating with `add-stereo-ping-pong-crescendo`, derive the shared dial from the saved Ping Pong value and use that preset's per-hit implementation. Older revisions keep the accepted render and their saved parameter semantics. Verify this integration before shipping both changes.

### Prepare filters and response shaping independently

Use one stereo-linked second-order Butterworth high-pass and one low-pass on the worker's wet source copy, following the existing source-edge conditioning. Use independent channel state and start each render with clean state. Off skips the corresponding stage exactly. Keep canonical cutoffs in the 20 Hz–20 kHz range. For a lower sample rate, bound coefficients to a documented stable frequency below Nyquist while preserving the saved value. Permit crossed cutoffs so users can explore steep attenuation. Level remains a gain edit and Envelope remains a shape edit.

For a full response, shape the forward-time IR before reverse convolution, linking the gain across channels and retaining the start, frame count and channel layout. Start from the approved spike mapping. Locate the first meaningful onset and the point that contains 90% of the summed stereo energy. Use their interval with a 45 ms minimum as the reference, leave the first 2 ms after onset protected, and taper exponentially from there. At the reference interval the extra attenuation is 0 dB at 0%, 18 dB at 50% and 65 dB at 100%. Interpolate attenuation between these points. Handle silent and very short responses safely. The listening evidence is in `out/shared-sound-controls-spike-v2` and covers Preverb Hall, Intimate Plate and a synthetic mono resonant room.

Check the production wet output after musical matching and headroom. Use level-matched listening to confirm the decay shape itself changes and an actual Audition comparison to check what users hear at fixed Level. Verify that matching leaves intentional source cuts audible and that quiet or crossed-cut signals remain bounded.

### Reuse render identity and publication guards

Hash the canonical non-neutral shaping settings and algorithm version into the render dependency alongside the immutable source and response identity. Keep the source dependency readable for automatic generation. Preserve the existing neutral key and accepted asset paths. Reuse raw renders for Level and Envelope. Treat Damping and cuts as render-changing requests, coalesce them after gesture settlement and publish settings and render together through the existing worker handover. The callback, offline snapshots and Freeze keep their current contracts.

### Use existing Sound controls

Use the existing compact two-column grid and `KnobValue` family. Present the shared knobs in the same relative order in both contexts. Use a logarithmic position mapping for active cutoffs and an explicit Off position at High Pass's low end and Low Pass's high end. Support exact entry, accessible values, keyboard adjustment and reset. Keep the existing contextual import row, delayed preparation overlay and bottom hover help. Inspect Learn and Perform at 900×600 and an expanded size, including the additional rows in the compact inspector. The proposal requires no new styling tokens or control family.

## Risks / Trade-offs

- Very short, sparse or resonant imports respond differently to one decay curve. Include mono and stereo Custom IRs, silence, a short impulse and the approved listening set in verification.
- Gain matching can obscure damping or compensate for strong cuts. Check both spectral and time-domain changes at fixed Level through native Audition and FL Studio.
- New worker dependencies can invalidate caches or race with target changes. Prove exact neutral bypass and reject outdated publications with the existing ownership tokens.

## Migration Plan

Version the new accepted settings and decode absent values as neutral. Preserve exact accepted audio, IDs, source references and existing Level or Ping Pong Damping on reopen. Enable shaping only for new swells inheriting edited defaults or an explicit per-swell edit. Keep the prior render accepted on failure and preserve existing asset recovery behavior.

# Design

## Context

The #190 foundation keeps a versioned Sound assignment and its exact managed IR. The #191 editor already has a shared parameter surface and accepted waveform in Learn and Perform. The #183 response replacement path fits wet bounds and publishes the response, render and shape together. `renderReverseConvolution` reverses the IR samples before convolution. The host supplies current block timing, while the existing Song live and offline paths admit a compatible fixed mapping. Loop has committed phrase tempo.

## Goals / Non-Goals

- Preserve the experimental sound while retiming its events, and use the listening comparison to decide whether the production source is ready.
- Keep musical timing resolution, response preparation and render publication distinct. Tempo observation is cheap and response generation happens on the worker.
- Preserve the existing Song and Loop admission and Envelope contracts. This design does not add a host tempo map, new automation parameters, or a new handover mechanism.

## Decisions

### Start with the measured PCM

Use the owner-supplied Creative IR Crescendo v2 file `12_Stereo_Ping_Pong_Crescendo.wav` (SHA-256 `bc5ee590f3d4caff32d639d6b5d1b6320251ad53adee7fd7c7c34694cdede91d`). It is stereo 24-bit PCM at 48 kHz, 120,000 frames. Analysis in audible order found 11 isolated, alternating bursts with about 62.5 ms support, about 220 ms peak spacing and a rise of about 12 dB. The strongest peak is about 30.4 ms before the response end. Use measured sample coordinates as the basis for extraction. The owner-supplied synthesis notes describe a proposed recipe, and the original source code remains unavailable.

First render the unchanged reference and two arrival-relative variants at 150 BPM, with 200 ms eighth-note and 300 ms dotted-eighth spacing. Keep the original burst PCM, side, relative gain and final-hit offset in these baseline clips. Add ruler-aligned variants at those rates and compare a snapped arrival, a deliberate off-grid arrival, and positions just before and after the next eligible grid boundary. Exercise both Target and Move Wet, then trim and fade the same swell. Verify grid positions with a dry stereo impulse through the existing engine, then render a shared musical source with matching arrival, Level and Envelope within each comparison. Keep gain fixed and use a common timeline so the final gap, stereo sequence and timing differences remain audible. A failed comparison sends the work back to the response design.

Choose the shipped source after the listening checkpoint and record its provenance and version. Verified, cleared reference burst kernels are a direct path. If original material cannot be shipped, deterministic resonant kernels from filtered noise and damped oscillators are a candidate subject to a fresh comparison with the same reference. Production preparation must work offline without Drive or the original external file.

### Place hits relative to the arrival

With Sync on, share the timing authority behind the ruler. Song Beat 1 is PPQ zero under the ruler's accepted sample/PPQ mapping. Loop uses the committed phrase's Bar 1 and tempo in phrase-local coordinates. Generate a continuous grid from that origin at the Echo Rate, including across bar lines for dotted rates. Use quarter-note beat duration multiplied by 1, 1/2, 3/4, 1/4, 3/8 or 1/8, and round each grid point independently. The spike measures alignment at each burst's peak in an impulse render.

Choose the latest grid point that can hold the full final burst within the available pre-arrival response support, then work backward. Account for the effective wet placement when mapping these absolute hits into the stored IR and reverse that orientation once. Retain up to 11 bursts within the authored 2.5-second cap and available render support, omitting the earliest bursts first as needed. An off-grid target remains where the user put it. Its final gap can vary and the last eligible hit can change by a subdivision across a boundary. Crop, fades and curves may change which hits can be heard without shifting their scheduled positions. The spike must verify the full output path, since placing hits correctly in the raw IR alone does not account for Move Wet or the existing shape fitter.

Use the authored 220 ms eighth-note spacing as the Sync-off clock, equivalent to about 136.36 BPM. Sync off uses arrival-relative timing, retains the reference final-hit offset and still allows all six subdivisions. The approved preview's starting values are Sync on, eighth notes, 85% Spread, 75% Build and 0 dB Level for a new default. Use the unchanged burst channels and gains for the timing-only comparison. The preset pans alternating bursts with equal-power gains. Revision 3 varies the gain progression around the reference build and offsets musical gain matching so it preserves the intended change in late-hit level. Existing headroom protection applies. Keep the accepted assets for saved revisions 1 and 2. Leave Level and Envelope on their existing update paths.

### Resolve tempo and publish an accepted revision

Resolve fixed Song tempo from a compatible mapping for the target and Loop tempo from committed phrase timing. A Learn default may use the authored reference clock before a compatible target exists. New assignments without valid Sync timing can prepare a marked reference-tempo response. For an existing synced assignment, keep the accepted response while timing is unavailable. A newly authoritative compatible tempo can queue eligible work. Seeks and unstable host observations must not churn requests or displace accepted audio. Keep current host wet-admission behavior, including explicit rejection of incompatible live or offline mappings.

Coalesce work by definition and source version, canonical creative values, effective tempo, sample rate, response extent and the rounded event offsets relative to the final render placement. If an edit changes the offsets, prepare off the audio callback and publish placement and its matching response together. Keep the last coherent accepted placement and render while replacement is pending. Reuse immutable managed responses when the event offsets match. A response edit carries the existing session, scope, target and revision guards through preparation and atomic publication. If Freeze prevents a required rephase, keep the accepted frozen result and explain why the edit cannot publish. Host offline bounce and internal Export use accepted snapshots. A preparation finishing during either operation can affect a subsequent eligible snapshot.

### Reuse the shared editor

Bind the versioned definition through the existing Learn and Perform Sound editor. Place the labeled Tempo Sync switch above the grid. Put Echo Rate and Stereo Spread in the first row, Build Intensity and Gate Amount in the second, and Damping and Level in the third. Reuse the shared stepped dial and read-only value, exact-entry knobs, selector waveform, delayed panel overlay and Freeze help. Use contextual feedback for authored or retained timing. No new control family, geometry token or one-off styling is planned. Inspect the real controls at 900×600 and expanded, with existing scale behavior, before accepting integration.

Gate Amount blends the original stereo wet excitation with revision 4's shared excitation, which retains the attack and tapers around the arrival over one echo interval. The render worker applies the blend before convolution. At 0% use the original path and at 100% use the revision 4 path. Include intermediate gate values in the render dependency while reusing the same IR. Store the dial value in preset revision 5. Revisions 1–3 load at 0% and revision 4 at 100%, retaining their accepted renders and assets.

Damping smoothly reduces each prepared burst away from its existing peak. Revision 7 retains the revision 6 curve through 50%, the moderate preview setting. Above 50% it steepens the taper, progressively narrows the protected region around each peak to 0.25 ms, and lowers the musical matching target by up to 6 dB so the ringing can recede in the mix. Compute the taper in the response worker so the changed PCM determines the response and render identities. A value of 0% skips the taper. Revisions before 6 load with 0% Damping. Revision 6 retains its saved value and accepted render on reopen.

## Risks / Trade-offs

- Close subdivisions can overlap bursts at high tempos. Preserve event anchors and kernel duration, bound mix and clipping, and include the tightest supported case in the spike evidence.
- A full tempo map would require transport changes beyond the existing fixed mappings. Keep changes within a lead-in and tempo ramps outside the supported scope, and verify the explicit timing feedback.
- A slower subdivision can yield fewer hits and change the perceived build. Omit early hits first and listen for a convincing approach to the same arrival.
- Grid-locked timing can leave a larger final gap or expose a stereo side change at a boundary. Listen across the boundary and record the behavior before production integration.

## Migration Plan

Add the new definition alongside existing bundled and custom identities. Preserve saved settings, exact managed responses and compatibility identifiers for existing projects. Keep Ping Pong unavailable in production until listening acceptance and source provenance are recorded. Reopening a saved Ping Pong assignment uses its verified accepted asset. A supported rebuild uses the same versioned inputs, and an unavailable exact response uses existing recovery feedback.

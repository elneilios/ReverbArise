# Proposal

## Why

Issue [#11](https://github.com/elneilios/ReverbArise/issues/11) needs a focused learning workspace after #10's safe Capture/Live/Finalise foundation. The temporary read-only swell editor obscures capture coverage, and current default assignment can read mutable settings long after a pass was recorded.

## What Changes

- Replace the Capture-mode body with the approved narrow left settings column and one large right full-song dry timeline. Hide the Swells list, selected inspector, audition band and duplicate bottom Overview in this mode; preserve the Perform editor.
- Expose independent Recapture and Automatic Swells controls. Automatic Swells off hides generation preferences and learns dry only, without automatically starting analysis/convolution; retain hidden values, source and existing swell definitions.
- **BREAKING scheduling change:** dry-only learning also blocks legacy forced-Recapture refresh from launching automatic convolution; explicit permitted manual Perform work remains available.
- Provide the real bundled Default IR catalogue and Open from File using the existing WAV/WavPack importer. Pending/cancelled/failed imports retain the previous accepted default and disclose status.
- Use a horizontal Marker Sensitivity slider with a typeable percentage. Omit Character presets/parameters until their DSP actually exists.
- **Confirmed decision:** lock Automatic Swells, Default IR and Marker Sensitivity from DAW Play through Stop validation. Freeze an immutable generation context for that pass and persist its association with accepted source, so deferred work and reopen use the intended settings.
- Show real provisional capture separately from committed audio/silence, uncaptured gaps and unavailable assets. Add a concise coverage key/readout, absolute ruler/playhead and stable origin-anchored auto-fit, including sparse seeks.
- Preserve #10's atomic Stop publication, automatic entry into Perform after useful acceptance, dry passthrough, source protection and bounded realtime processing. Require early native visual checkpoints and a short consolidated host checklist.

## Capabilities

### New Capabilities

None; extend the existing capture/workspace/session boundaries.

### Modified Capabilities

- `swell-workspace`: Dedicated Learn composition, progressive controls, pass locks, real default-IR selection and legible coverage.
- `song-timeline-audio`: Bounded provisional display, independent committed readers, stable full-song extent and coherent capture-context publication.
- `automatic-swell-processing`: Immutable pass defaults, deterministic deferred assignment, and dry-only learning with automatic generation off.
- `swell-session`: Persist selected defaults and accepted source/context associations; preserve old sessions and interrupted-work semantics.

## Impact

Expected areas: editor layout/components and Settings bindings; capture worker/session snapshots and versioned manifest metadata; automatic target reconciliation/import jobs; session migration and host-save notifications; focused source, UI, assignment, persistence and realtime regression tests.

Implementation depends on #10 (PR #18) and #16's separate Learn/Perform/Export display-name change. This branch starts from #10's implementation for planning only; implementation must reconcile the merged baseline first. Do not duplicate #16 or rename persisted/internal identities.

Not included: Character DSP, Live bulk editing (#9), full-timeline audition (#5), export workspace/composition (#6), storage relocation/cleanup (#8), or Live Pattern/Song discrimination (#1). Reuse approved branding and 1100x850 opening size; mockup gradients/cards or speculative controls do not override the flat style guide.

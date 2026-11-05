# Proposal

## Why

GitHub issue: #51 (https://github.com/elneilios/ReverbArise/issues/51).
The Export crossfade reduces both branches and its aggregate protection further changes unity layering. Independent levels will provide predictable additive output and musical starting points while preserving accepted-revision safety.

## What Changes

- Replace the crossfade with independent editable Dry and Wet percentages, each 0–100%, with 100% meaning branch unity.
- Use transparent addition, preserving per-swell calibration/Level/shape but removing Export wet-aggregate and combined attenuation. Show measured peak/overload information and retain above-unity Float32 samples. No new output attenuation control.
- Add a compact accessible Mix Character selector that changes only the two levels; proposed preset values remain subject to listening/native review.
- Make waveform, legend, audible Fit, preview, filenames, immutable identities and readiness agree with the accepted pair; 0/0 is an explicit silent disabled state.
- **BREAKING**: replace the developer-only crossfade preference with a documented independent-level state migration/default reset; preserve host identity, captured assets and immutable export history.
- Preserve #6 composition, live-wet bypass, bounded preparation, cancellation, Save/drag ownership and song alignment. Require native control approval before integration and one final targeted FL Studio pass.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `song-export`: independent gain plan, pair-based revision identity/state, preset interaction, truthful waveform/peak presentation and delivery.

## Impact

Touches SongExport/ExportState serialization and identity, ExportSource branch gating, SongMix measurements/read/detail, SwellEngine export requests/publication, ExportVoice preview parity, ExportFiles naming/verification and ExportWorkspace shared-control bindings. Extend SongMixCases, ExportControlCases and export/persistence integration coverage. Update current song-export and Export workspace guidance plus dated evidence. No changes to Perform/Learn DSP, VST3 identity, #5 audition or #30 processing. Owner approved this headroom policy in chat on 2026-11-05; Apply still requires a fresh request.

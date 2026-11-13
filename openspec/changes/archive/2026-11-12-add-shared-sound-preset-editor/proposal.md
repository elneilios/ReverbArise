# Proposal

## Why

Learn and Perform share an IR chooser today, but they do not share a complete Sound editor. The versioned Sound assignments delivered in [#190](https://github.com/elneilios/ReverbArise/issues/190) let both workspaces present musical presets while preserving accepted responses and saved projects.

[GitHub issue #191](https://github.com/elneilios/ReverbArise/issues/191) · [Epic #189](https://github.com/elneilios/ReverbArise/issues/189) · [Approved interactive preview](https://github.com/elneilios/ReverbArise/blob/29be4f1973653540b6bf57571d9d6562537e92c5/docs/design/sound-preset-system/ReverbArise-Sound-Preset-Preview.html)

## What Changes

- Use one Sound editor and guarded binding in Learn Settings and Selected Swell. Expose wrapped Previous/Next, direct selection, and editable Level in both. Present only the controls supported by the selected definition.
- Group retained managed files and WAV/WavPack import under Custom IR. Preserve the accepted response until a compatible replacement succeeds.
- Put decorative response waveforms behind the closed selector and each available menu option. Keep descriptions below the controls and route disabled-control help to the existing footer.
- Show the approved panel tint, waveform and accessible caption only when preparation exceeds two seconds. Keep workspace navigation and the fixed Reset strip available.
- Review the composition and scoped control variants natively before production adoption. Verify compact and expanded layouts, keyboard and accessibility behavior, guarded edits, and the existing IR handover.

## Capabilities

### New Capabilities

### Modified Capabilities

- `swell-workspace` — shared preset and contextual file selection, Level editing, guarded presentation and preparation feedback in Learn and Perform.

## Impact

The change affects the shared Sound control and binding, their Learn and Perform hosts, and presentation-only IR preview generation and caching. It reuses the #190 assignment and Level paths, including the #183 shorter-response handover. The current delivery exposes the shipped bundled presets and Custom IR. Future creative definitions, host automation, persistence schema changes, and undo history are outside #191.

# Issue #6 — brief FL Studio acceptance

Use the Release bundle from the validated build:
`out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.
The build does not install it or update an already loaded FL Studio instance.
Binary identity and final build results are recorded in
[implementation evidence](2026-11-04-song-export-implementation.md).

Use the usual FL Studio 26.1.7 / 48 kHz / 512-sample setup unless changed.
Native visual acceptance is already complete; do not repeat component, sparse
coverage, failure-injection or software-raster tests.

1. **Preview and delivered audio:** with a project containing several prepared
   swells, stop the DAW and compare Export preview at 0%, 50% and 100% Wet. Check
   that the saved WAV agrees with the preview, including swell lead-ins/tails.
   The waveform Fit can omit leading silence; the WAV remains song-aligned.
2. **Delivery and routing:** drag a Ready WAV into the Playlist and place it at
   song origin. If FL Studio declines the drag, use Save WAV and report the drag
   as unsupported. Open Save once and cancel, then save successfully. With an
   intermediate dry-inclusive mix, mute the original source; with wet-only,
   retain the original. Stay in Export during WAV playback to avoid extra wet.
3. **Persistence:** save/reopen the project. Export percentage and the published
   file reference should survive, without autoplay or modifying the WAV already
   imported into the Playlist.

Report Pass/Fail for each group, noting any untested delivery path or changed
setup. Actual Windows DPI coverage remains unverified unless explicitly tested.

The fuller musical ruler and shared navigation controls are a coordinated #46
integration, separate from this current visual acceptance. Do not merge or
archive #6 until that integration completes or the owner explicitly moves it
to a follow-up issue.

## Owner result — 2026-11-04

All three groups passed on the usual setup; no setup changes or untested paths
were reported. This includes actual preview/WAV comparison, Ready-file delivery,
Save cancellation/success, routing guidance and save/reopen retention. No separate
Windows DPI sweep or forced unsupported-drag host configuration was performed.
The requested future click/drag preview positioning is recorded in the Export
handover, rather than treated as a failure of this accepted implementation.

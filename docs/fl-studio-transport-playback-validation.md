# M4 Song playback - FL Studio validation

User acceptance now covers host checks 7.1, 7.2, 7.4 and 7.5. Task 7.3 remains open because envelope extremes still failed. Predecessor M3/workspace acceptance is recorded separately below.

Use a saved copy of your project. Close/remove the old plugin before replacing its loaded binary. The complete bundle is `out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3`; the exact tested hash will be recorded in the transport guide after final verification. Record FL Studio/Windows versions, driver, rate, buffer size and underruns. Previous settings were FL Studio 26.1.7 build 5653, Windows 11, FL Studio ASIO, 48 kHz, 512 samples; record actual values if different.

Repeatable media is generated in `out/test-media/transport/`: `m2-source.wav` (stereo, 48 kHz, two seconds with a decaying 220 Hz note), `m2-ir-mono.wav` and `m2-ir-stereo.wav` (half-second generated responses). No private Large Hall IR is included. At 120 BPM place source copies at bars 3, 6 and 9 on a routed Playlist track. Use bundled Preverb Hall or load a generated response, leaving room before the first note. Capture a complete pass, stop and wait for preparation, then replay from before a swell. First-pass future input cannot supply elapsed preroll.

1. **Song result and routing - task 7.1.** Enable Live playback, Dry + Wet, Live wet 0 dB. Press FL Studio Song Play: hear prepared swells without pressing Audition. Compare Live playback off (dry only) and live Wet Only. Change selection and Isolate selected: neither filters live results. Create overlapping swells; test enabled/mute/solo and signed wet placement. Dry notes must remain fixed. Captured dry is never replayed through the live path. Monitor combined levels: protection applies only to wet.

2. **Position and readiness - task 7.2.** Start inside a prepared swell and seek within the prepared window: join at the actual offset. Loop across overlaps, with the loop start inside a swell. Seek far outside the window: expect a temporary missing/preparing message followed by a current-position join, never a replay of missed preroll. Supplied loop destinations are prefetched separately. Record whether FL Studio supplies usable loop points and any misses.

3. **Edits, boundaries and handles - task 7.3.** Stop/restart on nonzero wet, toggle routing/enable, change level and edit/move a swell while playing. Listen for clicks or sustained duplicate preview. Start Audition while stopped, then host Play: live playback takes priority; host Stop must not restart Audition. Disable live playback to use independent Audition while transport runs. Compare identical and changed recapture; valid neighbours should continue during changed wet preparation. Drag endpoints/fades/tension to extremes, including zero fades, crossed endpoints and viewport edges. Stationary drags must not jump; returning within the same endpoint gesture should restore fade intent. Test Snap both ways and numeric corrections.

4. **Restore and failures - task 7.4.** Save/reopen with edited swells and live preferences; wait for validation and fresh host timing, then Play. Close the editor while listening. On a separate copy test missing/corrupt managed assets and incompatible tempo/rate: affected wet must not be presented as current. Labeled manual last-committed Wet Only recovery must stay out of Song playback. Restore compatible settings and regenerate/recapture as directed. Preserve original assets for recovery.

5. **Host scheduling and bounce - task 7.5.** Test routed-silent input when a prepared swell is due, and a fully idle insert with smart disable enabled/disabled. Record callback suspension or missing audio; plugin code cannot generate output without callbacks. Export the complete Song to WAV using offline rendering, including a cold start and larger blocks if used by the host. Every eligible captured/prepared swell should be present. Compare WAV and realtime playback at the same gain, inspect the persistent export status, and report any `Export incomplete` message. Host-marked offline rendering now prepares pages synchronously. Project packaging and standalone export controls remain later work.

For each check report pass/fail, position/range, settings and observed issues. Leave incomplete OpenSpec checks unchecked.

Loop-feedback retest: load the replacement loop-feedback build identified in the transport guide, keep automatic processing and mapping enabled, and let the Song complete one backward loop so its boundary is observed if FL Studio supplies no cycle points. Check that the final untouched automatic marker becomes Ready. Continue for several unchanged passes: existing definitions should remain Ready with stable captures/renders, then remain settled after Stop. Change actual source audio and confirm only dependent results refresh. Check a manually edited swell retains its source/envelope choices, then save/reopen and verify the fitted final result remains usable. These are still host checks; automated results do not establish an audible pass.

## Performance/export feedback retest

- Loop a live/performance source at default Regeneration sensitivity 50%. Lower it if minor variations still refresh too often. Verify larger changes still update; 100% restores exact comparisons. Marker sensitivity should not move. Stop and check statuses settle.
- Freeze one ready swell, change or recapture its source, and compare with an unfrozen neighbour. Frozen wet should stay fixed while its gain/envelope/placement remain editable. Unfreeze and confirm it updates. Save/reopen and verify both sensitivity and Freeze render choices.
- Capture the full Song first. Export WAV immediately after seeking to an unprepared distant position, and compare every expected swell against realtime playback, including the last marker and overlaps. Repeat with a larger export block size if available. Export must not record its own input or continually refresh source definitions. Record any export diagnostic and whether FL Studio identifies offline processing.
- Compare Live Wet Only and Dry + Wet exports, then return to normal Song playback. Check no dry leak at the start of Wet Only export and no stuck processing state after bounce.

These checks are pending user results; automated export tests do not establish FL Studio host acceptance.

## Final marker and two-instance export retest

1. Reload the replacement bundle in both mixer inserts. Loop the complete Song at the original fractional tempo; after a closed pass, the untouched final automatic marker should become Ready. Save/reopen and repeat without changing the marker manually.
2. Export the two tracks together, then each separately. Compare both wet contributions with live playback. Available swells must render even if another marker is deliberately left waiting; a warning must identify the skipped marker. Test both Dry + Wet and Wet Only.
3. In Diagnostics, record `Offline blocks` and the last export result for each instance before/after export. If an insert remains absent, report whether its count increased and the exact warning/error. A zero/unchanged count points to host scheduling; it does not establish a decoder or mixer failure.
4. Retest the final marker after reopen, and confirm manually edited/frozen endpoints were not shortened.

All these host checks remain pending. The available-swell export policy supersedes the earlier whole-instance refusal on any missing dependency.

Offline completion follow-up: export the same complete Song and verify final swells remain present. After export returns, the plugin should retain a successful result or legitimate skipped-swell warning instead of `Song mapping changed during export` caused solely by a PPQ reset after the last wet interval. If it still reports a mapping failure, copy the new sample/PPQ/expected/final-end details. Check both Dry + Wet and Wet Only. This host check is pending.

## Reported combined acceptance, 2026-11-01

User reports groups 1 (automatic capture/detection), 2 (sparse capture/IRs), 5 (regeneration/recapture), 6 (Song playback/navigation), 7 (save/reopen/recovery), and 8 (silent inserts/export) pass. Group 3 fails envelope usability; screenshot shows displaced fade/tension controls and crossed connectors. Group 4 passes wet isolation and remaining audition behavior but reports dry outside the selected trigger and requests a preview playhead. The precise dry span was hard to identify without that cursor. User accepts a practical dry-isolation option.

Recorded full task acceptance: M3 10.2/10.3; workspace 8.1/8.2/8.5/13.2/19.6; transport 7.1/7.2/7.4/7.5. Mixed envelope/audition tasks remain open: M3 10.1; workspace 8.3/8.4/13.3/18.3; transport 7.3. Earlier recorded host/audio settings remain the reference; the exact binary loaded for this report was not independently confirmed. No archives were performed.

Focused retest after the envelope/audition follow-up: repeat check 3 with full/zero/overlapping fades, maximum tension and small/large windows. Handles should remain aligned with their sample times and separately grabbable. For check 4, compare context Dry + Wet with Isolate dry source, follow the pink Audition playhead and highlighted dry span, then test Stop/completion and one/two-second tails. Report any dry replay before/after the highlighted span separately from unchanged live plugin input. Previously passed groups do not need repeating unless a regression appears. These two checks remain open.

Handle-stacking retest: at a distant zoom, close controls should cover each other on the real curve instead of spreading into floating callouts. Zoom in to reach individual fade/tension/end controls; the visible topmost control should receive clicks. Zero-length fades must remain separately grabbable because they genuinely coincide with an endpoint. Repeat extreme drags, viewport-edge clicks and return to a close zoom. Envelope/audition acceptance remains open.

Audition acceptance follow-up (2026-11-01): user explicitly reports the new audition playhead and isolation work well. Combined with the earlier confirmation that the rest of audition check 4 was fine, workspace task 8.3 is accepted. Envelope-related mixed tasks remain open pending the true-position stacking retest; this does not imply envelope acceptance or authorize archival of unfinished changes.

Centre-line endpoint retest: compare zero fades with nonzero and full fades. Start/End squares must remain vertically centred on the waveform; fade/tension controls stay on the envelope. Check the two zero-fade circles are independently grabbable without artificial displacement, then drag start/end and verify their timing and z stacking still behave correctly. Envelope acceptance remains open.

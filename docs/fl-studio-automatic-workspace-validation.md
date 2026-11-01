# Automatic workspace — FL Studio acceptance

All host checks below are **pending**. Automated results are separate and cannot
establish host audibility or user acceptance. Continue M3 signoff alongside this
change; do not mark its three outstanding host tasks passed by implication.

Use the newly built `out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3`.
Close/remove the old instance before replacing a loaded binary. Use a new test
project or saved copy. Record FL Studio version, Windows version, driver, sample
rate, buffer length, routing and any underruns with the results. The previously
reported setup was FL Studio 26.1.7 build 5653, Windows 11, FL Studio ASIO,
48 kHz and 512 samples; record actual settings if changed.

1. **Fresh insert, no plugin preparation buttons (task 8.1).** Route musical input
   through a new instance, choose FL Studio Song mode and press host Play. Leave
   Diagnostics closed. Coverage and automatic targets should appear, then complete
   source windows should render using Preverb Hall. Wait for Ready, stop the host,
   select a swell and press Audition. Confirm useful Wet Only output at Trim 0 dB.
   An incomplete last source window should say Waiting for source until that tail
   is captured. This change does not play swells automatically with Song transport.

2. **Sparse capture, ownership and refresh (8.2).** Capture bars 86–90, then 1–4;
   the gap must stay unmapped. Replay unchanged audio: IDs/counts should remain
   stable with no unnecessary renders. Rename, shape/move and delete targets;
   replay and confirm edits/deletions survive. Assign different WAV/WavPack IRs
   through the inspector; changing one must leave neighbors unchanged. Recapture
   changed audio, including rapid source/IR edits, and check only dependencies
   update. Inspect drop/resource/error diagnostics and responsiveness.

3. **Monitoring (8.3).** With the host stopped but processing the insert, compare
   Wet Only with Dry + Wet at 0%, 50% and 100%. The original dry attack must align
   after wet pre-roll and remain in original order. Return between modes and check
   the mixed percentage survives. Save a nonzero trim, check its visible indication
   and reset it to zero. Stop/change selection and confirm old audio stops. In a
   controlled project copy with missing dry assets, Dry + Wet must be unavailable;
   only independently verified, explicitly labeled last-committed wet can audition.
   Do not remove working assets from the main project.

4. **UI and recovery (8.4).** Check default, 1100x850 and larger sizes; select many
   swells, pan/zoom/snap, navigate the full-width overview, drag handles and type
   musical/fade values. Open Source and audible timing for precise bounds. Confirm
   no overlap or inaccessible controls; scroll inspector/Diagnostics where needed.
   Exercise retained manual analysis, regeneration, recapture and asset tools.
   Record whether the layout is usable and matches the intended wireframe hierarchy.

5. **Save/reopen (8.5).** Save different IRs, edited/protected targets, a deleted
   target and mixed audition preferences. Reopen: all identities/edits/settings
   should survive; no audition should start. Mapping resumes only on permitted
   valid host playback. Open an M3 project copy: it must retain edits/trim and use
   protected manual mode until automation is enabled. Test missing/corrupt assets
   and render rebuild only in controlled copies, recording unaffected swells and
   explicit recovery states.

Record results against tasks 8.1–8.5 and the still-open M3 checklist. M3's former
-12 dB fresh default and manual/off new-instance startup are intentionally replaced
by this successor; saved trim values and manual recovery behavior remain supported.
After actual signoff, sync/archive M3 first, then this change, through the explicit
OpenSpec workflows so the older deltas cannot overwrite successor requirements.

Transport-synchronized second-pass playback, starts inside a swell, overlap mixing,
seek/loop scheduling and wet playback on silent/unused inserts are deferred to the
transport change. They are not claimed as passing here.


## Feedback revision acceptance (pending)

These checks are additional to the original five host checks; no host pass is claimed by automated tests.

1. Start with Sensitivity 0.50 / separation 80 ms. Capture isolated plucks with long decays, a noisy tail, a modulated sustained note, a soft attack and clearly separate repeated notes. Check one target for each isolated event, useful arrival placement and retained genuine repeats. Repeat capture unchanged; verify stable IDs, edited names/shapes and deleted-target suppression. Record actual audio, tempo, buffer settings and results for task 13.2.
2. Select a swell. Use its square wet start/end handles, circular fade-duration handles and diamond tension handles; hover for labels. Shift-drag for fine movement and compare the typeable inspector values. With Song Snap on, check envelope motion stays fine. Check zero fades can be opened numerically and neighboring points remain selectable. Inspect at 1100x850, 1400x900 and 1600x1000 with Diagnostics both closed and open.
3. Move wet end earlier than the dry arrival to create a gap, then later to overlap. Check the arrival marker and base extraction remain fixed. Adjust fade-out duration and tension independently. For an endpoint beyond existing support, verify the amber source extension and disclosure, requested ghost end and honest Waiting for source if needed; capture through the requested endpoint and verify completion. Extra source may include later notes. Confirm dry timing and level remain unchanged (apart from explicit monitoring blend/trim/protection).
4. In Dry + Wet, test Listen after at 0, 1 and 2 seconds, and blends 0/50/100%. Hear the original source after the wet transition, without a dry fade-in or duck. A longer selected source must play fully. Missing required context should disable Dry + Wet with the needed range, while verified Wet Only still works with no added tail. Capture the missing context and confirm it becomes available. Listen for clicks during monitor changes.
5. Save/reopen with a gap/overlap, non-neutral tensions, nonzero fade-out, custom IR, monitoring mode/blend/tail/trim and edited/suppressed targets. Verify all settings and timing survive without autoplay. Report explicit usability/transition acceptance for task 13.3. Transport-synchronized swell playback remains a separate follow-up.


## Second feedback host acceptance (pending: tasks 18.3 and 19.6)

- With the new build, create a fresh manual/automatic swell and check its 20 ms overlap/20 ms Smooth wet release. Open a saved zero-fade swell and verify its edits remain intact. Grab both zero-duration fade controls directly; test label/leader collision handling at several zooms and all three supported editor sizes with Diagnostics open/closed.
- Drag the wet body earlier/later, including before Song zero; check all envelope points/audio move together and fade/tension/length stay fixed. Try Move wet (ms) and one-sample arrows. The original dry event and base extraction must stay fixed. Extend an individual endpoint after moving and check genuine source support, waiting and cancellation. Save/reopen without autoplay.
- Listen to zero-fade wet cuts, natural audition completion, Stop/replay and rapid monitoring edits using sustained/nonzero-ending material. Check the short disclosed wet/outer safety ramps eliminate boundary clicks while the musical transition remains usable. Original dry stays unshifted/unfaded by wet controls; only recorded replay has short outer boundary ramps.
- Arrange overlapping swells, including a chain extending outside the anchor window. Default audition should mix eligible overlaps only inside that fixed window; captured dry plays once. Compare Isolate selected, mode/blend/tail/trim and mute/solo, including a muted anchor with audible neighbours. Required bad members should identify themselves and block overlap listening; isolate a valid selection explicitly to recover. Verify saved scope/placement and no automatic playback.
- On busy captured input, turn the prominent Marker sensitivity knob down and compare salient attacks versus minor events at low/default/high values without changing separation. Test typed values and changes while playing/stopped. Record actual musical material, count/placement usefulness and limitations; this is event selectivity, not key-change recognition.
- Start with Auto markers On, switch it Off during capture/analysis, and verify no automatic targets publish afterward while capture/manual editing and existing-render refresh continue. Adjust sensitivity while Off, enable again and check catch-up retains manual edits and deletion suppression. Save Off with a nondefault value and reopen; also check Master paused when the existing processing master is disabled. Record actual FL Studio/audio settings and explicit acceptance rather than marking host checks from automated results.

# FL Studio M3 Validation

Status: implementation and automated verification complete; all M3 host acceptance
tasks below remain pending. M2's earlier acceptance is not evidence for M3.

## Build and setup

Build/test commands are in [development.md](../development/development.md). Close FL Studio before
replacing the plugin with this bundle:

`out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3`

Use a separate test project and a fresh plugin instance, fixed 120 BPM, 4/4, 48 kHz,
Song mode, tone off. Keep the driver buffer at the previously tested 512 samples
unless testing another setting. Record actual FL Studio/Windows/driver versions,
configured buffer, observed block sizes, underruns and dropped capture blocks.
The previous M2 setup was FL Studio 26.1.7 build 5653, Windows 11 Pro 26H2 build
26300.9807, FL Studio ASIO, 48 kHz / 512 samples; reconfirm it for M3.

Use existing `out/test-media/m2/m2-source.wav`, `m2-ir-mono.wav` and
`m2-ir-stereo.wav`. If absent, run `preverb_create_m2_test_media.exe`; it refuses to
overwrite existing files. Use a duplicate of the user's large-hall IR for external
file-move tests. Never manipulate another project's working assets.

## 10.1: Musical editing and independent swells

1. Place the source WAV at bars 1 and 5 without stretching and route it to Preverb.
   Map bar 1 through bar 2, stop/disable mapping and wait for committed capture.
   Separately map bar 5 through bar 6. The middle unvisited region stays uncaptured,
   while played zero input is covered silence. Check dropped blocks remain zero.
2. Add target A at bar 1 by double-clicking source with Snap on; its inspector arrival
   should be sample 0. Suggest onsets, select/confirm a candidate near the second clip,
   name it B, and adjust its arrival numerically to 384000 for exact comparisons.
   Suggestions stay visually distinct; reanalysis/sensitivity changes preserve A/B
   and their edits. Also test manual add, move, delete and multiple list selection.
3. Select A/B, load the generated mono IR and Regenerate all. Defaults select a
   quarter-note source: A [0,24000), B [384000,408000). For the 24000-frame IR, raw
   results have 47999 frames: A [-23999,24000), B [360001,408000). If a confirmed
   suggestion initially differs, correct the arrival/source fields before comparing.
   The first raw result extends before bar 1; its energy peak need not equal arrival.
4. Select B, assign the stereo IR and Regenerate selected. A's render/edit state stays
   unchanged. Repeat with the native WavPack large-hall IR and file-dialog Cancel.
   Missing/invalid IRs and source gaps must report errors without partial ready output.
5. Select A. Adjust crop handles, independent fades/curves and gain; try numeric fields
   and drag with Snap off. Raw frame count/bounds stay unchanged and no convolution
   starts. Full response exposes the entire raw result; Reset shape restores raw start,
   arrival+1 exclusive end, zero fades and 0 dB gain. Narrowing bounds clamps fades;
   overlapping fades combine. Test an explicit crop that no longer fits a shorter IR:
   retain invalid intent with an explanation until corrected/reset.
6. Page/zoom/scroll, click the overview and inspect source/targets/wet at distant and
   negative positions. Navigation never moves or trims edits. Confirm readable raw
   and shaped waveforms, a labeled display scale, and no flat-topped over-full-scale
   drawing. Check all controls at the minimum supported editor size.
7. Audition one selected swell, then Stop/replay. Confirm edits, mute/enable/solo rules,
   smooth audition-level changes, live dry passthrough, and wet peak protection.
   Selection/shape/activity changes stop preview and do not start another voice.
   Seek/loop and stopped transport must not make audition follow Song position.
   Pressing FL Studio Play alone does not schedule swells in M3; that remains M4.

Result: pending user acceptance.

## 10.2: Regeneration and scoped recapture

1. Reset A's shape; change only A's source end to 12000. Only A becomes stale;
   Regenerate selected gives A 35999 raw frames, still starting at -23999, ending
   at 12000. B remains unchanged. Regenerate all reuses current results and processes
   missing/stale definitions even when muted/disabled; one failed source must not
   prevent another valid swell from completing.
2. With ordinary auto-refresh off, repeat exactly the same source-file capture over
   A. Identical decoded samples retain the render; changed samples stale only A.
   For changed input, alter the clip gain before recapturing. Regenerate explicitly.
   An unrelated captured section must not invalidate A/B. Instrument randomness can
   change samples even when a performance sounds similar; use the WAV fixture here.
3. Click Recapture & regenerate with A selected. Read its fresh-coverage range and
   manual playhead/Play instruction. Stop before the end: old source coverage cannot
   complete the refresh. Fill the missing part on a second pass, including a seek or
   loop if desired. Only input inside the armed range replaces capture; outside data
   stays intact. Complete fresh committed coverage disarms and refreshes affected
   changed swells, independently of the ordinary auto-refresh preference.
4. Cancel a partial arm and verify no fabricated coverage or automatic transport
   movement. Enable ordinary auto-refresh, recapture changed source and verify latest
   affected work is queued; repeated edits/commits cannot publish obsolete results.
   Change/delete a target or assign another IR while rendering to test supersession.
5. Audition A while mapping/arming an unrelated empty range. Covered source must be
   silence, with no wet swell captured. Generate while FL Studio plays/captures:
   check responsiveness, uninterrupted dry processing and zero dropped blocks in
   the tested setup. Record stopped-host processing or any manual playback fallback.
6. Change tempo/meter and sample rate: old audition stops, results become stale,
   and capture rate mismatch prevents incompatible recording. Restore settings and
   explicitly verify/regenerate. Do not expect tempo-map support or time stretching.

Result: pending user acceptance.

## 10.3: Same-PC persistence and dependency recovery

1. Save the dedicated test project with two edited swells, named targets, distinct IRs,
   nonzero fades/curves, changed source bounds, activity/selection settings, audition
   level -24 dB and automatic-refresh preference. Record these values. Close/reopen.
   Edits/settings/source/tone survive; audition stays stopped, mapping/arms stay off.
   Wait for capture/timing/asset checks, then audition the restored selection.
2. Close the project, rename the original external IR test copy, and reopen. The managed
   import still works without selecting the external file again. Restore its filename
   afterward if desired; the import itself must not have modified the original.
3. Select A and click Copy asset info; paste the paths into a note. Save/close the test
   project. Temporarily move A's listed render WAV out of its managed directory,
   retaining it safely. Reopen: valid source/IR rebuild the missing render in the
   background, retaining shape/settings, without recapture or autoplay. The newly
   published render path can differ. Keep the moved file until testing is complete.
4. In the fresh test instance only, use the listed managed IR path to temporarily move
   A's IR WAV while the project is closed. Reopen: A must report a missing dependency
   and not audition as current; B with an independent valid IR remains accessible.
   Close, put the managed IR back at its original path, and reopen to recover. Do not
   replace it with unrelated audio under the old identity. General relinking is M5.
5. Save, then Clear while auditioning. Audio stops and source coverage clears; old
   previews become unavailable. Close without saving and reopen the saved test project:
   definitions and their old committed assets return. A separate saved copy must not
   have its immutable references overwritten by later edits.
6. Save while import/render/recapture is in flight, then reopen. Only committed assets
   can be ready; no arm or playback resumes. A legacy v2 M2 project restores capture
   and tone with an empty swell collection at -12 dB; unsaved M2 previews never existed
   in that state. Keep a pre-M3 project for rollback; an M2 binary cannot read v3 edits.

Result: pending user acceptance.

## Automated evidence

Final automated result, 1 November 2026: Windows MSVC Debug VST3 built successfully;
19/19 CTest executables passed (14.05 seconds). Strict OpenSpec validation passed.
The VST3 includes the WavPack decoder license. Main specs remain unchanged from M2;
only this change's delta specs describe the proposed M3 contracts until sync/archive.

Headless coverage includes model/metadata,
canonical managed IR/render storage, onset fixtures, shape numerical oracles, exact
content dependencies, multi-swell jobs and memory admission, fresh committed recapture,
processor v1/v2/v3 restoration and capture isolation, viewport transforms, and offscreen
UI interaction/layout. Software snapshots use `PREVERB_UI_M3_OUTPUT` with
`preverb_swell_ui_test`; they do not establish FL Studio host behavior.

Software-rendered ready, shaped, stale, error, empty and many-target states were
visually reviewed, including 1100x850, 1200x900 and 1600x1100 layouts. Interaction
tests cover target confirmation/add/delete/move, numeric and drag source/crop/fade
editing, gain, navigation without invalidation, invalid-crop visibility and 500-row
list virtualization. High-peak test audio has raw/shaped peaks above full scale;
the display preserves its complete envelope and audition remains protected.

The coordinator test restores over 256 MiB of logical render references with resident
working data below 32 MiB. Processor tests verify save-in-flight references, fresh
host timing after restore, v1/v2/v3 compatibility, missing-render rebuilding, moved
external IR, rate mismatch, prior-project recovery after Clear, allocation-free
audition/recapture callbacks, and exclusion of preview from captured dry audio.

No M3 FL Studio acceptance result is implied by these automated/offscreen results.

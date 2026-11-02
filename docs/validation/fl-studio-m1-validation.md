# FL Studio M1 Persistent Song Map Validation

Status: M1 accepted by the user; all tracked tasks closed on 1 November 2026.
Tasks 4.1 and 5.1 were closed by explicit user acceptance ("tick them off it's fine"),
following the previously confirmed save/reopen and overlap checks. No additional host
test was performed by the agent; the exact bars 86–90 / 1–4 sequence and captured-silence
visual state were not independently verified. Bar-ruler alignment has no separate
recorded host measurement.
Do not infer M1 host acceptance from the completed M0 report.

## First user test results (1 November 2026)

The supplied screenshot shows a working overview with waveform regions and a dark gap,
at 48 kHz, 82 BPM, 4/4, and 460-sample blocks. It reports 163 captured regions,
revision 107, 97 discontinuities, and **2,429 dropped blocks**. Capture throughput
needs correction and retesting; discontinuities alone cannot be attributed
to seeking rather than dropped capture blocks. Save/reopen and overlap preservation
have not yet been confirmed by the user.

The user reports hearing a tone after turning off **Enable 440 Hz test tone**.
The screenshot shows Tone: Off and transport stopped; the source of the sound during
playback is not established. This report remains unresolved until host retesting.
Processor tests cover switching Off inside the scheduled interval, preserving dry
input exactly, and restoring a saved Off state.

Changes prepared for retest:

- Synchronize the checkbox and bar controls with processor settings after host state
  restore; previously the displayed checkbox could be stale.
- Display **Last-block tone** as the peak of this instance's generated contribution,
  excluding dry input. `silent` means the last processed block added no test tone;
  if the host stops calling the plugin, the last-block readout is retained.
- Reuse verified immutable assets and existing waveform peaks during commits. Disk
  metadata changes invalidate cached validation; project restore always validates
  referenced audio again. This removes full-map PCM rereads on every commit.
- Show an incomplete-capture warning after dropped blocks. Clear resets the dropped
  block counter for a new attempt; it does not repair gaps in a previous capture.

Retest with the rebuilt bundle: start inside bars 8–10, toggle tone On then Off, and
check that Last-block tone becomes silent while transport continues. If a tone remains,
bypass this exact effect slot and check other Preverb instances and routed sources.
Repeat the capture test and record whether dropped blocks stay at zero.

## Build to test

### Subsequent user confirmation (1 November 2026)

After installing the updated build, screenshots show zero dropped blocks, continuous
captured waveforms with distinct gaps, and Tone: Off / Last-block tone: silent.
The user then confirmed the requested save/reopen and overlap-recapture checks.
The latest screenshot shows 48 kHz, 82 BPM, 4/4, 164-sample blocks, 15 captured
regions, revision 17, zero dropped blocks, and one discontinuity. Earlier successful
capture used 512-sample blocks. The precise bars 86–90 / 1–4 sequence has not been
separately established, and no grey captured-silence region is visible in these images.

The requested next iteration adds Playlist bar labels and grid lines above the same
absolute waveform scale. Labels use the host sample/PPQ anchor, tempo and meter;
capture starts do not reset bar numbering. Label density adapts to the displayed span.
This is a fixed-tempo/fixed-meter ruler, not a tempo-map implementation. With missing
timing or mismatched capture/host sample rates it reports bars unavailable. Compare
several bar boundaries with the Playlist after installing this build.

Build using the commands in [development.md](development.md). The VST3 bundle is:

`out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/Preverb Spike.vst3`

Close existing plugin instances before replacing an installed build. Load the rebuilt
bundle using your existing FL Studio plugin installation workflow. This work has not
installed the binary into the system VST3 directory.

## Record the environment

- FL Studio version/build: pending
- Windows version: pending
- Sample rate / tempo / meter / buffer size: pending
- Plugin build and date: pending

## Task 4.1: Overview inspection

1. Use fixed-tempo, fixed-meter Song mode, with the same routed source used in M0.
2. Enable Map while playing. Capture bars 86–90, then bars 1–4.
3. Include one separately visited range with zero-valued dry input.
4. Stop transport and wait for Capture to show Ready (committed) with a stable revision.
5. Inspect the Song-sample overview: audio is teal with peaks, recorded silence is grey
   with a centre line, and unvisited gaps stay dark. All regions use the same absolute
   scale. Check that labels, diagnostics and controls fit and remain responsive.

Result: closed by explicit user acceptance on 1 November 2026; see the acceptance
scope at the top of this report.

## Task 5.1: Same-PC project reopen

1. Save the `.flp` after the preceding capture has committed. Note the revision and
   sample positions; use a known test signal to check alignment.
2. Close and reopen on the same PC and Windows account.
3. Check Loading transitions to Ready, both distant regions retain their positions,
   the gap stays uncaptured, and captured silence remains grey.
4. Confirm tone controls were restored and dry passthrough still works. Mapping is off
   after state restore; enable it explicitly for additional capture.

Result: closed by explicit user acceptance on 1 November 2026 after confirmation
of general non-contiguous capture save/reopen; see the acceptance scope above.

## Task 5.2: Overlap recapture and reopen

1. Change the routed test signal in a short interval inside one captured range.
2. Enable mapping and play only that interval; disable mapping and stop transport.
3. Wait for Ready with a new committed revision, save, close, and reopen.
4. Confirm only the recaptured overlap changed; earlier/later material and the distant
   captured section remain intact and aligned. Record observations and host settings.

Result: user confirmed on 1 November 2026 after the save/reopen and overlap checks
were requested. Test conditions and screenshots are recorded above.

## Additional recovery checks

- Change the host sample rate: old coverage must remain visible and stale, with mapping
  paused. Explicit Clear followed by enabling mapping permits recapture at the new rate.
- On a disposable test project/cache copy, temporarily remove one WAV asset. Reopen:
  that range must be red/unavailable, other valid ranges retained, and mapping paused.
  Restore the asset and reopen to recover. Do not delete sole project assets.
- Save a committed revision, capture another section without saving, then reopen the
  saved project. It must use its saved manifest rather than silently adopting the newer
  capture. Unsaved snapshots and their assets remain on disk for future recovery tooling.

## Automated checks

The CTest suite covers lossless WAV/manifest round trips, malformed metadata and unsafe
paths, interrupted publication, bounded worker assembly, queue overflow gaps,
discontinuities, exact overlap replacement, failed capture/Clear retention, sample-rate
mismatch and recovery, missing/corrupt assets with unaffected ranges retained,
min/max peaks, immutable saved-revision selection, compact v2 state, v1 migration,
in-flight blocks around Clear, dry passthrough, and deterministic tone scheduling.

Automated checks do not establish FL Studio UI layout, host save/reopen behavior,
or real-time performance under a full arrangement. User acceptance and the limits
of the recorded host evidence are documented above.

# Stereo Ping Pong Crescendo listening spike

Issue #192. Windows x64 MSVC Release on `codex/192-stereo-ping-pong-crescendo`, based on epic commit `aecf721`. The opt-in native fixture uses the existing reverse convolution, ruler and timing candidates, wet-shape fitting, and envelope preparation.

## Reference and measurement

The owner supplied `12_Stereo_Ping_Pong_Crescendo.wav` from Creative IR Crescendo v2. The file is 48 kHz, stereo 24-bit PCM, with SHA-256 `bc5ee590f3d4caff32d639d6b5d1b6320251ad53adee7fd7c7c34694cdede91d`. The original generation source is unavailable. This analysis uses the measured source bursts.

| Impulse case | Hits | Last peak to arrival | Maximum ruler error |
| --- | ---: | ---: | ---: |
| Unchanged original, approximately 220 ms apart | 11 | 30.375 ms | |
| Arrival-relative eighths, 150 BPM | 11 | 30.375 ms | |
| Arrival-relative dotted eighths, 150 BPM | 9 | 30.375 ms | |
| Tight sixteenths, 300 BPM | 11 | 30.375 ms | |
| Ruler eighths, snapped Target | 11 | 200 ms | 0 samples |
| Ruler eighths, Target 25 ms before a beat | 11 | 175 ms | 0 samples |
| Ruler eighths, Target 25 ms after the beat | 11 | 25 ms | 0 samples |
| Ruler dotted eighths across the bar, Target 25 ms after the beat | 9 | 25 ms | 0 samples |
| Ruler dotted eighths, fractional sample spacing at 137 BPM | 7 | 226.458 ms | 0 samples |
| Ruler eighths, Move Wet with Snap off | 11 | 225 ms | 0 samples |
| Ruler eighths, Move Wet with Snap on | 11 | 25 ms | 0 samples |
| Ruler eighths, edited source start | 11 | 25 ms | 0 samples |

The original and retimed impulse renders retain alternating stereo peaks and increasing amplitudes. The peak was 0.355 in all listed cases. At 300 BPM the sixteenth-note bursts overlap across channels and the tested peaks still alternate. Changing source bounds kept the measured anchors. Wet crop masked the first hits, and fade-in scaled surviving samples at their original anchors.

## Listening and reproduction

The workspace listening page is `out/issue-192-listening-spike/index.html`. Its nine stereo WAVs compare original spacing, eighth and dotted-eighth baselines, and ruler-aligned edits. All clips use the same synthetic plucked chord and shared gain, with a quiet quarter-note click. Each A/B pair has the same arrival and envelope. The gaps reported above are from the impulse measurement.

On 12 November 2026 the owner accepted the listening result, describing it as sounding good and musically in time. The owner also approved the supplied WAV as the preset source. Version 1 uses its 11 measured bursts so the prepared response retains the sound heard in the approved comparison.

The versioned WAV and placement manifest are in [resources/presets](../../resources/presets/). The extraction tool's `--check` reproduces both files byte for byte and verifies exact reconstruction of the reference stereo PCM.

Set `REVERBARISE_PING_PONG_SPIKE_REFERENCE` to the supplied WAV and `REVERBARISE_PING_PONG_SPIKE_OUTPUT` to an absolute output directory, then run `./scripts/build.ps1 -Configuration Release -TestTarget preverb_ping_pong_spike`. The focused fixture passed 1/1 in 1.36 s CTest elapsed. The nine listening files were read back as distinct, four-second, 48 kHz stereo 24-bit PCM. The native fixture exercises fixed Song timing. Live drag publication, Loop, FL Studio and offline bounce remain integration checks.

## Production-bound native preview

`./scripts/build.ps1 -Configuration Release -TestTarget preverb_perform_composition_preview -TestFilter '^preverb_ping_pong_native_smoke$'` passed 1/1 in 1.94 s CTest elapsed on the issue worktree based on `aecf721`. It verified all four note rates at 44.1, 48 and 96 kHz, deterministic PCM and response identities, Sync off, spread, build, and peak limits.

The actual editor and engine accepted independent Learn and Perform settings. A new swell inherited Learn's spread and aligned its own response to the Song ruler. The accepted placement and managed IR stayed unchanged while a Target replacement was held pending. Off-grid Target and Move Wet changes kept the approved hit peaks within one sample of the ruler, checked with the saved managed response and native impulse render. A Move Wet change by a full subdivision reused the exact response asset. Freeze rejected a move requiring realignment. The saved state was decoded and restored into the engine, which verified and reused the exact render.

The preview captured Learn and Perform at 900×600 and 1100×760 in `out/issue-192-native-preview`. The interactive window uses the same production bindings and a disposable capture fixture. Loop playback, live DAW playback, FL Studio offline bounce and the standard Release regression remain outside this native smoke.

The focused Release Sound Preset UI and IR editor binding smoke checks passed 2/2 in 7.07 s CTest elapsed. Strict OpenSpec change validation and the offline user-guide currency check passed.

## FL Studio hands-on preview

The plugin-only Release build installed to `C:\Program Files\Common Files\VST3\ReverbArise.vst3`. The installer verified binary SHA-256 `22d1a7ef5d0487860d6392cad1e41a0f6057497f8dbb735d6fd859dca0e08b9f` and retained the previous bundle under `out/vst3-install-backups/20261112-215302-106b0a2441ca4f8097a705121dc67583`. This install did not rerun CTest.

FL Studio 2026 loaded that exact binary for `out/issue-192-fl-smoke/Ping Pong Beat 1 Smoke.flp`, a copy of the prior disposable project. At 82 BPM, selecting Stereo Ping Pong Crescendo reached Ready. With Perform Snap off, dragging the Target from 2.1 to 2.1.162092361 accepted a synced eighth-note response. The saved creative v13 state contained the preset settings, exact managed response and ruler alignment. The response checksum matched the saved state, and all seven peaks in that response measured zero samples from the saved ruler's eighth-note grid. The last hit peaked 118.60 ms before the off-grid arrival.

Audition entered its looping state and returned to Ready on Stop. Keyboard stepping of the Echo Rate dial prepared and accepted dotted eighths, then returned to eighths. The project is open for hands-on listening and control review. The host checks did not include a live Song playback measurement or offline bounce.

## Hands-on feedback follow-up

The owner reported sluggish gestures, weak stereo alternation and Build Intensity sounding stronger when reduced. They also reported that dotted eighths and sixteenths sounded in time against a kick. The original 82 BPM host renders measured 365.85 ms for eighths and 548.78 ms for dotted eighths. The strongest local wet energy in those renders followed the nominal peaks by approximately 5.6–5.8 ms. The continuous dotted grid put the last echo 301.52 ms before the tested off-grid arrival, compared with 118.60 ms for eighths. The subdivisions and ruler phase were retained.

Revision 2 keeps the same approved burst samples, strengthens default stereo separation from approximately 11 dB to 18.5 dB, and makes higher Build increase the late hits while keeping the quietest reference hit fixed. Previously saved revision 1 assets remain readable and retain their exact audio. Sound knobs and synced timing drags preview the proposal immediately. A settled gesture submits one render request. The prior response and placement stay accepted while preparation runs.

The native Release smoke passed 1/1 in 1.94 s. It verified multiple draft values with no worker submission, one request at settlement, no timing render during a real Target pointer gesture, Beat 1 alignment after release, monotonic Build energy, default stereo separation and restored audio. Four controlled 82 BPM kick comparisons were rendered in `out/issue-192-gesture-retest`, including dotted eighths and sixteenths. At the same mix gain, Build 100 had 12.4 dB more pre-arrival energy than Build 0 in this fixture. All four files were verified as unclipped 48 kHz stereo WAVs.

## Added Echo Rate subdivisions

The shared dial adds dotted sixteenths and thirty-seconds after the original four choices. Their saved values extend the existing enumeration. The native Release smoke passed 1/1 in 2.51 s. All six rates aligned to within one sample at 82 BPM at 44.1, 48 and 96 kHz. A 300 BPM thirty-second-note fixture verified deterministic preparation and bounded peaks with overlapping bursts. The native editor accepted both new choices and the state round trip retained the thirty-second setting. The 82 BPM kick comparisons are in `out/issue-192-echo-rates`.

## Build Intensity through Wet Audition

The previous fixed-gain comparison bypassed musical gain matching, which could amplify quieter late hits and mask the change. Preset revision 3 adjusts that matching relative to the authored 75% build before the existing headroom scan. The shared dial's hover help describes the effect.

The focused native Release smoke passed 1/1 in 3.23 s on the issue worktree based on `aecf721`. The fixture used the shared dial's gesture path, confirmed no render request during the gesture, waited for the replacement on release, and recorded the engine's Wet Audition output with musical gain matching and the same Level, Envelope, rate and placement. Energy in the final half-second rose 10.95 dB from 0% to 75% and 5.50 dB from 75% to 100%. The comparison WAV in `out/issue-192-build-intensity` plays the three settings in that order at the recorded gains. Strict OpenSpec and user-guide currency checks passed. FL Studio listening acceptance is pending.

The plugin-only Release installer verified SHA-256 `a2fff2bd052ee35c712090f11ac5ec27bd20e932fcb8af3ecf574252c7ef7a18` in the system VST3 folder. It retained the previous bundle under `out/vst3-install-backups/20261112-223308-15446f559f1e4fee828537b1954ff1c3`. CTest ran separately as part of the focused native smoke.

## Stereo separation with sustained input

The earlier timing spike used a centered 250 ms pluck. In the saved 82 BPM FL Studio piano render, several rightward echoes were only 0–3 dB right-biased over the quarter second after the nominal hit, while leftward echoes remained more prominent. The longer captured piano tail overlapped adjacent echoes, and the previous channel-by-channel convolution let the source's stereo balance affect the wet panning.

Revision 4 uses a shared wet excitation and tapers it over one echo interval around the arrival. The native Release smoke passed 1/1 in 3.18 s on the issue worktree based on `aecf721`. On the 82 BPM eighth-note fixture with a sustained stereo source strongly biased left, the weakest echo moved from 9.89 dB toward the wrong side to 18.54 dB toward its intended side. The before-and-after WAV uses the same source, stereo response and playback gain in both passes. The native checks also verified Beat 1 alignment, restoration, and that the Build Intensity differences survived the change. FL Studio listening remains pending.

## Epic branch sync

Fast-forwarded the issue branch to the epic merge `f440ae2`, which contains main at `8f64d22`, and reapplied the working-tree generator changes without conflicts. The plugin-only Release build passed with binary SHA-256 `11e7d6a679bfcfab2a8985ee4886b437de2ece580e1f18332010f677ece79a9a`. Strict OpenSpec validation and user-guide currency passed. This build did not rerun CTest.

## Sound dial defaults

The shared native preview exercised individual reset actions for Perform Build Intensity and Echo Rate and Learn Stereo Spread and Level. The Learn resets left the selected swell's Sound assignment unchanged. The preview also reproduced the stereo comparison and Build Intensity measurements on the merged branch.

The installer built Release with the reset fix and verified SHA-256 `3c2c9232a05b567cc9d626ef3af931324f4f356ef3603a213e8db3ba1245f1d7` in the system VST3 folder. The previous bundle is under `out/vst3-install-backups/20261112-225900-30ab8c878106446188da80c23d6f1255`. CTest did not run during installation. FL Studio listening is pending.

## Gate Amount

The Release native preview on `f440ae2` with the issue edits verified the unchanged 0% source, the 50% excitation blend, the existing tight response at 100%, and reuse of the same IR when changing Gate Amount in Perform. A serialized revision 4 swell reopened at 100% with its exact accepted render, and resetting it to 0% restored the original render PCM. Learn stored a separate 50% default and restored it through the current format without changing the selected swell. The 900×600 Perform preview showed Gate Amount alongside Build Intensity in the shared scrollable inspector. The plugin-only Release build, strict OpenSpec validation and user-guide currency check passed. The native preview ran directly, and CTest did not run.

The Release test install verified SHA-256 `5832921eba465594f506bda0e6c98c8073a3a567800529e451c11db2ec9ff5df` in the system VST3 folder and retained the previous bundle under `out/vst3-install-backups/20261112-230844-b16e17ebe9284cca947d8a3e73ee75c5`. FL Studio listening is pending.

## Damping

The Release native preview verified progressively lower burst energy at 0%, 50% and 100%, identical peak positions and amplitudes on the reference response, and ruler alignment after the selected-swell edit. It checked revision 5 loading at 0%, saving a nonzero setting, restoring the original render PCM at 0%, and independent Learn defaults. Strict OpenSpec and user-guide currency checks passed. The offline piano comparison in `out/issue-192-damping` uses the packed bursts in production wet order with a shared playback gain.

The Release installer verified SHA-256 `0b378f14710b31383e390d4840dc5eb16342a8c32bf2cf18ad3ea7027670f2b7` in the system VST3 folder and retained the previous bundle under `out/vst3-install-backups/20261112-231907-90217d4773314ae8ac1ff28ee3b3a13a`. CTest and FL Studio listening remain pending for this revision.

## Damping range follow-up

The native Wet Audition capture reproduced the reported weak range. Revision 6 at 100% reduced final half-second wet energy by 0.33 dB on the fixture. A steeper response taper alone also yielded little change in wet energy after gain matching. Revision 7 extends the upper half of the taper and lowers the musical matching target above 50%. The same fixture then measured a 6.03 dB reduction. The direct native preview passed its grid, peak, saved-render and dial checks. The comparison in `out/issue-192-damping-range` presents 0%, revision 6 at 100%, and revision 7 at 100%.

The plugin-only installer verified SHA-256 `f977acfa097d9517c1b3104caef652c981c5178dd9b4e33022ee848d42a88999` and retained the previous bundle in `out/vst3-install-backups/20261112-233132-a1298d2d9dae4003aa2433bd3156254f`. CTest did not run during installation. FL Studio listening remains pending.

## Combined review candidate

The owner reviewed the native layout, requested the final dial labels and range-label removal, and accepted the musical result of the filters in FL Studio. The Learn and Perform captures at 900×600 and 1100×760 and the production Wet Audition comparisons are in `out/shared-sound-shaping-approved`. The offline guide and [IR library guidance](../development/ir-library.md) cover the shared controls and the supported ruler timing.

Release candidate `3dc4e44` passed the standard PR regression (15/15, `20261113-021641-70f6f98e`) and the focused native suite (6/6, `20261113-022108-13682fbb`) for Ping Pong, gestures, bindings, Loop, Level and Audition. The latter included an added fixture checking retained PCM after a failed IR request and Freeze, and changed PCM after an accepted High Pass edit. See [draft PR #202](https://github.com/elneilios/ReverbArise/pull/202) and the [combined host evidence](2026-11-13-shared-sound-shaping.md).

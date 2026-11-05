# Native FL connector session (local spike v1)

Goal: establish **real script → VST3 audio-callback consumption**, then measure the gating result. This probe never loads ReverbArise sessions or assets. Budget about 5–8 minutes after scanning/activation. Stop after connection if anything is unclear; keep the logs rather than guessing parameter indexes.

## Prepared outputs

- Separate effect: **ReverbArise FL Context Probe**, version 0.0.1; manufacturer `RASp`, plugin code `RCp1`, bundle ID `org.reverbarise.spike.contextprobe.v1`. Production ReverbArise is untouched.
- Receiver build: repository `out/issue-1-native/build/ra_context_probe_artefacts/Release/VST3/ReverbArise FL Context Probe.vst3`.
- Script: `device_ra_context_native.py` beside this guide. Installed copy goes in the owned `Settings/Hardware/ReverbAriseNativeContextProbe` folder.
- Build/test: `./docs/validation/issue-1-connector/native/build-probe.ps1`. It reuses pinned JUCE 9.0.4 sources, uses its own build tree and the repository lock/tool discovery, and runs the focused Release CTest.
- Install while FL is **closed**: `./docs/validation/issue-1-connector/native/install-probe.ps1`. It rebuilds/tests, stages and hash-checks the complete uniquely named system bundle, retains any previous probe, and copies the script only if its owned folder does not already exist. It does not activate a device. The default configured Hardware location on this machine is `D:/Users/neila/Image-Line/FL Studio/Settings/Hardware`; override `-ScriptHardwareRoot` if needed.

## Setup (one explicit controller choice)

1. In FL Plugin Manager, enable **Verify plugins** and **Rescan previously verified plugins**, then Find installed plugins. Find **ReverbArise FL Context Probe** and confirm the path is `C:/Program Files/Common Files/VST3/ReverbArise FL Context Probe.vst3`. Do not replace or select production ReverbArise.
2. Start a **new disposable project**. Put the probe on Master, with no production ReverbArise instances. Turn **Smart disable OFF** for the probe. Make a short silent Playlist loop/clip so Song playback can keep running. Keep the probe editor open.
3. F10 → MIDI: explicitly choose a **spare input**. The previous inspection showed disabled generic Minilab3 DIN THRU/ALV/MCU inputs; choose one only if it is unused. Note its original Enable/controller type/port first. Select **ReverbArise Native Context Probe v1 (user)** as its Controller type and enable that input. Leave its port unchanged: direct plugin API calls do not use a MIDI output route. No notes need to be sent. This input assignment is manual and temporary; do not overwrite a controller you use. If no spare input is available, a separate virtual/hardware input is needed; none has been installed for you.
4. Open **View → Script output**, select this script's tab. Expect `RA native probe init OK` and a target such as `[(0, 0, (0, 1, 2, 3))]` (track, slot, discovered parameter indexes; extra wrapper parameters may shift the indexes). If the newly copied script is absent, refresh/reopen MIDI settings; if necessary restart FL **once** and reselect it. First-install discovery is still being measured. Existing script edits can use Script Output **Reload**, without a required host restart. Do not use “Update MIDI scripts” as proof that this custom script was activated.

## Short test sequence

| Step | Action | Expected / evidence |
| --- | --- | --- |
| **1. Prove delivery** | Select SONG; Play; watch the receiver. | `Received: Song`, `FRESH`, increasing **callback consumed seq** and block count. Query/write delay should be plausible positive milliseconds. Native CSV must show nonzero packets, mode 2 and playing 1. Script target/write messages alone are insufficient. |
| **2. Hear actual gating** | Enable the probe's **-24 dBFS tone**. Switch PAT/SONG during playback. | Song: 440 Hz tone, signal peak about 0.0631. Pattern: dry stays unchanged and tone fades out within 5 ms **after the callback sees Pattern**. Observe/report any delay before that point. Script output says whether the update came via `refresh` and which flags, or `idle`. |
| **3. Stopped / near Play** | Stop; switch mode a few times. Then switch immediately before Play in each mode, twice. | Correct mode must eventually be consumed. Some hosts may suspend stopped callbacks: `STALE` with a non-advancing block count is evidence, not a pretend fresh connection. At Play note any brief tone in PAT; logs measure sampled query/write → callback, not the unknown time of your mouse click. |
| **4. Two instances** | Duplicate the probe into another Master slot; turn the duplicate's tone OFF. Move it to a different Master effect slot, then repeat PAT/SONG during Play. | Discovery finds it within the next reconciliation (nominal 1 second plus host delay). Both editors show distinct transient instance IDs and advancing sequences. Only the first tone is audible. No unrelated effect parameter should change. |
| **5. Reload / reopen** | Script Output Reload while playing; then save/reopen **only the disposable project**. | New script log. Unknown/stale may appear briefly, then fresh mode resumes. User tone enable restores separately; context is reset, not serialized. Report any stale/replayed context or failure to rediscover. |
| **6. Offline** | With Song selected and tone enabled, export the short loop as **16-bit stereo WAV** into the receiver's evidence folder (use the Open evidence folder button). | This probe has **no unconditional offline mute**. A fresh Song packet permits tone even if Playing is false; Pattern/Unknown/stale does not. Record actual `offline` and `fresh` CSV fields plus WAV result. Whether FL continues the script during bounce is an observation to collect, not an assumed pass/fail. No production bounce behavior is changed. |

If query/write delays are huge (hundreds/thousands of milliseconds), keep the logs and clock information from Script Output. That can indicate clock mismatch, timestamp ordering/coalescing or stale delivery. Do not adjust the 750 ms experimental lease to hide it. ACK parameter readback may be cached by FL; the **native editor/CSV** is the authority for callback consumption.

## Evidence is automatic

- Each receiver writes `receiver-<transient UUID>.csv` under this worktree's `out/issue-1-native/evidence`, even with its editor closed, using a message-thread timer. Check the editor for log failures and dropped rows. Do not move this worktree before testing; the receiver evidence directory is compiled into this local build.
- The script writes `script-*.csv` under its installed Hardware folder's `evidence` subfolder. It records idle/refresh/full-refresh flags, mode samples, write sequences, QPC timestamps and errors. No project or capture data is logged.
- After testing, run `./docs/validation/issue-1-connector/native/collect-evidence.ps1`. It collects only these owned diagnostic folders, summarizes received timings/event flags and makes a local ZIP. Tell the investigating chat that the session is done; it can read those files directly. A screen recording is optional if you want exact mouse-click-to-sound timing; the CSV does not independently timestamp the host UI's mode change.
- Native-test CSV files start `native-test-` and are excluded from host summaries. The callback logs include per-block generated-signal peak and host sample position, but are not a recording of the actual DAW output. The exported WAV supplies output evidence.

## Removal / return to normal

First restore the spare MIDI input's original controller type, Enable and port. Close FL. Run `./docs/validation/issue-1-connector/native/install-probe.ps1 -Remove` to move only the probe bundle into retained installs. The owned script folder and its evidence are left intact; retain that folder elsewhere or remove it **after detaching** the input. Never delete the parent Hardware folder. Do not use the probe in real projects. Production ReverbArise and all its assets remain untouched.

This is an Explore experiment. Host results will inform a separately reviewed OpenSpec proposal, not authorize production rollout.

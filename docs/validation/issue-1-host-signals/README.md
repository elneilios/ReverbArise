# Host Signal Probe v1 — short FL session

Purpose: see whether upstream mute produces useful **direct JUCE/VST3 callbacks**, with no MIDI script or connection. This is a disposable effect, not a ReverbArise replacement or a proposed mute policy.

## Delivery and removal

Build/test: `./docs/validation/issue-1-host-signals/build-probe.ps1`. Install: `./docs/validation/issue-1-host-signals/install-probe.ps1` with FL closed; Windows elevation may be required. These adapt the repository [test-install guidance](../../development/fl-studio-test-install.md) for the diagnostic's separate name/IDs and use the same build locks, complete-bundle inventory/hash verification and retained previous bundle pattern. Neither production nor the previous context probe is replaced. No script is copied or activated.

Installed name/path: `C:/Program Files/Common Files/VST3/ReverbArise Host Signal Probe.vst3`. Bundle ID `org.reverbarise.spike.hostsignals.v1`; codes `RASp/RHp1`; version `0.0.1`. Build, logs and retained installs: `out/issue-1-host-signals/` in this checkout. A full first compilation uses cached JUCE source but a new probe build tree; it is not a fully warm build.

Removal with FL closed: run `install-probe.ps1 -Remove` elevated. It moves only this exact diagnostic bundle to `out/issue-1-host-signals/retained-installs/<timestamp-guid>/`, keeping CSV evidence. It does not edit FL's plugin database, controller settings or projects. Do not save a diagnostic instance into a valued project.

## One session, approximately three minutes after scanning

1. **Start clean.** Use a new/disposable FL project. If the old connector is still active, detach only that test script by selecting `(generic controller)` for its previously assigned test input; preserve other controller/port settings. This probe exposes no context parameters and never needs an endpoint. In Manage plugins, enable **Verify plugins** and **Rescan previously verified plugins**, then Find installed plugins. Load **ReverbArise Host Signal Probe**, confirming the system path above.
2. **One source.** Put a stock instrument/sample on one mixer insert and the probe on that same insert. Put its pattern in the Playlist with a gap before/between notes, in Song mode. Loop a short region containing both silence and sound. Keep the probe's Smart Disable **OFF** initially. Keep listening levels comfortable. Its optional tone peaks at -36 dBFS per channel and only plays when FL reports Play. Dry passes unchanged. No captured audio/assets or ReverbArise state is used.
3. **Mark and compare.** For each row below, select the matching numbered step in the probe, click **Mark step**, then perform the action. Keep each state about two seconds and toggle twice if convenient. The counters and CSV are the evidence; no screenshots are necessary. Marker time brackets actions, not an exact sample timestamp of the FL toggle.

| Step | Action | Expected evidence / question |
| --- | --- | --- |
| 01 Enabled source / ordinary silence | Play through a sound and its gap with the source enabled. First leave tone OFF briefly, then enable the tone for the remaining checks. | Input peak falls during genuine gaps. Establish baseline callbacks/bus flags. Tone continues over silence while playing. This is the control against falsely identifying silence as mute. |
| 02 Stop / Play | Stop, wait, Play twice. | Playing flag changes; lifecycle callbacks, if any, are transport controls to compare against mute. Tone stops on Stop, with its short ramp. |
| 03 Playlist mute / unmute | During Play, mute then unmute the source's Playlist track. | Does input disappear while callbacks/buses remain unchanged? Does reset/layout/bypass occur? Tone is deliberately not gated by input and may continue. Record what actually happens. |
| 04 Channel Rack mute / unmute | Repeat using only the source instrument/sample's enable switch. | Same comparison, separately from Playlist mute. Other sources must not share this test insert. |
| 05 Mixer mute / unmute | Repeat using this mixer insert's mute switch. | Audible output should be silenced by FL's track mute; callbacks may or may not continue. This host output control is different from upstream source mute. |
| 06 Effect slot off / on | Toggle the probe's mixer effect-slot enable switch. | Host may bypass without callbacks, call bypassed processing, or suspend processing. Determine which. Do not assume a specific implementation; the output should no longer include this diagnostic's tone. |
| 10 End | Restore switches; disable tone; mark End; stop. | Dropped rows should remain zero. Click **Open evidence folder** or tell the agent the session is done; CSVs are already in the checkout. |

Only if a distinctive event appears, extend with steps 07–09: Smart Disable on an inactive probe, seek/loop controls, and editor close/reopen. Do not broaden into tempo maps or bounce until a candidate signal justifies it. No guarantee of immediate wet suppression is claimed from a correlated event.

## Automatic evidence and interpretation

The processor's message-thread timer writes `out/issue-1-host-signals/evidence/host-<runtime-instance>.csv` with QPC seconds, marker, block/host position, playing/looping/offline, input/output bus flags, pre-tone input peak and final output peak. It records prepare/release/reset/layout/channel/track/host-interface/bypass/state callbacks and **changes** to nonrealtime mode. Logging works without an open editor. No host calls are polled from a background thread. The editor displays last callback age and event counts, which continue to be useful if FL stops callbacks.

Run `python -B docs/validation/issue-1-host-signals/analyse.py out/issue-1-host-signals/evidence` for per-instance/per-marker summaries. The agent can run this after the session; the owner need not run a command or export Script Output. Native-test files are deliberately named `native-test-*` and excluded from host summaries. Close the test project or stop playback before final analysis so the snapshot is settled; leave the evidence files in place.

This probe **observes** reset/lifecycle events; it does not disarm itself on them. Its tone follows only local enable and transport Play, not source activity, offline context, Song/Pattern, bus enable heuristics or any connector. The 5 ms tone ramp is diagnostic only; explicit plugin bypass removes it immediately. Audio callbacks only do bounded sample processing, atomics and one nonblocking queue attempt. Queue contention/full capacity drops rows and increments a visible/logged count. A trace with losses cannot establish absence of a notification. Message-thread logging and an idle host are not latency guarantees; callback gaps include host scheduling/device load.

This first test observes processor-facing callbacks. It does not intercept raw VST3 `activateBus`/`setActive` methods; bus flags/layout and prepare/release are their available JUCE consequences. Standard plugin bypass may be implemented entirely in the host, so not seeing `processBlockBypassed` does not imply the switch failed. No private FL interface, source routing graph, future audio or source-mute semantic field has been added. A useful candidate must survive controls before any separately reviewed production proposal.

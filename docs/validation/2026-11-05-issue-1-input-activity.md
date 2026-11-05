# Issue #1: input-activity auto-mute feasibility — 5 November 2026

**Conclusion:** detecting input activity and temporarily gating scheduled wet is technically straightforward. **Inferring an upstream mute from silence cannot reliably preserve reverse-reverb behavior.** The source may legitimately be silent during the very lead-in that should play. Recommend keeping normal Perform scheduling independent of dry amplitude, explaining that prepared swells remain active until explicitly disabled, and parking connector development per the owner's preference. An optional activity-following policy is possible only with explicitly accepted musical compromises; it is not automatic track-mute or Song/Pattern detection.

Scope: Explore extension of PR #74 / [Refs #1](https://github.com/elneilios/ReverbArise/issues/1). Baseline is `3c9bffbcf087c4e0352445d1aff4c2f14b4b9d4c`. No production source/spec changes, installed receiver changes, controller reconfiguration, project edits, capture or render mutation. No computer control or additional owner host tests were requested for this analysis.

## What the current implementation can observe

`PluginProcessor::processBlock` has the incoming stereo samples before generated tone and wet mixing. A future detector could inspect those original samples in a bounded pass, keep a small envelope/hold state and apply only a transient Perform wet gate through the existing short transitions. It must tap input **before** diagnostic tone, scheduled wet and preview: otherwise generated audio could keep its own detector active. No file access, capture replacement, render invalidation or persistent identity change is necessary.

The current [transport contract](../../openspec/specs/transport-swell-playback/spec.md) deliberately permits prepared wet over zero dry input, preserving absolute placement, lead-ins, silent gaps and tails. Live/Perform never captures. The current mix path does not treat low input as disabled playback. Source/render readiness and user enable remain separate from incoming activity.

[VST3 silence flags](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/structSteinberg_1_1Vst_1_1AudioBusBuffers.html) describe optional per-channel all-zero buffers, not the reason for silence or a Playlist/instrument mute state. [FL Channel Rack documentation](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/channelrack.htm) describes channel muting and mixer routing, but that does not expose the originating mute control to this VST3 effect. Several sources can share a mixer path; a mute elsewhere may leave other signal, upstream effect tails or noise at the plugin input. Playlist/channel mute behavior at each routing position was **not host-tested** in this extension.

## The unavoidable lead-in ambiguity

Consider the same prepared swell in two sessions:

| Time | Valid source enabled | Source muted upstream | Required scheduled wet |
| --- | --- | --- | --- |
| 0–2 s | Dry silence | Dry silence | Neither yet |
| 2–4 s | Still dry silence; source note is about to arrive | Dry silence | Valid session should play the reverse swell; muted session should not |
| 4 s | Source note begins | Still silent | Only now can actual input differ |

Until the future source arrives, the plugin can receive identical audio and transport observations in both cases. No causal input-level policy can choose different wet output from identical available input history. Inspecting a whole callback provides only that block's limited future samples; it cannot reveal a note seconds ahead. Stored source predicts the arrangement but does not reveal whether the current upstream instrument/clip has been disabled.

## Executed synthetic fixture

The retained [fixture](issue-1-connector/input_activity_fixture.py) runs a causal level detector at a synthetic 1 kHz rate (one sample per millisecond), with a dry note at 4 s, a prepared lead-in from 2–4 s and an illustrative tail from 4–4.5 s. These are policy counterexamples, **not native DSP, FL mute or audio-quality measurements**. Fractions count scheduled frames admitted by the hard gate, not audio energy. The threshold is -80 dBFS; holds/grace periods are test inputs, not proposed defaults.

```text
python -B docs/validation/issue-1-connector/input_activity_fixture.py
PASS: counterexamples reproduced; no mute-identification claim
```

| Policy | Valid first reverse lead-in retained | Muted lead-in allowed | Illustrative valid tail retained |
| --- | ---: | ---: | ---: |
| Activity only | 0% | 0% | 20% |
| Activity + 250 ms hold | 0% | 0% | 70% |
| Activity + 5 s hold, starts closed | 0% | 0% | 100% |
| Start armed for 5 s | 100% | 100% | Depends on subsequent activity/hold |

Additional passing counterexamples, recorded in [metrics JSON](2026-11-05-issue-1-input-activity-metrics.json):

- A valid first note delayed until 12 s loses its 10–12 s swell once the 5 s startup grace has elapsed.
- Two valid notes at 1 s and 12 s lose the second lead-in with a 5 s hold. An 11 s hold retains that lead-in but also retains the corresponding unwanted wet after muting the source following the first note.
- A valid -86 dBFS note is not detected at the -80 dBFS test threshold. Conversely, an exact-zero detector remains open on a constant `1e-7` noise floor. Our earlier native host evidence also contained tiny nonzero input residue in a region whose PCM16 export was silent; exact zero is not a robust semantic test.
- Comparing actual input with expected captured activity can first distinguish this missing note at 4 s, after its 2–4 s swell has already passed. A loop that never reaches the expected note cannot obtain that later evidence at all. The latter loop consequence is inference from the timing model, not an observed FL test.

The detector is mechanically feasible with O(block frames) work and constant runtime state. No native CPU/latency benchmark, gate sound-quality test or MSVC/CTest result is claimed for this Python fixture.

## Options and user consequences

| Option | Consequence | Recommendation |
| --- | --- | --- |
| Mute wet whenever current input is silent | Suppresses valid lead-ins and can chop tails/between-note swells; quiet/noisy sources need arbitrary thresholds. | Reject as default behavior. |
| Hysteresis, release and an inactivity hold | Reduces chatter and delays shutdown; does not create advance knowledge. Longer holds preserve more music but make upstream mute response slower. | Possible explicit optional behavior, not reliable source-enable detection. |
| Start each Play armed, close after inactivity | Preserves early lead-ins by also permitting muted-source wet during the grace period; late first notes and long gaps still fail. | Not a transparent fix. |
| Gate after expected captured dry fails to appear | May suppress later swells after evidence of a missing event. Cannot retract the preceding swell; differences in live material/timing require further tolerance policy. | At most best-effort suppression after a missed event; does not meet immediate mute behavior. |
| Look ahead into actual future live input | Could resolve future activity only by buffering/delaying enough audio or obtaining a separate reliable intent signal. Multi-second reverse lead-ins imply material latency/architecture changes; captured old input alone is insufficient. | Outside this bounded spike; not a small low-friction solution. |
| Explicit per-instance scheduled-wet enable | Immediate understandable intent; independent of silence, live articulation, routing and host mode. Existing saved preference and assets remain intact. | Preferred honest workflow. Improve discoverability/help in a separately reviewed proposal if requested. |

An optional feature would need an accurate name such as **Follow input activity**, never “follow track mute.” It must disclose lost first/late lead-ins, potential tail truncation and delayed mute/unmute; it should not silently become the default. This is a candidate behavior description, not approved UI, control defaults or a proposal to add more threshold/hold settings.

Proposed plain-language product explanation: **“Perform plays prepared swells at their saved Song positions, including before source notes and through silence. Muting an upstream clip or instrument does not necessarily mute those prepared swells; use scheduled playback enable to remove them from the mix.”** A later host-automatable wet enable or shared mute workflow could make explicit control more convenient, but at the inspected baseline the internal live preference is not a registered host automation parameter. Adding one requires its own reviewed compatibility/UI contract; do not claim it already exists.

## Follow-up boundary

Before any production proposal for activity following, the owner would need to choose which musical compromises are acceptable. Tests should then assert first-note lead-in, late first note, sparse notes, tails, quiet/noisy input, shared mixer sources, mute/unmute during playback, seeks/loops, Wet Only dry behavior, offline block partition independence, multiple instances and saved user-enable/source/render identity preservation. Use audio-time rather than wall-clock hold semantics if proposed; no script/heartbeat dependency is needed for this idea.

**NOT RUN:** actual FL Playlist/channel-rack mute traces at different routing positions, native threshold/ramp behavior, a new installed gate probe, realtime residual PCM or offline activity-gate output. The synthetic ambiguity is already sufficient to reject a claim of transparent upstream-mute detection; no further owner UAT is needed to establish that limitation. Learn Pattern capture, accepted source, last-good renders and real-time safety remain mandatory. Connector onboarding work is parked; no production fix, Apply, merge or issue closure is authorized.

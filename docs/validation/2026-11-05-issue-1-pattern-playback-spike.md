# Issue #1: FL Studio Song/Pattern investigation — 5 November 2026

**Completed native follow-up:** see the [final connector findings](2026-11-05-issue-1-connector-findings.md). An explicitly configured script successfully delivered Song/Pattern to the diagnostic VST3 in realtime; its freshness policy suppressed the measured offline tone. The original bounded investigation below is preserved as the pre-connector evidence baseline.

**Conclusion:** no reliable Song/Pattern discriminator available to this VST3 effect was established. Standard JUCE/VST3 timing cannot prove mode. Recommend exposing the existing per-instance **Live playback** enable prominently in Perform as an explicit scheduling switch, with clear guidance to turn it off before Pattern playback. This is an operator-controlled fallback, not automatic detection or a guarantee against forgetting. Production behavior is unchanged; [#1](https://github.com/elneilios/ReverbArise/issues/1) remains open.

**Requested extension:** the [optional FL connector spike](2026-11-05-issue-1-fl-connector-spike.md) tests a disposable direct-parameter script against API doubles, records observed MIDI setup friction, and separates correct mode samples from safe, timely delivery. Live connector timing is blocked by the lack of an isolated activated scripting input and a diagnostic receiver endpoint; no connector rollout is recommended yet. The original results below remain unchanged.

## Baseline, scope and reproduction

- Fetched main: `520a4669572b105848d00763a4dd3fc2550919ea` (PR #72). Clean, isolated existing worktree `C:/Users/neila/.codex/worktrees/d976/k7ma-preverb`; investigation branch `codex/1-pattern-playback-spike`. No other worktree was changed.
- Windows API version `10.0.26300.0`, x64; VS 2026 Community, MSVC tools directory `14.51.36231`; project pins JUCE `9.0.4`. FL Studio 2026 executable file/product version `26.1.7.5654` is installed; no FL Studio process was running at discovery.
- Read AGENTS, CONTRIBUTING, delivery routing/checklist, vanilla Explore instructions, current `workflow-modes`, `song-timeline-audio`, `transport-swell-playback` specs, relevant processor/capture/scheduler paths, and issue #1's body/discussion. The only active local OpenSpec change is unrelated completed waveform work; open-PR check found no competing #1 PR. Read #73/#71 scope only to establish separation.
- **FL Studio current-main reproduction: NOT RUN.** No build exists in this worktree. An installed ReverbArise bundle exists, but its exact source revision was not established and it was not loaded or replaced. A revision-matched isolated host run was not available under the no-install constraint. No user project, asset or installed plugin was modified.
- **Historical capture loss: UNCONFIRMED.** No current-main host or PCM reproduction of disappearing wet/source loss was obtained. Timing rejection, missing preparation and actual source mutation must be distinguished before attributing any disappearance to capture.

The current code still explains a possible unwanted-wet path: `PluginProcessor::updateHostPositionDiagnostics` reads samples/PPQ/tempo/meter/playing/recording/loop information, without host mode. `SwellEngine::processTransportAudio` admits the live path only in Perform; `TransportPlayback::process` further requires live enable, playing, compatible timing, valid range and prepared coverage. If Pattern supplies an admitted position intersecting a prepared Song swell, that same scheduler can mix it. This is **source-trace inference**, not observed FL playback.

Capture admission now explicitly requires Learn and a recording pass. Ordinary capture protects accepted and already-staged positions; Recapture permission is explicit and publication transactional. Perform/Export do not enqueue source. Thus the old simultaneous-live-auto-capture explanation is not a demonstrated current-main path. Retain valid source, accepted renders and identities when scheduling is disabled; do not rebuild capture policy for this spike.

Current anchors: [mode contract](../../openspec/specs/workflow-modes/spec.md), [capture contract](../../openspec/specs/song-timeline-audio/spec.md), [playback contract](../../openspec/specs/transport-swell-playback/spec.md), [processor admission](../../Source/Plugin/PluginProcessor.cpp), [capture worker](../../Source/Capture/CaptureWorker.cpp), [live scheduler](../../Source/Playback/TransportPlayback.cpp), [live clock](../../Source/Playback/LiveTransportClock.h), and [existing enable setter](../../Source/DSP/SwellTransport.cpp).

## Signals and evidence matrix

Documentation was checked on 5 November 2026. “Not run” means no new host evidence, not a negative host test. Fixture results below are synthetic counterexamples, not recordings.

| Candidate | Documentation / source | New FL host evidence | Fixture result | Decision / inference |
| --- | --- | --- | --- | --- |
| JUCE `PositionInfo` | [Public API](https://docs.juce.com/master/classjuce_1_1AudioPlayHead_1_1PositionInfo.html) has optional time/tempo/meter/loop data and transport flags; no Song/Pattern field. Cached 9.0.4 wrapper maps these from VST3 context. | Not run | Application timing admits identical constructed observations for either mode label. | Not a discriminator. Missing data stays unknown. |
| VST3 `ProcessContext` | [Fields and flags](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/structSteinberg_1_1Vst_1_1ProcessContext.html): playing, recording, cycle, sample/music position, continuous/system time, tempo/meter and validity. No Song/Pattern enum. | Not run; raw context not probed | No raw-context fixture | Cycle means transport looping, not FL Pattern. Continuous time is not a mode flag. |
| Offline/realtime `processMode` | [ProcessData](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/structSteinberg_1_1Vst_1_1ProcessData.html) separates processing mode from transport context. | Not run | Not run | Offline does not identify Song vs Pattern; do not fold #73 into this issue. |
| Host identity / custom VST3 extension | [IHostApplication](https://steinbergmedia.github.io/vst3_doc/vstinterfaces/classSteinberg_1_1Vst_1_1IHostApplication.html) supplies name/object creation. [JUCE VST3 extensions](https://docs.juce.com/master/structjuce_1_1VST3ClientExtensions.html) expose a host-context hook and interface queries. | Not run; no FL interface query attempted | Not run | Technically an extension access path, but no documented FL VST3 mode IID/method/support contract was found. Host name alone cannot classify mode. |
| FL MIDI `transport.getLoopMode()` | [Image-Line MIDI API](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/midi_scripting.htm) explicitly reports Pattern/Song in controller scripts. | Not run | Not run | Correct signal in a different execution environment. No supported direct VST3-effect access established. A controller/IPC bridge would add installation, latency, stale-message and multi-instance concerns; outside the smallest fix. |
| Native Fruity SDK / IL VST extension | [Official SDK link target](https://forum.image-line.com/viewtopic.php?f=700&t=12092) currently says SDK unavailable and directs developers to other formats. [IL extension](https://forum.image-line.com/viewtopic.php?t=132491) documents a VST2 `audioMasterVendorSpecific` Piano Roll note-insertion callback. | Not run | Not run | Neither establishes a Song/Pattern query accessible to this VST3 effect. Old native SDK functions are not VST3 proof. |
| Bar 1, repeats, short loop, seeks, Stop/Play | All are legitimate Song transport journeys; [FL transport manual](https://cluster.image-line.com/fl-studio-learning/fl-studio-online-manual/html/toolbar_panels.htm) describes the separate Pat/Song selection. | Not run | Same positions admitted by current timing helpers for either label. | Reject heuristics: a short Song arrangement can reproduce these observations. |
| Sample/PPQ origin difference | Current live clock establishes a fresh playing baseline, independent of parked/source origin, then enforces its within-run tolerance. | No new observation; existing #53 timing evidence is historical Song evidence only. | 16-sample cross-origin mismatch rejected by mapping helper; asset compatibility and matching shifted baseline admitted. | Timing compatibility is not host-mode identity. Do not loosen or repurpose origin guards. |
| Silent input / instance count | Scheduler timing gate has no input-amplitude or host-mode argument; enable/preferences belong to each session. | Not run | No PCM tested; two observation slots retain independent revision values. | Silence is valid before Song swells. Multiple instances do not create a missing host-mode signal. Slot isolation alone is not a full multi-plugin test. |

This establishes **insufficient supported evidence for automatic detection**, not proof that no private or future extension exists. Before adopting any extension, require an Image-Line VST3 contract and actual successful queries from this effect, including lifetime/thread rules and paired Song/Pattern observations. Do not infer support from another plugin format or from a single differing trace.

## Executed fixture and coverage limits

[Standalone timing fixture](2026-11-05-issue-1-timing-fixture.cpp) compiled against this revision's actual `TransportTime.h` / `LiveDiagnostics.h`, without JUCE shims or modified production headers. From a VS x64 developer shell at repository root:

```text
cl /nologo /std:c++20 /EHsc /I Source docs\validation\2026-11-05-issue-1-timing-fixture.cpp /Fo:out\issue-1\timing.obj /Fe:out\issue-1\timing.exe
out\issue-1\timing.exe
PASS 44 assertions; no FL Studio, PCM, capture, or scheduler execution
```

Both compile and executable returned 0. This was a small cold standalone fixture compile using the installed compiler, not a plugin build or benchmark. It establishes that the timing helpers accept constructed ambiguous inputs; it does not prove FL reports identical inputs.

| Comparison | Synthetic evidence in this spike | Actual paired Song/Pattern host check |
| --- | --- | --- |
| Normal/bar-1 playback | `0,256,512` at 48 kHz/120 BPM admitted | **NOT RUN** |
| Short Song loop / repeated positions | `0,256,0,256`, supplied loop bounds admitted | **NOT RUN** |
| Seeks | `48000,0,96000` admitted with coherent PPQ | **NOT RUN** |
| Stop/start | Restart positions admitted; no actual live-clock Stop state machine executed | **NOT RUN** |
| Silent input | Code trace only; no PCM execution | **NOT RUN** |
| Multiple instances | Two independent diagnostic slots only | **NOT RUN** |

**NOT RUN:** native plugin MSVC/CTest suite, wet PCM reproduction, Learn Pattern capture/export, capture-failure injection, save/reopen and source/render hash comparisons, FL extension probing, host/DPI/UI acceptance, offline bounce and tempo-map checks. Production code is unchanged; no cold full plugin build or repeated owner UAT was requested for this evidence-only PR.

## Small explicit fallbacks

| Option | User consequence / limitation | Assessment |
| --- | --- | --- |
| Expose/relabel existing saved `liveEnabled` as Perform scheduling enable | Turn OFF in every affected instance before Pattern; restore ON for Song. OFF restores dry even with saved Wet Only routing, using existing short ramps. Forgotten ON remains unsafe; forgotten OFF makes Song wet absent. | **Recommend** as smallest explicit fallback: one existing authority and state field; retain existing defaults/restoration semantics. Label as scheduling intent, never “Song detected/confirmed”. |
| New temporary arm, cleared on every valid Stop/restore | Explicitly re-arm each Song run in each instance; reduces stale saved permission, but interrupts ordinary Stop/start workflow. A mode change while continuously playing remains unknowable. | Stronger friction-based safety, more state/transition/UI policy. Consider only if owner requires default-off per-run behavior. |
| Saved “Song confirmed” flag in addition to existing enable | Can remain true when FL switches to Pattern, including after reopen; duplicates scheduling authority. | No detection benefit over existing enable; reject new parallel flag. |
| Switch to Export or Learn / mute wet gain / DAW bypass | Export requires eligible wet and changes workspace; Learn may capture unmapped Pattern material; zero wet gain can leave Wet Only dry muted; host bypass changes host routing behavior. | Poor dedicated scheduling control. Do not suggest Learn as a capture-free bypass. |

The recommended UI would promote the existing control currently placed in the diagnostics content (`PluginEditor.cpp`), reuse its `setLivePlayback` binding and show “Song scheduling off — dry passthrough” with Pattern guidance. No new host-mode detector, persistence field, default migration or source/render invalidation is recommended. Explicit internal audition remains separate; manual preview is not evidence of scheduled leakage. Existing preference affects host offline rendering too: disclose that saved OFF can omit wet from a bounce and require intentional ON before Song bounce. Do not change offline timing under #1.

This recommendation is **not approved UI or a production fix**. It prevents scheduled wet only when explicitly disabled; unconditional automatic prevention cannot be promised with unknown mode. If that limitation is unacceptable, escalate the product decision rather than disguise a heuristic as detection.

## Historical reconciliation and follow-up boundary

[PR #2](https://github.com/elneilios/ReverbArise/pull/2) is closed, unmerged historical planning. Its closing discussion explicitly rejects its blanket Pattern-capture suppression. Retain its cautions about ambiguous timing, independent scheduling and realtime diagnostics; discard its Song-only capture premise, capture guard/migration scope and assumption that live auto-capture still explains disappearance. Today's Learn/Perform/Export correspond to durable Capture/Live/Finalise identifiers. No rename, asset rewrite or last-good-render discard follows from changing scheduling intent.

Next, after owner selection, use vanilla OpenSpec **Propose**, then stop for a fresh Apply request. A proposal should target Perform control placement/guidance and existing scheduling diagnostics, identify shared control reuse and scoped visual/interaction approval, and preserve current mode/capture/state contracts. This report changes no canonical spec and creates no implementation tasks/change claiming readiness. #73's constant-tempo/bounce investigation and #71's tempo-map feature remain independent; no direct dependency was established. Rebase any later proposal onto their delivered behavior if relevant.

Proposed follow-up acceptance tests (not completed by this report):

1. Automate actual processor/scheduler PCM tests: scheduling OFF gives unchanged stereo dry after the existing <=5 ms transition, including Wet Only; ON retains positioned Song output through bar 1, short loops, sparse seeks, leading silence, irregular blocks and Stop/start. Separate manual preview and diagnostic-tone output from scheduled swells.
2. Assert unchanged accepted source references, coverage, revisions, render hashes and durable project/instance/swell identities through scheduling toggles, Song→Pattern→Song, failed/interrupted capture, restore and duplicate instances. Never grant Recapture through a scheduling switch; retain last-good assets.
3. Test Learn Pattern one-shot capture into unmapped positions, repeated-loop protection, explicit Recapture only, generation, Perform editing and WAV export; Perform/Export enqueue zero source. Zero dry cannot erase accepted material.
4. Test two real processors with independent enable/routing/state and interleaved callbacks; add component/binding tests for the reused control, saved-state compatibility and missing/ambiguous timing. Run appropriate native wrapper tests and UI standards checks; obtain scoped native preview approval before rollout. Assert callback allocation/lock/I/O safety.
5. Consolidate unresolved host work into one disposable, revision-matched FL project session once isolated loading is available: known Song-positioned renders; paired Song and Pattern normal play, one-bar Song loop, seeks, stop/start, silent routed input and two instances. Record raw available context/validity, selected host mode independently, callback/worker gates and PCM. Compare OFF/ON output and before/after identities; save/reopen only disposable project copies. Treat any reproduced durable loss as a separate urgent defect. Repeat across supported FL versions only if evidence leaves version-specific behavior unresolved.

Any future detector must distinguish these cases without Song false positives. Until then, report unknown host mode honestly and use explicit scheduling intent.

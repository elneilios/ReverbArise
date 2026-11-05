# Issue #1: direct host-signal spike — 5 November 2026

**Status:** separate native probe built, tested and installed; owner FL mute checks **NOT RUN yet**. This extends PR #74 / [Refs #1](https://github.com/elneilios/ReverbArise/issues/1), following the [JUCE API audit](2026-11-05-issue-1-juce-disarm-audit.md). It tests whether FL emits any usable processor-facing notification without a script. No production fix or inference-based disarm is implemented.

## Revision, environment and delivered output

Source baseline before this extension: `c21d1e9` on `codex/1-pattern-playback-spike`. Windows x64 `10.0.26300`; MSVC VS 2026 `18.11.1`; Windows SDK `10.0.26100`; cached JUCE **9.0.4** / vendored VST3 **3.8.0**. FL environment from the previous owner session was **26.1.7.5654**; the new host session is pending. New build tree with cached dependency source, followed by incremental verification; no cache clearing. Initial sandbox build was blocked by Windows SDK registry access, then passed with normal filesystem escalation. OpenSpec launcher still has no configured Node version; relevant existing specs were read directly, not changed.

- **ReverbArise Host Signal Probe 0.0.1**, bundle `org.reverbarise.spike.hostsignals.v1`, manufacturer/plugin `RASp/RHp1`. Processor CID `ABCDEF019182FAEB5241537052487031`, controller CID `ABCDEF011234ABCD5241537052487031`; distinct from production and the previous context receiver.
- Installed complete bundle: `C:/Program Files/Common Files/VST3/ReverbArise Host Signal Probe.vst3`. Binary SHA256 **`75AFCB941BC55086B551792D967D8DDF3D33DC2E37DCA098F1D708D816A12E33`**.
- Current installed production binary hash before/after this installation: **`50C1A55B33C5AEC45F23FDC626330CFC8C8EDE83CCE4AF21E49B5B057480EB7E`**, unchanged by this spike. This is the current installed binary, not a claim it matches an earlier session's production revision.
- Earlier diagnostic copy retained at `out/issue-1-host-signals/retained-installs/20261105-152147-913d5243e2094f6f858102e1adddd9b3/ReverbArise Host Signal Probe.vst3`. An instrumentation edit during compilation left the initial binary stale; final source was rebuilt after the first build finished, CSV loss accounting verified, and the final bundle installed with hash comparison. First copy must not be used as evidence.
- [Build, installation/removal and brief owner guide](issue-1-host-signals/README.md). No MIDI script, endpoint or controller assignment is installed/modified. FL was closed for installation; no computer UI was taken over.

## Native verification versus host evidence

| Check | Executed result | Evidence boundary |
| --- | --- | --- |
| Release VST3 and native test build | PASS with the probe's wrapper importing repository build-tool/cache/lock helpers | Separate tree; production CMake/Source untouched. Not a full production build/test run. |
| Native Release CTest | **1/1 PASS** | Tests actual processor with a synthetic playhead, not FL. |
| Dry and bypass audio | Exact input samples retained with tone off; explicit bypass removes diagnostic tone; stopped transport produces no tone from a settled envelope | Confirms probe behaviour, not actual FL effect-slot bypass delivery. |
| Tone and transport | Quiet 440 Hz tone appears during synthetic Play; independent of source activity | Deliberate observation aid, not a proposed scheduling/freshness policy. |
| Lifecycle/state | Reset/layout/channel/nonrealtime-change/state/marker hooks exercised; runtime tone is OFF after restore | Does not prove which FL action invokes any hook. Saved ReverbArise user enable/source/renders are never accessed. |
| Realtime instrumentation | Watched callback allocation/deallocation counters both **0**; fixed queue tests cover overflow, contention and concurrent producer accounting | Queue uses one atomic try with explicit drops, never waits/spins; file formatting/writes are on the processor message-thread timer. No realtime CPU/latency benchmark claimed. |
| Evidence pipeline | Native CSV analysed successfully with zero reported queue loss; editor snapshot inspected, labels/buttons fit | Native filenames excluded from default host analysis. Marker time is not exact toggle time. |
| Playlist / Channel Rack / mixer / effect-slot toggles | **NOT RUN** | Requires owner session; no mute signal or defect fix claimed. |
| Offline, private FL interfaces, ARA, lookahead/PDC | **NOT RUN** | Out of the first bounded test unless a concrete candidate justifies extension. |

Build/test command: `./docs/validation/issue-1-host-signals/build-probe.ps1`. Final install used its adapted `install-probe.ps1` elevated after normal system-folder ACL denial, with native tests repeated by installation. Build/test/install logs, native CSVs, analyser JSON and editor snapshot are under ignored `out/issue-1-host-signals/`; no native result is relabelled as host evidence.

## Experiment and decision rule

The [guide](issue-1-host-signals/README.md) groups six comparisons in one disposable single-source project: enabled silence; stop/start; Playlist mute; Channel Rack mute; mixer mute; effect-slot off/on. Named markers bracket each action. Optional generated tone shows whether FL removes plugin-generated output even when original source input is zero. It deliberately ignores source activity and Song/Pattern. Baseline Smart Disable is OFF. The old connector should be detached by the owner from its test assignment; other controller settings are preserved.

The probe records JUCE reset, prepare/release, layout/channel/track changes, host/component-handler hookups, state callbacks, bypassed processing, nonrealtime **changes**, and every audio callback's buses, input/output peaks, transport and QPC time. These are JUCE consequences, not raw intercepts of VST3 activation methods. Host bypass may omit callbacks entirely. No background host/project query or undocumented interface is introduced.

Analyse after the session with `python -B docs/validation/issue-1-host-signals/analyse.py out/issue-1-host-signals/evidence`. Review events and gaps by marker; native-test files are excluded. Zero reported queue loss is necessary to support an absence claim but does not independently prove a complete file or explain a host scheduling gap. Editor counters/loss status and any owner observations remain part of interpretation.

If upstream mute only changes audio samples, this route adds no knowledge beyond the established silence ambiguity. If a notification correlates with mute, compare it against normal silence, stop/start, seeks/loops, Smart Disable and device/lifecycle changes before proposing a policy. Then test delivery timing, editor closed, two instances and offline behaviour. No arbitrary callback-gap threshold or persistent reset-to-disarm rule is authorised by this experiment. Explicit effect/mixer mute may already suppress output without providing an upstream source-state signal; those controls must be distinguished.

Protect Learn Pattern capture, dry pass-through, accepted source, last-good renders, persistent identities and saved user enable. Production implementation still requires a separately reviewed OpenSpec proposal. Connector onboarding remains parked; issue #1 stays open. This does not establish a dependency on #71 or #73.

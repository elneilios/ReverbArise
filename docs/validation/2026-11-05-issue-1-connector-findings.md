# Issue #1: connector findings and recommendation — 5 November 2026

**Result:** an optional FL Studio script can supply explicit Song/Pattern context to a VST3 audio callback through `plugins.setParamValue()`. The native probe demonstrated correct fresh reporting, audible realtime tone gating, two instances, script reload and save/reopen. **The same realtime freshness gate failed the offline test:** context was already stale when rendering began, and the exported tone was entirely absent. The connector is a useful realtime candidate, not a production-ready or seamless automatic solution.

This is PR #74 / [Refs #1](https://github.com/elneilios/ReverbArise/issues/1). Production behavior is unchanged; the issue remains open. The [native experiment record](2026-11-05-issue-1-native-session.md) preserves the setup/debugging chronology; [machine-readable measurements](2026-11-05-issue-1-host-metrics.json) and the [streaming analyser](issue-1-connector/analyse_host_session.py) support the results below.

## Exact environment and scope

Production baseline: main `520a4669572b105848d00763a4dd3fc2550919ea`; investigation branch `codex/1-pattern-playback-spike`, evidence through `97a555c` plus this analysis. Windows x64 `10.0.26300`, FL Studio `26.1.7.5654`, MIDI scripting API **45**, embedded Python **3.12.1**, JUCE **9.0.4**, MSVC VS 2026 / Windows SDK `10.0.26100`.

Validation distinguishes **native Release CTest 1/1 passed**, **18 Python fixture/wiring tests passed**, owner listening/screenshots, and actual WAV/CSV analysis. No full production MSVC/CTest rerun was needed or claimed for this diagnostic-only spike.

Separate receiver: **ReverbArise FL Context Probe 0.0.1**, `RASp/RCp1`, bundle ID `org.reverbarise.spike.contextprobe.v1`; installed binary SHA256 **`CC581F7FC5D398AFBB8E7D5DC8EAC3B4861BA933D7CD8F9F310238AA8F4C94D1`**. The installed production binary remained **`0A96FCCD07633CBAE7D00FB9A445E762A97C59F59AB6C3A2E7783F00B1605BD3`**. No production C++/spec changes, capture/render/asset mutation or state migration occurred. Owner testing used a saved disposable copy, `PreverbTesting_2.flp`; the agent did not edit/save the owner's project or controller settings.

## Evidence matrix

| Question | Documentation / source | Actual host evidence | Fixture / inference / limit |
| --- | --- | --- | --- |
| Native Song/Pattern signal | Standard JUCE/VST3 has no explicit flag; see [initial report](2026-11-05-issue-1-pattern-playback-spike.md). | No native extension established. | Do not classify by bar 1, loop length, seeks, silence or sample/PPQ origin. |
| Script signal and direct delivery | [Image-Line API](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/midi_scripting.htm) defines `getLoopMode()` and effect-slot parameter access. | Receiver consumed sequence **2 / Song**; continuous Pattern/Song changes stayed FRESH. Mode changes explicitly logged **OnRefresh flags 256**; idle heartbeat also ran. | No IPC needed for this experiment. A correct sampled mode does not imply a universal delivery deadline. |
| Activation | Script selection is separate from copying files; `device.isAssigned()` identifies a linked output. | Input-only initialization worked but automatic callbacks did not. Input port **1**, linked-output **0**; matching unused MCU/HUI **output port 1** started idle/refresh callbacks. Master sync stayed off. | Observed dependency in this setup, not proof for all FL versions/devices. Dedicated input/output availability is material setup friction. |
| Realtime behavior | Probe adds a known -24 dBFS 440 Hz signal only for fresh Song, with a bounded fade; dry always passes. | Owner reported expected tone behavior during Song/Pattern switching and the immediately-before-Play test. | Listening acceptance, not a sample-perfect residual measurement. Sub-audible leakage and exact UI-click-to-audio latency were not measured. |
| Multiple instances / lifecycle | Fresh per-runtime identities and name-based parameter discovery; saved tone enable separate from context. | Two distinct instances remained FRESH; reload recovered both. Owner performed move/save/reopen steps; reopened copies had new runtime IDs, correct Pattern and tone OFF. | Exact moved slot index and transient replay before the displayed recovered state were not independently captured. Check/write race and queued/replayed messages remain shipping risks. |
| Native implementation | Fixed atomic parameters, callback log ring; CSV formatting/writes outside audio callback. | Release VST3 loaded in FL; receiver logs captured **198,515 callback rows** across four runtimes. | Focused native CTest passed with watched allocation/deallocation counts **0/0**. This is not a full production realtime audit. |
| Offline bounce | Native probe has **no unconditional offline mute**. | Actual offline callbacks received Song but **fresh=0** throughout; enabled tone generated zero signal, and the WAV was digital silence. | This demonstrates the probe's realtime lease policy suppressing offline output, not a production #73 regression. |

## Measured delivery and offline result

QPC stamps use 0.125 ms ticks. The analyser takes the first observed row for each changed packet, filters fresh realtime receipts, and reports the stamp age at that callback. Timestamps and packet use separate parameters, so host ordering/coalescing can affect interpretation. These are **receipt-age fields**, not independently timestamped mouse-click or audio-leakage latency.

| Reopened receiver | Fresh realtime receipt samples | Median | p95 | Maximum |
| --- | ---: | ---: | ---: | ---: |
| `603fe4…` (tone-enabled bounce instance) | 358 | 4.000 ms | 13.875 ms | 157.625 ms |
| `4253a3…` | 356 | 3.875 ms | 14.375 ms | 113.125 ms |

The corresponding minima were 0.125 / 0.250 ms. Earlier setup had long stale intervals; those are excluded from fresh receipt statistics, not silently treated as low latency. The screenshot's continuously increasing timestamp age is not itself a new-packet delivery measurement. The initial pickup hint did not establish persistent blocking: subsequent fresh receipts succeeded without changing global pickup.

**Offline WAV:** `probe-song-bounce.wav`, SHA256 **`30933ae67aa6a1ab4e65c1ec3d5c44c9dd055a99d539736def7ae64e91ffbe9b`**, stereo PCM16, 48 kHz, **281,707 frames / 5.868895833 s**, peak **0**, RMS **0**, **zero nonzero sample values**.

Both receivers logged **387 offline callbacks** processing 283,171 frames, including host boundary/flush processing beyond the exported WAV. In the tone-enabled instance, all 387 had Song mode, tone enable ON and freshness OFF; 384 reported Playing and 3 stopped. First offline query age was **1565.25 ms** (write age 1565.125 ms), already beyond the experimental **750 ms** lease. Offline processing elapsed approximately **0.211641 s**; its sequence remained **4187**, with no new packet consumed. The second receiver likewise received no new packet. Generated signal peak stayed **0**; input was only subnormal-level residue (maximum about `4.08e-30`), quantizing to zero in PCM16.

Thus **the lease was stale before bounce started**, rather than expiring midway through this fast bounce. Fresh packets were not consumed during rendering; native logs alone cannot distinguish suspended script callbacks from host parameter-delivery suspension. This corrects the old API-double model's unconditional `not offline` result, which proved only its chosen mute policy and nothing about expiry.

## Setup, updates and removal

Installation required a separate system VST3 bundle, FL closed, Windows UAC, plugin rescan and an explicit spare MIDI-script assignment. Copying did not activate the script. FL omitted `__file__` and rejected `mkdir`; the script now performs no filesystem operations and uses Script Output plus bounded memory. Native CSV remains automatic. FL exposed **4240 wrapper parameters**; discovery now finds the four exact names, permits that count and caches verified indexes separately from frequent updates.

Existing script updates were delivered by replacing only the owned script and using Reload. No host restart was required for those tested updates. Initial plugin scan/setup was manual. There is no demonstrated seamless VST3-driven activation, virtual-port provisioning or controller configuration API. A virtual endpoint could be an alternative to spare hardware; **NOT RUN**. Automatic installer updates/removal were not host-tested end-to-end; retain prior files and require explicit controller detachment. The prepared removal procedure retains the separate probe bundle and script evidence; restore both temporary input/output assignments afterward. No computer control is permitted without the owner's prior agreement.

## Recommendation and proposal boundary

Keep the explicit per-instance Perform scheduling enable as the universal fallback. An **optional, explicitly configured realtime FL connector is worth proposing**: it supplies genuine mode context and worked in the tested realtime journeys. Do not promise zero-click setup or zero wet leakage. IPC is unnecessary for basic delivery; consider it only if addressing, replay or persistence requirements cannot be met by direct parameters. It would not cure missing host callbacks or offline ordering automatically.

A production proposal must separately define:

1. **Realtime admission:** transient versioned runtime context/heartbeat, bounded callback consumption and visible missing/stale/incompatible states. Song context must never turn saved user enable ON. Pattern inhibition must preserve dry, including a saved Wet Only route. Learn Pattern capture remains fully intentional and unaffected.
2. **Identity and lifecycle:** safe effect-slot reconciliation, duplication/project epochs, runtime challenge/ACK, stale/replayed/automation values, no saved host-mode authority, and bounded discovery work. Two-instance success alone does not solve a check/write race.
3. **Offline behavior:** do not apply the realtime wall-clock lease unchanged to host bounce. A longer timeout would only hide this particular delay and could admit stale Song during Pattern. Establish explicit operation-scoped Song intent or another reviewed offline contract, preserving #73's playback/bounce correctness work. Offline mode is not proof of Song. No automatic Pattern-bounce guarantee was established.
4. **Installation/UI review:** dedicated endpoint/paired-port onboarding, explicit owner configuration and update/removal recovery; reuse existing Perform controls/status policies with scoped native visual/interaction review before rollout.

Next phase is separately reviewed vanilla OpenSpec **Propose**, then stop before a fresh Apply request. Current canonical specs remain unchanged. Historical PR #2's blanket Pattern-capture suppression remains rejected. #71 tempo maps and #73 constant-tempo/bounce correctness remain separate; this probe's own offline suppression does not establish a shared production defect.

**Not run:** other FL versions/hosts, virtual endpoints, nested Patcher instances, independently instrumented short Song loops/seeks, queue/automation replay fault injection, native residual PCM measurement for realtime switching, and a corrected offline design. Historical capture loss remains unconfirmed. Existing listening and lifecycle checks need no repetition for this spike; follow-up tests should target these concrete unresolved properties.

Reproduce the analysis without modifying source evidence:

```text
python -B docs/validation/issue-1-connector/analyse_host_session.py out/issue-1-native/final-host-snapshot --wav out/issue-1-native/final-host-snapshot/probe-song-bounce.wav --output out/issue-1-native/reanalysed-host-metrics.json
```

Raw host snapshots, WAV and package remain local under `out/issue-1-native`; only compact metrics, reports and diagnostic sources are committed. No production fix, merge or issue closure is claimed.

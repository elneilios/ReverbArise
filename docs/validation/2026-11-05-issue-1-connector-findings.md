# Issue #1: connector findings and recommendation — 5 November 2026

**Result:** an optional FL Studio script can supply explicit Song/Pattern context to a VST3 audio callback through `plugins.setParamValue()`. The native probe demonstrated correct fresh reporting, audible realtime tone gating, two instances, script reload and save/reopen. **The same realtime freshness gate failed the offline test:** context was already stale when rendering began, and the exported tone was entirely absent. Technical feasibility is proven, but **product feasibility is conditional on seamless, controller-independent setup**, which is not established. The tested hardware-controller assignment is not an acceptable shipping experience.

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

Keep the explicit per-instance Perform scheduling enable as the universal fallback. **Do not proceed to a production connector proposal on technical delivery evidence alone.** The owner requires seamless setup without borrowing MIDI controllers or a sequence of MIDI-settings changes. A dedicated integration component is worth further consideration only if it passes the installation/activation gate below. Otherwise reject the script connector as a product approach. IPC is unnecessary for basic delivery; consider it only if addressing, replay or persistence requirements cannot be met by direct parameters. It would not cure activation, missing host callbacks or offline ordering automatically.

## Required installation and setup experience (owner requirement)

**One optional installer choice, then automatic connection for every instance.** A normal installer approval and, if necessary, one FL restart may be disclosed; a MIDI configuration tutorial is not acceptable. The user must not need a hardware controller, third-party virtual-cable utility, device-name selection, port-number choice, script selection/reload, a project template, MIDI routing, or per-instance setup. Existing controllers, assignments, ports, musical input and projects must remain untouched. This is an acceptance gate, not a claim about implemented capabilities.

Target journey:

1. The user selects **FL Studio integration** once in the normal ReverbArise installer or an equivalent first-run flow. The installer detects supported dependencies and the actual configured FL user-data location, including custom locations. It installs only owned adapter files and a small companion if needed. Unsupported systems get an honest unavailable result and the explicit scheduling fallback.
2. A companion supplies **ReverbArise-owned virtual input/output endpoints**, with a private stable identity. No physical device is reused, no keyboard/controller script is replaced, and no musical data is rerouted. Startup/lifetime is managed automatically; the user never launches a separate port utility. A device must remain available early enough for FL discovery and across all relevant instances, projects and restarts.
3. FL automatically associates the script with those endpoints, enables the required interfaces and establishes whatever paired-port condition is required for idle/refresh dispatch. The plugin reports **Connected: Song/Pattern** only after callback-consumed handshake/freshness validation. It must not claim success merely because the installer copied a file or the OS exposed a port.
4. New/duplicated/moved instances and projects discover the existing integration automatically. Updates coordinate plugin/script/companion versions and reconnect after the documented host restart, without asking the user to press Reload. Removal detaches/removes only owned components and endpoints, leaving existing controller configuration intact; no manual MIDI-settings restoration is part of normal uninstall.

The spike did **not** establish steps 1–4 end-to-end. Its native installer and manual hardware pairing are experiment tooling, not a proposed customer workflow.

| Candidate | What is supported / observed | Missing proof and UX verdict |
| --- | --- | --- |
| Reuse a physical controller/spare hardware port | Worked in this session after explicit input/output pairing. | **Rejected for product use.** Requires hardware and interferes with controller configuration. |
| Ask users to install a virtual cable and configure FL | Avoids borrowing hardware in principle; not tested. | **Rejected for normal onboarding.** Moves setup work to the user and adds another application/dependency. |
| Owned virtual endpoints + FL automatic script matching | [Image-Line documents `supportedDevices` / `supportedHardwareIds`](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/midi_scripting.htm) for automatic association with matching devices. Use only private ReverbArise identities; never list existing controller names. | **Candidate only.** Automatic association does not establish enabling, output pairing, collision-free ports, callback activation or custom-user-folder discovery. All must be demonstrated without opening MIDI Settings. |
| Companion-provided Windows virtual MIDI device | [Microsoft's virtual-device transport](https://microsoft.github.io/MIDI/kb/virtual-device-app/) documents application-created endpoints and optional legacy-client compatibility, with dynamic port-number caveats. [The SDK manager](https://microsoft.github.io/MIDI/sdk-reference/Transports/Virtual/MidiVirtualDeviceManager/) exposes transport availability and creation. | **Not tested in FL or on this machine.** Verify OS/service/SDK deployment, actual FL visibility, endpoint lifetime, restart and uninstall behavior. Do not assume the installed Windows build makes this available, or require users to change system MIDI modes. Supporting older stacks may entail a separately reviewed driver/distribution burden. |
| IPC instead of parameter delivery | Could change data addressing and replay properties. | **Does not solve script activation.** The FL side would still need a supported way to run. Not an installation workaround. |
| Direct supported FL/VST3 integration supplied by Image-Line | No suitable host extension/activation contract was established. | A vendor-supported route could avoid MIDI-device scaffolding; requires an actual contract, not assumptions or SDK material for another format. |

No documented supported API for silently enabling/pairing a new MIDI-script endpoint was established. **Do not automate MIDI Settings, edit private FL configuration/registry state, or hijack controllers to manufacture a seamless claim.** `supportedDevices` is a lead for a bounded onboarding feasibility test, not evidence that the whole journey works.

The next bounded feasibility experiment, if separately requested, should use a **clean FL profile with no MIDI hardware**, an owned virtual pair and automatic matching. Test installation/startup, custom user-data paths, all-instance callback delivery, restart/reboot, update and removal while retaining an independently configured controller as a non-interference control. Success means one installer opt-in and automatic useful connection without visiting MIDI Settings, choosing ports, reloading scripts or running helper utilities. If any such manual configuration remains unavoidable, **fail the UX gate and stop this connector route**; retain explicit Perform scheduling instead. This requires no repeat of the passing tone tests until onboarding itself works.

## Remaining production design boundaries

A production proposal must separately define:

1. **Realtime admission:** transient versioned runtime context/heartbeat, bounded callback consumption and visible missing/stale/incompatible states. Song context must never turn saved user enable ON. Pattern inhibition must preserve dry, including a saved Wet Only route. Learn Pattern capture remains fully intentional and unaffected.
2. **Identity and lifecycle:** safe effect-slot reconciliation, duplication/project epochs, runtime challenge/ACK, stale/replayed/automation values, no saved host-mode authority, and bounded discovery work. Two-instance success alone does not solve a check/write race.
3. **Offline behavior:** do not apply the realtime wall-clock lease unchanged to host bounce. A longer timeout would only hide this particular delay and could admit stale Song during Pattern. Establish explicit operation-scoped Song intent or another reviewed offline contract, preserving #73's playback/bounce correctness work. Offline mode is not proof of Song. No automatic Pattern-bounce guarantee was established.
4. **Installation/UI review:** dedicated endpoint/paired-port onboarding, explicit owner configuration and update/removal recovery; reuse existing Perform controls/status policies with scoped native visual/interaction review before rollout.

Production vanilla OpenSpec **Propose** is appropriate only after the onboarding feasibility gate passes and the owner elects to proceed; then stop before a fresh Apply request. This UX addition performs no new endpoint installation, helper rollout or controller configuration. Current canonical specs remain unchanged. Historical PR #2's blanket Pattern-capture suppression remains rejected. #71 tempo maps and #73 constant-tempo/bounce correctness remain separate; this probe's own offline suppression does not establish a shared production defect.

**Not run:** other FL versions/hosts, virtual endpoints, nested Patcher instances, independently instrumented short Song loops/seeks, queue/automation replay fault injection, native residual PCM measurement for realtime switching, and a corrected offline design. Historical capture loss remains unconfirmed. Existing listening and lifecycle checks need no repetition for this spike; follow-up tests should target these concrete unresolved properties.

Reproduce the analysis without modifying source evidence:

```text
python -B docs/validation/issue-1-connector/analyse_host_session.py out/issue-1-native/final-host-snapshot --wav out/issue-1-native/final-host-snapshot/probe-song-bounce.wav --output out/issue-1-native/reanalysed-host-metrics.json
```

Raw host snapshots, WAV and package remain local under `out/issue-1-native`; only compact metrics, reports and diagnostic sources are committed. No production fix, merge or issue closure is claimed.

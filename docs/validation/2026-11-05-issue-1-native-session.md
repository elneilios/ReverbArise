# Issue #1: native connector experiment — 5 November 2026

Status: **native experiment prepared; FL script → VST3 callback proof is awaiting the guided host session.** This extends PR #74's Explore spike, not production ReverbArise. The [short session guide](issue-1-connector/NATIVE-SESSION.md) starts with successful connection and then measures useful behavior; no universal zero-leakage claim is required to run it.

## Native output and identity

The separate CMake project under [native/](issue-1-connector/native/) builds **ReverbArise FL Context Probe** 0.0.1, with manufacturer `RASp`, plugin code `RCp1`, bundle ID `org.reverbarise.spike.contextprobe.v1`, processor CID `ABCDEF019182FAEB5241537052437031` and controller CID `ABCDEF011234ABCD5241537052437031`. These differ from production's identities. It does not link production capture, session, asset or render code.

Prepared bundle: `out/issue-1-native/delivery-v1`; local ZIP: `out/issue-1-native/ReverbArise-FL-Context-Probe-v1.zip`. Receiver binary SHA256: `CC581F7FC5D398AFBB8E7D5DC8EAC3B4861BA933D7CD8F9F310238AA8F4C94D1`. The package retains binary/script and source inventories. The installed production binary SHA256 remained `0A96FCCD07633CBAE7D00FB9A445E762A97C59F59AB6C3A2E7783F00B1605BD3` before/after preparation. Earlier failed native-test logs were retained separately under `pre-host-native-runs`; no native-test rows remain in the host evidence glob.

Baseline is PR commit `80ca43ac88a5fe236826b9f0a6653e2f2656ade8` plus this native experiment. Windows x64 API version 10.0.26300, MSVC VS 2026 / SDK 10.0.26100, cached pinned JUCE 9.0.4 / VST3 SDK 3.8.0. Build artifacts and source/binary hashes are retained under this worktree's ignored `out/issue-1-native`; no binaries are committed to the PR.

The [probe build wrapper](issue-1-connector/native/build-probe.ps1) reuses repository tool discovery, cache identity checks and `Enter-BuildLock`, operating on its own tree. It performs Release build plus focused CTest; it does not configure or build production. The adapted [installer](issue-1-connector/native/install-probe.ps1) uses the documented system VST3 folder, full-bundle staging/hash verification, prior-probe retention and rollback. The existing production installer cannot be invoked unchanged because its target name is hardcoded to ReverbArise.

## Results obtained before FL testing

| Evidence | Result / limit |
| --- | --- |
| Native MSVC Release receiver and VST3 module | **Built**, including moduleinfo generation. Unique processor/controller CIDs verified from moduleinfo. |
| Native callback CTest | **1/1 passed.** Actual receiver parameter consumption/ACK, unchanged dry with tone disabled, Song tone, Pattern fade-out, fresh offline Song admission, subsequent wall-clock freshness expiry, and transient state reset exercised. Watched callback path had **zero C++ allocations**. This is a native processor test, not FL delivery or a universal realtime proof. |
| Native UI | Editor rendered and inspected as a PNG. Mode, effective freshness, consumed sequence, block count, delay, host flags, tone level, log path and dropped-row status are visible. Snapshot is under `out/issue-1-native/editor.png`. |
| Script wiring regression | **15 Python tests passed** across retained historical fixtures and the native-script wiring checks. New checks verify refresh-driven mode change, heartbeat throttling, separate discovery and parameter lookup with extra wrapper parameters; these remain API-double results. |
| Real bug caught by native test | JUCE's convenience float parameter constructor uses a 0.01 interval, destroying packed packets. Replaced with explicit continuous ranges; native consumption now passes. This was invisible to the previous API doubles. |
| Initial configure | Sandbox MSBuild could not read Windows SDK registry information. Retried outside sandbox with the same wrapper/tree; compiler/dependency caches were preserved. Early compile/test failures were fixed before claiming a passing build. |
| Installation | **Not completed at preparation time:** installer detected running FL64 and stopped before copying any bundle or controller script. The owner was asked to save/close FL. Sandbox process visibility was incomplete; the escalated installer check is authoritative. No process was terminated or controller reassigned. |
| Actual FL callbacks, OnRefresh flags, delay, sound and offline export | **NOT RUN yet.** The owner-guided session is the next measurement, not another hypothetical receiver investigation. |

Canonical native command: `./docs/validation/issue-1-connector/native/build-probe.ps1`. Logs: `out/issue-1-native-build-*.log`; matching CTest detail: `out/issue-1-native/build/Testing/Temporary/LastTest.log`. Dependency sources were warm/cached; this separate target's initial native object compilation was cold. No comparative performance claim is made.

## Working measurement design

[device_ra_context_native.py](issue-1-connector/device_ra_context_native.py) addresses the actual receiver's four explicit parameters: context/sequence, callback ACK, query-clock stamp, write-clock stamp. It writes query and write stamps before the context packet. Parameter reads/writes use the direct FL API; no IPC, MIDI output route, production playback-enable write or asset mutation is involved.

Discovery is separate from frequent updates: full reconciliation nominally once per second, plus project/full-refresh invalidation. It finds the four parameters by exact unique names, allowing extra wrapper bypass/program parameters; it never assumes their numeric indexes. Known targets receive mode changes promptly on `OnRefresh` **if that event actually fires**, otherwise the next idle update; unchanged mode is heartbeated no more often than 100 ms. All refresh flags and idle/write sequences are logged, so the experiment can establish which notifications FL delivers instead of assuming `HW_Dirty_LEDs` means Pat/Song. Every target is revalidated before writing; the existing check/write race remains a shipping-design limitation, not a reason to postpone measurement.

The native callback loads atomic parameter values, consumes changed packets, updates ACK without host notification, performs a bounded stereo sample loop and pushes a POD record into a fixed SPSC ring. It performs no file access, waiting, mutex acquisition or dynamic allocation in the measured path. A message-thread timer drains the ring and writes CSV; an editor reports overflow/log failure. No production callback has changed.

An independent local **tone enable** defaults OFF and is the only saved probe preference. Context/ACK/timestamps are reset on state restore. The 440 Hz test signal is -24 dBFS peak (about 0.0631), added to unchanged dry input only for fresh Song context while host Playing or offline. Pattern, Unknown, disabled tone or expired context fades the generated signal out within 5 ms. The experimental lease is 750 ms. This is a measured-test mechanism, not a proposed production timeout/default. It never captures or alters accepted source/renders; the probe has no access to them.

Timestamps use Windows QPC: Python `perf_counter()` and native `QueryPerformanceCounter`, sent as 24-bit ticks at 0.125 ms resolution, modulo 2097.152 seconds. The script logs its runtime/clock implementation and full QPC query/write values; native logs callback QPC, host sample, block size, sequence, flags and per-block generated signal peak. Large/implausible deltas must be treated as clock/order/coalescing evidence, not presented as measured latency. Writing multiple parameters is not a transactional timestamp/packet protocol: inspect logs for coherency. Native CSV/editor, not potentially cached host ACK readback, proves callback consumption.

The [analyser](issue-1-connector/analyse_native.py) reports first-consumption query/write delays, observed modes, fresh/stale/offline rows and script event flags. It deliberately rejects an empty receiver directory as no delivery evidence. It does not infer exact mouse-click-to-audio latency from a later mode query. The [collector](issue-1-connector/native/collect-evidence.ps1) packages only owned diagnostic CSV/WAV files; it does not collect DAW projects or production assets. Native-test CSVs are explicitly named/excluded from host summaries.

## Corrected offline interpretation

The earlier Python model's unconditional `not offline` predicate proved only its selected mute policy. Its 1,000-block result established **nothing about heartbeat expiry during bounce**. That wording and test commentary have been corrected.

The native receiver has **no unconditional offline mute**. Its focused native test first admits fresh Song tone with offline true, then advances wall-clock time beyond the lease and verifies fade-out. This distinguishes the native gate's actual freshness behavior from the old model's policy. FL's script scheduling and resulting WAV during offline bounce still need the prepared host test. A fast bounce may complete within the lease; a long bounce or modal UI may differ. Record actual flags/timestamps/audio rather than assume failure or success. #73 production bounce behavior is unchanged.

## Owner session and next recommendation

The full [guided session](issue-1-connector/NATIVE-SESSION.md) needs one explicit temporary input assignment, after the separate bundle is installed/scanned. The prior MIDI inspection showed existing hardware inputs; a spare disabled input is the simplest route **if the owner chooses it**. No virtual port has been installed. Direct parameter control does not require MIDI output routing; a selected/activated scripting input still needs confirmation in the real host. The guide records original controller settings and restores them at removal. Copying a script is never presented as activation. Script Output Reload supports the reconnect test; first-install discovery/restart friction remains an observed-session item.

First prove increasing callback-consumed sequences and fresh Song/Pattern. Then use the known tone for stopped/near-Play/mid-play switching, two instances/moved slot, script reload, disposable save/reopen and one short offline WAV. Expectations are conditional on actual callback processing and context; residual tone before mode delivery is measured honestly. No production projects or plugin instances are needed.

**Current recommendation:** proceed with this usable native experiment. Actual delivery, event frequency, latency distribution, residual audio and setup friction determine whether an optional connector is worthwhile. Shipping safety questions (runtime handshake/replay, moved-slot addressing, parameter persistence/automation, nested effects, scale, offline policy) remain separate follow-up work. Do not require a universal latency guarantee before collecting the first real host evidence. No rollout, merge, issue closure or production implementation is authorized; any implementation still needs separately reviewed vanilla OpenSpec Propose and a fresh Apply request.

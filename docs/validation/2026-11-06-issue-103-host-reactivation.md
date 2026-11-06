# Issue #103 host reactivation

## Current automated result

- Final Release wrapper configure/build/CTest: **75/75 pass**, 218.04 s, exit 0;
  `out/build-logs/Release/20261106-153141-46ebce6b/`.
- Debug: 74 unchanged tests passed in the full sequential run; the remaining
  popup test passes after the test-only Windows activation repair (1/1, 4.54 s).
  The failed original full-run results and focused reruns remain distinguished below.
- Read-only guide currency, UI standards, strict OpenSpec change validation,
  affected documentation link targets and `git diff --check` pass.
- Release identity: `v0.1.0-test.issue103`; numeric plugin version remains 0.1.0.
- System installation succeeds from clean code commit `87240d5`; the owner's
  exact FL Studio green-slot smoke remains pending. Draft PR: [#105](https://github.com/elneilios/ReverbArise/pull/105).

## Scope and environment

- Baseline: `dbfdb18`, issue branch `codex/103-host-reactivation`.
- Owner report: FL Studio mixer effect slot's small green enable/disable indicator, running the alpha 2 release. Exact alpha binary SHA and VST3 callback sequence were not supplied. The issue reports absent wet until loop wrap after re-enable while transport continues.
- Native environment: Windows x64, VS 2026 MSVC 19.51, Windows SDK 10.0.26100.0. This worktree starts with a cold compilation tree; pinned JUCE 9.0.4 and WavPack source directories are reused read-only from the existing checkout. No build cache or binaries are copied between worktrees.
- Configure: VS bundled CMake `--preset windows-msvc`, `FETCHCONTENT_UPDATES_DISCONNECTED=ON`, with `FETCHCONTENT_SOURCE_DIR_JUCE` and `FETCHCONTENT_SOURCE_DIR_WAVPACK` pointing to the existing populated dependency sources. Subsequent builds use `scripts/build.ps1` and its lock.
- Cached dependency Git metadata was verified clean at JUCE tag `9.0.4` and WavPack commit `5803634a030e2a11dba602ba057b89cc34486c67`.

## Observed implementation boundary

The pinned JUCE VST3 wrapper's `setActive(false/true)` forwards to `releaseResources`/`prepareToPlay`, which already reset ReverbArise's live clock. Separately, `setProcessing(false)` forwards to `AudioProcessor::reset()`. ReverbArise inherits JUCE's empty default reset. Framework bypass calls `processBlockBypassed`, whose inherited implementation passes input through without advancing plugin processing.

These are inspected wrapper paths, not proof of which calls FL Studio's green indicator emits. The regression harness explicitly labels its sequences as synthetic.

## Reproduction and results

`tests/ProcessorReactivationCases.h`, invoked by `preverb_workflow_processor_test --test-reactivation` and normal CTest, captures accepted source through the real processor and installs two verified managed renders with nonconstant stereo samples. It checks wet against an independent absolute-offset sample formula after the existing 5 ms join, with callback allocation/deallocation guards and saved definitions/source/render preservation checks.

Baseline Debug CTest and the focused executable both pass skipped callbacks,
release/prepare, inherited bypass and reset with unchanged origin, including
interior and later-swell sample checks. After a processor reset followed by a
16-sample origin change, both fail at resumed sample 1600: gate 10 (origin),
`liveActive && !liveIncompatible` false. Thus a concrete VST3 processing-reset
path retains stale run authority; this is not confirmation of FL Studio's exact
callback sequence or the cause in the owner's alpha 2 binary.

Focused baseline command: `out/build/windows-msvc/Debug/preverb_workflow_processor_test.exe --test-reactivation`
(exit 1, expected reproduced defect). It requires the normal native-test
permissions for temporary managed-asset writes, matching the build wrapper;
sandboxed attempts instead timed out during fixture setup and are not counted
as transport reproduction evidence.

The first serial cold Debug build was deliberately stopped with its identified
child tree and resumed through the wrapper with `CMAKE_BUILD_PARALLEL_LEVEL=4`,
retaining completed outputs. Its separate concurrently running Release build in
another checkout was untouched. No comparative speedup is claimed.

## Repair and focused verification

`PreverbAudioProcessor::reset()` now forwards to `resetTransportClock()`, an
atomic request already used by release/prepare. No asset, saved preference,
source or render state is cleared. No sample-jump heuristic, origin tolerance
change or seek/loop exception is introduced. Existing worker checks reject
superseded clock authority and retry using current dependencies/position.

The repaired focused Debug command exits 0. The full Debug suite's
`preverb_workflow_processor_test` also passes (3.27 s). The new fixture covers:

- Four unchanged-origin paths: skipped callbacks, release/prepare, inherited bypass and reset; each checks interior wet and a later swell before wrap.
- Reset followed by a new valid 16-sample origin: both previously failing interior wet and later wet now match the stereo sample oracle.
- Three repeated cycles with duplicate reset requests, distinct reset authority, 23/37/97/127-frame blocks and ordinary forward/backward seeks/loops.
- A two-sample origin change without reset remains rejected across internal Live disable/re-enable; returning to the retained mapping restores output.
- Cold windows recover at the advanced current offset; a held publication is rejected after reset and retried, and session restore supersedes held work.
- Missing timing after reset cannot admit stale wet or establish a false Stop; valid timing subsequently joins current PCM.
- Callback/reset allocation and deallocation guards, unchanged serialized swell definitions/preferences and source reference, and checksum verification of both renders.

Guide review found no musician-facing control/action change. Existing Perform
bypass, asset-retention and compatible-timing/readiness prose remains accurate;
the semantic review record was explicitly refreshed. Read-only guide currency,
UI standards (61 files; existing 60 bounded occurrences) and strict change
validation pass. No UI occurrence inventory was regenerated.

## Integration gate and pause

Repaired Debug wrapper run:
`CMAKE_BUILD_PARALLEL_LEVEL=4`, `CTEST_PARALLEL_LEVEL=4`,
`scripts/build.ps1 -Configuration Debug -Test`.
Configure/build pass; 74/75 CTest pass in 205.03 s, wrapper exit 8.
Logs: `out/build-logs/Debug/20261106-143659-4ab743c5/`.
The transport playback test passes (0.14 s), engine test including existing
independent-instance/#73 negative coverage passes (95.96 s), and processor
lifecycle test passes (3.27 s). The only failure is the unchanged broad
`preverb_swell_ui_test`, reported as SegFault without output.

The baseline build's earlier interrupted suite also records this same UI
SegFault (4.65 s) in
`out/build-logs/Debug/20261106-142543-07cbf3cf/test.log`.
An isolated repaired rerun using VS bundled CTest
`--preset windows-msvc-debug -R '^preverb_swell_ui_test$' --output-on-failure`
also fails (4.88 s total). It therefore is not attributable solely to parallel
CTest execution.

Windows Application Error events at 14:36:13 (baseline), 14:38:38 (repaired
parallel suite) and 14:42:00 (repaired isolated rerun), London time, all identify
`nvwgf2umx.dll` version 32.0.16.1088, exception `0xc00000fd`, offset `0xeaa127`.
This is evidence of the same native graphics failure before and after this
change, not a complete root-cause diagnosis. No graphics driver, renderer,
test standard, production UI or required gate was changed to bypass it.

At this checkpoint Apply paused for owner authorization to investigate the
adjacent validation blocker. Release compilation/tests, system test installation
and FL Studio smoke had not run. FL Studio was observed closed before the pause.
No host acceptance or full validation pass was claimed.

## Authorized native UI validation repair

The owner authorized investigating the validation blocker on 2026-11-06.
VS `dumpbin /headers` reports a `0x100000` (1 MiB) stack reserve and `0x1000`
commit for the original executable. `dumpbin /symbols` and `/disasm` on
`preverb_swell_ui_test.dir/Debug/swell_ui_test.obj` identify the main test
lambda at section offset `0x24A70`; its prologue loads `0xBA888` (764,040 bytes,
about 746 KiB), calls `__chkstk` and subtracts that amount from RSP. This leaves
little of the default reserve for nested fixtures, native peers and graphics.

Copied the executable into ignored `out/issue103-stack-probe/`, changed only
that copy's PE stack reserve with VS `editbin /STACK:8388608`, and ran the full
UI executable. It exits 0 with the existing renderer, driver, assertions and
test code. No original binary or graphics setting was changed for this probe.
This isolates stack headroom as the demonstrated validation repair; the NVIDIA
fault location alone did not establish a driver defect.

`CMakeLists.txt` now applies `/STACK:8388608` only to the MSVC
`preverb_swell_ui_test` target. The generated normal executable reports reserve
`0x800000`, with commit still `0x1000`. Its normal Debug CTest run passes in
73.05 s during wrapper run
`out/build-logs/Debug/20261106-144715-c6d14aa6/`. No required test, assertion,
renderer or production target is removed or altered. Local development guidance
documents the test-harness constraint; the user-guide review includes this
test-only build-input change.

That parallel suite completed 73/75 in 196.05 s: the UI crash was gone, but
`preverb_perform_knob_reset_test` reported a missing native popup and
`preverb_processor_test` timed out at `LiveResourceCases.h:34`. Both had passed
in the preceding parallel run. Rather than modify those tests, the final suite
was restarted with `CTEST_PARALLEL_LEVEL=1`, restoring the wrapper's default
sequential test model. Build parallelism remains four; the popup/timing failures
are recorded rather than presented as a complete graphics or scheduling diagnosis.

## Sequential Debug integration and popup fixture

The sequential wrapper run at
`out/build-logs/Debug/20261106-145116-a523809f/` completes 74/75 in 679.18 s.
The processor resource test now passes (46.87 s), as do the broad UI test
(58.36 s), new lifecycle test (2.26 s) and engine test (81.27 s). The sole
remaining failure is the native reset-popup test, so concurrency alone cannot
explain that failure.

Inspection of pinned JUCE `PopupMenu::doesAnyJuceCompHaveFocus` shows that it
rejects a background owning process before considering JUCE component focus.
The fixture previously asserted only `level.hasKeyboardFocus(false)` after
creating a temporary peer. Its native setup now follows the existing broad UI
test's peer-focus pattern: a normal test peer, `grabFocus()`, a bounded pump,
and explicit native-peer/foreground-process assertions before invoking the
unchanged asynchronous menu. All menu, gain, cancellation, stale-action,
identity and persistence assertions remain intact. No production control code
or popup behavior is changed.

Built the changed test through `scripts/build.ps1 -Configuration Debug`
(exit 0; tests not requested in that build invocation), logs
`out/build-logs/Debug/20261106-150353-8f971a7f/`. VS CTest
`--preset windows-msvc-debug -R '^preverb_perform_knob_reset_test$' --output-on-failure`
then passes 1/1 (6.46 s total), retained in
`out/issue103-stack-probe/knob-focus-debug.log`.

Debug evidence is the 74 passing tests from the full sequential run plus the
repaired focused popup test, not a claim that the previous full wrapper exited
0. No product/library implementation changed after that run; the later edit is
inside `Composition::verifyKnobResets`, dispatched only by `--test-knob-resets`.
The unchanged passing coverage is retained rather than repeating every test
for a fixture-only change. Full Release integration remains required.

## Explicit Windows activation

The first cold Release wrapper build succeeds, but CTest records 74/75 in
278.51 s at `out/build-logs/Release/20261106-150506-5a3e1d60/`.
The popup fixture fails its new native-focus precondition, before invoking a
menu. All product/audio regression targets pass. This shows that the initial
peer-focus setup alone is insufficient under the full CTest launch context.

Further pinned-JUCE inspection finds Windows peer `toFront` calls
`BringWindowToTop`, `grabFocus` calls `SetFocus`, and
`Process::makeForegroundProcess()` is empty. The fixture now explicitly calls
Windows `SetForegroundWindow` for its own peer and retains both native focus
assertions, the bounded wait and all real menu/reset checks. No system focus
policy, external window, production control or graphics driver is modified.

The normal Debug build succeeds at
`out/build-logs/Debug/20261106-152954-f4643964/`; the repaired popup CTest then
passes 1/1 in 4.54 s total, recorded in
`out/issue103-stack-probe/knob-activation-debug.log`. This supersedes the earlier
focused popup setup result. Final Release integration is rerun with this
test-only change; the 74 unchanged passing Debug tests remain applicable.

The final incremental Release command is
`scripts/build.ps1 -Configuration Release -Test -ReleaseTag v0.1.0 -Candidate test.issue103`,
with `CMAKE_BUILD_PARALLEL_LEVEL=4` and `CTEST_PARALLEL_LEVEL=1`.
It exits 0 with all 75 CTest targets passing in 218.04 s. The reset-popup test
passes (1.73 s), and all lifecycle/audio/asset/persistence coverage remains active.
This is native MSVC/CTest evidence, not simulated orchestration fixtures or
FL Studio acceptance. Unmodified packaging/installer fixture suites were not
rerun; no package, release, driver update or system focus-policy change is made.

## Local test installation and review delivery

Committed reviewed code/planning/evidence as
`87240d58653ed43fa8f291a816740ec1660b669f` on
`codex/103-host-reactivation`, based on current main `dbfdb18`.
The stock `scripts/install-test-vst3.ps1 -ReleaseTag v0.1.0 -Candidate test.issue103`
rebuilds the Release VST3 from that clean commit. Its normal-permissions attempt
successfully builds but encounters a Windows ACL denial opening the system
installation lock before replacing anything. The documented UAC retry runs the
same installer with a hidden launcher, captures its transcript/exit status and
succeeds (exit 0); no folder ACL is changed.

- Installed bundle: `C:\Program Files\Common Files\VST3\ReverbArise.vst3`.
- Binary SHA256, independently checked against the installer receipt:
  `2F0C596EE1D38D5D52862F8380D78D07199F1D7344A2F63C89547480F49EF4A1`.
- Retained previous bundle:
  `out/vst3-install-backups/20261106-153946-41449f80760a41249af4c99e7c81eede/ReverbArise.vst3`.
- Clean installed build identity: `v0.1.0-test.issue103`, Git `87240d5`,
  `worktreeDirty=false`. Later evidence/task-only commits do not change that binary.
- Elevated installer transcript/result: `out/issue103-stack-probe/install-elevated.log`
  and `install-elevated-result.json`; native plugin-only build logs:
  `out/build-logs/Release/20261106-153940-f597cb3a/`.
- Installer rebuild passes; it does not rerun CTest. The full Release CTest pass
  above covers the same code before committing, with its earlier build identity
  recorded separately. Installer inventory/version/hash checks cover the installed artifact.

[Draft PR #105](https://github.com/elneilios/ReverbArise/pull/105) targets main,
uses `Refs #103`, links this change and evidence, and is left open for owner
review. It is not merged; the issue and OpenSpec change remain open/unarchived.
The owner was asked for one consolidated smoke after rescan with Verify plugins
and Rescan previously verified plugins, confirming the system bundle and About
identity, then two green-slot disable/re-enable cycles during running transport.
Actual FL Studio reactivation, its exact callback sequence and physical host/DPI
coverage remain unverified pending that response.

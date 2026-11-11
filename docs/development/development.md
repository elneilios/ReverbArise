# Development Setup

## Requirements

- Windows x64
- Visual Studio 2026 Community with the **Desktop development with C++** workload, MSVC x64/x86 build tools, and a Windows SDK
- Visual Studio's **C++ CMake tools for Windows** component
- Git

The project pins JUCE 9.0.4 through CMake FetchContent. First configure needs Git and network access unless sources are already populated locally. The project/API minimum is CMake 3.22, but the selected CMake must support the **Visual Studio 18 2026** generator. The VS 2026 bundled CMake/CTest provides that support. Windows PowerShell 5.1 and PowerShell 7 are supported; no PowerShell modules are required.

## License Scope

JUCE is used for this feasibility spike as an internal evaluation dependency only. Do not redistribute JUCE source or a JUCE-based binary until the distribution model and applicable JUCE license terms have been reviewed. Any later closed-source or commercial release requires the appropriate JUCE licensing path. This note is a project constraint, not legal advice.

## Configure, Build, and Test

Use the local wrapper from the repository root in PowerShell:

```powershell
./scripts/build.ps1 -Configuration Debug
./scripts/build.ps1 -Configuration Debug -Test
./scripts/build.ps1 -Configuration Release
./scripts/build.ps1 -Configuration Release -Test
./scripts/build.ps1 -Configuration Release -PluginOnly
./scripts/build.ps1 -Configuration Release -PrRegression
```

Debug is the default. `-Test` builds first, then runs the full matching CTest
configuration; zero discovered tests is an error. Build-only success says testing
was not requested. You can invoke the script by absolute path from any directory,
including a checkout path containing spaces; the caller's location is restored.

During implementation and draft review, build the binaries needed for owner
previews and DAW checks. Start automated testing after the owner approves
marking the draft ready. For targeted checks, run the relevant CTest cases
against current binaries in an idle build tree using the matching preset with
`-R` and `--no-tests=error`. Choose Debug, Release or both based on which
configuration exercises the affected behaviour. For guidance-only diffs, use
relevant document checks as the targeted checks. The full Release suite runs
when the owner requests [release packaging](releases.md).

`-PluginOnly` builds `preverb_plugin_VST3` and its dependencies instead of the full
solution. It cannot be combined with `-Test`, since tests need their executables
built. Use the test installer's default plugin-only build for owner DAW checks.

### Standard PR regression set

After owner approval to mark the draft ready, run
`./scripts/build.ps1 -Configuration Release -PrRegression` alongside the
targeted checks. The wrapper builds `preverb_pr_regression` and runs all 15
cases labelled `pr-regression` under the same build-tree lock.
`CMakeLists.txt` defines the membership and build dependencies.

| Coverage | Existing cases |
| --- | --- |
| Build and plugin identity | `preverb_build_smoke`, `preverb_product_version_test`, `preverb_built_version_test` |
| Workflow and state | `preverb_workflow_processor_test`, `preverb_workflow_state_test`, `preverb_swell_session_test` |
| Capture and source | `preverb_capture_test`, `preverb_capture_transaction_test`, `preverb_persistent_capture_test`, `preverb_source_range_test` |
| Transport and audio | `preverb_scheduled_tone_test`, `preverb_transport_playback_test`, `preverb_reverse_convolution_test` |
| UI | `preverb_waveform_display_test`, `preverb_ui_standards_check` |

Record the tested revision, configuration, selected cases and outcomes in the
PR. Keep manual FL Studio acceptance separate in the record.

### Build tools and troubleshooting

The wrapper prefers a compatible CMake on PATH with adjacent CTest and otherwise
discovers Visual Studio 2026's bundled tools. It requires Git and VS C++ tools;
configure verifies the compiler and SDK. It never installs prerequisites. Configure,
build and test errors return nonzero native exit codes; preflight errors return 1.
Invalid configuration is rejected by PowerShell before native work. The final summary
includes configuration, Git revision/dirty state (or explicit unavailability), stage
outcomes and log paths. Full stdout/stderr is retained in per-run directories under
`out/build-logs/<configuration>/`; failures print the last 40 log lines. CTest also
retains its normal `Testing/Temporary/LastTest.log` in the build tree.

Both configurations share `out/build/windows-msvc` and its `_deps` sources. Each
invocation configures in place, then builds incrementally: no default clean, cache
deletion or forced dependency update. `FETCHCONTENT_UPDATES_DISCONNECTED=ON` skips
update checks for populated dependencies while allowing first downloads. A first
Release build still needs Release compilation. Incompatible cached checkout,
generator or architecture is rejected; use the original checkout/toolchain or
explicitly relocate the old tree while idle. Never copy a CMake cache between worktrees.

The wrapper holds an exclusive file handle on `.reverbarise-build.lock` **inside
the selected binary directory** throughout configure, build and tests. A second
wrapper fails immediately; different worktrees with separate binary directories
are independent. Exit or process termination releases the handle. The persistent
file is harmless: do not delete it to override an active lock. All local agents
must use the wrapper for mutations of this tree; raw CMake/MSBuild commands cannot
participate in this cooperative lock automatically. Wait for any raw build before
starting the wrapper.

After forcibly terminating a wrapper shell, confirm its child CMake/MSBuild
processes have stopped before retrying; raw or orphaned processes do not hold
the wrapper's file handle.

The broad `preverb_swell_ui_test` target reserves an 8 MiB main-thread stack
under MSVC. Its Debug fixture body alone uses 764,040 bytes (about 746 KiB), leaving
insufficient headroom in Windows' default 1 MiB reserve for nested native UI and
graphics calls. This test-only linker setting retains the normal renderer,
driver and assertions; it does not change the VST3 stack or audio callback.
See [issue #103 validation](../validation/2026-11-06-issue-103-host-reactivation.md)
for the original stack-exhaustion evidence and validation.

The native reset-popup fixture also activates its own Windows peer explicitly
and asserts native focus before opening the menu. JUCE component keyboard focus
alone does not satisfy the popup's foreground-process guard under CTest.

For troubleshooting, the equivalent preset commands are below. Run them only when
no wrapper or other build is active. They do not provide the wrapper's locking,
logging or exit summary:

```powershell
$cmake = "$env:ProgramFiles\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ctest = Join-Path (Split-Path $cmake) "ctest.exe"

& $cmake --preset windows-msvc
& $cmake --build --preset windows-msvc-debug
& $ctest --preset windows-msvc-debug --no-tests=error
& $cmake --build --preset windows-msvc-release
& $ctest --preset windows-msvc-release --no-tests=error
```

If compiler detection fails (for example, MSBuild tracking reports sandbox
`E_ACCESSDENIED`), resolve the permission/toolchain problem before retrying. A failed
configure can leave an incomplete cache, or empty compiler flags that later trigger
JUCE's "compiling without exceptions enabled" error. Inspect the configure log and
`CMakeFiles/CMakeConfigureLog.yaml` first. Missing source/generator/platform identity
is rejected by the wrapper; preserve the logs and explicitly relocate that incomplete
tree while idle before starting a new one. The wrapper never repairs caches automatically.

For a complete-identity cache with unexpectedly empty compiler flags, the
[recorded manual recovery](../validation/2026-11-02-local-windows-build.md#native-windowsmsvc-validation)
restores CMake's defaults without deleting dependency sources. Back up
`CMakeCache.txt` and confirm the flags are corrupt rather than intentionally
customized first. Perform any recovery with the build directory idle and its
`Enter-BuildLock` handle held through `try/finally`; do not add flag overrides or
automatic repair to the normal build command.

The first configure fetches the pinned JUCE release and libwavpack 5.9.0 (commit
`5803634a030e2a11dba602ba057b89cc34486c67`). WavPack is linked statically for
background mono/stereo lossless PCM/float IR decoding, including WavPack content
named `.wav`. Its BSD notice is copied into the VST3 bundle's `Contents/Resources`.
Build output is placed under `out/build`, which is ignored by Git.

Wrapper checks use native-command doubles, not MSVC or the plugin test suite:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/build_wrapper_test.ps1
pwsh -NoProfile -File tests/build_wrapper_test.ps1
```

They require Windows PowerShell's installed framework compiler but download no
packages. Fixtures and logs stay under ignored `out/build-wrapper-fixtures/`.
Native Debug/Release validation is separate and requires the supported local
Windows toolchain. In cloud or toolchain-limited environments, record builds and
CTest as **not run**, never passed. Automated tests do not establish FL Studio
manual acceptance. See the dated [Issue #19 evidence](../validation/2026-11-02-local-windows-build.md)
for actual commands, results and outstanding checks.

## Local release packages

For a one-command Release build directly into the system VST3 folder, see
[FL Studio test installation](fl-studio-test-install.md). This local testing helper
also preserves the previous bundle and supports optional Release CTest.

See [local Windows releases](releases.md) for validated internal ZIPs, checksums,
source manifests, extraction/FL Studio checks and the separate licensing/publication gate.

## Capture Cache (M1)

### M3 editable swell sessions

See [M3 editing](m3-editing.md), [regeneration](m3-regeneration.md),
[persistence](m3-persistence.md), and the separate [FL Studio acceptance guide](../validation/fl-studio-m3-validation.md).
M3 writes plugin state v3 with managed IR/render references while preserving capture
schema v1 and legacy plugin state v1/v2 restoration. Manual selected-swell audition
remains independent of host transport; Song-synchronized playback is M4.

Set `PREVERB_UI_M3_OUTPUT` to an output directory when running `preverb_swell_ui_test`
to render software PNGs for ready, shaped, stale, error, empty and many-swell states.
Set `PREVERB_SWELL_UI_PROFILE=1` and run the exact CTest case with `-V` to
log elapsed time at scenario and snapshot checkpoints, queued keyboard commands
and worker waits of at least 100 ms. For timing comparisons, leave screenshot
output variables unset and use the same build configuration and toolchain.

See [Automatic swell workspace](automatic-swell-workspace.md) for successor v4
state/defaults, automatic capture/detection/rendering, monitoring gain/blend and
the lean editor. [Host acceptance](../validation/fl-studio-automatic-workspace-validation.md)
keeps historical FL Studio checks separate from automated verification. Current
UI access and remaining acceptance are in [the UI checklist](../design/ui-diagnostics-and-acceptance.md);
Song transport playback is implemented. The older milestone notes below describe their historical versions.

### M2 source selection and reading

See [M2 rendering internals](m2-rendering.md) for IR channel/resampling conventions,
convolution alignment, numerical tolerance, and the preview memory budget.

M2 source controls select `[start bar, end bar)` in absolute Playlist coordinates,
independently of the tone controls. One host sample/PPQ/tempo/meter snapshot resolves
both boundaries with nearest-sample rounding. This supports fixed tempo and meter;
missing timing, incompatible sample rates, invalid bounds, and overflow are rejected.
The interval reader consumes an immutable committed manifest, decodes each intersecting
chunk once, retains covered silence, and reports uncaptured or invalid intervals.
It never substitutes zeros for missing source audio or repeatedly reads WAVs per sample.

Current Learn capture extends this original source format with schema-2 generation
contexts, published atomically with source coverage at Stop acceptance. Creative
state v9 retains imported IR choices; the processor envelope remains v8. See the
[Learn source/context protocol](learn-source-context-protocol.md) for bounds,
legacy migration, provisional-versus-accepted ownership and rollback copies.
Generation-consumer acceptance is tracked separately in the active OpenSpec tasks.

Capture assets are stored under `%LOCALAPPDATA%\K7MA\Preverb\Projects\<project-id>\<instance-id>`. Plugin state version 2 stores stable UUIDs, a revision, and an immutable `manifests/<uuid>.json` reference, not captured PCM. Each manifest records absolute sample positions, coverage kind, sample rate, frame count, revision, relative asset path, and SHA-256 checksum. Audio chunks are immutable stereo 32-bit-float WAV files in `segments/`. Captured silence is represented as coverage metadata without an audio file; uncaptured gaps have no entry.

The worker assembles at most five seconds per chunk (also capped at 1,920,000 frames), splitting at timeline discontinuities and audio/silence boundaries. Chunk files are written and validated before a new manifest is atomically published. The previous committed map stays authoritative until publication succeeds. A failed capture or Clear leaves the previous manifest, coverage, and referenced chunks intact and reports an Uncommitted status. Retry capture or Clear after resolving storage errors. Temporary and orphaned files may remain and are not automatically purged; never delete the working cache as routine disposable data.

Project save records the currently committed revision. Learn stages chunks privately throughout a pass and accepts one validated revision on explicit DAW Stop; idle time alone does not accept a partial pass. Wait for acceptance and a stable revision before saving. Unaccepted work is aborted on worker shutdown, and captures accepted after the last project save are not automatically adopted on reopen. Older immutable manifests remain available so a saved project restores its own revision, even after later unsaved capture or Clear operations. See [capture failure diagnostics](capture-failure-diagnostics.md) for retained failure evidence, provisional waveform removal and retry behavior.

Restore runs asynchronously with mapping paused and a Loading status. It rebuilds compact stereo min/max peaks (512 frames per peak) one bounded chunk at a time; it does not retain full-song PCM in RAM. Missing or corrupt chunks appear unavailable in red, while unrelated valid audio and captured silence remain visible. Mapping pauses until assets are restored and the project reopened, or the user explicitly clears and recaptures. A host sample-rate mismatch retains the old map and pauses mapping until Clear and recapture at the new rate. Tone scheduling and dry passthrough remain independent. Version-1 project state preserves tone settings and starts a new empty capture session.

The M1 portability guarantee is reopening the same saved FL Studio project on the same PC and Windows user profile. Save As/duplicate-project isolation, Pack Project, Relink, and cross-PC moves are deferred. Copied projects can share asset identities; concurrent editing is unsupported. The plugin does not infer the `.flp` path. See [M1 validation](../validation/fl-studio-m1-validation.md) for the host acceptance workflow and outstanding checks.

# Issue #19 local Windows build validation — 2026-11-02

Implementation branch: `codex/19-local-windows-validation`, based on `77438b7`.
The successful cold Debug build reports `77438b7 (dirty)`; the subsequent warm
Debug test run reports implementation commit `553c443`. Only fixture/documentation/task
tracking changes occur after that commit during the remaining validation.

## Environment

Windows x64, VS Community 2026 18.11.12328.437, MSVC 19.51.36260.0
(tools 14.51.36231), Windows SDK 10.0.26100.0 and bundled CMake/CTest
4.3.1-msvc1. PowerShell 7.6.9 and Windows PowerShell 5.1.26100.9807.

## Fixture checks (not native plugin validation)

- `pwsh -NoProfile -File tests/build_wrapper_test.ps1`: 49 assertions passed.
- `powershell -NoProfile -ExecutionPolicy Bypass -File tests/build_wrapper_test.ps1`: 49 assertions passed on Windows PowerShell 5.1.
- Native .NET Framework command doubles exercise argument selection/order,
  default Debug and explicit Release, no-test reporting, invalid input, unavailable
  tools, incompatible PATH CMake fallback, failed configure/build/test, zero tests,
  missing revision (including stderr), executable launch failure after discovery,
  logs, paths with spaces, caller location,
  cache preservation, subprocess lock contention, independent tree progress and
  lock release after failure. No compiler, native CTest suite or downloads occur.
- Logs retained under ignored `out/build-wrapper-fixtures/` and summary logs
  `out/wrapper-fixtures-ps7.log`, `out/wrapper-fixtures-ps51.log`.

## Native Windows/MSVC validation

This worktree started without a CMake build tree. Clean pinned JUCE and WavPack
sources were initially seeded from existing local `_deps` checkouts using local
Git clones. CMake's first population replaced those unstamped clones with its
normal FetchContent checkouts; this cold configure is not claimed download-free.
No CMake cache was copied from another checkout. JUCE HEAD:
`3fb5cbc0340758596dac39f7c8de47d562b4896e`; WavPack HEAD:
`5803634a030e2a11dba602ba057b89cc34486c67`.

The initial sandboxed Debug configure failed with MSBuild compiler tracking
`E_ACCESSDENIED`; exit 1, build and tests not run. Log:
`out/build-logs/Debug/20261102-232126-c88fe928/configure.log` plus
`out/build/windows-msvc/CMakeFiles/CMakeConfigureLog.yaml`. Authorized native
validation was retried outside the sandbox. It configured successfully but the
build failed with JUCE C1189 (exceptions disabled): the initial compiler-detection
failure had left `CMAKE_CXX_FLAGS` and configuration-specific flags empty in the
cache. This was a validation-cache issue, not a repository setting change.
Failed build output: `out/build-logs/Debug/20261102-232148-98ab1c1b/build.log`.

With the wrapper idle and its build-directory lock held, the targeted recovery was:

```powershell
& $cmake --preset windows-msvc -U 'CMAKE_C*_FLAGS*' -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
```

This restored normal MSVC defaults (`/DWIN32 /D_WINDOWS /EHsc`, Debug
`/Zi /Ob0 /Od /RTC1`, Release `/O2 /Ob2 /DNDEBUG`), preserving dependencies and
outputs. Recovery output: `out/cache-recovery.log`. No automatic cache repair or
compiler flag override was added to the wrapper.

| Command | Actual result |
| --- | --- |
| `pwsh -NoProfile -File scripts/build.ps1 -Configuration Debug` | Pass, exit 0; full build including VST3, tests not requested. Build log span 766.39 seconds. |
| `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Configuration Debug -Test` | Pass, exit 0; warm build about 4.98 seconds; **32/32 native CTest tests passed in 110.95 seconds**; complete invocation 124.46 seconds. |
| `pwsh -NoProfile -File scripts/build.ps1 -Configuration Release` | Pass, exit 0; full Release build including VST3, tests not requested; complete invocation 567.39 seconds. |
| `pwsh -NoProfile -File scripts/build.ps1 -Configuration Release -Test` | Pass, exit 0; warm build about 4.51 seconds; **32/32 native CTest tests passed in 37.46 seconds**; complete invocation 48.20 seconds. |

Debug logs: `out/build-logs/Debug/20261102-232457-ab1c5393/` (successful
cold build) and `out/build-logs/Debug/20261102-233758-ba5a2167/` (warm build and
CTest). The latter command used Windows PowerShell 5.1. The warm Debug and Release
invocations set the existing CMake environment variable
`CMAKE_BUILD_PARALLEL_LEVEL=4`; no compiler flags, presets or native test scheduling
were changed for parallelism. Full VST3 bundle:
`out/build/windows-msvc/preverb_plugin_artefacts/Debug/VST3/ReverbArise.vst3`.
Release build logs: `out/build-logs/Release/20261102-234009-e4659e03/`.
Warm Release build/test logs: `out/build-logs/Release/20261102-235003-7e69e0b6/`.
Release bundle: `out/build/windows-msvc/preverb_plugin_artefacts/Release/VST3/ReverbArise.vst3`.

## Reuse and exclusion checks

- The warm Debug invocation and subsequent Release builds preserved hashes and UTC last-write times for both
  dependency `.git/HEAD` files and both FetchContent `gitclone-lastrun` stamps.
  Snapshots: `out/dependency-reuse-before.json`, `out/dependency-reuse-after-debug.json`,
  `out/dependency-reuse-after-release.json`.
- While the real Debug build owned the tree, a separate Release wrapper invocation
  returned exit 1 with a busy-directory diagnostic and did not configure or build.
  Output: `out/native-lock-contention.log`. Independent-tree progress and lock
  release after failure were verified by fixtures on both shells.
- After Release validation, an absolute-path wrapper invocation with
  `-Configuration Debug` from the separate visualization workspace passed (exit 0)
  in **12.54 seconds**, with about **6.51 seconds** in the warm build. Logs:
  `out/build-logs/Debug/20261102-235110-3e911f39/`. All four dependency HEAD/stamp
  hashes and UTC timestamps were still unchanged after this configuration switch.
- The actual bundled CTest was also run with
  `--test-dir out/empty-ctest-validation --no-tests=error`: it reported no tests
  and returned **exit 8**. Output: `out/empty-ctest-validation.log`. This is a real
  CTest guard check against an empty directory, not a plugin test-suite pass.

Path-with-spaces and independent-tree concurrency coverage is fixture-only; no
second full native checkout/build was created solely for path testing.
FL Studio manual acceptance was not run and is not implied by CTest results.

## Issue #19 acceptance mapping

| Acceptance criterion | Evidence |
| --- | --- |
| Debug builds | Actual full MSVC Debug build passed, exit 0 |
| Release builds | Actual full MSVC Release build passed, exit 0 |
| Matching optional CTest | Actual Debug 32/32 and Release 32/32 passed; fixtures verify preset arguments and no-test behavior |
| Nonzero failures | Both-shell fixtures cover invalid configuration, missing tools, launch/configure/build/test failures; actual sandbox configure failure and busy-tree rejection exit 1; actual empty-suite CTest exits 8 |
| Reuse without cache deletion | Warm Debug/Release builds and configuration switch passed; source HEAD and population-stamp hashes/timestamps unchanged |
| Verified commands/prerequisites documented | Living development guide, AGENTS.md pointer and this environment/command record |
| No hosted CI or paid runner | Focused branch diff contains only presets, local PowerShell tooling/tests, docs and active OpenSpec artifacts; CMakeLists.txt and C++ sources unchanged |

Strict OpenSpec validation uses the installed CLI via its existing Node 26.13.0
runtime directly because the NVM shim has no active Node version. No runtime,
dependency, SDK or build-tool installation was performed for this change.
`openspec validate local-windows-build-validation --strict` and `git diff --check`
both passed. The OpenSpec change remains active; merge and archive require
separate owner approval.

## PR review refinements

Follow-up to the owner review of PR #26, based on `57b6c4a`:

- Existing caches now fail preflight when source directory, generator or platform
  identity is absent, as well as when it mismatches. Focused fixtures omit each
  entry individually and all three together; each verifies exit 1, an actionable
  diagnostic, no configure/build/test commands and unchanged cache contents.
- The living development guide now explains compiler-detection failures and
  corrupted empty compiler flags, with a link to the recorded manual recovery.
  It adds no automatic cache repair or normal-build compiler flag overrides.
- `pwsh -NoProfile -File tests/build_wrapper_test.ps1`: **65 fixture assertions
  passed** on PowerShell 7.6.9. Summary: `out/wrapper-review-ps7.log`.
- `powershell -NoProfile -ExecutionPolicy Bypass -File tests/build_wrapper_test.ps1`:
  **65 fixture assertions passed** on Windows PowerShell 5.1.26100.9807.
  Summary: `out/wrapper-review-ps51.log`.
- Strict OpenSpec validation and `git diff --check` passed again. Native MSVC
  builds and CTest were **not rerun for these preflight/documentation changes**;
  the earlier native 32/32 results above remain historical validation, separate
  from these fixture results. No dependency downloads or native rebuilds occurred.

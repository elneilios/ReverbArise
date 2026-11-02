# Issue #19 local Windows build validation — 2026-11-02

Implementation branch: `codex/19-local-windows-validation`, based on `77438b7`.
Commands below run with working changes present; summary revisions include `(dirty)`.

## Environment

Windows x64, VS Community 2026 18.11.12328.437, MSVC 19.51.36260.0
(tools 14.51.36231), Windows SDK 10.0.26100.0 and bundled CMake/CTest
4.3.1-msvc1. PowerShell 7.6.9 and Windows PowerShell 5.1.26100.9807.

## Fixture checks (not native plugin validation)

- `pwsh -NoProfile -File tests/build_wrapper_test.ps1`: 48 assertions passed.
- `powershell -NoProfile -ExecutionPolicy Bypass -File tests/build_wrapper_test.ps1`: 48 assertions passed on Windows PowerShell 5.1.
- Native .NET Framework command doubles exercise argument selection/order,
  default Debug and explicit Release, no-test reporting, invalid input, unavailable
  tools, incompatible PATH CMake fallback, failed configure/build/test, zero tests,
  missing revision (including stderr), logs, paths with spaces, caller location,
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

With the wrapper idle and its build-directory lock held, the targeted recovery was:

```powershell
& $cmake --preset windows-msvc -U 'CMAKE_C*_FLAGS*' -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
```

This restored normal MSVC defaults (`/DWIN32 /D_WINDOWS /EHsc`, Debug
`/Zi /Ob0 /Od /RTC1`, Release `/O2 /Ob2 /DNDEBUG`), preserving dependencies and
outputs. Recovery output: `out/cache-recovery.log`. No automatic cache repair or
compiler flag override was added to the wrapper. Results below remain pending.

| Command | Actual result |
| --- | --- |
| `pwsh -NoProfile -File scripts/build.ps1 -Configuration Debug` | Retrying after targeted cache recovery |
| `pwsh -NoProfile -File scripts/build.ps1 -Configuration Debug -Test` | Not yet run |
| `pwsh -NoProfile -File scripts/build.ps1 -Configuration Release` | Not yet run |
| `pwsh -NoProfile -File scripts/build.ps1 -Configuration Release -Test` | Not yet run |

Path-with-spaces and independent-tree concurrency coverage is fixture-only; no
second full native checkout/build was created solely for path testing.
FL Studio manual acceptance was not run and is not implied by CTest results.

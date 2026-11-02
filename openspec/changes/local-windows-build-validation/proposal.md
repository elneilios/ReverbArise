# Proposal

## Why

[Issue #19](https://github.com/elneilios/ReverbArise/issues/19) needs a repeatable local Windows build and PR validation command without hosted runner cost. Current presets cover Debug only and leave tool discovery, failure handling and evidence collection to the caller.

## What Changes

- Add `scripts/build.ps1` with `-Configuration Debug|Release` (default Debug) and optional `-Test`, usable from any working directory.
- Add Release build/test presets using the existing Windows x64 MSVC configure preset and shared build tree; retain Debug and existing CMake settings.
- Report stage failures with nonzero exit status and concise configuration, revision and test outcomes; retain diagnostic logs.
- Reuse incremental outputs and pinned dependency sources without default clean, cache deletion or forced dependency updates.
- Serialize wrapper operations per CMake binary directory; fail clearly on contention while independent worktrees remain concurrent (added by the implementation request).
- Update the living development guide and a short AGENTS.md pointer; record actual local validation separately from unavailable environment checks.

## Capabilities

### New Capabilities

- `local-windows-validation`: Developer-facing local build/test execution, failure reporting, incremental reuse and trustworthy validation evidence.

### Modified Capabilities

None. Existing capabilities concern plugin behavior; the new command introduces an observable developer workflow.

## Impact

Implementation will touch `CMakePresets.json`, a new PowerShell wrapper and focused wrapper checks, `docs/development/development.md`, AGENTS.md and dated evidence under `docs/validation/`. Preserve CMakeLists.txt, C++20, Visual Studio 18 2026 x64, dependency pins, plugin identifiers and audio behavior. No GitHub Actions, hosted CI, remote build service, paid runner, dependency migration, automatic plugin deployment or DAW acceptance automation. This change currently contains planning artifacts only.

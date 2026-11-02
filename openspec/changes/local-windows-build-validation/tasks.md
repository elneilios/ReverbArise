# Tasks

## 1. Release preset support

- [x] 1.1 Add `windows-msvc-release` build and test presets referencing `windows-msvc`, configuration Release and output-on-failure; verify CMake/CTest preset listing shows Debug and Release and a focused diff preserves the generator, architecture, binaryDir, dependency pins and Debug settings.
- [x] 1.2 Update the build section of `docs/development/development.md` with the Release preset commands and shared multi-configuration tree explanation; verify names match the preset JSON and clearly mark commands awaiting native verification.

## 2. Local command and focused checks

- [x] 2.1 Add `scripts/build.ps1` with default Debug, validated configuration, optional tests, script-relative root and literal/argument-array invocation; verify syntax plus isolated fixture cases for default/explicit configurations, paths containing spaces, arbitrary caller directories and directory restoration after success/failure.
- [x] 2.2 Implement compatible PATH/bundled CMake and adjacent CTest discovery plus Windows/Git/VS C++ prerequisite diagnostics; verify doubles cover missing and incompatible tools, bundled fallback, missing toolchain and nonzero exit without native compilation or downloads.
- [x] 2.3 Implement sequential in-place configure, incremental default build and optional matching CTest with zero-tests treated as error; preserve shared caches and avoid repeated dependency updates using supported update-disconnected controls while permitting first population. Verify fixture command arguments, Debug/Release selection, no-test behavior, no later stages after failure and incompatible-cache rejection without deletion. Prevent concurrent builds or configuration operations against the same CMake binary directory, using a lightweight lock or clear failure message. Independent worktrees with separate build directories must remain free to build concurrently.
- [x] 2.4 Add per-run/configuration logs, bounded failure output and final configuration/revision/dirty/stage summary; verify isolated configure/build/test failures (including zero tests) return nonzero, retain stdout/stderr and logs, preserve the native result through logging and never claim skipped tests passed. Verify unavailable revision is explicitly reported.
- [x] 2.5 Keep focused wrapper checks runnable without new packages; run them on PowerShell 5.1 and 7 where installed and record unavailable shell checks explicitly. Verify fixtures use temporary state and never modify real build caches or project source.
- [x] 2.6 Complete the living guide's wrapper prerequisites, invocations, diagnostic locations, exit behavior, cache reuse/recovery and local-versus-cloud guidance; add the short AGENTS.md pointer and docs/README.md link if needed. Verify documentation matches the implemented interface and no hosted CI or runner configuration is introduced.

## 3. Supported local integration evidence

- [x] 3.1 On the supported Windows x64 VS 2026 machine, run `./scripts/build.ps1 -Configuration Debug` then `./scripts/build.ps1 -Configuration Debug -Test`; verify the VST3 builds, the second build reuses state and the full Debug CTest suite reports its actual counts/result. Record environment/tool versions, commands, revision, exit codes and log paths in a dated `docs/validation/` report; if unavailable, record not run and leave native acceptance outstanding.
- [x] 3.2 Run `./scripts/build.ps1 -Configuration Release` then `./scripts/build.ps1 -Configuration Release -Test`; verify Release binaries and the full matching CTest suite, shared dependency checkout reuse and no forced clean/repopulation. Append actual counts, timings, outcomes and any existing test failures to the same report; flag unrelated failures instead of changing DSP/UI behavior or weakening tests.
- [x] 3.3 Verify a warm configuration switch and invocation from outside the repository using a checkout path containing spaces where available; record incremental build/dependency observations and native versus fixture coverage in the report. Avoid copying CMake caches between worktrees or creating a second full build solely for path testing; retain fixture-only coverage explicitly if needed.
- [x] 3.4 Reconcile the living guide with actually verified commands and evidence, clearly distinguish build-only success, CTest success, unavailable environments and unperformed FL Studio checks; verify every Issue #19 acceptance criterion maps to a recorded result or explicit outstanding check.
- [ ] 3.5 Run `openspec validate local-windows-build-validation --strict`, inspect the focused diff and prepare one owner-reviewed PR linked to #19 and this change; verify no Actions/hosted service, generated build outputs, plugin compatibility changes or unrelated edits are included. Do not merge.

## Workflow follow-up

- Apply only after a new implementation request; this proposal does not authorize implementation.
- Archive through the existing OpenSpec archive workflow only after implementation, validation and required owner review are complete.

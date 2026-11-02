# Design

## Context

See proposal.md for motivation and the local-windows-validation delta for the contract. CMakePresets.json schema v3 defines `windows-msvc`, generator `Visual Studio 18 2026`, architecture x64, `BUILD_TESTING=ON`, and `out/build/windows-msvc`. Only Debug build/test presets exist. CMakeLists.txt already registers native tests, pins JUCE 9.0.4 and WavPack 5.9.0, and builds the VST3. `tests/TestSupport.h` uses throwing REQUIRE checks that remain active in Release; the smoke target alone is not sufficient validation.

The living setup guide uses explicit Visual Studio bundled CMake/CTest paths, so PATH-only discovery would regress the documented environment. The preset generator requires a CMake version supporting VS 2026 even though the project/API minimum remains 3.22. No existing specification owns the developer command interface; plugin specs remain untouched.

## Goals / Non-Goals

**Goals:** One predictable entry point for local developers and agents, accurate stage outcomes and inexpensive reuse across Debug/Release. Support Windows PowerShell 5.1 and PowerShell 7 without adding a module dependency.

**Non-Goals:** Toolchain installation, alternate generators, dependency version changes, cross-checkout cache sharing, automatic cleanup, plugin installation into DAWs or changes to native test behavior.

## Decisions

1. **Keep one multi-configuration tree.** Add `windows-msvc-release` build and test presets mirroring Debug, referencing the existing configure preset. Do not add CMAKE_BUILD_TYPE or separate configuration trees. A separate Release tree would duplicate FetchContent downloads and generation. Build the normal default target set so plugin and registered tests exist before CTest; do not substitute a smoke-only target.

2. **Thin sequential wrapper.** Resolve root from `$PSScriptRoot`, validate parameters before native work, use literal paths and argument arrays with the call operator, and restore location in `finally`. Default Debug; map the validated configuration to the corresponding preset. Avoid expression-built commands and shell string evaluation. Run preflight, configure, build, then optional tests; check native exit codes immediately rather than relying on PowerShell exception settings alone. Preserve native nonzero codes where practical; use a stable nonzero wrapper code for preflight/errors. Parameter-binding failures can use PowerShell's own diagnostic and exit code rather than a custom summary.

3. **Discover tools without installing them.** Prefer a compatible CMake on PATH with adjacent CTest; fall back to the documented VS 2026 bundled tools, using installed Visual Studio discovery (vswhere if available) rather than requiring Community edition's path. Confirm Windows, Git, generator availability and a VS 2026 C++/SDK installation; use the configure result as final compiler/SDK authority. Print actionable diagnostics when unavailable. Do not require `cl.exe` on PATH because the Visual Studio generator discovers MSVC. Do not change machine environment, generator or installed workloads. Bare CMake 3.22 compatibility is insufficient: check that the selected CMake lists the required generator.

4. **Safe incremental configure.** Use the existing configure preset in place on each invocation so preset/source changes are respected; accept the small CMake configure cost rather than maintaining a fragile timestamp fingerprint. Preserve its tree and `_deps`; never use `--fresh`, `--clean-first`, deletion or forced repopulation by default. Existing pinned dependency checkouts should be reused, with configure-time FetchContent update checks suppressed for already populated sources using supported CMake update-disconnected controls; first population remains permitted. Do not force fully disconnected mode on an empty tree. Cache home/generator/architecture mismatch must fail with explicit recovery guidance. Do not copy caches between worktrees. CMake's ordinary incremental regeneration remains enabled.

5. **Matching, fail-closed validation.** Use the selected CTest preset, keeping output-on-failure and adding `--no-tests=error` to the wrapper invocation. Do not filter or parallelize the suite by default: timing/memory-sensitive DSP and UI tests should retain current scheduling. Missing executables, failed tests and zero discovery all fail. Build-only success must state tests were not requested; earlier failures leave later stages not run.

6. **Bounded console, retained logs.** Stream stage output to separate log files under ignored `out/` while displaying stage progress and a bounded failure tail. Preserve full native stdout/stderr, identify log paths, and keep CTest's normal Testing logs. Avoid swallowing exit status through logging pipelines. End valid invocations with configuration, short Git HEAD plus dirty indicator, each stage's pass/fail/not-run result and overall outcome. Revision lookup failure is explicit, never invented. Logs are per invocation/configuration so a later run does not erase failure evidence.

7. **Proportionate verification.** Use lightweight native-command doubles in an isolated temporary fixture to verify wrapper orchestration and failure paths without compiler runs, dependency downloads or edits to real project caches. No new Pester dependency is required. Cover both shells where installed. Then perform actual Debug and Release builds and matching full CTest once each on the supported local machine; repeat warm invocations to observe incremental reuse. Doubles establish wrapper behavior only and must not be presented as MSVC validation. Record actual generator/tool versions, commands, revision, counts, timings, logs and any unavailable checks in dated evidence. CTest success does not imply FL Studio acceptance.

8. **Exclude concurrent work per binary directory.** The apply request adds this
requirement. Hold an exclusive `FileShare.None`
handle on `.reverbarise-build.lock` in the resolved preset binary directory for all
mutating stages and CTest. Fail immediately on contention; release in `finally`.
Keep the file instead of deleting it to avoid unlink/recreate races. Separate trees
have separate handles and proceed independently. This is a cooperative wrapper
lock: raw CMake/MSBuild users must first ensure the tree is idle. Process termination
releases the handle; confirm any orphaned native process has stopped before retrying.
Fixtures test subprocess contention, cross-tree operation and failure release.

## Risks / Trade-offs

- [First configure may need network and first Release compilation may be substantial] → Reuse the same tree and pinned sources, perform cheap wrapper checks first, and avoid extra clean builds. No promise of an offline first build.
- [Reconfiguring may still perform dependency update work] → Apply update-disconnected behavior for populated sources and verify warm runs preserve dependency checkout state; keep first population supported.
- [Existing Release-only or machine-dependent tests may fail] → Report exact failures and leave unrelated DSP/UI fixes outside Issue #19; never weaken tests to produce a green summary.
- [PowerShell native error and logging behavior differs by version] → Explicit exit handling plus isolated failure injection and location-restoration checks on installed shells.
- [MSVC unavailable in a cloud agent or checkout cache tied to another source directory] → Mark native checks not run or cache incompatible, with local recovery instructions; do not install tools or erase caches automatically.

## Migration Plan

Add presets and the wrapper, validate its orchestration, then run the supported local matrix. Update `docs/development/development.md` and the AGENTS.md pointer only once commands exist; preserve dated prior evidence. Link the guide from docs/README.md if needed for discoverability. Submit one owner-reviewed PR linked to #19. Rollback removes the wrapper, Release presets and new documentation pointers; existing build/dependency data remains intact.

## Open Questions

None blocking planning. Exact local installed CMake/Visual Studio versions and available PowerShell shells are recorded during implementation validation; they do not change the chosen generator or workflow. At proposal time no native build or CTest run had been performed; actual implementation results are recorded in `docs/validation/2026-11-02-local-windows-build.md`.

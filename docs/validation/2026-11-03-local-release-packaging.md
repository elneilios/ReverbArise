# Issue #21 local release packaging evidence — 2026-11-03

## Scope and environment

Issue [#21](https://github.com/elneilios/ReverbArise/issues/21), OpenSpec change
`local-windows-release-packaging`. New isolated worktree from fetched `origin/main`
`b79e25cc06119b17357047dbfc7bc14c40a86d5f`, branch
`codex/21-local-release-packaging`. No plugin/DSP/host identity or storage code changed.

Local Windows x64, Visual Studio 18 2026 Community, MSVC 19.51.36260.0;
VS-bundled CMake/CTest. Windows PowerShell 5.1 and PowerShell 7 are installed.
The OpenSpec launcher needed the installed Node 26.13.0 directory prepended to PATH;
no tools were installed and generated workflow files/configuration were not changed.

## Simulated orchestration checks

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/release_wrapper_test.ps1
pwsh -NoProfile -File tests/release_wrapper_test.ps1
```

Windows PowerShell 5.1 and PowerShell 7: **39 checks passed in each**. The initial cross-version ZIP-path
fixture exposed Framework backslash entries; the implementation now writes explicit
forward-slash entries and loads both compression assemblies.

The fixtures use disposable real Git repositories and the real #20 CMake verifier,
but simulate Release builds, CTest/PE checks, dependency notices and all gh commands.
Coverage includes dirty tracked/untracked source, wrong/changed revisions and tags,
version/candidate rejection, build/CTest failure propagation, missing or empty
resources/notices, invalid built metadata, build-lock contention, stable/candidate
naming, complete extra-resource inclusion, checksums/manifests, non-overwrite, gate/
confirmation rejection, remote tag mismatch, annotated tags, candidate prerelease
arguments, ZIP tampering and public CLI nonzero failure exits. Publication messages
inside fixtures refer only to intercepted commands; no real gh publishing occurs.
Fixture output is ignored under `out/release-wrapper-fixtures/`.

## Native validation

A preparatory `scripts/build.ps1 -Configuration Release -Test` was started in the new
worktree to fetch pinned dependencies and compile the native targets. Native package
validation will be recorded below after running the release entry point from a clean
committed revision. Simulated checks above are not MSVC/CTest results.

## Outstanding owner acceptance

FL Studio installation/rescan, existing-project copy restore/save/reopen and listening
acceptance were **not run**. No DAW projects, captured source or caches were altered.
No redistribution/license eligibility review was approved. Packages remain internal
evaluation artifacts. No tag, GitHub release or binary publication is authorized by
this evidence. No merge, canonical-spec sync or OpenSpec archive was performed.

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

Actual supported local toolchain: VS 2026 Community, MSVC 19.51.36260.0,
CMake/CTest 4.3.1-msvc1. No native-command doubles were used in this section.

The preparatory `scripts/build.ps1 -Configuration Release -Test` fetched the pinned
dependencies and compiled all targets in the new isolated tree. It passed **35/35
CTest tests** in 38.80 seconds. Its log summary recorded the initial
`b79e25c (dirty)` checkout while implementation was being written, so it was not
used alone as release provenance. Logs:
`out/build-logs/Release/20261103-010023-b6ed69ad/`.

After committing implementation, the following real command ran from clean
`259c4f245db8952b785026f6091027b53623c8f1`:

```powershell
powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File scripts/release.ps1 -ReleaseTag v0.1.0 -Candidate rc.issue21.1
```

**Passed, exit 0.** The entry point reused incremental dependency/build state, ran
Release configure/build and **35/35 CTest tests** again (36.44 seconds), verified
built identity and the actual x64 PE/file version, and created the internal package.
Build/CTest logs: `out/build-logs/Release/20261103-012027-1ec41291/`.

| Field | Actual result |
| --- | --- |
| Numeric plugin version | 0.1.0 |
| Explicit release identity | v0.1.0-rc.issue21.1 |
| Artifact | ReverbArise-v0.1.0-rc.issue21.1-win-x64.zip |
| Location | out/releases/v0.1.0-rc.issue21.1/ |
| UTC manifest timestamp | 2026-11-03T01:21:23.7550191Z |
| ZIP size | 8,600,410 bytes |
| ZIP SHA-256 | 63a619c57b7126f395f0dbf7b231e62bb5ee7c2684d98695fcee599eb9377309 |
| Extracted payload files | 21, including complete VST3 resources and 14 additional notice/provenance files |

Independent PowerShell 7 verification matched the ZIP hash against both sidecars,
extracted it to ignored `out/release-validation/`, compared every path/size/hash
against the manifest, and checked the extracted PE/version. An initial whole-JSON
comparison flagged only differing Framework/Core file sort order; path-based
comparison confirmed identical contents. No production script change was needed.

Repeating the same release command returned **exit 1** before building because the
output already existed. The original ZIP SHA-256 remained unchanged. Requesting
`-ReleaseTag v0.2.0` returned **exit 1** for the version mismatch before building or
creating a completed package. The worktree remained clean after these commands.

The internal candidate above identifies the implementation commit exactly. Subsequent
commits only record evidence and workflow completion; this is not a package of a
later evidence-only revision. No stable release tag was created. Native Debug was
not rerun for this Release-only scripting change. The release publication path was
not invoked against the real checkout or GitHub; only fixtures exercised it.

## OpenSpec and review

`openspec validate local-windows-release-packaging --strict` passed; `git diff --check`
passed. Generated workflow tooling/configuration and canonical specs were unchanged.
Owner review: [PR #28](https://github.com/elneilios/ReverbArise/pull/28).

## Outstanding owner acceptance

FL Studio installation/rescan, existing-project copy restore/save/reopen and listening
acceptance were **not run**. No DAW projects, captured source or caches were altered.
No redistribution/license eligibility review was approved. Packages remain internal
evaluation artifacts. No tag, GitHub release or binary publication is authorized by
this evidence. No merge, canonical-spec sync or OpenSpec archive was performed.

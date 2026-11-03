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

The internal candidate above identifies the initial implementation commit exactly.
The initial follow-up commit `4e495cd` only records evidence and workflow completion;
the later integrated candidate is recorded separately below. No stable release tag was created. Native Debug was
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
this evidence. At this initial review checkpoint, no PR merge, canonical-spec sync
or OpenSpec archive had been performed. The owner-approved follow-up is below.

## Owner-approved integration follow-up — 2026-11-03

The positive owner review requested latest-main integration, a fresh genuine internal
package, publication-example/privacy refinements, then spec sync/archive. These
results supplement the initial evidence above; no earlier package was replaced.

Merged fetched `origin/main` at `29bd265c6c7565089c1f1d9aa86f1eb61403f40d`,
including merged [PR #24](https://github.com/elneilios/ReverbArise/pull/24), approved
static Inter integration and the documentation navigation cleanup. The local merge
commit is `bb06c1333f155acdfe31ef5b762f89f7fc380fba`.
Product `Source/`, CMakeLists.txt and resources match that main revision exactly;
this follow-up introduced no DSP, UI, saved-state or plugin-identity changes.

After committing the documentation changes, ran the actual entry point from clean
`446ea9af88ea910076f43747e8d6c2aba05ab442`:

```powershell
powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File scripts/release.ps1 -ReleaseTag v0.1.0 -Candidate rc.issue21.2
```

**Passed, exit 0:** real MSVC Release configure/build, **35/35 native CTest tests**
in 38.38 seconds, built product/class identity checks and actual x64 PE/file-version
checks. No build, CTest, font or packaging doubles were used. Logs:
`out/build-logs/Release/20261103-012802-08f6a153/`.
The passing UI test includes runtime loading checks for all four static Inter faces.

| Field | Integrated package result |
| --- | --- |
| Numeric plugin version | 0.1.0 |
| Explicit candidate | v0.1.0-rc.issue21.2 |
| Artifact | ReverbArise-v0.1.0-rc.issue21.2-win-x64.zip |
| Location | out/releases/v0.1.0-rc.issue21.2/ |
| UTC manifest timestamp | 2026-11-03T01:31:58.4064849Z |
| ZIP size | 8,960,337 bytes |
| ZIP SHA-256 | a28eff0ce022ca09501e287013b7a550d4c9d9c507c04aa5ffbea79339d16755 |
| Payload inventory | 21 files: complete current six-file VST3 bundle, 14 additional notices/provenance files and README |

Independent Python standard-library verification read the actual ZIP, matched its
SHA-256 against both sidecars, checked the manifest source/target/candidate fields,
and compared every entry's path, size and SHA-256. Every current built-bundle file
was present and byte-identical in the ZIP. Extraction and a separate PowerShell
x64 PE/product-version check also passed. Local verification script/result:
`out/verify-issue21-integration.py` and
`out/release-validation/rc.issue21.2/verification.json` (ignored local evidence).

The four complete source TTF byte streams were each found in the **packaged plugin
binary**, rather than inferred from CMake declarations or the presence of source
fonts alone:

| Embedded face | Bytes | Source SHA-256 | Packaged binary offset |
| --- | --- | --- | --- |
| Inter-Regular.ttf | 411640 | 40d692fce188e4471e2b3cba937be967878f631ad3ebbbdcd587687c7ebe0c82 | 14725088 |
| Inter-SemiBold.ttf | 419744 | 78a843fade9d4612a5567302fb595b56976eb5fcebf4fea5a5912d638bafcde3 | 15136736 |
| Inter-Bold.ttf | 420428 | 288316099b1e0a47a4716d159098005eef7c0066921f34e3200393dbdb01947f | 15556496 |
| Inter-Italic.ttf | 417388 | bbc051dd204b5019a1aa0bc0ae2aa8a05ab13e7a3f979fa357631dc7feb6833a | 15976928 |

Packaged `Contents/Resources/Inter-OFL.txt` matches the approved static-font
`resources/fonts/inter/OFL.txt` byte-for-byte. WavPack, IR licence/manifest, original
Hall provenance, JUCE licence/SPDX and every copied dependency notice also match
their source files; the generated FLAC notice matches its complete upstream header.
All 14 additional notice/provenance files were nonempty and covered by the inventory.

The publication example now uses an `& { ... }` block, reads identity/base tag/SHA
from the selected build manifest, checks HEAD and requires typed publication
confirmation. The example parsed successfully in PowerShell 5.1 and 7; **none of
its tag or publication commands were executed**. The guide explicitly states that
`LicenseReview` text may appear in public release notes and must exclude confidential
evidence, private links, credentials or sensitive review material.

After successful native/package verification, used the installed OpenSpec Archive
and Sync workflows with all four tracked tasks and all artifacts complete. Created
`openspec/specs/local-release-packaging/spec.md` with the delta's verbatim Purpose
and five requirement blocks under canonical Requirements; verified exact sync.
All **15 canonical specs** passed validation and the new spec passed strict
validation. Archived all five change files unchanged to
`openspec/changes/archive/2026-11-03-local-windows-release-packaging/`, preserving
`.openspec.yaml`; the active change list is empty.

The candidate identifies `446ea9a` exactly; subsequent commits record this evidence
and sync/archive metadata. No external publication, tags or GitHub Releases were
created. No PR merge was performed; [PR #28](https://github.com/elneilios/ReverbArise/pull/28)
remains for owner approval. FL Studio was not rerun, and redistribution/licensing
approval remains separate; this candidate is internal evaluation only. Prior static
Inter visual approval was reused; no additional owner UAT was requested.

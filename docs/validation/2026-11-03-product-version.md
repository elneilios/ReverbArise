# Issue #20 product version validation — 2026-11-03

Worktree: `issue-20-version-identity/k7ma-preverb`; branch
`codex/20-product-version-identity`, based on latest fetched origin/main
`4ff6195a41e260e5a1b250a4c9d980521b4ea742`.

## Design and compatibility review

The proposal/design were reviewed before implementation and passed strict OpenSpec
validation. Existing JUCE metadata already inherited 0.1.0; explicit VERSION wiring
retains that behavior. Manufacturer K7Ma, plugin code Prv1, bundle ID
com.k7ma.preverb and the existing VST3 component/controller CIDs are unchanged.
No Source/ serialization, DSP, UI, capture or asset files were edited. No packaging
or publication workflow is introduced; candidate labels are release metadata only.

## Compiler-free checks

`cmake -P tests/product_version_test.cmake`: **36 contract cases passed**. These use
isolated CMake declarations, generated-identity JSON and dummy VST3 files under
ignored out/. They exercise source reads, a manual numeric edit, component grammar
and bounds, tag mismatch/malformed tags, candidate labels, output preservation on
failure, stale/missing build metadata, wrong class versions/CIDs and bundle identity.
They are fixtures, not native DLL or MSVC build validation.

Documented source reads and matching `'-DRELEASE_TAG=v0.1.0'` JSON output passed.
PowerShell -D arguments must be quoted (unquoted dotted tokens can be split).
The actual JUCE manifest, including trailing commas, was also read successfully
using CMake's built-in JSON reader; no parser package was installed.

## Native validation

Windows x64, Visual Studio 2026 Community, bundled CMake/CTest 4.3.1-msvc1,
MSVC tools 14.51.36231, Windows SDK 10.0.26100.0. Native validation runs outside the
sandbox to avoid the MSBuild tracking permission failure observed during Issue #19.

This new worktree uses its own binary tree and normal MSVC defaults. While holding
the build-directory lock, a new untracked cache was seeded only with identity
entries and FETCHCONTENT_SOURCE_DIR_JUCE/WAVPACK overrides to clean existing local
pinned sources. No other worktree's cache, objects or binaries were copied; no
new dependency download was needed. Initial configure retained `/EHsc` and normal
configuration defaults. `CMAKE_BUILD_PARALLEL_LEVEL=4` is set for compilation.

`pwsh -NoProfile -File scripts/build.ps1 -Configuration Release -Test`: **passed,
exit 0; 35/35 CTest tests passed in 39.30 seconds**. This includes the actual
compiled JUCE definition assertion, compiler-free fixtures, built VST3 manifest
verification and the pre-existing persistence/processor tests. The wrapper started
at `4ff6195 (dirty)`; tested implementation is captured in commit `19b052e`.
Logs: `out/build-logs/Release/20261103-003135-f5126cb9/`.

## Actual binary and focused Debug results

- Built Release verification with RELEASE_TAG=v0.1.0 and BINARY_DIR/CONFIGURATION
  succeeded with tagVerified=true and builtVerified=true. Generated release JSON:
  `out/release-identity.json`.
- The same built check with CANDIDATE=rc.1 produced v0.1.0-rc.1 while retaining
  numeric version 0.1.0; DLL SHA-256 was unchanged:
  `7F160A1ECE55611F87C7ECA9875F734A936A13E75474AD004D485A2A4C9D02E0`.
- Windows FileVersion and ProductVersion both report **0.1.0**; numeric resource
  parts both report **0.1.0.0**. ProductName remains ReverbArise. Actual inspection:
  `out/native-product-version.json`.
- The VST3 check verified component CID `ABCDEF019182FAEB4B374D6150727631` and
  controller CID `ABCDEF011234ABCD4B374D6150727631`, with version 0.1.0 on both.
- With the full wrapper idle, the same Enter-BuildLock handle was held for
  `cmake --build --preset windows-msvc-debug --target preverb_product_version_test`
  and `ctest --preset windows-msvc-debug -R '^preverb_product_version_test$' --no-tests=error`.
  **1/1 actual native Debug assertion passed**, test time 0.06 seconds. Logs:
  `out/debug-product-version-build.log` and `out/debug-product-version-test.log`.
  This compiled only the small assertion target; a full Debug VST3 build/suite
  was deliberately not repeated. The supported full Release run used build.ps1.

## Acceptance and scope

| Issue #20 criterion | Verification |
| --- | --- |
| One authoritative numeric version | Root CMake remains 0.1.0; source-only query and changed-version fixtures pass |
| Built intended version and stable identity | Actual JUCE assertions, VST3 class checks and Windows resources pass |
| Mismatch stops release work | Negative tag cases fail nonzero; stale successful JSON is unchanged and must not be consumed after failure |
| Independent persisted schemas | No Source/ or schema code changes; existing restore tests pass in the full native suite |
| Documented manual procedure | docs/development/product-version.md, with source/tag/candidate/built commands executed |
| No automation/service/packaging creep | Scoped CMake/scripts/tests/docs/OpenSpec diff; no CI, version service or #21 packaging implementation |

Strict OpenSpec validation passed for all 14 items; diff and scope checks passed. PR #27 is open for owner review. The change
remains active; neither archive nor merge is authorized.
FL Studio manual save/reopen is not run and cannot be inferred from CTest. No
product version change, persisted-state migration or host identity change occurs.

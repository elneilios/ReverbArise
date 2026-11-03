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

`pwsh -NoProfile -File scripts/build.ps1 -Configuration Release -Test`: running.
Logs: `out/build-logs/Release/20261103-003135-f5126cb9/`.

Actual binary version inspection and focused Debug verification: pending.
FL Studio manual save/reopen is not run and cannot be inferred from CTest. No
product version change, persisted-state migration or host identity change occurs.

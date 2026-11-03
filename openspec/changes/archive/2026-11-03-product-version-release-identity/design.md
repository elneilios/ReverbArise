# Design

## Context

See proposal.md. Root CMake declares `project(Preverb VERSION 0.1.0 ...)`. JUCE 9.0.4 currently defaults JUCE_VERSION from PROJECT_VERSION, then derives JucePlugin_VersionString/VersionCode, resources and VST3 module metadata. Existing Release moduleinfo.json reports 0.1.0. Plugin and swell saved-state versions are 8; capture schema is 1. These storage contracts belong to their existing modules and must not depend on product versions.

## Goals / Non-Goals

**Goals:** Make version flow explicit, cheaply check tags, expose reusable release metadata, verify built metadata and preserve compatibility.

**Non-Goals:** Packaging (#21), installation, signing, publication, automatic bumps, tag creation, UI changes, storage migrations or third-party version tooling.

## Decisions

1. Keep the existing literal root project VERSION as the only source. A small CMake helper reads that narrowly defined declaration for script-mode checks; normal configure verifies the read value equals PROJECT_VERSION. Pass `VERSION ${PROJECT_VERSION}` explicitly to juce_add_plugin. Reject noncanonical three-component versions before FetchContent. Bound components to 0..255 to prevent overlapping fields in JUCE's packed numeric version. Do not add a second handwritten version file or derive versions from Git tags.
2. `cmake -P scripts/verify-version.cmake` uses existing CMake only. Optional `RELEASE_TAG` must equal `v<version>` exactly; optional `CANDIDATE` is a safe nonempty alphanumeric/dot/hyphen label such as rc.1. A candidate is metadata, never a VERSION suffix or plugin-name/code change. Optional OUTPUT_FILE emits JSON with numeric version, expected/requested tag, tagVerified, candidate label and release identity. Source-only reads explicitly mark tags unverified when none was requested. Future packaging must call this verifier before creating a release; no packaging is implemented here.
3. Optional BINARY_DIR and CONFIGURATION checks read the configure-generated version/identity JSON plus the built VST3 moduleinfo.json; compiled assertions separately verify the JUCE definitions. Reject stale source-versus-build version and identity mismatches, missing metadata and class version drift. Check the established component/controller CIDs from the existing Windows bundle and leave JUCE_VST3_CAN_REPLACE_VST2=0 untouched. A native assertion executable checks the actual compile definitions; built metadata check also inspects Windows binary product/file versions where feasible in PowerShell validation.
4. Generate build identity metadata with CMake's evaluated project version and target identity properties. It is a derived build artifact, not another authority. A script-mode verifier can run without MSVC, JUCE or downloads unless built verification is explicitly requested. Tests use isolated source/build fixtures to exercise changed versions, malformed input, stale builds, candidate labels and failure exit codes. Register focused CTest checks alongside the native definition assertion.
5. Native validation uses scripts/build.ps1 and the worktree's own binary tree. Reuse existing local dependency source directories via explicit FetchContent source overrides in an untracked local initial cache, never copy another worktree's CMakeCache or build outputs. Use normal MSVC defaults from the start and keep all operations under the build lock. Run a supported Release build/full CTest plus focused Debug validation if economical; document fixture versus actual native coverage and any omitted manual FL Studio acceptance.

## Compatibility review before implementation

- Identity is not a version: preserve K7Ma, Prv1, com.k7ma.preverb, product name, target names, processor/controller CIDs and VST2 replacement setting. No state serialization code will be edited.
- Keep schema numbers out of product metadata and version helper dependencies. Existing restoration tests remain applicable; no saved projects or caches are rewritten.
- Candidate suffixes apply to a release metadata field only. The exact stable tag grammar avoids confusing a prerelease filename with a new plugin identity.
- Built outputs can be stale after a bump: compare them to the current source authority and fail until rebuilt, rather than blessing old artifacts.

## Risks / Trade-offs

- [Small CMake declaration reader is not a general CMake parser] -> Require one literal, unambiguous root project VERSION declaration, document its form and test comments/duplicates/invalid numbers.
- [JUCE moduleinfo.json contains trailing commas] -> Use the existing CMake JSON reader, verified against the actual JUCE manifest and trailing-comma fixtures; do not add a new JSON dependency.
- [First isolated native build is expensive] -> Reuse only clean local dependency sources; keep cheap negative checks compiler-free and avoid rebuilding unrelated worktrees.
- [Real FL Studio reopen cannot be automated here] -> Preserve all identifiers/state code, run existing restoration tests and explicitly record manual host acceptance as not run.

## Migration Plan

Add the explicit version wiring and verifier, test the contract, then validate existing 0.1.0 native metadata. Document the owner-edited bump procedure. Review in one PR. Rollback removes only tooling/version wiring; no data migration is needed.

## Open Questions

None blocking. The exact #21 packaging command will be chosen in that issue; the version JSON/tag contract is sufficient without implementing it now.

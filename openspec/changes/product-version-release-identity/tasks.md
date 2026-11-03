# Tasks

## 1. Authoritative version and release verification

- [x] 1.1 Add a shared CMake version reader and explicit JUCE VERSION wiring with derived build identity metadata; verify the source stays 0.1.0 and stable identity declarations/schema code are unchanged.
- [x] 1.2 Add the cheap source/tag/candidate verifier and machine-readable output; add isolated script-mode tests for matching and malformed/mismatched tags, manual version edits, leading zeroes/ranges and invalid labels; verify no configure/dependency download is needed.
- [x] 1.3 Document manual bumps, metadata fields, candidate versus stable tag rules and independent schemas in living development guidance; execute documented source verification commands.

## 2. Built identity and compatibility verification

- [x] 2.1 Add compiled JUCE definition assertions and built-metadata verification with CTest registration; verify fixture failures for missing/stale metadata, incorrect class versions and changed identifiers.
- [ ] 2.2 Review the completed diff for identity/schema separation and run the supported native Release build/full CTest through scripts/build.ps1; inspect actual binary version and VST3 CIDs, record environment/results and any unperformed host checks in dated evidence.
- [ ] 2.3 Run feasible focused Debug/native checks without an unnecessary second full build, rerun strict OpenSpec validation and scope checks, then commit/push and open one PR linked to #20 and this change for owner review. Do not merge or archive.

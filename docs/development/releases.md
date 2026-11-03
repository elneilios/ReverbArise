# Local Windows releases

Use the [development prerequisites](development.md) and [product-version contract](product-version.md).
Windows PowerShell 5.1 and PowerShell 7 are supported. No extra PowerShell modules,
installer, signing service or hosted runner is required. Keep the release checkout
idle throughout validation. Commit all intended source changes first.

## Internal local package

```powershell
./scripts/release.ps1 -ReleaseTag v0.1.0
# Separate candidate label; the numeric plugin version remains 0.1.0:
./scripts/release.ps1 -ReleaseTag v0.1.0 -Candidate rc.1
```

The script can be invoked by absolute path from another directory. It requires a
clean Git worktree (tracked and untracked files), records full HEAD, and rejects an
existing identity tag pointing elsewhere. A local tag need not exist for an internal
package. It reuses `scripts/build.ps1 -Configuration Release -Test`, then verifies
#20's built version/class identities, the actual x64 PE/file version, required
resources and the source revision again. There is no dirty-tree or skip-test option.
Dependencies may need downloading on first configure. With populated dependencies,
package generation makes no GitHub requests and needs no gh login.

Outputs for a stable release are under `out/releases/v0.1.0/`:

- `ReverbArise-v0.1.0-win-x64.zip`
- `ReverbArise-v0.1.0-win-x64.zip.sha256`
- `build-manifest.json`
- `payload/`, the inspected package contents retained for local review

Candidates use `v0.1.0-rc.1` consistently in directory, ZIP and manifest. The manifest
records the numeric version, candidate, full Git SHA, clean state, UTC timestamp,
Windows x64/Release target, configured MSVC/CMake identity, ZIP hash and an inventory
of every payload file with its size and SHA-256. The manifest is a sidecar, avoiding
a circular ZIP checksum. Output is traceable, not guaranteed bit-identical across
compiler installations or build times. These are unsigned binaries.

The ZIP contains the **whole** `ReverbArise.vst3` bundle, including WavPack, Inter,
IR license/provenance and module metadata. `licenses/` adds the pinned JUCE license
and SPDX dependency inventory, VST3 SDK and codec/font/image notices, and original
Hall provenance. The SPDX inventory also lists JUCE components not used by this
Windows build; inclusion is not a claim that every listed component is linked.
The packager copies texts from the actual dependency checkout and fails if expected
notices are absent. Owner review must still establish all applicable obligations.

Existing final directories are never overwritten, even on concurrent invocations.
Use a new explicit candidate identity for another attempt. If a failed operation
leaves `out/releases/.staging-*`, preserve it for diagnostics or deliberately relocate
it while idle. It is not a completed release. No default clean, cache deletion or
package replacement occurs. Build logs remain under `out/build-logs/Release/`.
The wrapper's build lock is reacquired during verification/copying; another wrapper
owning that tree causes release validation to fail. Do not edit source, run raw
build tools or alter output files while releasing.

## Redistribution gate

A generated ZIP is **internal evaluation only**. No license review or distribution
permission is implied. Before sharing any binary externally, the owner must record:

- The chosen JUCE licensing path and evidence that the intended recipients/use and
  distribution model meet its terms. Review the pinned JUCE license and its linked
  terms, including any corresponding-source obligations of the chosen path.
- Rights to distribute the project's code/binary; this repository currently has no
  general project license grant. Resolve that before external distribution.
- Applicable third-party notices/obligations: WavPack, Inter OFL, VST3 SDK, JUCE's
  transitive dependencies and all bundled IRs, including original Hall provenance.
- Any required additional material or notices, plus manual host acceptance below.

The script records an owner's affirmative review; it does not determine legal
eligibility. Keep dated supporting evidence outside the binary output and reference
it in the publication review text. Local copying outside this workflow also needs
this review. No licensing review was approved by implementing or testing this script.

## Extraction and FL Studio acceptance

1. Verify `(Get-FileHash -Algorithm SHA256 <zip>).Hash` against both `.sha256` and
   `build-manifest.json`. Inspect the recorded commit and explicit candidate label.
2. Extract to a temporary directory. Preserve the complete `ReverbArise.vst3/`
   directory and its `Contents/`; do not extract only the inner `.vst3` binary.
3. Close the DAW. Preserve the installed bundle for rollback. Copy the whole bundle
   into your user VST3 folder, normally `%LOCALAPPDATA%\Programs\Common\VST3`, or the
   machine folder `%COMMONPROGRAMFILES%\VST3` if deliberately using administrator
   installation. Add the chosen folder to FL Studio's scan paths if necessary.
4. Rescan in FL Studio's Plugin Manager. Confirm ReverbArise appears once, loads,
   reports the intended numeric version and produces expected capture/render/playback.
5. Open a **copy** of an existing saved FL Studio project. Verify plugin recognition,
   saved settings, captured source and render/cache references, playback and save/
   reopen. Do not rename plugin IDs or delete captured/cache assets to resolve a scan.
6. For rollback, close FL Studio, restore the preserved bundle and rescan. Preserve
   original projects and assets. Automated CTest does not establish host acceptance.

## Optional manual GitHub publication

This is a separate action after owner review, licensing approval and host validation.
Package mode never creates tags or releases. The following commands are operational
instructions for an authorized owner; **do not run them during implementation/tests**.
Use the exact packaged full SHA from `build-manifest.json`, not a moving branch.

```powershell
# Deliberately create and push the exact identity tag only after review:
git tag -a v0.1.0-rc.1 <packaged-full-sha> -m 'ReverbArise v0.1.0-rc.1'
git push origin refs/tags/v0.1.0-rc.1

# From that same clean commit, after authenticated gh setup:
./scripts/release.ps1 -ReleaseTag v0.1.0 -Candidate rc.1 -Publish `
  -RedistributionReviewed `
  -LicenseReview 'Owner review record: <dated evidence covering JUCE, project and dependency rights>' `
  -ConfirmPublish 'v0.1.0-rc.1@<packaged-full-sha>'
```

Stable publication omits `-Candidate` and uses `vX.Y.Z@<sha>` as confirmation.
The exact confirmation is separate from creating the local package. The command
rechecks clean source, version, manifest, ZIP/checksum and payload inventory before
network access; it verifies the already-pushed remote tag, including annotated tags,
resolves to the packaged commit. It then uses authenticated `gh release create
--verify-tag`, uploads ZIP/checksum/manifest and marks candidates `--prerelease`.
It always uses `--latest=false`, leaving promotion to the owner. Existing releases
are not edited and assets are not clobbered. If upload fails, inspect GitHub's state
manually before retrying; gh may have created a release with partial uploads.

## Verification scope

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/release_wrapper_test.ps1
pwsh -NoProfile -File tests/release_wrapper_test.ps1
```

These fixtures use real disposable Git repositories and CMake version checks but
**simulate** the build, PE validation and GitHub commands. They download no modules,
produce dummy ZIPs under ignored `out/release-wrapper-fixtures/`, and never publish.
Actual native Release/CTest/package results and remaining manual checks belong in
[the dated issue #21 evidence](../validation/2026-11-03-local-release-packaging.md).

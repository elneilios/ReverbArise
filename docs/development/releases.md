# Local Windows releases

Use the [development prerequisites](development.md) and [product-version contract](product-version.md).
Windows PowerShell 5.1 and PowerShell 7 are supported. No extra PowerShell modules,
installer, signing service or hosted runner is required. Keep the release checkout
idle throughout validation. Commit all intended source changes first.

Before packaging, use [maintain-user-guide](../../.agents/skills/maintain-user-guide/SKILL.md)
and the [currency workflow](user-guide-maintenance.md). `scripts/check-user-guide.ps1`
must pass for the committed review. Packaging checks this read-only before
building and never invokes a skill or rewrites guide content/review records.
The generated Settings About identity must match the requested release version,
candidate and clean source SHA; stale/local metadata fails packaging.

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

The payload also includes `README.txt`, `User-Guide.html` and
`guide-provenance.json`. The guide is a static offline document: all four native
production-editor screenshots and the existing production wordmark are embedded PNG data URIs, with no scripts or
external resources. Its source template is [alpha-guide.html](alpha-guide.html).
After Release/CTest and built identity verification, the packager runs
`Release/preverb_learn_generation_ui_test.exe --guide-only` under the build lock
with a fresh `PREVERB_UI_M3_OUTPUT` staging folder. Real fixture capture,
generation, render and Export readiness are asserted before snapshots are saved.
Missing/invalid captures or mismatched provenance fail packaging. All delivered
guide files participate in the existing payload inventory and ZIP verification.

For #59, rehearse on the issue branch with a distinct label such as
`-Candidate alpha.rehearsal.1`. For #59 the owner-approved order is to package a clean committed PR revision
containing the required integrations with `-Candidate alpha`, sign off that exact
candidate, archive the signed-off change and merge, then verify clean merged main separately.
If that output identity already exists, preserve it and use a recorded fresh
`alpha.N`; never overwrite it. Extract into a new folder, verify hashes and view
the HTML locally offline, checking every image. The final extracted binary needs
its own consolidated FL Studio capture/generate/Perform/save-reopen smoke test, or an explicit owner waiver with the earlier trial and untested coverage recorded.
Earlier fixture or host runs do not establish that final acceptance. Record
clean-machine and second-host coverage explicitly, including unverified coverage.
See the [alpha preparation evidence](../validation/2026-11-04-issue-59-alpha.md)
and [pending redistribution review](../validation/2026-11-04-issue-59-redistribution-review.md).

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
it with a non-confidential summary in the publication review text. `-LicenseReview`
text may appear in public GitHub release notes: do not include confidential evidence,
private document links, credentials or other sensitive review material. Keep that
supporting evidence private. Local copying outside this workflow also needs
this review. No licensing review was approved by implementing or testing this script.

## Extraction and FL Studio acceptance

1. Verify `(Get-FileHash -Algorithm SHA256 <zip>).Hash` against both `.sha256` and
   `build-manifest.json`. Inspect the recorded commit and explicit candidate label.
2. Extract to a temporary directory. Preserve the complete `ReverbArise.vst3/`
   directory and its `Contents/`; do not extract only the inner `.vst3` binary.
3. Close the DAW. Preserve the installed bundle for rollback. Copy the whole bundle
   into the supported system folder `%PROGRAMFILES%\Common Files\VST3` with the
   required Windows permission. FL Studio does not discover VST3 from custom user
   search folders. For local testing use the repository's
   [backed-up system installation helper](fl-studio-test-install.md).
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
& {
    $manifest = Get-Content -LiteralPath 'out/releases/v0.1.0-rc.1/build-manifest.json' -Raw | ConvertFrom-Json
    $packagedSha = $manifest.gitSha
    $identity = $manifest.releaseIdentity
    $headSha = git rev-parse --verify 'HEAD^{commit}'
    if ($LASTEXITCODE -ne 0 -or $headSha -cne $packagedSha) { throw 'Check out the packaged commit before publication' }

    $confirmation = Read-Host "Type ${identity}@${packagedSha} to approve tag/release publication"
    if ($confirmation -cne "${identity}@${packagedSha}") { throw 'Publication not confirmed' }

    # Deliberately create and push the exact identity tag only after owner review:
    git tag -a $identity $packagedSha -m "ReverbArise $identity"
    if ($LASTEXITCODE -ne 0) { throw 'Tag creation failed; inspect existing tags before proceeding' }
    git push origin "refs/tags/$identity"
    if ($LASTEXITCODE -ne 0) { throw 'Tag push failed' }

    # Supply only a public-safe summary; keep confidential review evidence private.
    $publication = @{
        ReleaseTag = $manifest.releaseTag
        Publish = $true
        RedistributionReviewed = $true
        LicenseReview = 'Owner reviewed JUCE, project and third-party redistribution rights; public summary only'
        ConfirmPublish = $confirmation
    }
    if ($manifest.candidate) { $publication.Candidate = $manifest.candidate }
    ./scripts/release.ps1 @publication
}
```

For stable publication, select its manifest; the empty candidate field omits
`-Candidate`, and confirmation uses `vX.Y.Z@<sha>`.
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

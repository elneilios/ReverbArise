# Clean PR alpha sign-off — 5 November 2026

Owner explicitly selected clean PR sign-off before merge, followed by separate main verification. OpenSpec proposal/design/spec/tasks and developer release guidance now use that order; task 4.5 preserves post-merge verification before archive. No merge, archive, tag, release publication or external sharing has occurred.

## Exact acceptance build

Clean committed source **e2cef2b8c0dec50e9c6b6dab2e21351bbf61bd50**, PR #63 branch `codex/59-private-alpha`, includes main through PR #66 (`7fc2f08`). Main integration changed only documentation/canonical specs; the sole docs/README conflict retained both references. Explicit maintain-user-guide review covers the canonical sync and approved acceptance ordering; no musician-facing prose or production-code change was needed.

`scripts/release.ps1 -ReleaseTag v0.1.0 -Candidate alpha` passes on the warm existing MSVC tree: configure/build, **68/68 native Release tests**, **211.98 s**, built numeric/version/x64 PE/notices, generated identity and native guide evidence. Logs: `out/build-logs/Release/20261105-083534-c020bed7/`. Toolchain remains Visual Studio 18 2026 / MSVC 19.51.36260.0 / CMake 4.3.1-msvc1. This is the selected pre-merge acceptance candidate, not a main build or a relabelled rehearsal.

Output: `out/releases/v0.1.0-alpha/`; fresh extraction: `out/alpha-review/v0.1.0-alpha/`. ZIP SHA-256 matches manifest/checksum and `Assert-ReleaseArchive`. All **23** extracted files match path/length/hash with exact file count. All **five** embedded PNGs decode and match provenance dimensions/hashes; no unresolved template slots and About/Copy instructions present. Generated JSON reports version **0.1.0**, build **v0.1.0-alpha**, full source SHA above and unmodified source. Provenance agrees. Tracked worktree remained clean throughout packaging and installation; packaging did not modify guide/review content.

- ZIP SHA-256: `2d268b8bdbfaf43c5568551b0108234c6f430476dcf68d889985d28c896845ad`.
- Module SHA-256: `0ca3890233fcd6ff764eb4152b5bbf14ed5f9aba3167248caf327a1d67f4a4cb`.
- [Manifest](2026-11-05-issue-59-alpha-manifest.json), [guide provenance](2026-11-05-issue-59-alpha-guide-provenance.json), [installation receipt](2026-11-05-issue-59-alpha-installation.json).

UI standards pass: **50 files / 61 unchanged bounded occurrences**. Changed alpha spec passes strict validation. Repository-wide normal OpenSpec validation **19/19**, with eight pre-existing description-length warnings introduced by main PR #66; strict **15/19** fails those four canonical specs for those warnings only, as already documented in the [housekeeping audit](2026-11-05-openspec-housekeeping-audit.md). No unrelated canonical-spec rewrite or weakened validation. Guide currency and diff whitespace checks pass. Previous [readiness fixtures](2026-11-05-issue-59-release-readiness.md) retain their stated scope; no redundant fixture rerun or native timing speedup is claimed.

## Installed package

With FL Studio closed, the repository `install-test-vst3` helper ran through Windows UAC: `scripts/install-test-vst3.ps1 -ReleaseTag v0.1.0 -Candidate alpha`. Exit **0**, warm plugin-only Release build passes; no repeated CTest during installation after the package's full pass. Build logs: `out/build-logs/Release/20261105-084046-3ebfcc0b/`. Elevated launcher/output retained under `out/installation-evidence/alpha/`.

Installed at **C:\Program Files\Common Files\VST3\ReverbArise.vst3**. Every installed file matches the extracted acceptance package. Prior system bundle retained at `out/vst3-install-backups/20261105-084053-6e7e199cbd694ec8a1c69fd6b3a10958/ReverbArise.vst3`; its complete inventory matches the before-install system inventory. Actual user-folder copy is unchanged. No host was terminated; no project/source/cache assets were modified. Later evidence/task commits do not change this installed build's identity.

## Owner acceptance pending

- Task 4.2: open the extracted `User-Guide.html` locally with network unavailable; confirm readable text, logo and all four workflow images. Automated inventory/embedded-resource checks are complete; final browser rendering remains owner inspection because local-file browser automation was blocked.
- Task 4.3: FL Studio Verify plugins + Rescan previously verified plugins + Find installed plugins, confirm system path and Settings About `v0.1.0-alpha` / `e2cef2b...`. On a copied project: capture during DAW playback → Stop → Generate Swells → ready Perform Audition/Stop → DAW playback/render routing → save/reopen copy and confirm state/audio. Record FL Studio version, sample rate, buffer and pass/issues; do not repeat automated edge cases. Clean-machine, second-DAW and physical accessibility/DPI coverage remain unverified.
- Task 4.4: explicit owner confirmation of [redistribution review](2026-11-04-issue-59-redistribution-review.md), Starter eligibility/entitlement, project rights, notice/material obligations and intended private recipients/use, tied to this exact package and host result. Free-tier preference alone is not that approval. Candidate stays internal.
- Task 4.5: after the owner-authorized merge, verify clean main separately before archive. No current main acceptance is claimed.

Installation is complete; owner host acceptance and final offline guide inspection are not inferred from installation, native tests or earlier rehearsals.

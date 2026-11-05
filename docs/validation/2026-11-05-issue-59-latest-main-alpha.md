# Latest-main alpha repackaging and local installation

Owner request: include the new main bug-fix PRs and install the alpha locally.

## Source and route

Main refreshed to `cbe5e0f` (PR #65 Learn capture diagnostics, PR #64 FL Studio test installer, PR #62 manual swell preparation). PR #63's guide work is still unmerged. Main is merged into the existing `codex/59-private-alpha` branch without rebasing or self-merging the PR. The only conflict, docs/README.md, retains both references. This combined-branch candidate is an internal latest-main integration build, not completion of the clean-main acceptance task 4.1.

The existing `scripts/release.ps1` pipeline will produce a fresh immutable `alpha.rehearsal.3` candidate with current native screenshots, final tagline/optional Export wording and corrected FL Studio system-folder instructions. Prior candidates remain preserved. Tests, exact source, artifacts and installed identities are recorded below as they complete.

## Installation plan and scope

Use the newly merged install-test-vst3 skill and `scripts/install-test-vst3.ps1`. The system target is `C:\Program Files\Common Files\VST3\ReverbArise.vst3`; its previous bundle is retained under the helper's `out/vst3-install-backups/` outside the scan folder. The prior user-folder copy is left untouched. Initial read-only inspection finds both copies present and no running FL Studio/known DAW process. Hosts are checked again immediately before installation. No host is terminated, no project/cache assets are changed, and no licence approval or external sharing is inferred from local installation.

After the helper's required incremental Release build/install, compare every installed file against the extracted package. Any mismatch prevents claiming the packaged alpha is installed. Installation is separate from scan/load and the final FL Studio capture/generation/Perform/save-reopen smoke.

## Package and integration verification

- Clean packaged source **`7ea715ba08bef612d7389c19ccd90764d91bd02c`**, includes main through **`cbe5e0f`** plus PR #63's final guide. `release.ps1 -ReleaseTag v0.1.0 -Candidate alpha.rehearsal.3` passes: native Release **66/66**, **232.15 s**. Configure/build reuse this worktree's existing tree; new main test targets compile as needed. Logs: `out/build-logs/Release/20261105-011941-2e6312f4/`.
- UI standards **50 files / 61 unchanged bounded occurrences**; strict OpenSpec **23/23 items**; diff checks pass.
- ZIP SHA-256 **`3c7cde9de159d127c810f37302075e2779724d1388a7936e7e7abccee6775577`**; module SHA-256 **`150e5fabcedcda49cd5dce9af25c48b53dc83fabe7566188cbec908120c3bd1f`**. Output: `out/releases/v0.1.0-alpha.rehearsal.3/`; fresh extraction: `out/alpha-review/v0.1.0-alpha.rehearsal.3/`.
- All **23** payload files match manifest path/size/hash; ZIP hash matches manifest and checksum. All five embedded images decode and match provenance, dimensions and hashes. Guide includes the production logo, requested tagline, optional Export/DAW rendering guidance and current system-folder installation instructions. Offline browser rendering of this integrated revision remains untested; earlier owner acceptance retains its recorded scope.
- Installer fixtures initially failed because their fake checkout omitted `release-guide.ps1`, now imported by release-support. Fixture-only follow-up includes that dependency in its copied scripts. Both Windows PowerShell 5.1 and PowerShell 7 then pass first install, exact replacement, full backup, lock, missing source, verification failure, rollback, failed-build propagation and host-guard cases. No production installer/build/audio change is made by this follow-up, and it does not relabel the packaged source or require rebuilding the identical runtime. The fixtures are separate from native CTest and host acceptance.

## Installed alpha

- `scripts/install-test-vst3.ps1` successfully built Release VST3 on current checkout `2ac68c9` (only fixture/evidence changes after packaged source). Its first non-administrator attempt failed opening the system install lock before any replacement. The documented Windows elevation retry succeeded with exit **0**; no ACL was changed. The hidden elevated launcher and full output/exit receipt remain under `out/installation-evidence/alpha.rehearsal.3/`.
- Installed complete bundle: **`C:\Program Files\Common Files\VST3\ReverbArise.vst3`**. Installer build logs: `out/build-logs/Release/20261105-013113-9c39ef9e/`. Plugin-only Release build passes; CTest is deliberately not repeated during installation because the package's matching **66/66** run already completed.
- All installed files match the extracted package inventory/hash, including module SHA-256 **`150e5fabcedcda49cd5dce9af25c48b53dc83fabe7566188cbec908120c3bd1f`**. This establishes that the installed runtime is the packaged alpha despite the subsequent fixture-only source commit.
- Prior system bundle preserved intact at **`out/vst3-install-backups/20261105-013120-c0325961adae4d7f94d67eeab70c143a/ReverbArise.vst3`**. Its complete inventory was compared to the before-install record and matches. Preserve this directory before removing the worktree/out tree if rollback may be needed.
- The existing `%LOCALAPPDATA%\Programs\Common\VST3\ReverbArise.vst3` inventory is unchanged. No captured/cache assets or DAW projects were touched; no host process was stopped. [Installation receipt](2026-11-05-issue-59-rehearsal-3-installation.json) records successful verification at **2026-11-05T01:31:50.5359846Z**.
- Installation is complete; FL Studio discovery/load and the combined-binary host smoke are **not performed**. Owner next step: reopen FL Studio, Options → Manage plugins; enable **Verify plugins** and **Rescan previously verified plugins**, then **Find installed plugins**. Confirm ReverbArise resolves to the system path above. The consolidated host smoke remains capture → Generate → Perform audition/Stop and DAW playback → project-copy save/reopen. Earlier host acceptance is not relabelled as this binary's result.

Task progress remains **8/12**: the latest-main integrated candidate and requested local installation do not waive the issue's final clean-main/owner-review, offline final-guide, host and redistribution gates. No tag, public release, external binary sharing or self-merge occurred.

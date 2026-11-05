# Latest-main alpha repackaging and local installation

Owner request: include the new main bug-fix PRs and install the alpha locally.

## Source and route

Main refreshed to `cbe5e0f` (PR #65 Learn capture diagnostics, PR #64 FL Studio test installer, PR #62 manual swell preparation). PR #63's guide work is still unmerged. Main is merged into the existing `codex/59-private-alpha` branch without rebasing or self-merging the PR. The only conflict, docs/README.md, retains both references. This combined-branch candidate is an internal latest-main integration build, not completion of the clean-main acceptance task 4.1.

The existing `scripts/release.ps1` pipeline will produce a fresh immutable `alpha.rehearsal.3` candidate with current native screenshots, final tagline/optional Export wording and corrected FL Studio system-folder instructions. Prior candidates remain preserved. Tests, exact source, artifacts and installed identities are recorded below as they complete.

## Installation plan and scope

Use the newly merged install-test-vst3 skill and `scripts/install-test-vst3.ps1`. The system target is `C:\Program Files\Common Files\VST3\ReverbArise.vst3`; its previous bundle is retained under the helper's `out/vst3-install-backups/` outside the scan folder. The prior user-folder copy is left untouched. Initial read-only inspection finds both copies present and no running FL Studio/known DAW process. Hosts are checked again immediately before installation. No host is terminated, no project/cache assets are changed, and no licence approval or external sharing is inferred from local installation.

After the helper's required incremental Release build/install, compare every installed file against the extracted package. Any mismatch prevents claiming the packaged alpha is installed. Installation is separate from scan/load and the final FL Studio capture/generation/Perform/save-reopen smoke.

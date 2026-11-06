# Completed-work cleanup

Run automatically as part of completed delivery, within existing filesystem permissions. This is an agent completion step, not a background scheduler or permission to sweep all worktrees.

## Decide what is complete

- Clean only generated output owned by this task in its current checkout or an explicitly task-owned temporary workspace.
- Finish required validation, packaging, installation and delivery first. Preserve output needed for known follow-up builds, debugging, owner UAT, or a preview/review window. A planning stop, approval pause, failed check or partial implementation is not completion.
- An open PR alone does not require keeping compiler intermediates once delivery is complete. If work resumes after cleanup, rebuild normally and report the cold build honestly.

## Preserve work and evidence

- Inspect Git status, including ignored output, and classify candidate paths before deletion. Never use blanket `git clean`, delete all of `out/`, or infer disposability solely from `.gitignore` or file age.
- Preserve tracked files, staged/unstaged edits and non-ignored untracked files. Preserve release ZIPs and their manifests/hashes, release notes, installation/rollback bundles, screenshots, listening samples, logs and validation reports, including ignored evidence under `out/`.
- Preserve captured audio, IRs, rendered assets, saved DAW projects and other durable user data. Distinguish application asset caches from compiler caches; application caches are not eligible.
- If needed evidence or deliverables live inside a candidate build subtree, retain that subtree or copy them to a stable retained location first, verify the copy, and update affected references. Keep dated evidence in `docs/validation/` and releases in their established location. Do not silently break evidence links.

## Remove only verified generated output

1. Identify explicit regenerable compiler/CMake output paths, normally below `out/build/`, using the build configuration/cache and contents. Confirm no tracked or locally modified source, embedded repository, durable data or retained artifact lies in each deletion target. Retain uncertain paths and explain why.
2. Check that no build, test, packager, installer, preview or review process uses the target. For each affected build directory, acquire and hold the same exclusive `.reverbarise-build.lock` used by `scripts/build.ps1` through `Enter-BuildLock` in `scripts/build-support.ps1`. If busy or inaccessible, skip it and report the reason; never stop processes or delete a lock to force cleanup. An unlocked file's existence is not evidence of an active build.
3. With the lock held, delete only verified generated children of the build directory; keep the directory and its lock file intact. Do not remove a parent that contains a held lock. Preserve mixed-content children rather than deleting them wholesale.
4. On Windows, resolve each absolute deletion target, verify it remains beneath the intended task-owned output directory, reject reparse points/junctions in the target or its parent path, and use native PowerShell `Remove-Item -LiteralPath`. Do not pass enumerated paths to another shell. Release locks in `finally` and check deletion errors.
5. Verify Git status matches the pre-cleanup state and protected artifacts still exist. Record removed paths and measured bytes reclaimed; disclose partial failures or skipped output. Do not claim a drive free-space change is solely from cleanup when other chats are building concurrently.

Do not remove checkouts, branches or Git refs, archive chats, or change app retention settings as part of this step. Whole-worktree retirement is separate: when explicitly requested, prefer the app's recoverable archive mechanism where available and preserve needed ignored artifacts first. Never operate on another chat's worktree merely because it appears idle.

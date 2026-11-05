---
name: install-test-vst3
description: Build ReverbArise Release and install the complete VST3 bundle into the owner's user VST3 folder for local FL Studio testing. Use for host-test build delivery, not release publication.
---

Use `scripts/install-test-vst3.ps1` from the checkout being tested. Read
[installation guidance](../../../docs/development/fl-studio-test-install.md) when
changing destinations or diagnosing installation failures.

- Default destination is `%USERPROFILE%\AppData\Local\Programs\Common\VST3`.
  Preserve that real user path when sandbox `LOCALAPPDATA` is redirected.
- Run the script, using the default plugin-only build for ordinary host testing.
  Add `-Test` only for explicitly requested or required full Release CTest; it builds all test executables and can be slow in a fresh worktree. The default always builds Release through the supported wrapper;
  do not substitute manual copy commands or stale binaries from another checkout.
- Use normal filesystem escalation for the user VST3 destination when outside the
  sandbox. The user's request to install authorizes that operation; do not ask them
  to manually copy the bundle. Do not silently change to a sandbox destination.
- FL Studio must be closed. If running, ask the owner to save/close it and retry
  once closed; never terminate it. Keep other hosts using the plugin closed too.
- On failure, inspect the reported error/log and preserve any printed backup.
  Stop on permission, loaded-file or rollback failures until the cause is resolved;
  do not delete installations, kill hosts or clear build caches to force success.
- Report installed path, Release build result, whether CTest ran, and retained
  backup path when present. Installation is not manual host acceptance. If FL has
  a system-folder copy too, point out that its resolved plugin path must be the
  newly installed user copy; do not remove other installations unrequested.

This helper is for local testing and accepts dirty worktrees. It neither creates a
release package nor publishes a release. Existing issue delivery and UI approval
checkpoints still apply to the underlying changes.

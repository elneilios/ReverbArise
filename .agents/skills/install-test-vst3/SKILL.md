---
name: install-test-vst3
description: Build ReverbArise Release and install the complete VST3 bundle into the system VST3 folder for local FL Studio testing. Use for host-test build delivery, not release publication.
---

Use `scripts/install-test-vst3.ps1` from the checkout being tested. Read
[installation guidance](../../../docs/development/fl-studio-test-install.md) when
changing destinations or diagnosing installation failures.

- Default destination is `%ProgramFiles%\Common Files\VST3`. FL Studio scans VST3
  only in supported system locations. Adding a custom user folder as a VST search
  path does not make it scan VST3 there. Do not install into user/sandbox folders
  for FL Studio testing.
- Use the default plugin-only build for owner DAW checks. The contribution
  workflow defers automated tests until the owner approves marking the draft
  PR ready. The full Release suite runs during release packaging. Do not
  substitute stale binaries from another checkout.
- Use normal filesystem escalation for the system VST3 destination. The user's
  request to install authorizes that operation; do not ask them to copy files.
  Sandbox escalation does not grant Windows administrator rights. On an ACL
  denial, rerun the script with Windows elevation using `Start-Process -Verb RunAs
  -WindowStyle Hidden -PassThru -Wait` and an absolute quoted `-File` path. Tell
  the owner to expect UAC. Preserve child output/exit status through a launcher
  under ignored `out` if needed. Never change system-folder ACLs to force access.
- FL Studio must be closed. If running, ask the owner to save/close it and retry
  once closed; never terminate it. Keep other hosts using the plugin closed too.
- On failure, inspect the error/log and preserve any printed backup. Resolve
  permission/loaded-file/rollback failures before retrying; do not delete
  installations, kill hosts or clear caches to force success.
- Report installed path, Release build result, whether CTest ran, and the retained
  backup path. Installation is not manual host acceptance. Instruct the owner to
  scan with Verify plugins and Rescan previously verified plugins, then confirm
  the resolved system bundle path. Leave other installations untouched.

This helper is for local testing and accepts dirty worktrees. It neither creates a
release package nor publishes a release. Existing issue delivery and UI approval
checkpoints still apply to the underlying changes.

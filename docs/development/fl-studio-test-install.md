# Install a Release build for FL Studio testing

Close FL Studio, then run from the repository root:

```powershell
./scripts/install-test-vst3.ps1
```

By default this incrementally builds only the **Release VST3 target** through the
existing build wrapper, excluding unrelated test/preview executables, and installs
the complete `ReverbArise.vst3` bundle into
`%USERPROFILE%\AppData\Local\Programs\Common\VST3`. This is the ordinary user VST3
folder; the profile-based spelling avoids redirected `LOCALAPPDATA` in agent
sandboxes. An agent needs filesystem authorization for this real user folder;
request the normal sandbox escalation when required. Never report a sandbox copy
as a host installation.

```powershell
./scripts/install-test-vst3.ps1 -Test      # Full Release CTest before installation
./scripts/install-test-vst3.ps1 -WhatIf    # Show target; no build or install
./scripts/install-test-vst3.ps1 -Destination 'D:\Audio\VST3'
```

`-Destination` names the containing VST3 folder, not the bundle itself. Without
`-Test`, CTest is not run. `-Test` opts into the full solution build and Release
CTest; a fresh worktree may take substantially longer because its test executables
also compile JUCE. Use the default for ordinary host-test delivery and request
`-Test` when full automated validation is needed. A build/test failure stops before installation; there is
no skip-build option that could silently deliver stale output. Dirty worktrees are
allowed for local testing. This is independent of tagged release packaging and
does not publish or redistribute anything.

The installer takes the existing build-directory lock while reading the output,
checks the required resources, x64 binary and version, then stages a full copy and
compares every file's SHA256. It serializes installs to the same destination.
The prior bundle is moved intact into
`out/vst3-install-backups/<timestamp-guid>/ReverbArise.vst3`, outside FL's scan folder.
It then renames staging into place, restoring the backup if that rename fails.
Backups are retained; no project, capture cache or other plugin is modified.
Replacing an existing bundle requires the backup directory and destination to be
on the same volume; a cross-volume move fails before replacement. Loaded files or
permission failures are reported without killing any host. Close other DAWs using
this plugin too, and keep hosts closed throughout installation.

Reopen FL Studio. In **Options > Manage plugins**, ensure the user VST3 folder is
searched and run **Find installed plugins** when needed. Check the resolved plugin
path: an existing copy in `C:\Program Files\Common Files\VST3` is left untouched and
may otherwise be selected instead. The script reports the installed path and binary
hash. A successful install does not establish FL Studio acceptance.

To restore a retained backup, close the host, move the current installed bundle
aside, and move the printed backup bundle back to the same target. Preserve any
backup you need before deleting this worktree's ignored `out` directory.

Agent entrypoint: [install-test-vst3 skill](../../.agents/skills/install-test-vst3/SKILL.md).
Dependency-free installer fixtures (separate from MSVC/CTest and host testing):

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_install_vst3_test.ps1
pwsh -NoProfile -File tests/test_install_vst3_test.ps1
```

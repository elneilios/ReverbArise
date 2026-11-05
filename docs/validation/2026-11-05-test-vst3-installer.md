# Local VST3 test installer validation — 2026-11-05

Base revision: `1f47acf`; branch: `codex/fl-studio-test-install`.
Environment: Windows, Windows PowerShell 5.1 and PowerShell 7. No downloads,
native compilation, cache clearing or writes to the real VST3 folders were needed.

Passed:

- `powershell -NoProfile -ExecutionPolicy Bypass -File tests/test_install_vst3_test.ps1`
- `pwsh -NoProfile -File tests/test_install_vst3_test.ps1`
- `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/install-test-vst3.ps1 -WhatIf`
- Skill Creator `quick_validate.py .agents/skills/install-test-vst3`
- `git diff --check`

Fixtures prove first install, full-bundle replacement without stale files, exact
retention of the previous bundle, destination locking, missing-source rejection,
staging verification failure preserving the current installation, injected final
rename failure restoring the previous installation, failed Release build/CTest
exit-code propagation without installation, and running-FL-Studio rejection.
They use ordinary temporary files under ignored `out/test-install-fixtures` and
double the host check, build command and rename failure where needed.

The dry run resolved the real user's VST3 folder rather than sandbox-redirected
`LOCALAPPDATA`. No build or installation occurred during that dry run.

Native MSVC Release build and CTest: **not run** for this scripting change.
Real installed x64 bundle, real loaded-DLL errors, FL Studio scan/load, host/DPI
coverage and actual user-folder deployment: **not tested**. The existing production
bundle/version validator is reused; these fixtures do not establish native audio
or host acceptance. No timing or build-speed improvement is claimed.

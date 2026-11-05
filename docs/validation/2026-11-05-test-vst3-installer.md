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

## First real installation and plugin-only correction

On the same date, the owner requested a real installation. `origin/main` was
checked at `1f47acf`; the checkout added installer commit `91d0f5f`.
The host guard correctly refused the first attempt while FL Studio was running.
After the owner closed it, a cold `install-test-vst3.ps1 -Test` configured and
compiled the plugin, then continued building many test executables. The owner
reported the wait was too long. Only our installer/build process tree was stopped;
its CMake/MSBuild/compiler children were confirmed stopped and caches retained.
That full build was interrupted; native CTest did **not** run.

The helper now defaults to the wrapper's explicit `-PluginOnly` path, which builds
`preverb_plugin_VST3` and dependencies. `-Test` remains an opt-in full build/CTest.
Conflicting wrapper `-PluginOnly -Test` options fail before native work. Wrapper
fixtures cover target selection, absence of CTest, and that conflict.

`powershell -NoProfile -ExecutionPolicy Bypass -File scripts/install-test-vst3.ps1`
then **passed native Release configure/build and actual user-folder installation**
using the warmed cache. Logs: `out/build-logs/Release/20261105-005530-cccd1fb4`.
Installed: `C:\Users\neila\AppData\Local\Programs\Common\VST3\ReverbArise.vst3`.
Binary SHA256: `C5F07FA956775A3DE7BD33C7990AE2CE543C19EF6724045F33A2A4F8D18C858E`.
No prior user-folder bundle existed, so no backup was needed. The system-folder
copy was untouched. The code identity was `91d0f5f` with script/docs edits; native
plugin sources were unchanged. This warm targeted run is not a comparable cold
build benchmark. FL Studio scan/load and manual acceptance remain untested.

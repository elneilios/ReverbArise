# Persistent Perform playback warning — 5 November 2026

**Result:** implemented the presentation-only Perform fallback for [Refs #1](https://github.com/elneilios/ReverbArise/issues/1), with owner-approved copy/layout and notice replacement/restoration. [PR #78](https://github.com/elneilios/ReverbArise/pull/78) is separate from the completed investigation in [PR #74](https://github.com/elneilios/ReverbArise/pull/74). No automatic suppression or defect-fixed claim; issue #1 remains open.

## Delivered behaviour and scope

The existing reserved header NoticeBar shows a warning whenever Perform has no other applicable notice:

> Caution: In Perform, swells can remain audible during DAW playback, even if the source is silent or muted.

Full help/accessibility detail:

> Perform plays prepared swells at saved Song positions, including before notes and through silence. Pattern playback or muting a Playlist track or Channel Rack instrument may not stop them. Bypass the effect or mute its mixer track to stop these swells.

All existing selected notices, including information-level debounced IR preparation, take precedence. The caution returns on clearance. It has no timeout/dismiss preference and remains general guidance even with empty data or playback disabled. Learn and Export retain their existing notice policies. Same-message refresh reuses the existing accessibility deduplication; the bar never moves controls or takes focus.

Production source diff is confined to `Source/UI/WorkspaceNotice.h`, a message-thread presentation selector. NoticeBar rendering, typography, warning roles, layout and engine interfaces are unchanged. No audio callback, transport, capture, routing, parameter, persistent schema, source asset or render change. Mixer mute affects the whole path; bypass can restore dry. The warning does not detect host mode/source mute or change saved playback enable. The updated offline guide and living workflow/Perform guidance explain this distinction.

## Revision and environment

- Branch `codex/1-perform-warning`, based on main `ae47594`; native build configured at planning commit **`9347563936fe6fc8dd7317af7a8352cb37a678ce` with the implementation diff present (dirty)**. Subsequent task/report edits do not change the executable.
- Windows x64 `10.0.26300`, VS 2026/MSVC, Windows SDK `10.0.26100`, pinned JUCE **9.0.4**. Fresh checkout build tree with existing JUCE/WavPack source caches; no cache copying/clearing or global toolchain changes. This is not a fully warm build or a performance comparison.
- Reviewed executable: `out/build/windows-msvc/Release/preverb_swell_ui_test.exe`, SHA256 **`B5CD8E13C47DDDE1753CBC811A2DB36EF1257842E679516844E4544C6E408EA9`**.
- Initial UI validation made no plugin installation, FL/controller configuration, user project or asset modification. The later owner-requested installation is recorded below. The native review window was opened at the owner's explicit request and left open for their use.

## Validation

| Check | Result | Scope |
| --- | --- | --- |
| Repository Release build | **PASS** `scripts/build.ps1 -Configuration Release -PluginOnly` | Full VST3 built; no install and no full production CTest claim. |
| Focused native Release UI CTest | **1/1 PASS**, `preverb_swell_ui_test`, **26.17 s** | Notice priority/restoration, existing component/header/editor coverage and native screenshots. |
| Warning fit | **PASS**, Inter width **579.083** | Actual text areas: **628** at 1100x850, **890** at 1400x900 and 1400x1000, **1090** at 1600x1000. Full primary copy fits without elision, smaller fonts or geometry exceptions. |
| Accessibility/refresh | **PASS** | Warning title/detail/help retained; identical refresh returns no new display/announcement update; temporary information replaces warning and warning restores at unchanged bounds. Existing focus/semantic-ink cases retained. |
| Historical failure precedence | **PASS** | Current errors remain errors; historical failed capture cannot replace the current Perform caution. Learn clean-idle and Export information cases remain distinct. |
| UI standards | **PASS**, 51 files / 61 bounded occurrences | No new token/control-family/debt inventory exception. |
| Offline user guide | **PASS** currency after explicit semantic review | Exact caution, help detail and playback/mute implications updated; no screenshot fabrication or packaging. |
| OpenSpec / formatting | **PASS** strict change validation and `git diff --check` | No canonical spec sync/archive in Apply. |
| Owner native review | **APPROVED** 5 November 2026: “Copy/layout approved” | Scoped warning, temporary IR preparation and restored warning in the real shared header. Not FL acceptance. |

Focused UI compilation and CTest used the same build-directory lock after the repository wrapper completed; no raw build overlapped a wrapper. Source caches were explicitly selected at initial configure, then normal wrapper configuration reused them. Focused command: `ctest --test-dir out/build/windows-msvc -C Release -R '^preverb_swell_ui_test$' --output-on-failure --no-tests=error`.

Recovery: an initial filter matched no tests and was corrected to the actual registered name; this was not counted as a test pass. The first real UI run exposed a standalone accessibility fixture without a native peer; the fixture was corrected, rebuilt and the complete focused test passed. No production workaround was introduced.

Evidence under ignored output: wrapper logs in `out/build-logs/Release/20261105-163152-fbf14bcd/`; focused build/test logs `out/perform-warning-ui-build-final.log` and `out/perform-warning-ui-test.log`; detailed CTest output in `out/build/windows-msvc/Testing/Temporary/LastTest.log`. Native PNGs are in `out/perform-warning-ui/`, including `ready-1100.png`, `ready-1400.png`, `ready-1600.png`, software-scale ready views and `perform-warning-review-1/2/3.png`. The actual minimum-size editor and shared-header warning preview were visually inspected. Preview entry point: `preverb_swell_ui_test.exe --review-perform-warning`; its dropdown changes only synthetic presentation snapshots, not the processor or saved content.

**NOT RUN:** installed FL warning smoke test, physical Windows DPI/multiple-monitor review, new host transport/mute experiments, bounce/audio regression or full repository CTest. Existing native software-scale renders are not physical host/DPI checks. This read-only presentation change does not warrant repeating the completed issue #1 host experiments; untested installed-host coverage is explicit. No merge, issue closure or archive is performed.

## Subsequent owner-requested test installation — 5 November 2026

Installed clean revision **`3f62614`** using `scripts/install-test-vst3.ps1` with FL closed. Release plugin-only build **PASS**; CTest was **not rerun** for installation (the focused native UI result above still applies). Normal system-folder ACL denial was resolved with Windows UAC elevation; complete-bundle inventory/hash verification passed. No controller/project/asset changes or merge occurred.

- Installed bundle: `C:/Program Files/Common Files/VST3/ReverbArise.vst3`.
- Installed binary SHA256 independently rechecked: **`8F7F20BBC964F339EE86C32C0ADACFB3D4921D6C7D41967E148B94DE70D7DEF9`**.
- Previous complete bundle retained: `out/vst3-install-backups/20261105-165428-a0a0a4caebdb496eabc0bd1833d34479/ReverbArise.vst3`.
- Installer log: `out/perform-warning-install-elevated.log`; wrapper logs: `out/build-logs/Release/20261105-165421-0f2518a5/`.

Reopen FL, rescan with Verify plugins and Rescan previously verified plugins if needed, and confirm the resolved system path. Installation does not establish FL warning acceptance; that host check remains **NOT RUN** here. This later documentation record does not change the installed revision.

# Issue #8 — FL Studio project-folder discovery spike, 2026-11-06

**Result:** no reliable project-folder signal was established through optional VST3 state-stream metadata in FL Studio **26.1.7 build 5654**, Windows x64, JUCE **9.0.4**. All seven observed state calls supplied a stream, but none supported `IStreamAttributes`. This is a scoped negative result, not proof that every DAW or proprietary extension lacks project-folder support. Keep user-data fallback and an explicit folder picker in the plan.

The owner authorized the existing plan/PR update and starting task 1.4. Planning was committed first as `d049cfc`; this diagnostic/evidence commit follows it. Phase 1 remains delivered. Backup/Restore, association, re-learn recovery, cache eviction and cleanup are still planned, separately gated work.

## Method and actual observations

An opt-in `PREVERB_PROJECT_FOLDER_PROBE` build copies the exact inspected JUCE VST3 wrapper into the build tree, inserts observation at component `getState`/`setState`, and substitutes only this plugin target's wrapper. The shared dependency cache and production `Source/` files are untouched. Default builds use the original wrapper. Tagged/candidate builds reject enabled probing.

The probe queries `IStreamAttributes`, and only if available would query the stream filename, `StateType` and `FilePathString`. It never reads/writes/seeks the state stream or interprets a filename as a folder association. Events are bounded (512 accepted, queue 128, 1 MiB per log) and flushed by a separate worker to a local temporary log; no observation or disk work occurs in the audio callback. Logs contain metadata, never serialized state or audio. Native control-boundary observation does not implement host path discovery for the product.

| Scenario | UTC | Observed calls | Path metadata |
| --- | --- | --- | --- |
| Fresh unsaved instance | 10:55:50 | getState | Interface absent |
| First save to disposable original folder | 10:56:51 | getState | Interface absent |
| Reopen saved FLP | 10:57:19 | getState, setState | Interface absent |
| Save As into different folder | 10:57:40 | getState | Interface absent |
| Open FLP moved into third folder | 10:58:16 | getState, setState | Interface absent |

The test used a new empty project and newly created `out/project-folder-spike/` directories. Save As used a different filename and parent directory. Only the newly created original FLP was moved, after checking both resolved paths stayed inside that fixture root; its SHA-256 was unchanged. No user song was opened, no transport played, no song audio captured, and no existing managed assets relocated or removed. A fresh plugin instance performs its ordinary empty-session/default-IR setup; this is not a claim of zero filesystem writes by FL Studio or the plugin.

The exact seven observations, scenario mapping, hashes and limits are in [machine-readable evidence](2026-11-06-issue-8-project-folder-spike.json). The probe's historical field name `hostExecutable` records JUCE `currentExecutableFile`, which returned the loaded system VST3 module here; it is **not** project-location evidence. The loaded module path was also inspected in the FL64 process.

## Interpretation

JUCE's normal processor state callback receives bytes, not the raw host stream. Static inspection of its pinned wrapper found no consumption of the optional stream metadata. The standard host application interface supplies application name/object creation, not a project-folder getter; no general JUCE project-path API was established. Runtime instrumentation bypassed the JUCE abstraction only for observation and found the optional stream interface unavailable on this tested FL Studio route. `FilePathString` can describe a preset origin when provided, so even a future positive result must validate project context and freshness; this host returned no positive signal to discriminate.

Primary API references: [Steinberg stream metadata](https://steinbergmedia.github.io/vst3_dev_portal/pages/Technical%2BDocumentation/Change%2BHistory/3.6.0/IStreamAttributes.html), [JUCE VST3 extension hooks](https://docs.juce.com/master/structjuce_1_1VST3ClientExtensions.html). These describe possibilities, not FL Studio support guarantees. No CWD, window-title scraping or filename guessing is proposed. Other hosts/versions, proprietary extensions and positive preset-stream classification remain untested.

## Build, installation and rollback

- Used the existing warm MSVC Release tree and pinned dependency sources under `Enter-BuildLock`; no cache clearing or speedup claim. `cmake --preset windows-msvc -DFETCHCONTENT_UPDATES_DISCONNECTED=ON -DPREVERB_PROJECT_FOLDER_PROBE=ON`, then Release `preverb_plugin_VST3` target: passed.
- Installed with `scripts/install-test-vst3.ps1` while FL Studio was closed. The first attempt hit a system-folder ACL denial; the documented hidden UAC launcher then passed. Wrapper build log: `out/build-logs/Release/20261106-105448-aef648dd`. It did not request CTest. The existing registered ReverbArise entry loaded the installed system bundle; no rescan or host-path claim is inferred from the plugin database entry alone.
- Diagnostic DLL SHA-256: `b5c774d1b9fbfa2886bea789827d7b34f2fcdb38a147d3b237806106abfa9165`. Complete prior installation retained at `out/vst3-install-backups/20261106-105501-5a46187bb6474ec8a40917fb16254dbe/ReverbArise.vst3`.
- Closed only the disposable FL Studio session normally, then restored that exact prior bundle through `Install-TestBundle`, preserving the diagnostic bundle separately. Restored DLL SHA-256 matched the pre-spike value: `94bee38a215d6564ebbdcfada6cb76acdfdc2082e895e848c09f32001d109774`. Restoration used the same verified staging/rollback helper, not manual file copying.
- Reset the local CMake option to OFF and verified the generated VST3 project compiles the original JUCE wrapper, with no probe source. Normal Release VST3 build passed; focused Release CTest passed 4/4 (swell UI, inventory backend, Storage component and actual Settings integration), 11.83 seconds; log `out/issue8-probe-final-tests.log`. UI standards, explicit guide review/currency, strict OpenSpec validation and whitespace checks passed. The candidate/tag diagnostic guard rejection was separately exercised with CMake script mode. No full suite, Debug, second-PC, DPI, audio processing acceptance, release packaging or publishing was performed.

## Reproduction

Enable the option through CMake under the build lock, then use the repository test installer. Logs default to `%TEMP%/ReverbAriseFolderProbe/host-<uuid>.jsonl`; an explicitly set `PREVERB_FOLDER_PROBE_LOG_DIR` can select a local output directory before launching the host. Use disposable FLPs only, repeat the five scenarios, and retain exact host/build versions. Always close the host, restore the retained bundle, and reset the option OFF afterward. The wrapper SHA guard deliberately fails on a changed JUCE file until its injection boundaries are reviewed.

See the [revised plan](../../openspec/changes/portable-project-assets/design.md) and [compatibility/backup matrix](../development/asset-storage-contracts.md). No later storage gate is waived by this result.

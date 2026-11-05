# Issue #93 — Swells-list viewport reveal

## Scope and revision

Branch `codex/93-reveal-selected-swell`, based on `84894ee` (origin/main at start). Native validation uses that revision with the issue implementation present as a dirty diff. OpenSpec change: `reveal-selected-swell`.

Explicit Perform list selection now calls the exact contextual Fit operation used by the toolbar, changing pan and zoom to frame the accepted primary's complete source/wet/requested-envelope and overlap context. Re-clicking the same selected row after manual pan/zoom applies Fit again. The owner explicitly requested this during native review on 5 November 2026, superseding #93's original zoom-preservation criterion. The initial translation-only implementation was removed; no second framing algorithm remains.

A separate epoch-bearing list reveal intent handles same-row reactivation and keyboard selection. Pointer selection suppresses duplicate reveal; checkbox/menu actions do not request Fit. The binding checks mode, session epoch and accepted primary. Existing Fit publication synchronizes toolbar, scrollbar, lanes, ruler, handles, Overview and optional saved navigation. Incidental refresh and late render publication never initiate Fit.

Reused controls: SwellList, specialized SwellTimeline/SwellOverview, TimelineZoomControls and musical ruler; production standards revision 5. No new styling token, variant or exception. No audio callback, render algorithm, captured PCM, asset-retention, parameter or persistent-schema change. Existing engine selection semantics remain unchanged. Production source changes are limited to `Source/Plugin/PluginEditor.cpp` and `Source/UI/Controls/SwellList.h`; no layout geometry or Fit implementation is changed.

## Build environment and history

Windows x64, VS 2026/MSVC, Windows SDK 10.0.26100, pinned JUCE 9.0.4 and WavPack. This worktree started without a build tree. Existing dependency source directories were copied from the local checkout; no CMake cache or compiled output was copied. Initial native compilation was cold with reused source downloads; revised validation uses the resulting warm cache. No comparable speedup measurement is claimed.

- Initial sandbox wrapper configure stopped at Windows SDK registry access; build/CTest did not run. Log: `out/build-logs/Debug/20261105-214336-bf22a603/`.
- Native serial build was deliberately interrupted after verifying process ownership. Its process tree exited before the same wrapper restarted with `CMAKE_BUILD_PARALLEL_LEVEL=4`. Other worktree builds and all caches were preserved. Log: `out/build-logs/Debug/20261105-214350-2a9784d4/`.
- Initial translation-only implementation: native build PASS; Debug CTest **70/71 PASS**, 611.15 s. `preverb_processor_test` timed out at `tests/M3ProcessorCases.h:90` waiting for restored audition readiness; all UI/Fit/navigation/list tests passed. This run does not validate the later owner-requested automatic Fit revision. Log: `out/build-logs/Debug/20261105-220216-e80553df/`.

## Revised automatic Fit validation

`scripts/build.ps1 -Configuration Debug -Test` with `CMAKE_BUILD_PARALLEL_LEVEL=4`: revised native configure/build PASS; full Debug CTest **70/71 PASS**, 947.19 s. The previously failing processor regression passed (47.67 s). `preverb_ir_streaming_test` failed its live-readiness assertion at `tests/ir_streaming_test.cpp:45` after reporting missing preparation at sample 767744 (1019.32 ms). All Perform/UI/navigation tests passed. Log: `out/build-logs/Debug/20261105-221921-3a99ceaa/`.

After the full wrapper exited, ran matching VS CTest with `--test-dir out/build/windows-msvc -C Debug -R '^preverb_ir_streaming_test$' --output-on-failure --no-tests=error`: **1/1 PASS**, 18.58 s. Log: `out/build-logs/Debug/20261105-221921-3a99ceaa/ir-streaming-rerun.log`. This does not turn the original full run into an all-pass result. Final disposition: a further CTest `--repeat until-fail:3` run passed all three consecutive executions (23.61 s, 25.89 s, 22.12 s; 71.64 s total). Together with the first isolated rerun, four isolated executions passed. Log: `out/build-logs/Debug/20261105-221921-3a99ceaa/ir-streaming-repeat-until-fail.log`. The full-run readiness miss remains a disclosed intermittent validation limitation; its cause is not established. This UI-only change does not alter that test or its IR/DSP implementation, and no threshold was relaxed.

- Revised focused native tests PASS: `preverb_initial_perform_view_test`, `preverb_swell_list_test`, and `preverb_perform_composition_preview --test-fit`, `--test-editor`, `--test-navigation` (separate commands). The editor-binding check confirms automatic framing exactly equals manual Fit and re-clicking after manual zoom restores that framing.
- `scripts/check-ui-standards.ps1`: PASS (51 files; 61 bounded definition/debt occurrences).
- `openspec validate reveal-selected-swell --strict`: PASS.
- `scripts/check-user-guide.ps1`: PASS after explicit semantic review through maintain-user-guide and revised Perform instructions describing automatic pan/zoom like Fit.
- `git diff --check`: PASS.

Revised binding checks compare automatic framing endpoints with manual Fit, cover distant and repeated selections after pan/zoom, Ctrl/Shift selection, stale epochs, checkbox/menu independence, shared geometry, saved navigation/reopen and unchanged musical/source/render state. Native list tests cover real keyboard input and held pointer ownership across reorder/rename. Existing Fit tests cover selected, pending, overlapping, sparse and empty context; initial-Perform tests retain the independent Generate handoff.

## Review and limitations

The first native preview was launched directly. Its translation-only behavior passed the agent's mechanical smoke, but the owner requested automatic complete-region Fit instead. That first result does not approve the revised implementation. After the owner's clarification, code, tests, guide and OpenSpec artifacts were revised; the updated preview was rebuilt and directly reopened after tests on 5 November 2026. The old preview was closed before relinking.

**Revised native approval — 5 November 2026:** the owner reviewed the updated **Issue #93 - Swell selection navigation** window and confirmed, “The revised navigation looks good,” then explicitly requested remaining validation and the PR. This completes the scoped native navigation review and brief native Perform smoke; detailed pan/zoom, re-click, modifier, ownership and persistence cases are established by the automated checks above. The approved preview is preserved; no additional appearance or control-family change followed approval.

The review executable is `preverb_perform_composition_preview --review-selection`, using production editor bindings and actual fixture capture/renders without an audio device. No acceptance is inferred from compilation or automated tests. FL Studio and actual Windows DPI/monitor transitions have not run for this change. No system VST3 installation, PR, merge, issue closure or OpenSpec archive has occurred.

## Delivery status

Validation and owner native approval are complete with the full-run readiness limitation disclosed above. PR publication is the remaining tracked delivery step on codex/93-reveal-selected-swell. #58 layout integration remains separate; this patch introduces no geometry change. Owner review/merge is required; no self-merge or archive is authorized.

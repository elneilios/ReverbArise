# Issue #107 P7 connected tabs

## Native preview approval

The owner approved the native JUCE connected tabs preview for rollout on
10 November 2026. It used the shared `SegmentedSelector` for the workflow
header, Learn sections, Swell Inspector and sample Settings pages. The
preview included unavailable workflow choices, keyboard inspection and
activation, distinct hover and held feedback, and About last after Storage.
Its focused component and seven-tab layout checks passed at 100% and 125%
preview scale.

## Production coverage

The three existing editor strips and Settings now use the same control.
Settings selects by page identity. Responsive Settings also exposes the
existing Appearance page before Storage, so both seven- and eight-tab
layouts finish with Storage and About. The Recovery warning continues to
open Storage.

A focused Windows MSVC Release fixture passed the final tab component
checks and measured all eight labels at the 640-unit production Settings
width. It covered release outside, right-click cancellation, selected and
hovered seams, keyboard and accessibility inspection, accepted selection
and wheel forwarding. The component fixture linked against cached JUCE
build output.

## Integrated Windows verification

- The Debug wrapper built the VST3 and ran all 99 CTest entries. The first
  run passed 95, including the responsive editor and scale checks. The knob
  reset and exact-value readability tests passed on isolated Debug retries
  after Windows foreground focus was available. The Storage test passed in
  isolation and intermittently timed out in Debug during the background
  inventory refresh after a working-folder change.
- The broad Swell UI test initially crashed in an existing command-button
  fixture that read a capture after destroying its own callback. The test
  now increments its counter before destruction. The repaired Debug test
  reached the dense list scenario, where the Storage timer kept serializing
  the 500-swell fixture while the JUCE message queue drained. That retry was
  stopped. The rebuilt Release Swell UI and Storage tests both passed.
- UI standards, user-guide currency, strict OpenSpec validation and diff
  whitespace checks passed. Physical Windows DPI and FL Studio were outside
  these runs.

The full Debug log is `out/build-logs/Debug/20261110-014400-45ce20fc/test.log`.
The focused results are in `out/build-logs/p7-connected-tabs-ctest-retry.log`
and `out/build-logs/p7-connected-tabs-release-focused-test.log`.

## PR #139 accessibility follow-up

The shared strip keeps one keyboard tab stop and gives the inspected tab's
handler native accessibility focus. Windows UI Automation can query that tab
through the same `getChildFocus()` path used by the provider. The regression
checks entry, Left and Right, Home and End, explicit accessibility focus,
unavailable inspection and focus leaving the strip. It checks the native
handler separately from the accepted page and the painted brackets.

The focused Windows MSVC Release rebuild passed both CTest entries
(`preverb_swell_ui_test` and `preverb_storage_settings_test`). Results are in
`out/build-logs/p7-uia-release-test.log`.

## 11 November 2026 pointer regression from FL Studio smoke testing

The installed `5fc1240` build cancelled normal tab clicks. JUCE dispatches
`mouseUp` with the modifiers from before release, so the tab interpreted a
normal left release as a button still being held. The old tests called
`mouseUp` directly with empty modifiers. Settings integration tests selected
pages programmatically or with keys and did not cover the missing pointer path.

The failure was reproduced by sending a click through a JUCE component peer.
Against `5fc1240`, hit-testing and press succeeded but release emitted no
selection request. The same peer check passed with the fix in the Release
suite. The permanent test reproduces the native event/source modifier mismatch
without relying on desktop hit-testing through potentially overlapping windows.
The fix retains same-source, in-bounds and availability checks. The focused
test is available via `preverb_swell_ui_test --test-connected-tabs`.

Windows MSVC Release built successfully. The full sweep passed 98/100 tests,
including the native-peer tab check. The reset and readability fixtures stopped
while waiting for native focus. After replacing the peer fixture with the
deterministic release-event regression, the focused tab check and all four
targeted CTest entries passed, including both focus retries and the Swell UI and
Storage Settings tests. Logs are under
`out/build-logs/Release/20261111-084404-6c0be8fc/` and
`out/build-logs/tab-pointer-release-focus-retry.log`.

The owner confirmed that clicking tabs worked in FL Studio on the installed
Release build from `4844c92` on 11 November 2026.

The Debug build and standalone tab check passed. The full Debug run failed at
the Swells multi-delete assertion in `SwellListBindingCases.h:277` and was
stopped during the later composition fixture. The full Debug gate remains open.
See `out/build-logs/Debug/20261111-085722-42028b05/`.

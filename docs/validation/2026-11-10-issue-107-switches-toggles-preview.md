# Issue #107 P6 JUCE preview

The standalone JUCE fixture uses the shared track and inline checkbox forms.
It shows the Learn capture options, representative Settings switches, a retained
unavailable on value, a long label and the Enabled, Solo and Freeze row at the
current inspector width. It uses sample values without opening an audio device.

On Windows MSVC Release, the focused `preverb_boolean_toggle_test` build and
CTest entry passed. The test compares on/off, hover, held, unavailable and focus
ink for both forms. It exercises the shared label and indicator click path,
cancellation, Space and Return, accessibility activation and focus provenance,
disabled invocation hooks, and silent state refresh. Software layout checks
passed at 900 × 600 and 1100 × 700 with 100% and 125% UI scale. Logs are under
`out/build-logs/Release/p6-preview`.

The owner approved this JUCE preview for production adoption on 10 November
2026. The style guide records the approved forms in revision 20. Physical
DPI, FL Studio and the full plugin regression suite were outside the
focused preview run.

## Production rollout

Learn retains its label-left, track-right layout with one clickable row per
switch. Settings has eight labeled switches. The Swell Inspector keeps its
three inline checkboxes and fixed Reset placement. Hidden compatibility
controls remain available to their existing callers.

The Windows MSVC Release wrapper with `-Test` built the VST3 and passed
99 of 99 CTest entries on the first run. The build log is
`out/build-logs/Release/20261110-001213-7d1e33b0`. Integrated checks include
Learn pass changes and the allowed turn-off path, selected-swell flag
reconciliation, stale Settings pass rejection, and retained dry isolation
during wet-only audition. The responsive and scale checks passed, including
the compact inspector row and fixed Reset. UI standards, guide currency and
strict OpenSpec change validation passed. Physical DPI and FL Studio were
outside this run.

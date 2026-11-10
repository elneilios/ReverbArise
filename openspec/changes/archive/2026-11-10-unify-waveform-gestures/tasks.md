# Tasks

## 1. Production baseline and approval

- [x] 1.1 Before production edits, verify [PR #143](https://github.com/elneilios/ReverbArise/pull/143) is merged and record its target, merge SHA and the current implementation base. Compare its Learn and Perform changes with the base and record how the gesture work will preserve them when the branches meet.
- [x] 1.2 Record the owner's 2026-11-10 native preview approval and standalone gesture-check result as sample-data evidence for the approved mapping, including left-drag-only Overview panning. Use that approval for the matching implementation and seek further review only for material design deviations.

## 2. Shared policy and canvas bindings

- [x] 2.1 Reuse the existing viewport math and shared focus renderer for all three main waveforms. Verify in native component tests that wheel anchors, middle drag pans, right input is idle, release ends the gesture and keyboard/accessibility focus is visible independently of hover and held states. Update the C8 contract, inventory and production style guide with the final policy.
- [x] 2.2 Adopt the gestures in Learn's inspection canvas. Extend Learn timeline tests for absolute pin/coverage under zoom, bounded pan, routine capture refresh, keyboard inspection and reset, and verify capture and Perform state stay independent. Update Learn navigation help.
- [x] 2.3 Adopt the gestures in Export's `TimelineNavigation` and keep the ready-preview binding authoritative. Extend Export control tests for ready, playing, pending and superseded previews, cursor/ruler alignment, unaffected WAV invocations and existing Fit actions. Update timeline controls guidance and the Export help.
- [x] 2.4 Adopt the gestures in `SwellTimeline` and make the compact `SwellOverview` left-drag-only. Extend Perform component and binding tests for open waveform positioning, handle precedence, Snap/Shift/Ctrl, frozen and stale rejection, Overview drag direction and fixed span, and unchanged DAW/audition state. Verify ignored buttons and wheel on the Overview. Update Perform navigation guidance and contextual help.
- [x] 2.5 Update the offline user guide and currency record for the changed gestures. Verify the guide and built-in help describe the same input map and preserve guidance for knobs and Export WAV Save/Drag.

## 3. Integrated verification

- [x] 3.1 Run `scripts/check-ui-standards.ps1` and the supported Windows build/test wrapper. Verify the integrated editor at minimum and larger sizes and 100%/125% scale with empty and populated waveforms. Record native component and integration evidence separately from FL Studio and physical-DPI coverage.
- [x] 3.2 Verify navigation and local markers do not affect accepted source, swells, renders, export content or host transport across mode/selection changes and editor reopen. Run strict OpenSpec validation and prepare the scoped review with the dependency/base record and test results.

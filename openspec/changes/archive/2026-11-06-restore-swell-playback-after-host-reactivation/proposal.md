# Proposal

## Why

GitHub issue: #103 reports that host disable/re-enable during running transport leaves prepared swells silent until a loop wrap. Re-enabling should restore eligible wet at the current Song position without losing source, prepared audio or edits.

## What Changes

- Define recovery after host processing suspension/reactivation, including joining inside a swell and playing later scheduled swells before a loop wrap.
- Reproduce and isolate the failing lifecycle/admission/preparation sequence before selecting the smallest repair; distinguish host disable from saved Live enable and internal bypass.
- Preserve existing timing validation, bounded preparation, absolute-offset playback and real-time safety; do not treat arbitrary callback gaps or seeks as permission to rebase the clock.
- Add processor-level and focused audio regression coverage for repeated cycles, warm/cold coverage, seek/loop behavior and durable-state retention.
- Include the owner-authorized native validation follow-up: give the broad MSVC UI test adequate stack headroom and establish native foreground focus in the reset-popup fixture, preserving production UI and existing assertions.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `transport-swell-playback`: specify host reactivation recovery at current absolute time while retaining timing and asset guards.

## Impact

Expected areas are `Source/Plugin/PluginProcessor.cpp/.h`, `Source/DSP/SwellEngine.cpp/.h`, live clock/playback/preparation code as demonstrated by reproduction, and existing processor/transport test harnesses. Living transport guidance, dated validation evidence and offline guide currency need review after implementation. No new UI, dependency, persistent format, asset migration or host transport control is proposed. The reported host mechanism and exact failing build remain unverified; synthetic results must be labelled separately from FL Studio evidence.

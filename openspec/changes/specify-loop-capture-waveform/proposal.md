# Proposal

## Why

PR #186 restores live waveform feedback during Loop capture. The Loop phrase capture spec defines take acceptance and replacement but leaves the recording display implicit, including what happens when detailed preview coverage fills up.

## What Changes

- Specify that Learn draws a growing provisional waveform from the start of the Loop phrase as audio is recorded.
- Define the bounded waveform fallback when detailed coverage reaches its limit and the display after a take settles or is abandoned.
- Clarify when accepted event markers are visible during a replacement attempt.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `loop-phrase-capture` defines provisional waveform progress, the coverage-limit fallback, and its relationship to the accepted source and event markers.

## Impact

The delta records behavior already implemented by PR #186. The relevant paths are capture-worker preview telemetry and the Learn waveform canvas. Capture timing, persisted source identity, and downstream source selection follow the existing Loop requirements. Song's provisional waveform is covered by `song-timeline-audio`.

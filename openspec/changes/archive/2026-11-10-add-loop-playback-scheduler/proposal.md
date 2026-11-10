# Proposal

## Why

GitHub issue [#125](https://github.com/elneilios/ReverbArise/issues/125) brings prepared wet playback to an accepted Loop. The integrated #133 capture can establish the phrase's local grid and recorded extent. Live Cue needs an independently verified reference that repeats in phase with that phrase.

## What Changes

- Add a headless Loop playback route that tracks the live reference in dry Standby and reports why Cue is unavailable until both phase and prepared wet are ready.
- Start on the earliest future matching boundary with enough time for the full wet lead-in. Repeat until Perform Stop, and fade wet immediately on Stop or loss of sync.
- Sum enabled, eligible swells across overlapping cycles using the shared render path. Preserve source-relative placement, negative lead-in and long tails, and reject work outside supported resource bounds.
- Verify synthetic scheduling and mixing against sample-level references, then exercise the production adapter with measured Rack CLKD timing at 137 and 280 BPM.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `transport-swell-playback` — add Loop phase admission, Cue and Stop, periodic wet composition and revision-safe playback beside the existing Song path.

## Impact

The processor, shared render preparation and transport playback gain a Loop route selected by the accepted source kind. The capture contract and stored Loop metadata remain the input to that route. This change supplies the scheduler for #126 and reusable wet composition for #128. Editor controls, FL Studio integration, WAV export and durable state expansion belong to #126–#129.
